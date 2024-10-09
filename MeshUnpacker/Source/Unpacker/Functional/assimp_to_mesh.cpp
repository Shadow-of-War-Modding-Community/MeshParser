#include "../Internal/includes-types-defs.h"


void error_checks(aiMesh* mesh) {
	if (!mesh->HasPositions()) {
		error("Mesh does not contain any positions!");
	}
	if (!mesh->HasTextureCoords(0)) {
		error("Mesh does not contains any UVs!");
	}
	if (!mesh->HasNormals()) {
		error("Mesh does not contain any normals!");
	}
	if (!mesh->HasTangentsAndBitangents()) {
		error("Mesh does not contain any tangents or bitangents!");
	}
	if (!mesh->HasVertexColors(0)) { // For now throw error, later just make them all RGBA8(255,255,255,255)
		error("Mesh does not contain any vertex colors!");
	}
	if (!mesh->HasFaces()) {
		error("Mesh does not contain any faces!");
	}

}

void create_mesh_header(std::shared_ptr<MESH_UNPACKER::Mesh> newMesh) {

}

void create_mesh_desc_section(std::shared_ptr<MESH_UNPACKER::Mesh> newMesh) {
	using namespace MESH_UNPACKER::INTERNAL::MESH;
}

void create_mesh_info_section(std::shared_ptr<MESH_UNPACKER::Mesh> newMesh) {

}

void create_mesh_data_section(std::shared_ptr<MESH_UNPACKER::Mesh> newMesh, std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {
	using namespace MESH_UNPACKER::INTERNAL::MESH;
	using AttributeLayout = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout;
	using Attribute = AttributeLayout::Attribute;
	using Type = AttributeLayout::Type;
	using Buffer = AttributeLayout::Buffer;

	int uv_counter = 0;

	auto parse_attribute = [&](std::vector<byte>* buffer, const AttributeLayout& attributeLayout, const Buffer& targetBuffer, int lodIndex, int meshIndex, int vertexAttribute)
		{
			if (attributeLayout.bufferIndex != targetBuffer)
				return;
			auto& vertAt = mesh->lodBuffers[lodIndex].meshVertexAttributeContainers[meshIndex][vertexAttribute];
			switch (attributeLayout.attribute) {
			case Attribute::POSITION:
			{
				if (attributeLayout.type == Type::VECTOR4F16) {					
					TYPES::Position16 position;
					position.x = vertAt.position.x;
					position.y = vertAt.position.y;
					position.z = vertAt.position.z;
					position.w = vertAt.position.w;	
					buffer->resize(buffer->size() + sizeof(TYPES::Position16));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Position16);
					memcpy(bufferEnd, &position, sizeof(TYPES::Position16));
				}
				else if (attributeLayout.type == Type::VECTOR4F32) {
					buffer->resize(buffer->size() + sizeof(TYPES::Position32));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Position32);
					memcpy(bufferEnd, &vertAt.position, sizeof(TYPES::Position32));
				}
				else {
					error("Unknown Type for POSITION");
				}
				break;
			}
			case Attribute::WEIGHT:
			{
				if (attributeLayout.type == Type::VECTOR4U8) {
					buffer->resize(buffer->size() + sizeof(TYPES::Weights));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Weights);
					memcpy(bufferEnd, &vertAt.weights, sizeof(TYPES::Weights));
				}
				else {
					error("Unknown Type for WEIGHT");
				}
				break;
			}
			case Attribute::VERTEXGROUP:
			{
				if (attributeLayout.type == Type::VECTOR4U8) {
					buffer->resize(buffer->size() + sizeof(TYPES::VertexGroups));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::VertexGroups);
					memcpy(bufferEnd, &vertAt.vertexGroups, sizeof(TYPES::VertexGroups));
				}
				else {
					error("Unknown Type for VERTEXGROUP");
				}
				break;
			}
			case Attribute::TEXCOORD:
			{
				if (attributeLayout.type == Type::VECTOR2F32) {
					TYPES::UV uv_correct_format = vertAt.uvs[uv_counter++].first;
					uv_correct_format.v = 1.f - uv_correct_format.v;
					buffer->resize(buffer->size() + sizeof(TYPES::UV));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::UV);
					memcpy(bufferEnd, &uv_correct_format, sizeof(TYPES::UV));
				}
				else if (attributeLayout.type == Type::VECTOR2S16) {
					short u, v;
					u = (short)(vertAt.uvs[uv_counter].first.u * 32767.f);
					v = (short)(1.f - vertAt.uvs[uv_counter].first.v * 32767.f);
					uv_counter++;
					buffer->resize(buffer->size() + sizeof(TYPES::UV)); // same size for S16 format...
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::UV);
					memcpy(bufferEnd, &u, sizeof(short));
					memcpy(bufferEnd + 2, &v, sizeof(short));
				}
				else {
					error("Unknown Type for TEXCOORD");
				}
				break;
			}
			case Attribute::NORMAL:
			{
				if (attributeLayout.type == Type::VECTOR3F32) {
					buffer->resize(buffer->size() + sizeof(TYPES::Normal));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Normal);
					memcpy(bufferEnd, &vertAt.normal, sizeof(TYPES::Normal));
				}
				else {
					error("Unknown Type for NORMAL");
				}
				break;
			}
			case Attribute::TANGENT:
			{
				if (attributeLayout.type == Type::VECTOR3F32) {
					buffer->resize(buffer->size() + sizeof(TYPES::Tangent));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Tangent);
					memcpy(bufferEnd, &vertAt.normal, sizeof(TYPES::Tangent));
				}
				else {
					error("Unknown Type for TANGENT");
				}
				break;
			}
			case Attribute::BITANGENT:
			{
				if (attributeLayout.type == Type::VECTOR3F32) {
					buffer->resize(buffer->size() + sizeof(TYPES::Bitangent));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Bitangent);
					memcpy(bufferEnd, &vertAt.normal, sizeof(TYPES::Bitangent));
				}
				else {
					error("Unknown Type for BITANGENT");
				}
				break;
			}
			case Attribute::COLOR:
			{
				if (attributeLayout.type == Type::VECTOR4U8) {
					buffer->resize(buffer->size() + sizeof(TYPES::Color));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(TYPES::Color);
					memcpy(bufferEnd, &vertAt.color, sizeof(TYPES::Color));
				}
				else {
					error("Unknown Type for COLOR");
				}
				break;
			}
			}
	};
	// Get the buffer layouts again...
	std::vector<BufferLayout> bufferLayouts{};
	for (int i = 0; i < mesh->meshInfoSection.bufferLayoutCount; ++i) {
		bufferLayouts.push_back(BufferLayout{});
		bufferLayouts[i].populate(mesh->meshInfoSection, i);
	}
	std::vector<byte> meshDataSection;
	int mesh_counted = 0;
	for (int lodCounter = 0; lodCounter < newMesh->lodBuffers.size(); ++lodCounter) {
		for (int meshCounter = 0; meshCounter < newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			auto current_layer_index = mesh->meshInfoSection.meshInfos[mesh_counted].layerIndex; // Only use the layers of the old mesh...
			for (int vertexCounter = 0; vertexCounter < newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
				for (auto& attribute : bufferLayouts[current_layer_index].order) {
					parse_attribute(&meshDataSection, attribute, Buffer::Buffer_0, lodCounter, meshCounter, vertexCounter);
				}
			}
			for (int vertexCounter = 0; vertexCounter < newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
				for (auto& attribute : bufferLayouts[current_layer_index].order) {
					parse_attribute(&meshDataSection, attribute, Buffer::Buffer_1, lodCounter, meshCounter, vertexCounter);
				}
			}

			mesh_counted++;
		}
	}

}

std::shared_ptr<MESH_UNPACKER::Mesh> assimp_to_mesh_ref(std::shared_ptr<aiScene> scene, std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {
	// This is just a very raw idea and implementation. sub functions, additional structs and overall more abstraction is very likely
	// For now, keep the mesh structure -> LOD count, mesh count in each LOD. Will be changed later when everything is discovered

	using namespace MESH_UNPACKER::INTERNAL::MESH;
	using namespace MESH_UNPACKER;
	
	if (!scene->HasMeshes()) {
		error("Scene doesn't contain any meshes!");
	}
	if (scene->mNumMeshes != mesh->meshInfoSection.meshCount) {
		error("Mesh count is not equal in scene and reference mesh file");
	}

	auto newMesh = std::make_shared<Mesh>();

	// Here just recreating the structure of the reference mesh, filling the new mesh with the actual data of the scene

	int assimp_mesh_counter = 0;
	for (int lodCounter = 0; lodCounter < mesh->meshInfoSection.lodCount; ++lodCounter) {
		newMesh->lodBuffers.push_back(LODBuffer{});
		error_checks(scene->mMeshes[assimp_mesh_counter]);
		for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers.push_back(std::vector<TYPES::VertexAttribute>{});
			for (int vertexAttributeCounter = 0; vertexAttributeCounter < scene->mMeshes[assimp_mesh_counter]->mNumVertices; ++vertexAttributeCounter) {
				TYPES::VertexAttribute vertexAttribute;
				vertexAttribute.position.x = scene->mMeshes[assimp_mesh_counter]->mVertices[vertexAttributeCounter].x;
				vertexAttribute.position.y = scene->mMeshes[assimp_mesh_counter]->mVertices[vertexAttributeCounter].y;
				vertexAttribute.position.z = scene->mMeshes[assimp_mesh_counter]->mVertices[vertexAttributeCounter].z;
				vertexAttribute.position.w = 1.0f;

				vertexAttribute.normal.x = scene->mMeshes[assimp_mesh_counter]->mNormals[vertexAttributeCounter].x;
				vertexAttribute.normal.y = scene->mMeshes[assimp_mesh_counter]->mNormals[vertexAttributeCounter].y;
				vertexAttribute.normal.z = scene->mMeshes[assimp_mesh_counter]->mNormals[vertexAttributeCounter].z;

				vertexAttribute.tangent.x = scene->mMeshes[assimp_mesh_counter]->mTangents[vertexAttributeCounter].x;
				vertexAttribute.tangent.y = scene->mMeshes[assimp_mesh_counter]->mTangents[vertexAttributeCounter].y;
				vertexAttribute.tangent.z = scene->mMeshes[assimp_mesh_counter]->mTangents[vertexAttributeCounter].z;

				vertexAttribute.bitangent.x = scene->mMeshes[assimp_mesh_counter]->mBitangents[vertexAttributeCounter].x;
				vertexAttribute.bitangent.y = scene->mMeshes[assimp_mesh_counter]->mBitangents[vertexAttributeCounter].y;
				vertexAttribute.bitangent.z = scene->mMeshes[assimp_mesh_counter]->mBitangents[vertexAttributeCounter].z;

				vertexAttribute.weights.weight1 = 0;
				vertexAttribute.weights.weight2 = 0;
				vertexAttribute.weights.weight3 = 255; // One is set to 255 so the weights are "correct" even if they arent used. 
				vertexAttribute.weights.weight4 = 0;

				vertexAttribute.vertexGroups.vertexGroupIndex1 = 0;  // Stiff objects dont use bones, even if there is a skeleton available. Just set them to 0
				vertexAttribute.vertexGroups.vertexGroupIndex2 = 0;
				vertexAttribute.vertexGroups.vertexGroupIndex3 = 0;
				vertexAttribute.vertexGroups.vertexGroupIndex4 = 0;

				//vertexAttribute.color.r = (byte)(scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->r * 255.f);
				//vertexAttribute.color.g = (byte)(scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->g * 255.f);
				//vertexAttribute.color.b = (byte)(scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->b * 255.f);
				//vertexAttribute.color.a = (byte)(scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->a * 255.f);

				vertexAttribute.color.r = 255;
				vertexAttribute.color.g = 255;
				vertexAttribute.color.b = 255;
				vertexAttribute.color.a = 255;

				for (int i = 0; i < scene->mMeshes[assimp_mesh_counter]->GetNumUVChannels(); ++i) {
					// Assuming that the fbx will use floats instead of those fucking shorts...
					TYPES::UV tmpUV;
					tmpUV.u = scene->mMeshes[assimp_mesh_counter]->mTextureCoords[i][vertexAttributeCounter].x;
					tmpUV.v = 1.f - scene->mMeshes[assimp_mesh_counter]->mTextureCoords[i][vertexAttributeCounter].y;
					vertexAttribute.uvs.push_back(std::make_pair(tmpUV, false)); // false = float
				}
				newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].push_back(vertexAttribute);
			}
			newMesh->lodBuffers[lodCounter].meshFaceContainers.push_back(std::vector<TYPES::Face>{});
			for (int faceCounter = 0; faceCounter < scene->mMeshes[assimp_mesh_counter]->mNumFaces; ++faceCounter) {
				TYPES::Face face;
				if (scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mNumIndices != 3) {
					error("Faces are not triangulated!");
				}
				face.index1 = scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mIndices[0];
				face.index2 = scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mIndices[1];
				face.index3 = scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mIndices[2];
				newMesh->lodBuffers[lodCounter].meshFaceContainers[meshCounter].push_back(face);
			}
			// VertexGroup indices

		}
		assimp_mesh_counter++;
	}
	create_mesh_data_section(newMesh, mesh);
	//newMesh->meshDescSection = mesh->meshDescSection;
}