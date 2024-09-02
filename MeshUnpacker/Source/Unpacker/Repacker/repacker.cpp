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

void create_mesh_header(std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {

}

void create_mesh_desc_section(std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {
	using namespace MESH_UNPACKER::INTERNAL::MESH;
}

void create_mesh_info_section(std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {

}

void create_mesh_data_section(std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {

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

	int mesh_counter = 0;
	for (int lodCounter = 0; lodCounter < mesh->meshInfoSection.lodCount; ++lodCounter) {
		newMesh->lodBuffers.push_back(LODBuffer{});
		error_checks(scene->mMeshes[mesh_counter]);
		for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
			newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers.push_back(std::vector<TYPES::VertexAttribute>{});
			for (int vertexAttributeCounter = 0; vertexAttributeCounter < scene->mMeshes[mesh_counter]->mNumVertices; ++vertexAttributeCounter) {
				TYPES::VertexAttribute vertexAttribute;
				vertexAttribute.position.x = scene->mMeshes[meshCounter]->mVertices[vertexAttributeCounter].x;
				vertexAttribute.position.y = scene->mMeshes[meshCounter]->mVertices[vertexAttributeCounter].y;
				vertexAttribute.position.z = scene->mMeshes[meshCounter]->mVertices[vertexAttributeCounter].z;
				vertexAttribute.position.w = 1.0f;

				vertexAttribute.normal.x = scene->mMeshes[meshCounter]->mNormals[vertexAttributeCounter].x;
				vertexAttribute.normal.y = scene->mMeshes[meshCounter]->mNormals[vertexAttributeCounter].y;
				vertexAttribute.normal.z = scene->mMeshes[meshCounter]->mNormals[vertexAttributeCounter].z;

				vertexAttribute.tangent.x = scene->mMeshes[meshCounter]->mTangents[vertexAttributeCounter].x;
				vertexAttribute.tangent.y = scene->mMeshes[meshCounter]->mTangents[vertexAttributeCounter].y;
				vertexAttribute.tangent.z = scene->mMeshes[meshCounter]->mTangents[vertexAttributeCounter].z;

				vertexAttribute.bitangent.x = scene->mMeshes[meshCounter]->mBitangents[vertexAttributeCounter].x;
				vertexAttribute.bitangent.y = scene->mMeshes[meshCounter]->mBitangents[vertexAttributeCounter].y;
				vertexAttribute.bitangent.z = scene->mMeshes[meshCounter]->mBitangents[vertexAttributeCounter].z;

				vertexAttribute.color.r = (byte)scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->r * 255.f;
				vertexAttribute.color.g = (byte)scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->g * 255.f;
				vertexAttribute.color.b = (byte)scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->b * 255.f;
				vertexAttribute.color.a = (byte)scene->mMeshes[meshCounter]->mColors[vertexAttributeCounter]->a * 255.f;

				for (int i = 0; i < scene->mMeshes[meshCounter]->GetNumUVChannels(); ++i) {
					// Assuming that the fbx will use floats instead of those fucking shorts...
					TYPES::UV tmpUV;
					tmpUV.u = scene->mMeshes[meshCounter]->mTextureCoords[i][vertexAttributeCounter].x;
					tmpUV.v = 1.f - scene->mMeshes[meshCounter]->mTextureCoords[i][vertexAttributeCounter].y;
					vertexAttribute.uvs.push_back(std::make_pair(tmpUV, false)); // false = float
				}
				newMesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].push_back(vertexAttribute);
			}
			newMesh->lodBuffers[lodCounter].meshFaceContainers.push_back(std::vector<TYPES::Face>{});
			for (int faceCounter = 0; faceCounter < scene->mMeshes[meshCounter]->mNumFaces; ++faceCounter) {
				TYPES::Face face;
				if (scene->mMeshes[meshCounter]->mFaces[faceCounter].mNumIndices != 3) {
					error("Faces are not triangulated!");
				}
				face.index1 = scene->mMeshes[meshCounter]->mFaces[faceCounter].mIndices[0];
				face.index2 = scene->mMeshes[meshCounter]->mFaces[faceCounter].mIndices[1];
				face.index3 = scene->mMeshes[meshCounter]->mFaces[faceCounter].mIndices[2];
				newMesh->lodBuffers[lodCounter].meshFaceContainers[meshCounter].push_back(face);
			}
		}
	}
}