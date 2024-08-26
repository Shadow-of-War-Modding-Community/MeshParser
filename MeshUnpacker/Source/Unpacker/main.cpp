#include "Internal/includes-types-defs.h"



int main() {
	//MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh");
	MESH_UNPACKER::MeshLoader mesh("F:\\Sow\\unpacked-game\\characters\\modelcharacters\\balrog\\balrog_base.mesh");

	auto m = mesh.getMesh();


	//std::ofstream file("F:\\out1.obj");
	//for (int j = 0; j < m->meshBuffersV16.size(); ++j) {
	//	for (int i = 0; i < m->meshBuffersV16[j].subMeshesVertexContainer[0].size(); ++i) {
	//		auto v = m->meshBuffersV16[j].subMeshesVertexContainer[0][i].positions;
	//		file << "v " << v.x << " " << v.y << " " << v.z << "\n";
	//	}
	//	for (int i = 0; i < m->meshBuffersV16[j].subMeshesFaceContainer[0].size(); ++i) {
	//		auto f = m->meshBuffersV16[j].subMeshesFaceContainer[0][i];
	//		file << "f " << f.index1 + 1 << "/ " << f.index2 + 1 << "/ " << f.index3 + 1 << "\n";
	//	}
	//}

	//std::ifstream skel("F:\\Sow\\unpacked-game\\characters\\modelweapons\\balrog\\balrog_whip\\balrog_whip.skel");
	//MESH_UNPACKER::INTERNAL::SKEL::Header header;
	//MESH_UNPACKER::INTERNAL::SKEL::BoneSection boneSection;
	//
	//header.populate(skel);
	//boneSection.populate(skel, header);

}
