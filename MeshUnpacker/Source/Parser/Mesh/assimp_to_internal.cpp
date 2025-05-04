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
	//if (!mesh->HasVertexColors(0)) { // For now throw error, later just make them all RGBA8(255,255,255,255)
	//	error("Mesh does not contain any vertex colors!");
	//}
	if (!mesh->HasFaces()) {
		error("Mesh does not contain any faces!");
	}

}


void edit_header(PARSER::Mesh& mesh) {
	using namespace PARSER;
	using namespace INTERNAL;

	mesh.header.magic_u = 0x48534D4D;
	mesh.header.version = 0x11; // Version 17

	ulong descSectionRawSize = 12; // sectionID etc.

	descSectionRawSize += mesh.meshDescSection.unkMatIndices.size() * sizeof(long);
	descSectionRawSize += mesh.meshDescSection.materialIndices.size() * sizeof(long);
	descSectionRawSize += mesh.meshDescSection.vertexDataSectionSizes.size() * sizeof(ulong);
	descSectionRawSize += mesh.meshDescSection.faceDataSectionSizes.size() * sizeof(ulong);
	descSectionRawSize += mesh.meshDescSection.vertexGroupDataSectionSizes.size() * sizeof(ulong);

	ulong infoSectionRawSize = 72;

	infoSectionRawSize += mesh.meshInfoSection.lodGroupIDs.size() * sizeof(ulong);
	infoSectionRawSize += mesh.meshInfoSection.connections.size() * sizeof(MeshInfoSection::Connection);
	infoSectionRawSize += mesh.meshInfoSection.lodInfos.size() * sizeof(MeshInfoSection::LodInfo);
	infoSectionRawSize += mesh.meshInfoSection.meshInfos.size() * sizeof(MeshInfoSection::MeshInfo);

	infoSectionRawSize += 4;
	infoSectionRawSize += mesh.meshInfoSection.lodSection.lodSettings.size() * sizeof(ulong);

	infoSectionRawSize += mesh.meshInfoSection.lodSection.lodThresholds.meshLodThresholds.size() * sizeof(float);
	infoSectionRawSize += mesh.meshInfoSection.lodSection.lodThresholds.shadowLodThresholds.size() * sizeof(float);
	infoSectionRawSize += mesh.meshInfoSection.lodSection.lodConnections.meshLodConnections.size() * sizeof(ulong);
	infoSectionRawSize += mesh.meshInfoSection.lodSection.lodConnections.shadowLodConnections.size() * sizeof(ulong);

	infoSectionRawSize += 4;
	for (const auto& vertexBuffer : mesh.meshInfoSection.bufferLayoutSection.vertexBufferLayouts) {
		infoSectionRawSize += 4;
		infoSectionRawSize += vertexBuffer.attributesLayouts.size() * sizeof(MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout);
	}

	infoSectionRawSize += 4;
	infoSectionRawSize += mesh.meshInfoSection.boneSection.bones.size() * sizeof(MESH::Bone);

	ulong dataSectionRawSize = 4;

	for (int i = 0; i < mesh.meshDataSection.vertexDataSections.size(); ++i)
		dataSectionRawSize += mesh.meshDataSection.vertexDataSections[i].size();

	for (int i = 0; i < mesh.meshDataSection.faceDataSections.size(); ++i)
		dataSectionRawSize += mesh.meshDataSection.faceDataSections[i].size();

	for (int i = 0; i < mesh.meshDataSection.vertexGroupDataSections.size(); ++i)
		dataSectionRawSize += mesh.meshDataSection.vertexGroupDataSections[i].size();

	mesh.header.meshDescSectionSize = descSectionRawSize;
	mesh.header.meshInfoSectionSize = infoSectionRawSize;
	mesh.header.meshDataSectionSize = dataSectionRawSize;
}

void edit_mesh_desc_section(PARSER::Mesh& mesh) {
	mesh.meshDescSection.sectionID = 0xEBAEC3FA;
	//mesh.meshDescSection.materialIndicesCount = 1;
	//mesh.meshDescSection.dataSectionCount = mesh.lodContainers.size();

	//mesh.meshDescSection.materialIndices.push_back(3);
	//mesh.meshDescSection.unkMatIndices.push_back(1);

	for (int i = 0; i < mesh.lodContainers.size(); ++i) {
		//mesh.meshDescSection.vertexDataSectionSizes.push_back(0);
		//mesh.meshDescSection.faceDataSectionSizes.push_back(0);
		//mesh.meshDescSection.vertexGroupDataSectionSizes.push_back(0);

		mesh.meshDescSection.vertexDataSectionSizes[i] = mesh.meshDataSection.vertexDataSections[i].size();
		mesh.meshDescSection.faceDataSectionSizes[i] = mesh.meshDataSection.faceDataSections[i].size();
		mesh.meshDescSection.vertexGroupDataSectionSizes[i] = mesh.meshDataSection.vertexGroupDataSections[i].size();
	}
}

void edit_mesh_info_section(PARSER::Mesh& mesh) {
	using namespace PARSER;
	using namespace INTERNAL;
	using namespace MESH;


	// Everything with a // at the end has to be revised likely
	mesh.meshInfoSection.sectionID = 0x1A1541BC;
	//mesh.meshInfoSection.lodGroupCount = 1; //
	//mesh.meshInfoSection.connectionCount = 1; //
	//mesh.meshInfoSection.lodCount = 1;
	//mesh.meshInfoSection.meshCount = mesh.assimp_scene->mNumMeshes;
	//mesh.meshInfoSection.unknownULong1 = 0; //
	//mesh.meshInfoSection.lodSettingsCount = 1; //
	//mesh.meshInfoSection.meshLodCount = 1; //
	//mesh.meshInfoSection.shadowLodCount = 1; //
	//mesh.meshInfoSection.bufferLayoutCount = 1;
	//mesh.meshInfoSection.ulongReadCount = 8; //
	//mesh.meshInfoSection.unknownFloat = 500.f; //
	//mesh.meshInfoSection.unknown1[0] = 0; //
	//mesh.meshInfoSection.unknown1[1] = 0; //
	//mesh.meshInfoSection.unknown1[2] = 0; //
	//mesh.meshInfoSection.unknown1[3] = 0; //
	//mesh.meshInfoSection.boneCount = 1; //
	//mesh.meshInfoSection.boneSectionSize = 40; // exactly one bone

	//for(int i = 0; i < mesh.meshInfoSection.lodGroupCount; ++i)
	//	mesh.meshInfoSection.lodGroupIDs.push_back(i);
	//
	//for (int i = 0; i < mesh.meshInfoSection.connectionCount; ++i)
	//	mesh.meshInfoSection.connections.push_back({ 0, 0, 0, 3 });


	// For now since there is always just ONE LOD, the for loops will only run once. First LOD gets all meshes!
	for (int i = 0; i < mesh.meshInfoSection.lodCount; ++i) {
		//mesh.meshInfoSection.lodInfos.push_back(MeshInfoSection::LodInfo{});
		mesh.meshInfoSection.lodInfos[i].faceDataSize = mesh.meshDataSection.faceDataSections[i].size();
		mesh.meshInfoSection.lodInfos[i].vertexDataSize = mesh.meshDataSection.vertexDataSections[i].size();
		mesh.meshInfoSection.lodInfos[i].vertexGroupDataSize = mesh.meshDataSection.vertexGroupDataSections[i].size();
		//mesh.meshInfoSection.lodInfos[i].meshCount = mesh.meshInfoSection.meshCount; // Just assign all meshes to the only LOD available
		for (int j = 0; j < 16; ++j) {
			mesh.meshInfoSection.lodInfos[i].faceCount[j] = mesh.meshDataSection.faceDataSections[i].size() / 6;
		}
	}


	int global_mesh_counter = 0;
	for (int lodCounter = 0; lodCounter < mesh.meshInfoSection.lodCount; ++lodCounter) {
		for (int meshCounter = 0; meshCounter < mesh.meshInfoSection.lodInfos[lodCounter].meshCount; ++meshCounter) {
			//mesh.meshInfoSection.meshInfos.push_back(MeshInfoSection::MeshInfo{});
			auto& is = mesh.meshInfoSection.meshInfos[global_mesh_counter];
			is.dataOffset = 0;
			is.verticesOffset = 0;
			is.faceIndicesOffset = 0;
			//is.vertexGroupsOffset = 0;
			is.verticesCount = mesh.lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].size();
			is.faceIndicesCount = mesh.lodContainers[lodCounter].meshFaceContainers[meshCounter].size() * 3;
			//is.vertexGroupsCount = mesh.lodContainers[lodCounter].meshVertexGroupContainers[meshCounter].size();
			is.unknown7 = 0x3F000000;
			is.unknown8 = 0x3F000000;
			is.layerIndex = 0;

			if(meshCounter > 0) {
				//for (int lod_mesh_counter = 1; lod_mesh_counter <= meshCounter; ++lod_mesh_counter) {
				//	is.dataOffset
				//		+= mesh.lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter - lod_mesh_counter].size() * mesh.bufferLayouts[0].size;
				//}
				is.dataOffset = mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].dataOffset;
				is.dataOffset += mesh.lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter - 1].size() * mesh.bufferLayouts[0].size;

				is.verticesOffset = mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].verticesOffset;
				is.verticesOffset += mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].verticesCount;

				is.faceIndicesOffset = mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].faceIndicesOffset;
				is.faceIndicesOffset += mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].faceIndicesCount;

				//is.vertexGroupsOffset = mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].vertexGroupsOffset;
				//is.vertexGroupsOffset += mesh.meshInfoSection.meshInfos[global_mesh_counter - 1].vertexGroupsCount;
			}
			global_mesh_counter++;
		}
	}


	mesh.meshInfoSection.lodSection.sectionID = 0x8E3E068E;
	//mesh.meshInfoSection.lodSection.lodSettings.push_back(1);
	//mesh.meshInfoSection.lodSection.lodThresholds.meshLodThresholds.push_back(0);
	//mesh.meshInfoSection.lodSection.lodThresholds.shadowLodThresholds.push_back(0);
	//mesh.meshInfoSection.lodSection.lodConnections.meshLodConnections.push_back(1);
	//mesh.meshInfoSection.lodSection.lodConnections.shadowLodConnections.push_back(1);


	// Has to be the same as the buffer layout in the lodContainers
	using VertexBufferLayout = MeshInfoSection::BufferLayoutSection::VertexBufferLayout;
	using AttributeLayout = VertexBufferLayout::AttributeLayout;
	using Attribute = AttributeLayout::Attribute;
	using Type = AttributeLayout::Type;
	using Buffer = AttributeLayout::Buffer;
	using Channel = AttributeLayout::Channel;

	mesh.meshInfoSection.bufferLayoutSection.vertexBufferLayouts.clear(); // Remove the current layouts

	mesh.meshInfoSection.bufferLayoutCount = 1;
	mesh.meshInfoSection.bufferLayoutSection.sectionID = 0x37D749A6;
	mesh.meshInfoSection.bufferLayoutSection.vertexBufferLayouts.push_back(VertexBufferLayout{});
	auto& vBL = mesh.meshInfoSection.bufferLayoutSection.vertexBufferLayouts[0];
	vBL.attributesCount = 8;
	vBL.attributesLayouts.push_back({ Buffer::Buffer_0, Type::VECTOR4F16, Attribute::POSITION, Channel::Channel_0});
	vBL.attributesLayouts.push_back({ Buffer::Buffer_1, Type::VECTOR2F32, Attribute::TEXCOORD, Channel::Channel_0 });
	vBL.attributesLayouts.push_back({ Buffer::Buffer_1, Type::VECTOR3F32, Attribute::NORMAL, Channel::Channel_0 });
	vBL.attributesLayouts.push_back({ Buffer::Buffer_1, Type::VECTOR3F32, Attribute::TANGENT, Channel::Channel_0 });
	vBL.attributesLayouts.push_back({ Buffer::Buffer_1, Type::VECTOR3F32, Attribute::BITANGENT, Channel::Channel_0 });
	vBL.attributesLayouts.push_back({ Buffer::Buffer_1, Type::VECTOR4U8, Attribute::COLOR, Channel::Channel_0 });
	vBL.attributesLayouts.push_back({ Buffer::Buffer_0, Type::VECTOR4U8, Attribute::WEIGHT, Channel::Channel_0});
	vBL.attributesLayouts.push_back({ Buffer::Buffer_0, Type::VECTOR4U8, Attribute::VERTEXGROUP, Channel::Channel_0});

	// one default bone
	mesh.meshInfoSection.boneSection.sectionID = 0x93D9A424;
	//Bone bone;
	//bone.boneID = 0x4179E86A;
	//bone.parentIndex = -1;
	//bone.childCount = 0;
	//bone.translation = { 0.f, 0.f, 0.f };
	//bone.rotation = { 1.f, 0.f, 0.f, 0.f };
	//bone.scale = 1;
	//mesh.meshInfoSection.boneSection.bones.push_back(bone);
	
}

void lodContainers_to_internal(PARSER::Mesh& mesh) {
	using namespace PARSER;
	using namespace INTERNAL;
	using namespace MESH;

	using AttributeLayout = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout;
	using Attribute = AttributeLayout::Attribute;
	using Type = AttributeLayout::Type;
	using Buffer = AttributeLayout::Buffer;
	using Channel = AttributeLayout::Channel;

	// This is the default single channel UV map buffer layout for most of the meshes
	BufferLayout bufferLayout;
	bufferLayout.size = 60;
	bufferLayout.order.push_back({ Buffer::Buffer_0, Type::VECTOR4F16, Attribute::POSITION, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_0, Type::VECTOR4U8, Attribute::WEIGHT, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_0, Type::VECTOR4U8, Attribute::VERTEXGROUP, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_1, Type::VECTOR2F32, Attribute::TEXCOORD, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_1, Type::VECTOR3F32, Attribute::NORMAL, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_1, Type::VECTOR3F32, Attribute::TANGENT, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_1, Type::VECTOR3F32, Attribute::BITANGENT, Channel::Channel_0 });
	bufferLayout.order.push_back({ Buffer::Buffer_1, Type::VECTOR4U8, Attribute::COLOR, Channel::Channel_0 });
	

	mesh.bufferLayouts.push_back(bufferLayout);


	int uv_counter = 0;

	auto parse_attribute = [&](std::vector<byte>* buffer, const AttributeLayout& attributeLayout, const Buffer& targetBuffer, int lodIndex, int meshIndex, int vertexAttribute)
		{
			if (attributeLayout.bufferIndex != targetBuffer)
				return;
			auto& vertAt = mesh.lodContainers[lodIndex].meshVertexAttributeContainers[meshIndex][vertexAttribute];
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
	for (int lodCounter = 0; lodCounter < mesh.lodContainers.size(); ++lodCounter) {
		mesh.meshDataSection.vertexDataSections.push_back(std::vector<byte>{});
		for (int meshCounter = 0; meshCounter < mesh.lodContainers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			// For now there is only one layout buffer so its always 0
			constexpr int current_layer_index = 0;
			for (int vertexCounter = 0; vertexCounter < mesh.lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
				for (auto& attribute : mesh.bufferLayouts[current_layer_index].order) {
					parse_attribute(&mesh.meshDataSection.vertexDataSections[lodCounter], attribute, Buffer::Buffer_0, lodCounter, meshCounter, vertexCounter);
				}
				uv_counter = 0; // Buffer 0 shouldnt contain any UVs. Its there just in case
			}
			for (int vertexCounter = 0; vertexCounter < mesh.lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
				for (auto& attribute : mesh.bufferLayouts[current_layer_index].order) {
					parse_attribute(&mesh.meshDataSection.vertexDataSections[lodCounter], attribute, Buffer::Buffer_1, lodCounter, meshCounter, vertexCounter);
				}
				uv_counter = 0;  // This is the index of the vertexAttributes uv channel. Max is 2 so it has to be reset to 0 after each vertexAttribute parsing
			}
		}
	}
	for (int lodCounter = 0; lodCounter < mesh.lodContainers.size(); ++lodCounter) {
		mesh.meshDataSection.faceDataSections.push_back(std::vector<byte>{});
		for (int meshCounter = 0; meshCounter < mesh.lodContainers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			for (int faceCounter = 0; faceCounter < mesh.lodContainers[lodCounter].meshFaceContainers[meshCounter].size(); ++faceCounter) {
				mesh.meshDataSection.faceDataSections[lodCounter].resize(mesh.meshDataSection.faceDataSections[lodCounter].size() + sizeof(Face));
				byte* end = mesh.meshDataSection.faceDataSections[lodCounter].data() + mesh.meshDataSection.faceDataSections[lodCounter].size() - sizeof(Face);
				memcpy(end, &mesh.lodContainers[lodCounter].meshFaceContainers[meshCounter][faceCounter], sizeof(Face));
			}
		}
	}
	
	

	mesh.meshDataSection.sectionID = 0x95DBDB69;
	edit_mesh_info_section(mesh);
	edit_mesh_desc_section(mesh);
	edit_header(mesh);
}

void applyNodeTransform(const aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene) {
	aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
			aiVector3D& vertex = mesh->mVertices[j];
			vertex = globalTransform * vertex;
		}
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		applyNodeTransform(node->mChildren[i], globalTransform, scene);
	}
}

void assimp_to_lodContainers(PARSER::Mesh& mesh, PARSER::Mesh& reference_mesh) {
	using namespace PARSER;
	using namespace INTERNAL;
	using namespace MESH;

	// Will be edited later
	mesh.header = reference_mesh.header;
	mesh.meshDescSection = reference_mesh.meshDescSection;
	mesh.meshInfoSection = reference_mesh.meshInfoSection;

	// VertexGroups will be unchanged for now
	mesh.meshDataSection.vertexGroupDataSections = reference_mesh.meshDataSection.vertexGroupDataSections;
	//mesh.bufferLayouts = reference_mesh.bufferLayouts;

	if (!mesh.assimp_scene->HasMeshes()) {
		error("Scene doesn't contain any meshes!");
	}
	if (mesh.meshInfoSection.meshCount != mesh.assimp_scene->mNumMeshes) {
		error("Scene and reference mesh count dont match!");
	}

	// Apply the transformations of the objects in the scene to the actual vertex positions
	applyNodeTransform(mesh.assimp_scene->mRootNode, aiMatrix4x4(), mesh.assimp_scene);

	int lodCount = mesh.meshInfoSection.lodCount;
	
	// Sort the meshes by name (000, 001, 002) so the lods are correct
	std::vector<std::string> mesh_names;

	for (int i = 0; i < mesh.assimp_scene->mNumMeshes; ++i) {
		mesh_names.push_back(mesh.assimp_scene->mMeshes[i]->mName.C_Str());
	}
	std::vector<int> mesh_indices(mesh_names.size());
	for (size_t i = 0; i < mesh_names.size(); ++i) {
		mesh_indices[i] = i;
	}

	std::sort(mesh_indices.begin(), mesh_indices.end(), [&mesh_names](int a, int b) {
		return mesh_names[a] < mesh_names[b];
	});

	// Populate the lod buffers with the data from the assimp scene
	int assimp_mesh_counter = 0;
	for (int lodCounter = 0; lodCounter < lodCount; ++lodCounter) {
		mesh.lodContainers.push_back(LODContainer{});
		aiMesh* aiMesh = mesh.assimp_scene->mMeshes[mesh_indices[assimp_mesh_counter]];
		error_checks(aiMesh);
		int meshCount = mesh.meshInfoSection.lodInfos[lodCounter].meshCount;
		for (int meshCounter = 0; meshCounter < meshCount; ++meshCounter) {
			mesh.lodContainers[lodCounter].meshVertexAttributeContainers.push_back(std::vector<VertexAttribute>{});
			for (int vertexAttributeCounter = 0; vertexAttributeCounter < aiMesh->mNumVertices; ++vertexAttributeCounter) {
				VertexAttribute vertexAttribute;
				vertexAttribute.position.x = aiMesh->mVertices[vertexAttributeCounter].x;
				vertexAttribute.position.y = aiMesh->mVertices[vertexAttributeCounter].y;
				vertexAttribute.position.z = aiMesh->mVertices[vertexAttributeCounter].z;
				vertexAttribute.position.w = 1.0f;

				vertexAttribute.normal.x = aiMesh->mNormals[vertexAttributeCounter].x;
				vertexAttribute.normal.y = aiMesh->mNormals[vertexAttributeCounter].y;
				vertexAttribute.normal.z = aiMesh->mNormals[vertexAttributeCounter].z;

				vertexAttribute.tangent.x = aiMesh->mTangents[vertexAttributeCounter].x;
				vertexAttribute.tangent.y = aiMesh->mTangents[vertexAttributeCounter].y;
				vertexAttribute.tangent.z = aiMesh->mTangents[vertexAttributeCounter].z;
				
				vertexAttribute.bitangent.x = aiMesh->mBitangents[vertexAttributeCounter].x;
				vertexAttribute.bitangent.y = aiMesh->mBitangents[vertexAttributeCounter].y;
				vertexAttribute.bitangent.z = aiMesh->mBitangents[vertexAttributeCounter].z;

				vertexAttribute.weights.weight1 = 0;
				vertexAttribute.weights.weight2 = 0;
				vertexAttribute.weights.weight3 = 255; // One is set to 255 so the weights are "correct" even if they arent used. 
				vertexAttribute.weights.weight4 = 0;

				vertexAttribute.vertexGroups.vertexGroupIndex1 = 0;  // Stiff objects dont use bones, even if there is a skeleton available. Just set them to 0
				vertexAttribute.vertexGroups.vertexGroupIndex2 = 0;
				vertexAttribute.vertexGroups.vertexGroupIndex3 = 0;
				vertexAttribute.vertexGroups.vertexGroupIndex4 = 0;

				vertexAttribute.color.r = aiMesh->mColors[0][vertexAttributeCounter].r * 255;
				vertexAttribute.color.g = aiMesh->mColors[0][vertexAttributeCounter].g * 255;
				vertexAttribute.color.b = aiMesh->mColors[0][vertexAttributeCounter].b * 255;
				vertexAttribute.color.a = aiMesh->mColors[0][vertexAttributeCounter].a * 255;

				for (int i = 0; i < aiMesh->GetNumUVChannels(); ++i) {
					// Assuming that the fbx will use floats instead of those fucking shorts...
					UV tmpUV;
					tmpUV.u = aiMesh->mTextureCoords[i][vertexAttributeCounter].x;
					tmpUV.v = aiMesh->mTextureCoords[i][vertexAttributeCounter].y;
					vertexAttribute.uvs.push_back(std::make_pair(tmpUV, false)); // false = float
				}
				mesh.lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].push_back(vertexAttribute);
			}
			mesh.lodContainers[lodCounter].meshFaceContainers.push_back(std::vector<Face>{});
			for (int faceCounter = 0; faceCounter < aiMesh->mNumFaces; ++faceCounter) {
				Face face;
				if (aiMesh->mFaces[faceCounter].mNumIndices != 3) {
					error("Faces are not triangulated! This shouldn't happen");
				}
				face.index1 = aiMesh->mFaces[faceCounter].mIndices[0];
				face.index2 = aiMesh->mFaces[faceCounter].mIndices[1];
				face.index3 = aiMesh->mFaces[faceCounter].mIndices[2];
				mesh.lodContainers[lodCounter].meshFaceContainers[meshCounter].push_back(face);
			}
			// To allocate the memory, even if its empty. Otherwise we will run into issues later when accessing the data section
			//mesh.lodContainers[lodCounter].meshVertexGroupContainers.push_back(std::vector<byte>{});
			assimp_mesh_counter++;
		}	
	}
}

