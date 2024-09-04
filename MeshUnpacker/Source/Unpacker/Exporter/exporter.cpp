#include "../Internal/includes-types-defs.h"

std::shared_ptr<aiScene> mesh_to_assimp(std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {
	auto scene = std::make_shared<aiScene>();

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
	

	int mesh_counter = 0;
	aiMesh** mesh_buffer = new aiMesh*[mesh->meshInfoSection.meshCount];
	for (int lodCounter = 0; lodCounter < mesh->lodBuffers.size(); ++lodCounter) {
		for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			aiMesh* ai_mesh = new aiMesh;
			auto current_mesh_vertex_count = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size();
			aiVector3D* positionBuffer = new aiVector3D[current_mesh_vertex_count];
			aiVector3D* normalsBuffer = new aiVector3D[current_mesh_vertex_count];
			aiVector3D* tangentsBuffer = new aiVector3D[current_mesh_vertex_count];
			aiVector3D* bitangentsBuffer = new aiVector3D[current_mesh_vertex_count];
			aiColor4D* colorsBuffer = new aiColor4D[current_mesh_vertex_count];
			auto uvChannelCount = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][0].uvs.size();
			aiVector3D** uvsBuffers = new aiVector3D*[uvChannelCount];
			for (int i = 0; i < uvChannelCount; ++i)
				uvsBuffers[i] = new aiVector3D[current_mesh_vertex_count];

			//if (mesh->has_skeleton) {
			//	aiBone* bones = new aiBone[mesh->skeleton->header.boneCount1];
			//	for (int boneCounter = 0; boneCounter < mesh->skeleton->header.boneCount1; ++boneCounter) {
			//		bones[boneCounter].
			//	}
			//}

			for (int vertexCounter = 0; vertexCounter < current_mesh_vertex_count; ++vertexCounter) {
				const auto& vertex = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][vertexCounter];

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

				// Convert from byte representation to float representation
				colorsBuffer[vertexCounter].r = (float)vertex.color.r / 255.f;
				colorsBuffer[vertexCounter].g = (float)vertex.color.g / 255.f;
				colorsBuffer[vertexCounter].b = (float)vertex.color.b / 255.f;
				colorsBuffer[vertexCounter].a = (float)vertex.color.a / 255.f;

				for (int i = 0; i < uvChannelCount; ++i) {
					if (vertex.uvs[i].second) { // true = short
						uvsBuffers[i][vertexCounter].x = vertex.uvs[i].first.u;
						uvsBuffers[i][vertexCounter].y = 1.f - vertex.uvs[i].first.v;
						uvsBuffers[i][vertexCounter].x /= 32767.f;
						uvsBuffers[i][vertexCounter].y /= 32767.f;
						uvsBuffers[i][vertexCounter].z = 0.f;
					}
					else { // false = float
						uvsBuffers[i][vertexCounter].x = vertex.uvs[i].first.u;
						uvsBuffers[i][vertexCounter].y = 1.f - vertex.uvs[i].first.v;
						uvsBuffers[i][vertexCounter].z = 0.f;
					}
				}

			}
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
			auto current_mesh_face_count = mesh->lodBuffers[lodCounter].meshFaceContainers[meshCounter].size();
			aiFace* faceBuffer = new aiFace[current_mesh_face_count];
			for (int faceCounter = 0; faceCounter < current_mesh_face_count; ++faceCounter) {
				const auto& face = mesh->lodBuffers[lodCounter].meshFaceContainers[meshCounter][faceCounter];
				faceBuffer[faceCounter].mNumIndices = 3;
				faceBuffer[faceCounter].mIndices = new unsigned int[3];
				faceBuffer[faceCounter].mIndices[0] = face.index1;
				faceBuffer[faceCounter].mIndices[1] = face.index2;
				faceBuffer[faceCounter].mIndices[2] = face.index3;
			}
			ai_mesh->mNumFaces = current_mesh_face_count;
			ai_mesh->mFaces = faceBuffer;
			ai_mesh->mMaterialIndex = 0;
#pragma warning(disable: 6386) // Disables wrong buffer overflow warning 
			mesh_buffer[mesh_counter] = ai_mesh; 
			mesh_counter++;
		}
	}
	scene->mNumMeshes = mesh->meshInfoSection.meshCount;
	scene->mMeshes = mesh_buffer;


	aiNode* rootNode = new aiNode;
	rootNode->mName = "RootNode";
	//rootNode->mNumMeshes = mesh->meshInfoSection.meshCount;
	rootNode->mNumMeshes = 0;

	mesh_counter = 0;
	aiNode** lodNodes = new aiNode*[mesh->lodBuffers.size()];
	for (int lodCounter = 0; lodCounter < mesh->lodBuffers.size(); ++lodCounter) {
		lodNodes[lodCounter] = new aiNode;
		lodNodes[lodCounter]->mName = aiString("LOD" + std::to_string(lodCounter + 1));
		lodNodes[lodCounter]->mNumMeshes = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size();
		unsigned int* meshes = new unsigned int[mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size()];
		//aiNode** meshNodes = new aiNode*[mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size()];
		for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			meshes[meshCounter] = mesh_counter;
			mesh_counter++;
		}
		lodNodes[lodCounter]->mMeshes = meshes;

		//lodNodes[lodCounter]->addChildren(mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(), meshNodes);
	}
	rootNode->addChildren(mesh->lodBuffers.size(), lodNodes);
	 

	 

	//rootNode->mMeshes = new unsigned int[mesh->meshInfoSection.meshCount];
	//for (int i = 0; i < mesh->meshInfoSection.meshCount; ++i)
	//	rootNode->mMeshes[i] = i;


	scene->mRootNode = rootNode;
	scene->mNumMaterials = 1;
	scene->mMaterials = new aiMaterial*[1]{material};

	return scene;
}