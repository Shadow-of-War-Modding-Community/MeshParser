#include "mesh.h"

void PARSER::Mesh::mesh_to_internal(const std::string& mesh_file)
{
	using namespace INTERNAL;
	using namespace MESH;
	std::ifstream mesh(mesh_file, std::ios::binary);
	
	// Header
	{
		mesh.read((char*)&header, sizeof(header));

		expect(header.magic_u, 0x48534D4Du);
		expect(header.version, 0x11u);
	}

	// DescSection
	{
		auto& mDS = meshDescSection;

		mesh.read((char*)&mDS, sizeof(ulong) * 3); // parse first three attributes

		expect(mDS.sectionID, 0xEBAEC3FAu);

		mDS.unkMatIndices.resize(mDS.materialIndicesCount);
		mesh.read((char*)mDS.unkMatIndices.data(), sizeof(long) * mDS.materialIndicesCount);
		
		mDS.materialIndices.resize(mDS.materialIndicesCount);
		mesh.read((char*)mDS.materialIndices.data(), sizeof(long) * mDS.materialIndicesCount);
	
		mDS.vertexDataSectionSizes.resize(mDS.dataSectionCount);
		mesh.read((char*)mDS.vertexDataSectionSizes.data(), sizeof(long) * mDS.dataSectionCount);
	
		mDS.faceDataSectionSizes.resize(mDS.dataSectionCount);
		mesh.read((char*)mDS.faceDataSectionSizes.data(), sizeof(long) * mDS.dataSectionCount);
	
		mDS.vertexGroupDataSectionSizes.resize(mDS.dataSectionCount);
		mesh.read((char*)mDS.vertexGroupDataSectionSizes.data(), sizeof(long) * mDS.dataSectionCount);
	}

	// InfoSection
	{
		auto& mIS = meshInfoSection;

		mesh.read((char*)&mIS, 72); // parse until lodGroupIDs

		expect(mIS.sectionID, 0x1A1541BCu);
		
		mIS.lodGroupIDs.resize(mIS.lodGroupCount);
		mesh.read((char*)mIS.lodGroupIDs.data(), sizeof(ulong) * mIS.lodGroupCount);
	
		mIS.connections.resize(mIS.connectionCount);
		mesh.read((char*)mIS.connections.data(), sizeof(MeshInfoSection::Connection) * mIS.connectionCount);
	
		mIS.lodInfos.resize(mIS.lodCount);
		mesh.read((char*)mIS.lodInfos.data(), sizeof(MeshInfoSection::LodInfo) * mIS.lodCount);

		mIS.meshInfos.resize(mIS.meshCount);
		mesh.read((char*)mIS.meshInfos.data(), sizeof(MeshInfoSection::MeshInfo) * mIS.meshCount);
	
		// LOD Section

		read_ulong(mIS.lodSection.sectionID, mesh);

		expect(mIS.lodSection.sectionID, 0x8E3E068Eu);

		mIS.lodSection.lodSettings.resize(mIS.lodSettingsCount);
		mesh.read((char*)mIS.lodSection.lodSettings.data(), sizeof(ulong) * mIS.lodSettingsCount);
	
		mIS.lodSection.lodThresholds.meshLodThresholds.resize(mIS.meshLodCount);
		mesh.read((char*)mIS.lodSection.lodThresholds.meshLodThresholds.data(), sizeof(float) * mIS.meshLodCount);

		mIS.lodSection.lodThresholds.shadowLodThresholds.resize(mIS.shadowLodCount);
		mesh.read((char*)mIS.lodSection.lodThresholds.shadowLodThresholds.data(), sizeof(float) * mIS.shadowLodCount);
		
		mIS.lodSection.lodConnections.meshLodConnections.resize(mIS.meshLodCount);
		mesh.read((char*)mIS.lodSection.lodConnections.meshLodConnections.data(), sizeof(ulong) * mIS.meshLodCount);

		mIS.lodSection.lodConnections.shadowLodConnections.resize(mIS.shadowLodCount);
		mesh.read((char*)mIS.lodSection.lodConnections.shadowLodConnections.data(), sizeof(ulong) * mIS.shadowLodCount);
	
		// Buffer Layout Section

		read_ulong(mIS.bufferLayoutSection.sectionID, mesh);

		expect(mIS.bufferLayoutSection.sectionID, 0x37D749A6u);

		mIS.bufferLayoutSection.vertexBufferLayouts.resize(mIS.bufferLayoutCount);
	
		for (int i = 0; i < mIS.bufferLayoutCount; ++i) {
			read_ulong(mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesCount, mesh);
			mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesLayouts.resize(mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesCount);
			mesh.read((char*)mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesLayouts.data(), 
				sizeof(MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout) * mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesCount);
		}

		// Bones Section

		read_ulong(mIS.boneSection.sectionID, mesh);

		expect(mIS.boneSection.sectionID, 0x93D9A424u);

		mIS.boneSection.bones.resize(mIS.boneCount);
		mesh.read((char*)mIS.boneSection.bones.data(), sizeof(Bone) * mIS.boneCount);
	}

	// DataSection
	{
		auto& mDS = meshDataSection;
		read_ulong(mDS.sectionID, mesh);

		expect(mDS.sectionID, 0x95DBDB69u);

		for (int i = 0; i < meshDescSection.dataSectionCount; ++i) {
			std::vector<byte> tmp_buf(meshDescSection.vertexDataSectionSizes[i]);
			mesh.read((char*)tmp_buf.data(), meshDescSection.vertexDataSectionSizes[i]);
			mDS.vertexDataSections.push_back(tmp_buf);
		}

		for (int i = 0; i < meshDescSection.dataSectionCount; ++i) {
			std::vector<byte> tmp_buf(meshDescSection.faceDataSectionSizes[i]);
			mesh.read((char*)tmp_buf.data(), meshDescSection.faceDataSectionSizes[i]);
			mDS.faceDataSections.push_back(tmp_buf);
		}

		for (int i = 0; i < meshDescSection.dataSectionCount; ++i) {
			std::vector<byte> tmp_buf(meshDescSection.vertexGroupDataSectionSizes[i]);
			mesh.read((char*)tmp_buf.data(), meshDescSection.vertexGroupDataSectionSizes[i]);
			mDS.vertexGroupDataSections.push_back(tmp_buf);
		}
	}

	
}

void PARSER::Mesh::internal_to_lodContainers()
{
	using namespace INTERNAL;
	using namespace MESH;

	// Fill Buffer Layouts
	{
		using Buffer = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout::Buffer;
		using Type = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout::Type;

		auto size_check = [&](Type type) -> ulong {
			switch (type)
			{
			case Type::VECTOR2F32:
				return 8;
			case Type::VECTOR2S16:
				return 4;
			case Type::VECTOR3F32:
				return 12;
			case Type::VECTOR4F16:
				return 8;
			case Type::VECTOR4F32:
				return 16;
			case Type::VECTOR4U8:
				return 4;
			default:
				return 0;
			}
			};

		for (int i = 0; i < meshInfoSection.bufferLayoutCount; ++i) {
			bufferLayouts.push_back(BufferLayout{});
			const auto& layout = meshInfoSection.bufferLayoutSection.vertexBufferLayouts[i];
			for (int j = 0; j < layout.attributesCount; ++j) {
				if (layout.attributesLayouts[j].bufferIndex == Buffer::Buffer_0) {
					bufferLayouts[i].order.push_back(layout.attributesLayouts[j]);
					bufferLayouts[i].size += size_check(layout.attributesLayouts[j].type);
				}
			}
			for (int j = 0; j < layout.attributesCount; ++j) {
				if (layout.attributesLayouts[j].bufferIndex == Buffer::Buffer_1) {
					bufferLayouts[i].order.push_back(layout.attributesLayouts[j]);
					bufferLayouts[i].size += size_check(layout.attributesLayouts[j].type);
				}
			}
		}
	}

	// Fill LOD Containers
	{
		using AttributeLayout = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout;
		using Attribute = AttributeLayout::Attribute;
		using Type = AttributeLayout::Type;
		using Buffer = AttributeLayout::Buffer;

		int virtual_vertex_buffer_offset = 0, virtual_face_buffer_offset = 0, virtual_vertexGroup_buffer_offset = 0;


		auto read_attribute = [&](byte* vertexbuffer, const AttributeLayout& attributeLayout, VertexAttribute& va, const Buffer& targetBuffer)
			{
				if (attributeLayout.bufferIndex != targetBuffer)
					return;
				switch (attributeLayout.attribute) {
				case Attribute::POSITION:
				{
					if (attributeLayout.type == Type::VECTOR4F16) {
						Position16* position = (Position16*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.position.x = (float)position->x;
						va.position.y = (float)position->y;
						va.position.z = (float)position->z;
						va.position.w = (float)position->w;
						virtual_vertex_buffer_offset += sizeof(Position16);
					}
					else if (attributeLayout.type == Type::VECTOR4F32) {
						Position32* position = (Position32*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.position = *position;
						virtual_vertex_buffer_offset += sizeof(Position32);
					}
					else {
						error("Unknown Type for POSITION");
					}
					break;
				}
				case Attribute::WEIGHT:
				{
					if (attributeLayout.type == Type::VECTOR4U8) {
						Weights* weights = (Weights*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.weights = *weights;
						virtual_vertex_buffer_offset += sizeof(Weights);
					}
					else {
						error("Unknown Type for WEIGHT");
					}
					break;
				}
				case Attribute::VERTEXGROUP:
				{
					if (attributeLayout.type == Type::VECTOR4U8) {
						VertexGroups* vertexGroups = (VertexGroups*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.vertexGroups = *vertexGroups;
						virtual_vertex_buffer_offset += sizeof(VertexGroups);
					}
					else {
						error("Unknown Type for VERTEXGROUP");
					}
					break;
				}
				case Attribute::TEXCOORD:
				{
					if (attributeLayout.type == Type::VECTOR2F32) {
						UV* uv = (UV*)(vertexbuffer + virtual_vertex_buffer_offset);
						UV tmpUV = *uv;
						va.uvs.push_back(std::make_pair(tmpUV, false));
						virtual_vertex_buffer_offset += sizeof(UV);
					}
					else if (attributeLayout.type == Type::VECTOR2S16) {
						short* uv = (short*)(vertexbuffer + virtual_vertex_buffer_offset);
						UV tmpUV;
						tmpUV.u = uv[0];
						tmpUV.v = uv[1];
						va.uvs.push_back(std::make_pair(tmpUV, true));
						virtual_vertex_buffer_offset += sizeof(short) * 2;
					}
					else {
						error("Unknown Type for TEXCOORD");
					}
					break;
				}
				case Attribute::NORMAL:
				{
					if (attributeLayout.type == Type::VECTOR3F32) {
						Normal* normal = (Normal*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.normal = *normal;
						virtual_vertex_buffer_offset += sizeof(Normal);
					}
					else {
						error("Unknown Type for NORMAL");
					}
					break;
				}
				case Attribute::TANGENT:
				{
					if (attributeLayout.type == Type::VECTOR3F32) {
						Tangent* tangent = (Tangent*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.tangent = *tangent;
						virtual_vertex_buffer_offset += sizeof(Tangent);
					}
					else {
						error("Unknown Type for TANGENT");
					}
					break;
				}
				case Attribute::BITANGENT:
				{
					if (attributeLayout.type == Type::VECTOR3F32) {
						Bitangent* bitangent = (Bitangent*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.bitangent = *bitangent;
						virtual_vertex_buffer_offset += sizeof(Bitangent);
					}
					else {
						error("Unknown Type for BITANGENT");
					}
					break;
				}
				case Attribute::COLOR:
				{
					if (attributeLayout.type == Type::VECTOR4U8) {
						Color* color = (Color*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.color = *color;
						virtual_vertex_buffer_offset += sizeof(Color);
					}
					else {
						error("Unknown Type for COLOR");
					}
					break;
				}
				}
			};
		int mesh_counted = 0;
		for (int lodCounter = 0; lodCounter < meshInfoSection.lodCount; ++lodCounter) {
			lodContainers.push_back(LODContainer{});
			for (int mesh = 0; mesh < meshInfoSection.lodInfos[lodCounter].meshCount; ++mesh) {
				auto current_layer_index = meshInfoSection.meshInfos[mesh_counted + mesh].layerIndex;
				lodContainers[lodCounter].meshVertexAttributeContainers.push_back(std::vector<VertexAttribute>{});
				for (int vertexCounter = 0; vertexCounter < meshInfoSection.meshInfos[mesh_counted + mesh].verticesCount; ++vertexCounter) {
					VertexAttribute vertexAttribute{};
					for (auto& attribute : bufferLayouts[current_layer_index].order) {
						read_attribute(meshDataSection.vertexDataSections[lodCounter].data(), attribute, vertexAttribute, Buffer::Buffer_0);
					}
					lodContainers[lodCounter].meshVertexAttributeContainers[mesh].push_back(vertexAttribute);
				}
				for (int vertexCounter = 0; vertexCounter < meshInfoSection.meshInfos[mesh_counted + mesh].verticesCount; ++vertexCounter) {
					for (auto& attribute : bufferLayouts[current_layer_index].order) {
						read_attribute(meshDataSection.vertexDataSections[lodCounter].data(), attribute, lodContainers[lodCounter].meshVertexAttributeContainers[mesh][vertexCounter],
							Buffer::Buffer_1);
					}
				}
				Face* face = (Face*)(meshDataSection.faceDataSections[lodCounter].data() + virtual_face_buffer_offset);
				lodContainers[lodCounter].meshFaceContainers.push_back(std::vector<Face>{});
				for (int faceCounter = 0; faceCounter < meshInfoSection.meshInfos[mesh_counted + mesh].faceIndicesCount / 3; ++faceCounter) {
					lodContainers[lodCounter].meshFaceContainers[mesh].push_back(face[faceCounter]);
					virtual_face_buffer_offset += sizeof(Face);
				}
				byte* vertexGroupIndex = (byte*)(meshDataSection.vertexGroupDataSections[lodCounter].data() + virtual_vertexGroup_buffer_offset);
				lodContainers[lodCounter].meshVertexGroupContainers.push_back(std::vector<byte>{});
				for (int vGCounter = 0; vGCounter < meshInfoSection.meshInfos[mesh_counted + mesh].vertexGroupsCount; ++vGCounter) {
					lodContainers[lodCounter].meshVertexGroupContainers[mesh].push_back(vertexGroupIndex[vGCounter]);
					virtual_vertexGroup_buffer_offset += sizeof(byte);
				}
			}
			virtual_vertex_buffer_offset = 0;
			virtual_face_buffer_offset = 0;
			virtual_vertexGroup_buffer_offset = 0;
			mesh_counted += meshInfoSection.lodInfos[lodCounter].meshCount;
		}
	}
}

void PARSER::Mesh::lodContainers_to_assimp()
{
	scene = std::make_unique<aiScene>();

	aiMaterial* material = new aiMaterial();

	aiColor3D diffuseColor(1.0f, 1.0f, 1.0f);
	material->AddProperty(&diffuseColor, 1, AI_MATKEY_COLOR_DIFFUSE);

	aiColor3D specularColor(1.0f, 1.0f, 1.0f);
	material->AddProperty(&specularColor, 1, AI_MATKEY_COLOR_SPECULAR);

	aiColor3D ambientColor(1.0f, 1.0f, 1.0f);
	material->AddProperty(&ambientColor, 1, AI_MATKEY_COLOR_AMBIENT);

	aiColor3D emissiveColor(1.0f, 1.0f, 1.0f);
	material->AddProperty(&emissiveColor, 1, AI_MATKEY_COLOR_EMISSIVE);

	float shininess = 20.0f;
	material->AddProperty(&shininess, 1, AI_MATKEY_SHININESS);

	auto translation_matrix = [](const MESH::Bone& bone, const aiMatrix4x4& parentMatrix) {

		aiMatrix4x4 rotation = aiMatrix4x4();
		aiMatrix4x4 trans = aiMatrix4x4();
		aiMatrix4x4 result = aiMatrix4x4();

		//aiQuaternion quaternion = aiQuaternion(bone.rotation.w, bone.rotation.x, bone.rotation.z, bone.rotation.y);
		//rotation = aiMatrix4x4(quaternion.GetMatrix());
		aiMatrix4x4::Translation(aiVector3D(bone.translation.x, bone.translation.z, bone.translation.y), trans);

		result = trans * rotation;
		return result;
		};

	auto inverse = [&](const MESH::Bone& bone, const aiMatrix4x4& parentMatrix) {
		auto mat = translation_matrix(bone, parentMatrix);
		return mat.Inverse();
		};


	aiNode* rootNode = new aiNode;
	rootNode->mNumMeshes = meshInfoSection.meshCount;
	rootNode->mName = "RootNode";

	int mesh_counter = 0;
	aiMesh** mesh_buffer = new aiMesh*[meshInfoSection.meshCount];

	for (int lodCounter = 0; lodCounter < lodContainers.size(); ++lodCounter) {
		for (int meshCounter = 0; meshCounter < lodContainers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			aiMesh* ai_mesh = new aiMesh;
			auto current_mesh_vertex_count = lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter].size();
			aiVector3D* positionBuffer = new aiVector3D[current_mesh_vertex_count];
			aiVector3D* normalsBuffer = new aiVector3D[current_mesh_vertex_count];
			aiVector3D* tangentsBuffer = new aiVector3D[current_mesh_vertex_count];
			aiVector3D* bitangentsBuffer = new aiVector3D[current_mesh_vertex_count];
			aiColor4D* colorsBuffer = new aiColor4D[current_mesh_vertex_count];
			auto uvChannelCount = lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter][0].uvs.size();
			aiVector3D** uvsBuffers = new aiVector3D * [uvChannelCount];
			for (int i = 0; i < uvChannelCount; ++i)
				uvsBuffers[i] = new aiVector3D[current_mesh_vertex_count];

			// Allocate bones for the mesh
			//aiBone** bones = new aiBone * [mesh->skeleton->header.boneCount1];
			//if (mesh->has_skeleton) {
			//	for (int boneCounter = 0; boneCounter < mesh->skeleton->header.boneCount1; ++boneCounter) {
			//		auto bone = mesh->skeleton->boneSection.bones[boneCounter];
			//		bones[boneCounter] = new aiBone();
			//		bones[boneCounter]->mName = bone.boneName;
			//		//bones[boneCounter]->mOffsetMatrix = compute_bone_offset_matrix(mesh->meshInfoSection.boneSection.bones[boneCounter]);
			//	}
			//}
			//else
			//	delete[] bones;

			for (int vertexCounter = 0; vertexCounter < current_mesh_vertex_count; ++vertexCounter) {
				const auto& vertex = lodContainers[lodCounter].meshVertexAttributeContainers[meshCounter][vertexCounter];

				positionBuffer[vertexCounter].x = vertex.position.x;
				positionBuffer[vertexCounter].y = vertex.position.y;
				positionBuffer[vertexCounter].z = vertex.position.z;

				normalsBuffer[vertexCounter].x = vertex.normal.x;
				normalsBuffer[vertexCounter].y = vertex.normal.y;
				normalsBuffer[vertexCounter].z = vertex.normal.z;

				tangentsBuffer[vertexCounter].x = vertex.tangent.x;
				tangentsBuffer[vertexCounter].y = vertex.tangent.y;
				tangentsBuffer[vertexCounter].z = vertex.tangent.z;

				bitangentsBuffer[vertexCounter].x = vertex.bitangent.x;
				bitangentsBuffer[vertexCounter].y = vertex.bitangent.y;
				bitangentsBuffer[vertexCounter].z = vertex.bitangent.z;

				colorsBuffer[vertexCounter].r = (float)vertex.color.r / 255.f;
				colorsBuffer[vertexCounter].g = (float)vertex.color.g / 255.f;
				colorsBuffer[vertexCounter].b = (float)vertex.color.b / 255.f;
				colorsBuffer[vertexCounter].a = (float)vertex.color.a / 255.f;

				for (int i = 0; i < uvChannelCount; ++i) {
					if (vertex.uvs[i].second) { // true = short
						uvsBuffers[i][vertexCounter].x = vertex.uvs[i].first.u / 32767.f;
						uvsBuffers[i][vertexCounter].y = 1.f - vertex.uvs[i].first.v / 32767.f;
						uvsBuffers[i][vertexCounter].z = 0.f;
					}
					else { // false = float
						uvsBuffers[i][vertexCounter].x = vertex.uvs[i].first.u;
						uvsBuffers[i][vertexCounter].y = 1.f - vertex.uvs[i].first.v;
						uvsBuffers[i][vertexCounter].z = 0.f;
					}
				}
			}

			// Assign mesh data
			ai_mesh->mNumVertices = current_mesh_vertex_count;
			ai_mesh->mVertices = positionBuffer;
			ai_mesh->mNormals = normalsBuffer;
			ai_mesh->mTangents = tangentsBuffer;
			ai_mesh->mBitangents = bitangentsBuffer;
			ai_mesh->mColors[0] = colorsBuffer;
			for (int i = 0; i < uvChannelCount; ++i) {
				ai_mesh->mTextureCoords[i] = uvsBuffers[i];
				ai_mesh->mNumUVComponents[i] = 2;
			}

			// Process faces
			auto current_mesh_face_count = lodContainers[lodCounter].meshFaceContainers[meshCounter].size();
			aiFace* faceBuffer = new aiFace[current_mesh_face_count];
			for (int faceCounter = 0; faceCounter < current_mesh_face_count; ++faceCounter) {
				const auto& face = lodContainers[lodCounter].meshFaceContainers[meshCounter][faceCounter];
				faceBuffer[faceCounter].mNumIndices = 3;
				faceBuffer[faceCounter].mIndices = new unsigned int[3];
				faceBuffer[faceCounter].mIndices[0] = face.index1;
				faceBuffer[faceCounter].mIndices[1] = face.index2;
				faceBuffer[faceCounter].mIndices[2] = face.index3;
			}
			ai_mesh->mNumFaces = current_mesh_face_count;
			ai_mesh->mFaces = faceBuffer;

			// Process bone weights
			//if (mesh->has_skeleton) {
			//	for (int vertexGroupCounter = 0; vertexGroupCounter < mesh->lodBuffers[lodCounter].meshVertexGroupContainers[meshCounter].size(); ++vertexGroupCounter) {
			//		std::vector<aiVertexWeight> weights;
			//		auto currentVertexGroup = mesh->lodBuffers[lodCounter].meshVertexGroupContainers[meshCounter][vertexGroupCounter];
			//		for (int vertexCounter = 0; vertexCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
			//			aiVertexWeight weight;
			//			auto& vertex = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][vertexCounter];
			//
			//			int sum = vertex.weights.weight1 + vertex.weights.weight2 + vertex.weights.weight3 + vertex.weights.weight4;
			//			if (sum > 0) {
			//				if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex1) {
			//					weight.mVertexId = vertexCounter;
			//					weight.mWeight = (float)vertex.weights.weight1 / sum;
			//					weights.push_back(weight);
			//				}
			//				else if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex2) {
			//					weight.mVertexId = vertexCounter;
			//					weight.mWeight = (float)vertex.weights.weight2 / sum;
			//					weights.push_back(weight);
			//				}
			//				else if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex3) {
			//					weight.mVertexId = vertexCounter;
			//					weight.mWeight = (float)vertex.weights.weight3 / sum;
			//					weights.push_back(weight);
			//				}
			//				else if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex4) {
			//					weight.mVertexId = vertexCounter;
			//					weight.mWeight = (float)vertex.weights.weight4 / sum;
			//					weights.push_back(weight);
			//				}
			//			}
			//		}
			//		bones[currentVertexGroup]->mNumWeights = weights.size();
			//		bones[currentVertexGroup]->mWeights = new aiVertexWeight[weights.size()];
			//		std::copy(weights.begin(), weights.end(), bones[currentVertexGroup]->mWeights);
			//	}
			//	ai_mesh->mBones = bones;
			//	ai_mesh->mNumBones = mesh->skeleton->header.boneCount1;
			//}

			ai_mesh->mMaterialIndex = 0;
			mesh_buffer[mesh_counter] = ai_mesh;
			mesh_counter++;
		}
	}

	scene->mNumMeshes = meshInfoSection.meshCount;
	scene->mMeshes = mesh_buffer;

	//if (mesh->has_skeleton) {
	//	std::vector<aiNode*> nodes(mesh->skeleton->header.boneCount1);
	//	for (int i = 0; i < mesh->skeleton->header.boneCount1; ++i) {
	//		aiNode* node = new aiNode();
	//		node->mName = aiString(mesh->skeleton->boneSection.bones[i].boneName);
	//		nodes[i] = node;
	//	}
	//
	//	for (int i = 0; i < mesh->skeleton->header.boneCount1; ++i) {
	//		auto parent_index = mesh->meshInfoSection.boneSection.bones[i].parentIndex;
	//		if (parent_index == -1) {
	//			aiMatrix4x4 identity = aiMatrix4x4();
	//			nodes[i]->mTransformation = translation_matrix(mesh->meshInfoSection.boneSection.bones[i], identity);
	//			//for (int j = 0; j < mesh_counter; ++j) {
	//			//    scene->mMeshes[j]->mBones[i]->mOffsetMatrix = translation_matrix(mesh->meshInfoSection.boneSection.bones[i], identity);
	//			//}
	//			rootNode->addChildren(1, &nodes[i]);
	//		}
	//		else {
	//			nodes[i]->mTransformation = translation_matrix(mesh->meshInfoSection.boneSection.bones[i], nodes[parent_index]->mTransformation);
	//			//for (int j = 0; j < mesh_counter; ++j) {
	//			//    scene->mMeshes[j]->mBones[i]->mOffsetMatrix = translation_matrix(mesh->meshInfoSection.boneSection.bones[i], nodes[parent_index]->mTransformation);
	//			//}
	//			nodes[parent_index]->addChildren(1, &nodes[i]);
	//		}
	//	}
	//}
	// Set LOD nodes as children of the root node
	//mesh_counter = 0;
	//aiNode** lodNodes = new aiNode * [mesh->lodBuffers.size()];
	//for (int lodCounter = 0; lodCounter < mesh->lodBuffers.size(); ++lodCounter) {
	//    lodNodes[lodCounter] = new aiNode;
	//    lodNodes[lodCounter]->mName = aiString("LOD " + std::to_string(lodCounter + 1));
	//    lodNodes[lodCounter]->mNumMeshes = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size();
	//    unsigned int* meshes = new unsigned int[mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size()];
	//    for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
	//        meshes[meshCounter] = mesh_counter;
	//        mesh_counter++;
	//    }
	//    lodNodes[lodCounter]->mMeshes = meshes;
	//}
	//rootNode->addChildren(mesh->lodBuffers.size(), lodNodes);
	unsigned int* meshNr = new unsigned int[meshInfoSection.meshCount];
	for (int i = 0; i < meshInfoSection.meshCount; ++i)
		meshNr[i] = i;

	rootNode->mMeshes = meshNr;
	scene->mRootNode = rootNode;
	scene->mNumMaterials = 1;
	scene->mMaterials = new aiMaterial*[1]{material};
}


void PARSER::Mesh::assimp_to_mesh()
{

}

void PARSER::Mesh::import_mesh(const std::string& mesh_file)
{
	mesh_to_internal(mesh_file);
	internal_to_lodContainers();
	lodContainers_to_assimp();
}

void PARSER::Mesh::export_custom(const std::string& file, const std::string& format)
{
	Assimp::Exporter exporter;
	if (exporter.Export(scene.get(), format, file) != AI_SUCCESS) {
		error(exporter.GetErrorString());
	}
}
