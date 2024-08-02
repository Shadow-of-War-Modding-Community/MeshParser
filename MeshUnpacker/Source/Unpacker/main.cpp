#include "unpacker.h"

int main() {
	std::shared_ptr<MESH_UNPACKER::INTERNAL::MeshDescSection> meshDescSection;
	std::shared_ptr<MESH_UNPACKER::INTERNAL::MeshInfoSection> meshInfoSection;
	std::shared_ptr<MESH_UNPACKER::INTERNAL::MeshDataSection> meshDataSection;
	

	{
		MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh");
		
	}
}