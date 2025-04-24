#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include <optional>
#include <memory>
#include "half.h"

// Remove this includes if you only want to test the unpacker without assimp dependencies!
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace MESH_UNPACKER {
#define read_ulong(dst, hndl) hndl.read((char*)&dst, sizeof(ulong))
#define error(msg) MessageBoxA(NULL, msg, "MeshLoader-ERROR", MB_ICONERROR);\
exit(-1)

	namespace INTERNAL {
		using ulong = unsigned int;
		using ushort = unsigned short;
		using byte = unsigned char;
		using half_float::half;

		template<typename T>
			requires std::is_integral_v<T> || std::is_floating_point_v<T>
		inline void expect(T var, T val) {
			if (var != val) {
				error(("Variable contains unexpected value.\nExpected: " + std::to_string(val) + "\nFound: " + std::to_string(var)).c_str());
			}
		}

		namespace GLOBALTYPES {
			struct Vector3f {
				float x, y, z;
			};

			struct Quaternion {
				float w, x, y, z;
			};
		}

		namespace MESH {
			namespace TYPES {							
				using Normal = GLOBALTYPES::Vector3f;
				using Tangent = GLOBALTYPES::Vector3f;
				using Bitangent = GLOBALTYPES::Vector3f;

				struct Bone {
					ulong boneID;
					short parentIndex;
					ushort childCount;
					GLOBALTYPES::Vector3f translation;
					GLOBALTYPES::Quaternion rotation;
					float scale;
				};

				struct Position16 {
					half x, y, z, w;
				};

				struct Position32 {
					float x, y, z, w;
				};

				union Weights{
					byte weights[4];
					struct {
						byte weight1, weight2, weight3, weight4;
					};
				};

				union VertexGroups{
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
		}

		namespace SKEL {
			namespace TYPES {
#pragma pack(push, 1)
				struct Bone {
					ulong boneNameOffset;
					bool boneActive;
					GLOBALTYPES::Vector3f translation;
					GLOBALTYPES::Quaternion rotation;
					ulong childCount;
					std::string boneName;

					// The boneName has to be populated separately!!!
					void populate(std::ifstream& skel) {
						skel.read((char*)this, 37); 
					}
				};
#pragma pack(pop)
			}
		}
	}
}


#include "Skel/skel.h"
#include "Mesh/mesh.h"
