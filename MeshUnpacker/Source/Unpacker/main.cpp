#include "Internal/includes-types-defs.h"


extern const aiScene* mesh_to_assimp(const MESH_UNPACKER::Mesh& mesh);

int main() {
	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh");
	MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.mesh");

	auto m = mesh.getMesh();
	
	mesh_to_assimp(*m);

}
