#include "skel.h"

void MESH_UNPACKER::INTERNAL::SKEL::Header::UnknownMeshData::populate(std::ifstream& skel) {
	read_ulong(unknownMeshDataBlockCount, skel);

	unknownMeshDataBlock = new UnknownMeshDataBlock[unknownMeshDataBlockCount];
	skel.read((char*)unknownMeshDataBlock, sizeof(UnknownMeshDataBlock) * unknownMeshDataBlockCount);
}

MESH_UNPACKER::INTERNAL::SKEL::Header::UnknownMeshData::~UnknownMeshData() {
	delete[] unknownMeshDataBlock;
}

MESH_UNPACKER::INTERNAL::SKEL::Header::Header(std::ifstream& skel) {
	populate(skel);
}

void MESH_UNPACKER::INTERNAL::SKEL::Header::populate(std::ifstream& skel){
	skel.read((char*)this, 96);

	unknownMeshData = new UnknownMeshData[meshCount];
	for (int i = 0; i < meshCount; ++i)
		unknownMeshData[i].populate(skel);
}

MESH_UNPACKER::INTERNAL::SKEL::Header::~Header() {
	delete[] unknownMeshData;
}

void MESH_UNPACKER::INTERNAL::SKEL::BoneSection::populate(std::ifstream& skel, Header& header) {

	auto getNextString = [&]() -> std::string {
		std::string readString;
		for (;;) {
			char tmp;
			skel.read(&tmp, 1);
			if (tmp == '\0') {
				readString += tmp;
				return readString;
			}
			readString += tmp;
		}
	};

	auto backup = skel.tellg();

	read_ulong(sectionID, skel);

	expect(sectionID, 0xDCDCFF00);

	char useless;
	skel.read(&useless, 1);

	for (int boneCounter = 0; boneCounter < header.boneCount1; ++boneCounter)
		boneNames.push_back(getNextString());

	skel.seekg((ulong)backup + header.boneNameSecSize + 4, std::ios_base::beg); // Seek here is absolutely not the typical way to go...

	bones = new TYPES::Bone[header.boneCount1];

	for (int boneCounter = 0; boneCounter < header.boneCount1; ++boneCounter)
		bones[boneCounter].populate(skel, boneNames[boneCounter]);
}
