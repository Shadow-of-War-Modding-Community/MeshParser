#include "Internal/includes-types-defs.h"

//Likely exported later
extern std::unique_ptr<aiScene> mesh_to_assimp(const MESH_UNPACKER::Mesh& mesh);

int main() {
	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh");
	MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.mesh");

	auto m = mesh.getMesh();
	
	auto scene = mesh_to_assimp(*m);

	Assimp::Exporter exporter;
	if (exporter.Export(scene.get(), "fbx", "F:\\out.fbx") != AI_SUCCESS) {
		std::cout << exporter.GetErrorString();
		//error(exporter.GetErrorString());
	}
}
