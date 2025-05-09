#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include <optional>
#include <memory>
#include "half.h"

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace PARSER {
#define read_ulong(dst, hndl) hndl.read((char*)&dst, sizeof(ulong))
#define error(msg) MessageBoxA(NULL, msg, "PARSING-ERROR", MB_ICONERROR);\
exit(-1)

	template<typename T>
		requires std::is_integral_v<T> || std::is_floating_point_v<T>
	inline void expect(T var, T val) {
		if (var != val) {
			error(("Variable contains unexpected value.\nExpected: " + std::to_string(val) + "\nFound: " + std::to_string(var)).c_str());
		}
	}

	namespace TYPES {
		using ulong = unsigned int;
		using ushort = unsigned short;
		using byte = unsigned char;
		using half_float::half;

		struct Vector3f {
			float x, y, z;
		};

		struct Quaternion {
			float w, x, y, z;
		};

		namespace MESH {
			using Normal = Vector3f;
			using Tangent = Vector3f;
			using Bitangent = Vector3f;

			struct Bone {
				ulong boneID;
				short parentIndex;
				ushort childCount;
				Vector3f translation;
				Quaternion rotation;
				float scale;
			};

			struct Position16 {
				half x, y, z, w;
			};

			struct Position32 {
				float x, y, z, w;
			};

			union Weights {
				byte weights[4];
				struct {
					byte weight1, weight2, weight3, weight4;
				};
			};

			union VertexGroups {
				byte vertexGroupIndices[4];
				struct {
					byte vertexGroupIndex1, vertexGroupIndex2, vertexGroupIndex3, vertexGroupIndex4;
				};
			};

			struct UV {
				float u, v;
			};

			struct Color {
				byte r, g, b, a;
			};

			struct VertexAttribute {
				Position32 position;
				Weights weights;
				VertexGroups vertexGroups;
				std::vector<std::pair<UV, bool>> uvs; // true = short, false = float
				Normal normal;
				Tangent tangent;
				Bitangent bitangent;
				Color color;
			};

			struct Face {
				ushort index1, index2, index3;
			};
		}
		

		namespace SKEL {
#pragma pack(push, 1)
				struct Bone {
					ulong boneNameOffset;
					bool boneActive;
					Vector3f translation;
					Quaternion rotation;
					ulong childCount;
					std::string boneName;
				};
#pragma pack(pop)
		}
	}
}

#include "Mesh/mesh.h"