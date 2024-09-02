#pragma once
#include "../includes-types-defs.h"

namespace MESH_UNPACKER::INTERNAL::SKEL {

	struct Header {
#pragma pack(push, 1) // Tells the compiler to align the members in memory without any padding. Else the struct will be read wrong!
		struct UnknownMeshDataBlock {
			bool unknownBool1;
			bool unknownBool2;
			float lodThreshold;
			ulong unknownUInt;
			long unknownInt;
		};
#pragma pack(pop)

		struct UnknownMeshData {
			ulong unknownMeshDataBlockCount;
			UnknownMeshDataBlock* unknownMeshDataBlock = nullptr;

			void populate(std::ifstream&);

			~UnknownMeshData();
		};

		union {
			char magic_c[4]; // SKEL
			ulong magic_u;
		};
		ulong version;

		char unk1[4]; // ID?

		ulong boneNameSecSize;

		ulong unk2;
		ulong boneCount1;

		char unk3[4]; // ID?
		long unk4; // Animation count?

		ulong meshCount; // Unique Mesh count?
		ulong unk6; // FX count?
		ulong boneCount2;

		long unk7;
		long unk8;
		long unk9;
		long unk10;
		long unk11; // ID?

		ulong unk12;

		float unk13; // SectionID?
		float unk14[3]; // Transforms?
		float unk15[3]; // Transforms?

		UnknownMeshData* unknownMeshData = nullptr;

		Header() = default;

		Header(std::ifstream&);

		void populate(std::ifstream&);

		~Header();
	};

	struct BoneSection {
		ulong sectionID; // 0xDCDCFF00

		std::vector<std::string> boneNames;

		TYPES::Bone* bones = nullptr;

		void populate(std::ifstream&, Header&);
	};

	struct Skeleton {
		Header header{};
		BoneSection boneSection{};
	};
}
	