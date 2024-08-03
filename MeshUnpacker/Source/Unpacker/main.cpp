#include "unpacker.h"

int main() {
	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh");
	//auto m = mesh.getMesh();

	std::ifstream skel("F:\\Sow\\unpacked-game\\characters\\modelweapons\\balrog\\balrog_whip\\balrog_whip.skel");
	MESH_UNPACKER::INTERNAL::SKEL::Header header;
	MESH_UNPACKER::INTERNAL::SKEL::BoneSection boneSection;

	header.populate(skel);
	boneSection.populate(skel, header);

}