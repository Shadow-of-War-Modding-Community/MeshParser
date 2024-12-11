#include "mesh.h"


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

//void edit_mesh_header(const Mesh& refercence_mesh) {
//
//
//}

void PARSER::Mesh::edit_mesh_desc_section() {
	

}

void PARSER::Mesh::edit_mesh_info_section() {
	using namespace MESH;

	for (int i = 0; i < meshInfoSection.lodCount; ++i) {
		meshInfoSection.lodInfos[i].faceDataSize = meshDataSection.faceDataSections[i].size();
		meshInfoSection.lodInfos[i].vertexDataSize = meshDataSection.vertexDataSections[i].size();
		for (int j = 0; j < 16; ++j) {
			meshInfoSection.lodInfos[i].faceCount[j] = meshDataSection.faceDataSections[i].size() / 6;
		}
	}
	int global_mesh_counter = 0;
	for (int lodCounter = 0; lodCounter < meshInfoSection.lodCount; ++lodCounter) {
		for (int meshCounter = 0; meshCounter < meshInfoSection.lodInfos[lodCounter].meshCount; ++meshCounter) {
			if (meshCounter == 0)
				meshInfoSection.meshInfos[global_mesh_counter].dataOffset = 0;
			else {
				meshInfoSection.meshInfos[global_mesh_counter].dataOffset = 0;
				for (int lod_mesh_counter = 1; lod_mesh_counter <= meshCounter; ++lod_mesh_counter) {
					meshInfoSection.meshInfos[global_mesh_counter].dataOffset
						+= lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter - lod_mesh_counter].size() * bufferLayouts[global_mesh_counter - lod_mesh_counter].size;
				}
			}
			global_mesh_counter++;
		}
	}
}

void PARSER::Mesh::create_mesh_data_section() {
	using namespace MESH;
	using AttributeLayout = INTERNAL::MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout;
	using Attribute = AttributeLayout::Attribute;
	using Type = AttributeLayout::Type;
	using Buffer = AttributeLayout::Buffer;

	int uv_counter = 0;

	auto parse_attribute = [&](std::vector<byte>* buffer, const AttributeLayout& attributeLayout, const Buffer& targetBuffer, int lodIndex, int meshIndex, int vertexAttribute)
		{
			if (attributeLayout.bufferIndex != targetBuffer)
				return;
			auto& vertAt = lodContainers[lodIndex].meshVertexAttributeContainers[meshIndex][vertexAttribute];
			switch (attributeLayout.attribute) {
			case Attribute::POSITION:
			{
				if (attributeLayout.type == Type::VECTOR4F16) {
					Position16 position;
					position.x = vertAt.position.x;
					position.y = vertAt.position.y;
					position.z = vertAt.position.z;
					position.w = vertAt.position.w;
					buffer->resize(buffer->size() + sizeof(Position16));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Position16);
					memcpy(bufferEnd, &position, sizeof(Position16));
				}
				else if (attributeLayout.type == Type::VECTOR4F32) {
					buffer->resize(buffer->size() + sizeof(Position32));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Position32);
					memcpy(bufferEnd, &vertAt.position, sizeof(Position32));
				}
				else {
					error("Unknown Type for POSITION");
				}
				break;
			}
			case Attribute::WEIGHT:
			{
				if (attributeLayout.type == Type::VECTOR4U8) {
					buffer->resize(buffer->size() + sizeof(Weights));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Weights);
					memcpy(bufferEnd, &vertAt.weights, sizeof(Weights));
				}
				else {
					error("Unknown Type for WEIGHT");
				}
				break;
			}
			case Attribute::VERTEXGROUP:
			{
				if (attributeLayout.type == Type::VECTOR4U8) {
					buffer->resize(buffer->size() + sizeof(VertexGroups));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(VertexGroups);
					memcpy(bufferEnd, &vertAt.vertexGroups, sizeof(VertexGroups));
				}
				else {
					error("Unknown Type for VERTEXGROUP");
				}
				break;
			}
			case Attribute::TEXCOORD:
			{
				if (attributeLayout.type == Type::VECTOR2F32) {
					UV uv_correct_format = vertAt.uvs[uv_counter++].first;
					uv_correct_format.v = 1.f - uv_correct_format.v;
					buffer->resize(buffer->size() + sizeof(UV));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(UV);
					memcpy(bufferEnd, &uv_correct_format, sizeof(UV));
				}
				else if (attributeLayout.type == Type::VECTOR2S16) {
					short u, v;
					u = (short)(vertAt.uvs[uv_counter].first.u * 32767.f);
					v = (short)(1.f - vertAt.uvs[uv_counter].first.v * 32767.f);
					uv_counter++;
					buffer->resize(buffer->size() + 4); // 2 times size of short
					byte* bufferEnd = buffer->data() + buffer->size() - 4;
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
					buffer->resize(buffer->size() + sizeof(Normal));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Normal);
					memcpy(bufferEnd, &vertAt.normal, sizeof(Normal));
				}
				else {
					error("Unknown Type for NORMAL");
				}
				break;
			}
			case Attribute::TANGENT:
			{
				if (attributeLayout.type == Type::VECTOR3F32) {
					buffer->resize(buffer->size() + sizeof(Tangent));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Tangent);
					memcpy(bufferEnd, &vertAt.normal, sizeof(Tangent));
				}
				else {
					error("Unknown Type for TANGENT");
				}
				break;
			}
			case Attribute::BITANGENT:
			{
				if (attributeLayout.type == Type::VECTOR3F32) {
					buffer->resize(buffer->size() + sizeof(Bitangent));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Bitangent);
					memcpy(bufferEnd, &vertAt.normal, sizeof(Bitangent));
				}
				else {
					error("Unknown Type for BITANGENT");
				}
				break;
			}
			case Attribute::COLOR:
			{
				if (attributeLayout.type == Type::VECTOR4U8) {
					buffer->resize(buffer->size() + sizeof(Color));
					byte* bufferEnd = buffer->data() + buffer->size() - sizeof(Color);
					memcpy(bufferEnd, &vertAt.color, sizeof(Color));
				}
				else {
					error("Unknown Type for COLOR");
				}
				break;
			}
			}
		};
	int mesh_counted = 0;
	for (int lodCounter = 0; lodCounter < lodContainers.size(); ++lodCounter) {
		meshDataSection.vertexDataSections.push_back(std::vector<byte>{});
		for (int meshCounter = 0; meshCounter < lodContainers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			auto current_layer_index = meshInfoSection.meshInfos[mesh_counted].layerIndex;
			for (int vertexCounter = 0; vertexCounter < lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
				for (auto& attribute : bufferLayouts[current_layer_index].order) {
					parse_attribute(&meshDataSection.vertexDataSections[lodCounter], attribute, Buffer::Buffer_0, lodCounter, meshCounter, vertexCounter);
				}
				uv_counter = 0; // Buffer 0 shouldnt contain any UVs. Its there just in case
			}
			for (int vertexCounter = 0; vertexCounter < lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
				for (auto& attribute : bufferLayouts[current_layer_index].order) {
					parse_attribute(&meshDataSection.vertexDataSections[lodCounter], attribute, Buffer::Buffer_1, lodCounter, meshCounter, vertexCounter);
				}
				uv_counter = 0;  // This is the index of the vertexAttributes uv channel. Max is 2 so it has to be reset to 0 after each vertexAttribute parsing
			}
			mesh_counted++;
		}
	}
	mesh_counted = 0;
	for (int lodCounter = 0; lodCounter < lodContainers.size(); ++lodCounter) {
		meshDataSection.faceDataSections.push_back(std::vector<byte>{});
		for (int meshCounter = 0; meshCounter < lodContainers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			for (int faceCounter = 0; faceCounter < lodContainers[lodCounter].meshFaceContainers[meshCounter].size(); ++faceCounter) {
				meshDataSection.faceDataSections[lodCounter].resize(meshDataSection.faceDataSections[lodCounter].size() + sizeof(Face));
				byte* end = meshDataSection.faceDataSections[lodCounter].data() + meshDataSection.faceDataSections[lodCounter].size() - sizeof(Face);
				memcpy(end, &lodContainers[lodCounter].meshFaceContainers[meshCounter][faceCounter], sizeof(Face));
			}
		}
	}

}

void PARSER::Mesh::assimp_to_internal(const Mesh& reference_mesh) {
	using namespace MESH;

	header = reference_mesh.header;
	meshDescSection = reference_mesh.meshDescSection;
	meshInfoSection = reference_mesh.meshInfoSection;

	if (!scene->HasMeshes()) {
		error("Scene doesn't contain any meshes!");
	}
	if (scene->mNumMeshes != meshInfoSection.meshCount) {
		error("Mesh count is not equal in scene and reference mesh file");
	}

	// Populate the lod buffers with the data from the assimp scene
	int assimp_mesh_counter = 0;
	for (int lodCounter = 0; lodCounter < meshInfoSection.lodCount; ++lodCounter) {
		lodContainers.push_back(LODContainer{});
		error_checks(scene->mMeshes[assimp_mesh_counter]);
		for (int meshCounter = 0; meshCounter < meshInfoSection.lodInfos[lodCounter].meshCount; ++meshCounter) {
			lodContainers[lodCounter].meshVertexAttributeContainers.push_back(std::vector<VertexAttribute>{});
			for (int vertexAttributeCounter = 0; vertexAttributeCounter < scene->mMeshes[assimp_mesh_counter]->mNumVertices; ++vertexAttributeCounter) {
				VertexAttribute vertexAttribute;
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

				vertexAttribute.color.r = 255;
				vertexAttribute.color.g = 255;
				vertexAttribute.color.b = 255;
				vertexAttribute.color.a = 255;

				for (int i = 0; i < scene->mMeshes[assimp_mesh_counter]->GetNumUVChannels(); ++i) {
					// Assuming that the fbx will use floats instead of those fucking shorts...
					UV tmpUV;
					tmpUV.u = scene->mMeshes[assimp_mesh_counter]->mTextureCoords[i][vertexAttributeCounter].x;
					tmpUV.v = scene->mMeshes[assimp_mesh_counter]->mTextureCoords[i][vertexAttributeCounter].y;
					vertexAttribute.uvs.push_back(std::make_pair(tmpUV, false)); // false = float
				}
				lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].push_back(vertexAttribute);
			}
			lodContainers[lodCounter].meshFaceContainers.push_back(std::vector<Face>{});
			for (int faceCounter = 0; faceCounter < scene->mMeshes[assimp_mesh_counter]->mNumFaces; ++faceCounter) {
				Face face;
				if (scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mNumIndices != 3) {
					error("Faces are not triangulated!");
				}
				face.index1 = scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mIndices[0];
				face.index2 = scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mIndices[1];
				face.index3 = scene->mMeshes[assimp_mesh_counter]->mFaces[faceCounter].mIndices[2];
				lodContainers[lodCounter].meshFaceContainers[meshCounter].push_back(face);
			}

		}
		assimp_mesh_counter++;
	}

	create_mesh_data_section();
	edit_mesh_info_section();
}

