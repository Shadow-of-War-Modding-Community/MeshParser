#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <utility>

#define log(msg, val)\
std::cout << msg << ": " << val << "  Offset after read: " << sub_file_offset << "\n";


FILE* reader;
template<typename T>
T read_file() {
	T tmp{};
	fread(&tmp, sizeof(T), 1, reader);
	return tmp;
}
#define read(type) read_file<type>()

DWORD sub_file_offset = 0;
template<typename T>
T read_sub_file(byte* sub_file) {
	T tmp{};
	tmp = *(T*)(sub_file + sub_file_offset);
	sub_file_offset += sizeof(T);
	return tmp;
}
#define read_sub(type) read_sub_file<type>(sub_file)

byte* read_sub_file_bytes(byte* sub_file, int size) {
	byte* tmp = new byte[size];
	memcpy(tmp, sub_file + sub_file_offset, size);
	sub_file_offset += size;
	return tmp;
}

void processMeshInfoFile(byte* sub_file, int size) {
	
	std::cout << "================================================================\n";

	auto sub_magic = read_sub(DWORD); // Constant. Has to be always the same
	if (sub_magic != 437600700)
		return;

	log("subMagic", sub_magic);

	auto unknownCount = read_sub(DWORD);
	log("unknownCount", unknownCount);
	auto unknownCount1 = read_sub(DWORD);
	log("unknownCount1", unknownCount1);
	auto dataBufferCount = read_sub(DWORD);
	log("dataBufferCount", dataBufferCount);
	auto subMeshCount2 = read_sub(DWORD);
	log("subMeshCount", subMeshCount2);

	auto u4 = read_sub(DWORD);
	log("unknown", u4);

	auto unknownDWORDCount1 = read_sub(DWORD);
	log("unknownDWORDCount1", unknownDWORDCount1);
	auto unknownDWORDCount2Part1 = read_sub(DWORD);
	log("unknownDWORDCount2Part1", unknownDWORDCount2Part1);
	auto unknownDWORDCount2Part2 = read_sub(DWORD);
	log("unknownDWORDCount2Part2", unknownDWORDCount2Part2);

	DWORD unknownDWORDCount2 = unknownDWORDCount2Part1 + unknownDWORDCount2Part2;

	auto unknownCount4 = read_sub(DWORD);
	log("unknownCount4", unknownCount4);

	auto bytesReadCount = read_sub(DWORD);
	log("bytesReadCount", bytesReadCount);

	auto u10 = read_sub(DWORD);  // Very likely not mandatory
	log("u10", u10);

	// Junk. Never used
	{
		read_sub(DWORD);
		read_sub(DWORD);
		read_sub(DWORD);
		read_sub(DWORD);
	}

	auto u12 = read_sub(DWORD);  // Very likely not mandatory
	log("u12", u12);
	auto unknownCount3 = read_sub(DWORD);
	log("unknownCount3", unknownCount3);

	std::vector<DWORD> unknownValues;
	DWORD counter = 0;
	
	std::cout << "Will be repeated for unknownCount\n";
	do {
		unknownValues.push_back(read_sub(DWORD));
	} while (++counter < unknownCount);

	for (auto& it : unknownValues)
		log("val", it);

	struct UnknownInternalStruct1 {
		DWORD d1;
		DWORD d2;
		DWORD d3;
		DWORD d4;
		DWORD d5;
	};

	std::cout << "Will be repeated for unknownCount1\n";
	std::vector<UnknownInternalStruct1> unknownInternalStruct1s;
	for (int i = 0; i < unknownCount1; ++i) {

		UnknownInternalStruct1 tmp;
		tmp.d1 = read_sub(DWORD);
		tmp.d2 = read_sub(DWORD);
		tmp.d3 = read_sub(DWORD);
		tmp.d4 = read_sub(DWORD);
		tmp.d5 = 0;
		unknownInternalStruct1s.push_back(tmp);
	}

	for (auto& it : unknownInternalStruct1s) {
		log("struct1", it.d1);
		log("struct2", it.d2);
		log("struct3", it.d3);
		log("struct4", it.d4);
		log("struct5 (not read, always 0)", it.d5);
		std::cout << "\n";
	}

	std::cout << "Will be repeated for dataBufferCount\n";
	counter = 0;
	do {
		
		// Junk. Never used
		{
			read_sub(DWORD);
			read_sub(DWORD);
			read_sub(DWORD);
		}

		auto subMeshCountInBuffer = read_sub(DWORD);
		log("subMeshCountInBuffer", subMeshCountInBuffer);

		delete[] read_sub_file_bytes(sub_file, 32);
		log("buffer", "has size of 32 bytes");

	} while (++counter < dataBufferCount);

	std::cout << "Will be repeated for subMeshCount (very likely a subMeshDescription struct)\n";
	counter = 0;
	do {
		auto v1 = read_sub(DWORD);
		log("v1", v1);
		auto v2 = read_sub(DWORD);
		log("v2", v2);
		auto vertCount = read_sub(DWORD); // Vertices Count
		log("vertCount", vertCount);
		auto v3 = read_sub(DWORD);
		log("v3", v3);
		auto faceCount = read_sub(DWORD); // Face Count
		log("faceCount", faceCount);
		auto v4 = read_sub(DWORD);
		log("v4", v4);
		auto boneIDCount = read_sub(DWORD);
		log("boneIDCount", boneIDCount);
		auto v6 = read_sub(DWORD);
		log("v6", v6);
		auto v7 = read_sub(DWORD);
		log("v7", v7);
		auto v8 = read_sub(DWORD);
		log("v8", v8);
		std::cout << "\n";
	} while (++counter < subMeshCount2);


	auto check = read_sub(DWORD); // Constant. Has to be always the same. If not the mesh wont render
	log("check", check);

	if (check == 2386429582) {
		delete[] read_sub_file_bytes(sub_file, 4 * unknownDWORDCount1);
		log("buffer size", 4 * unknownDWORDCount1)
		delete[] read_sub_file_bytes(sub_file, 4 * unknownDWORDCount2);
		log("buffer size", 4 * unknownDWORDCount2)
		delete[] read_sub_file_bytes(sub_file, 4 * unknownDWORDCount2);
		log("buffer size", 4 * unknownDWORDCount2)

		auto check2 = read_sub(DWORD);

		log("check2", check2);

		if (check2 == 936855974) {
			counter = 0;

			// All the byte reads in those nested loops will sum up to the bytesReadCount
			std::cout << "Will be repeated for unknownCount4\n";
			do {
				DWORD counter2 = 0;
				std::cout << "Will be repeated for innerCount\n";
				auto innerCount = read_sub(DWORD);
				log("innerCount", innerCount);
				do {
					auto value = read_sub(DWORD);
					log("value", value);

				} while (++counter2 < innerCount);
			} while (++counter < unknownCount4);
		}

		auto check3 = read_sub(DWORD);
		log("check3", check3);

		if (check3 == 2480514084) {
			delete[] read_sub_file_bytes(sub_file, unknownCount4);
			log("buffer size", unknownCount4);
		}
		// Do some other stuff with this... No more actual reads follow at this point
		// The other bytes left are completely useless and not needed
	}

}

int read_mesh() {
	FILE* f = fopen("C:\\Users\\gerbd\\Desktop\\head.mesh", "rb+");
	reader = f;
	auto magic = read(DWORD); // Constant. Has to be always the same
	if (magic != 1213418829)
		return 0;
	auto version = read(DWORD); // Constant. Has to be always the same
	if (version == 17) {
		auto u1 = read(DWORD);
		auto MeshInfoFileSize = read(DWORD);
		auto u2 = read(DWORD);

		auto version_check = read(DWORD); // Constant. Has to be always the same
		if (version_check != 3954099194)
			return 0;

		auto unknownCount = read(DWORD);
		auto dataBufferCount = read(DWORD);

		// Those are unknown...
		{
			DWORD* buffer1 = new DWORD[unknownCount];
			fread(buffer1, unknownCount * sizeof(DWORD), 1, f);

			DWORD* buffer2 = new DWORD[unknownCount];
			fread(buffer2, unknownCount * sizeof(DWORD), 1, f);
		}

		// Those very likely contain the UVs too
		DWORD* subMeshVertSecSizes = new DWORD[dataBufferCount];
		fread(subMeshVertSecSizes, dataBufferCount * sizeof(DWORD), 1, f);

		DWORD* subMeshFaceSecSizes = new DWORD[dataBufferCount];
		fread(subMeshFaceSecSizes, dataBufferCount * sizeof(DWORD), 1, f);

		DWORD* subMeshBnIDSecSizes = new DWORD[dataBufferCount];
		fread(subMeshBnIDSecSizes, dataBufferCount * sizeof(DWORD), 1, f);


		// This buffer is internally treated as a complete separate information file. Hence the name...
		byte* MeshInfoFile = new byte[MeshInfoFileSize]; // Allocated by byte but its actually mostly a DWORD array
		fread(MeshInfoFile, MeshInfoFileSize, 1, f);


		auto check = read(DWORD); // Constant. Has to be always the same
		if (check != 2514213737)
			return 0;

		std::vector<std::pair<byte*, int>> vertices;
		DWORD counter = 0;
		do {
			std::cout << "Vertex section start for databuffer " << std::dec << counter << ": " << ftell(f) << "\n";
			byte* buf = new byte[subMeshVertSecSizes[counter]];
			fread(buf, subMeshVertSecSizes[counter], 1, f);
			vertices.push_back({ buf, subMeshVertSecSizes[counter] });
		} while (++counter < dataBufferCount);

		//for (auto& iter : vertices) {
		//	for (int i = 0; i < iter.second; ++i) {
		//		std::cout << std::hex << (int)iter.first[i] << " ";
		//	}
		//	std::cout << "\n\n\n";
		//}

		std::vector<std::pair<byte*, int>> faces;
		counter = 0;
		do {
			std::cout << "Face section start for databuffer " << std::dec << counter << ": " << ftell(f) << "\n";
			byte* buf = new byte[subMeshFaceSecSizes[counter]];
			fread(buf, subMeshFaceSecSizes[counter], 1, f);
			faces.push_back({ buf, subMeshFaceSecSizes[counter] });
		} while (++counter < dataBufferCount);


		//for (auto& iter : faces) {
		//	for (int i = 0; i < iter.second; ++i) {
		//		std::cout << std::hex << (int)iter.first[i] << " ";
		//	}
		//	std::cout << "\n";
		//}

		std::vector<std::pair<byte*, int>> boneIDs;
		counter = 0;
		do {
			std::cout << "BoneID section start for databuffer " << std::dec << counter << ": " << ftell(f) << "\n";
			byte* buf = new byte[subMeshBnIDSecSizes[counter]];
			fread(buf, subMeshBnIDSecSizes[counter], 1, f);
			boneIDs.push_back({ buf, subMeshBnIDSecSizes[counter] });
		} while (++counter < dataBufferCount);

		//for (auto& iter : boneIDs) {
		//	for (int i = 0; i < iter.second; ++i) {
		//		std::cout << std::hex << (int)iter.first[i] << " ";
		//	}
		//	std::cout << "\n";
		//}

		processMeshInfoFile(MeshInfoFile, MeshInfoFileSize);
	}
}

int read_skel() {
	FILE* f = fopen("skel", "rb+");
	reader = f;

	auto magic = read(DWORD);
	auto version = read(DWORD);
	if (magic != 1279609683)
		return 0;
	if (version <= 17) {
		auto u1 = read(DWORD);
		auto unknownBufferSize = read(DWORD);
		auto u3 = read(DWORD);
		auto u4 = read(DWORD);
		auto u5 = read(DWORD);
		auto u6 = read(DWORD);
		auto unknownCount = read(DWORD);
		auto u8 = read(DWORD);
		auto u9 = read(DWORD);
		auto u10 = read(DWORD);
		auto u11 = read(DWORD);
		auto u12 = read(DWORD);
		auto u13 = read(DWORD);
		auto u14 = read(DWORD);
		auto u15 = read(DWORD);
		
		auto u16 = read(DWORD);

		auto u17 = read(DWORD);
		auto u18 = read(DWORD);
		auto u19 = read(DWORD);

		auto u20 = read(DWORD);
		auto u21 = read(DWORD);
		auto u22 = read(DWORD);

		for (int i = 0; i < unknownCount; ++i) {
			auto val1 = read(DWORD);
			for (int j = 0; j < val1; ++j) {
				read(BYTE);
				read(BYTE);
				read(DWORD);
				read(DWORD);
				read(DWORD);
			}
		}


		auto check = read(DWORD);

		if (check != 3705470720)
			return 0;

		byte* buffer1 = new byte[unknownBufferSize];
		fread(buffer1, unknownBufferSize, 1, f);

		for (int i = 0;; i += 24) {
			auto val1 = read(DWORD);

		}

	}

}

int main() {
	read_skel();
	read_mesh();
}