#include "../Internal/includes-types-defs.h"

std::unique_ptr<aiScene> mesh_to_assimp(const MESH_UNPACKER::Mesh& mesh) {
	auto scene = std::make_unique<aiScene>();

	aiMaterial* material = new aiMaterial();

	aiColor3D diffuseColor(1.0f, 0.0f, 0.0f);
	material->AddProperty(&diffuseColor, 1, AI_MATKEY_COLOR_DIFFUSE);

	aiColor3D specularColor(1.0f, 0.0f, 0.0f);
	material->AddProperty(&specularColor, 1, AI_MATKEY_COLOR_SPECULAR);

	aiColor3D ambientColor(0.1f, 0.0f, 0.0f); 
	material->AddProperty(&ambientColor, 1, AI_MATKEY_COLOR_AMBIENT);

	aiColor3D emissiveColor(1.0f, 1.0f, 1.0f);
	material->AddProperty(&emissiveColor, 1, AI_MATKEY_COLOR_EMISSIVE);

	float shininess = 32.0f;
	material->AddProperty(&shininess, 1, AI_MATKEY_SHININESS);

	int mesh_counter = 0;
	aiMesh** mesh_buffer = new aiMesh*[mesh.meshInfoSection.meshCount];
	// First counter is to iterate over the lods
	for (int lodCounter = 0; lodCounter < mesh.lodBuffers.size(); ++lodCounter) {
		// Second counter is to iterate over the meshes in each lod
		for (int meshCounter = 0; meshCounter < mesh.lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			aiMesh* ai_mesh = new aiMesh;
			auto current_mesh_vertex_count = mesh.lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size();
			aiVector3D* positionBuffer = new aiVector3D[current_mesh_vertex_count];
			for (int vertexCounter = 0; vertexCounter < current_mesh_vertex_count; ++vertexCounter) {
				const auto& vertex = mesh.lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][vertexCounter];
				positionBuffer[vertexCounter].x = vertex.position.x;
				positionBuffer[vertexCounter].y = vertex.position.y;
				positionBuffer[vertexCounter].z = vertex.position.z;			
			}
			ai_mesh->mNumVertices = current_mesh_vertex_count;
			ai_mesh->mVertices = positionBuffer;
			auto current_mesh_face_count = mesh.lodBuffers[lodCounter].meshFaceContainers[meshCounter].size();
			aiFace* faceBuffer = new aiFace[current_mesh_face_count];
			for (int faceCounter = 0; faceCounter < current_mesh_face_count; ++faceCounter) {
				const auto& face = mesh.lodBuffers[lodCounter].meshFaceContainers[meshCounter][faceCounter];
				faceBuffer[faceCounter].mNumIndices = 3;
				faceBuffer[faceCounter].mIndices = new unsigned int[3];
				faceBuffer[faceCounter].mIndices[0] = face.index1;
				faceBuffer[faceCounter].mIndices[1] = face.index2;
				faceBuffer[faceCounter].mIndices[2] = face.index3;
			}
			ai_mesh->mNumFaces = current_mesh_face_count;
			ai_mesh->mFaces = faceBuffer;
			ai_mesh->mMaterialIndex = 0;
			mesh_buffer[mesh_counter] = ai_mesh;  // If the compiler warns here, ignore it...
			mesh_counter++;
		}
	}
	scene->mNumMeshes = mesh.meshInfoSection.meshCount;
	scene->mMeshes = mesh_buffer;

	aiNode* rootNode = new aiNode;
	rootNode->mName = "RootNode";
	rootNode->mNumMeshes = mesh.meshInfoSection.meshCount;
	rootNode->mMeshes = new unsigned int[mesh.meshInfoSection.meshCount];
	for (int i = 0; i < mesh.meshInfoSection.meshCount; ++i)
		rootNode->mMeshes[i] = i;
	rootNode->mNumChildren = 0;

	scene->mRootNode = rootNode;
	scene->mNumMaterials = 1;
	scene->mMaterials = new aiMaterial*[1]{material};

	return scene;
}