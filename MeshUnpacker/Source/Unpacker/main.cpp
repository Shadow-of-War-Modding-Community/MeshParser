#include "Internal/includes-types-defs.h"

//Likely exported later
extern std::shared_ptr<aiScene> mesh_to_assimp(std::shared_ptr<MESH_UNPACKER::Mesh> mesh);

extern std::shared_ptr<MESH_UNPACKER::Mesh> assimp_to_mesh_ref(std::shared_ptr<aiScene> scene, std::shared_ptr<MESH_UNPACKER::Mesh> mesh);

int main() {
	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh");
	MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.mesh", "F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.skel");
	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\player\\base\\celebrimbor_base.mesh");
	//auto m = mesh.getMesh();
	
	//auto scene = mesh_to_assimp(m);

	//auto rM = assimp_to_mesh(scene);

	//Assimp::Exporter exporter;
	//if (exporter.Export(scene.get(), "fbx", "F:\\out.fbx") != AI_SUCCESS) {
	//	std::cout << exporter.GetErrorString();
	//	//error(exporter.GetErrorString());
	//}
}
