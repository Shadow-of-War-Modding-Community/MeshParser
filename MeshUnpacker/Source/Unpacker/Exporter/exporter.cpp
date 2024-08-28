#include "../Internal/includes-types-defs.h"

const aiScene* mesh_to_assimp(const MESH_UNPACKER::Mesh& mesh) {
	// Only a temporal solution. This ensures that there is no important 
	// stuff missing in order to create a valid .fbx file
	Assimp::Importer referenceImporter;
	auto const_scene = referenceImporter.ReadFile("F:\\reference.fbx", 0);
	aiScene scene = *const_scene;

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
			mesh_buffer[mesh_counter] = ai_mesh;
			mesh_counter++;
		}
	}
	scene.mNumMeshes = mesh.meshInfoSection.meshCount;
	scene.mMeshes = mesh_buffer;

	Assimp::Exporter exporter;
	if (exporter.Export(&scene, "fbx", "F:\\out.fbx") != aiReturn_SUCCESS) {
		std::cerr << "Fehler beim Exportieren der FBX-Datei" << std::endl;
	}
	return nullptr;
}