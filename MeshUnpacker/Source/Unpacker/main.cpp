#include "Internal/includes-types-defs.h"

//Likely exported later
extern std::shared_ptr<aiScene> mesh_to_assimp(std::shared_ptr<MESH_UNPACKER::Mesh> mesh);

extern std::shared_ptr<MESH_UNPACKER::Mesh> assimp_to_mesh_ref(std::shared_ptr<aiScene> scene, std::shared_ptr<MESH_UNPACKER::Mesh> mesh);

extern void export_mesh(std::shared_ptr<MESH_UNPACKER::Mesh> mesh, const char* name);

extern void export_assimp(std::shared_ptr<aiScene> scene, const char* format, const char* name);

//int main() {
//	MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\sauron\\1h_blunt\\sauron_flangedmace\\sauron_flangedmace.mesh", "F:\\Sow\\unpacked-game\\characters\\modelweapons\\sauron\\1h_blunt\\sauron_flangedmace\\sauron_flangedmace.skel");
//	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.mesh", "F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.skel");
//	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\player\\base\\celebrimbor_base.mesh");
//	auto m = mesh.getMesh();
//	
//	auto scene = mesh_to_assimp(m);
//
//	//auto rM = assimp_to_mesh(scene);
//
//	Assimp::Exporter exporter;
//	
//	if (exporter.Export(scene.get(), "fbx", "F:\\out.fbx") != AI_SUCCESS) {
//		std::cout << exporter.GetErrorString();
//		//error(exporter.GetErrorString());
//	}
//}

int main() {
	MESH_UNPACKER::MeshLoader mesh("F:\\balrog_base.mesh");
	auto m = mesh.getMesh();


	auto scene = mesh_to_assimp(m);

	assimp_to_mesh_ref(scene, m);
	//auto scene = mesh_to_assimp(m);

	//assimp_to_mesh_ref(scene, m);
}

//int main() {
//
//	MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.mesh", "F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.skel");
//	auto m = mesh.getMesh();
//	Assimp::Importer importer;
//	auto scene = importer.ReadFile("F:\\balrog_base.fbx", 0);
//	
//	std::vector<const aiNode*> nodes;
//	for (int i = 0; i < m->skeleton->header.boneCount1; ++i) {
//		nodes.push_back(scene->mRootNode->FindNode(m->skeleton->boneSection.bones[i].boneName.c_str()));
//	}
//
//}