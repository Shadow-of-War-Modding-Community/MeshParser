#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <conio.h>
#include <direct.h>
#include <filesystem>
#include <regex>
#include "half.h"

#define l(x)\
std::cout << x << "\n"

namespace fs = std::filesystem;

using float16 = half_float::half;

struct MeshInfoStruct {
    int VertexCount;
    int VertexStart;
    int FaceCount;
    int FaceStart;
    int VertexId;
    int UsedBones;
    int UsedBonesStart;
    int VertexEnd;
};

struct Vertex {
    float x, y, z;
};

struct Face {
    unsigned short fa, fb, fc;
};

struct WeightData {
    std::vector<int> boneids;
    std::vector<float> weights;
};

int readlong(FILE* f) {
    int value;
    fread((void*)&value, sizeof(value), 1, f);
    return value;
}

unsigned int readlong(FILE* f, bool) {
    unsigned int value;
    fread((void*)&value, sizeof(value), 1, f);
    return value;
}

short readshort(FILE* f) {
    short value;
    fread((void*)&value, sizeof(value), 1, f);
    return value;
}

unsigned short readshort(FILE* f, bool) {
    unsigned short value;
    fread((void*)&value, sizeof(value), 1, f);
    return value;
}

void writeshort(FILE* f, unsigned short value, bool) {
    fwrite((void*)&value, sizeof(value), 1, f);
}

unsigned char readbyte(FILE* f) {
    unsigned char value;
    fread((void*)&value, sizeof(value), 1, f);
    return value;
}

float readfloat(FILE* f) {
    float value;
    fread((void*)&value, sizeof(value), 1, f);
    return value;
}

void writefloat(FILE* f, float value) {
    fwrite((void*)&value, sizeof(value), 1, f);
}

float16 readhalffloat(FILE* f) {
    float16 value;
    fread((void*)&value, 2, 1, f);
    return value;
}

void writehalffloat(FILE* f, float16 value) {
    fwrite((void*)&value, 2, 1, f);
}

std::string outputPath(const std::string& s)
{
    auto pos = s.find_last_of('\\');
    if (pos != std::string::npos)
        return s.substr(0, pos);
    else
        return s;
}

std::string outputFile(const std::string& s)
{
    auto pos = s.find_last_of('\\');
    if (pos != std::string::npos)
        return s.substr(pos, s.size() - 1);
    else
        return s;
}

std::string remSuffix(const std::string& s)
{
    auto pos = s.find_last_of('.');
    if (pos != std::string::npos)
        return s.substr(0, pos);
    else
        return s;
}

enum class mode {
    obj,
    mesh
};

int main(int argc, char** argv) {
    mode mode;
    std::ofstream info;

    auto compareFileNames = [](const fs::path& a, const fs::path& b) {
        auto aa = a.string();
        auto bb = b.string();
        std::regex pattern(R"(\d+)");

        std::smatch matchA, matchB;
        std::regex_search(aa, matchA, pattern);
        std::regex_search(bb, matchB, pattern);

        int numA = matchA.empty() ? 0 : std::stoi(matchA[0]);
        int numB = matchB.empty() ? 0 : std::stoi(matchB[0]);

        return numA < numB;
        };

    auto terminate = []() {
        std::cout << "------> [SOWMO]-Terminating in 7 seconds...\n";
        Sleep(7000);
        exit(-1);
        };

    std::cout << "---------------------survivalizeed's Shadow Of War .mesh <--> .obj converter---------------------\n";
    std::cout << "[SOWMO]-Credits to zaramot for the actual file reversal and to the awesome float16 library written by Christian Rau!\n\n";
    if (argc < 3) {
        std::cout << "[SOWMO]-First argument: .mesh file or folder containing .obj files\n";
        std::cout << "[SOWMO]-Second argument: -m or -o (.mesh or .obj)\n";
        std::cout << "[SOWMO]-If the second argument is -o:\n";
        std::cout << "[SOWMO]------> Third argument: Original .mesh file to get merged with .obj files passed as first argument\n";
        terminate();
    }
    if (argc > 4) {
        std::cout << "[SOWMO]-Invalid arguments!\n";
        terminate();
    }
    if (std::string(argv[2]) == "-m") {
        mode = mode::mesh;
    }
    else if (std::string(argv[2]) == "-o") {
        mode = mode::obj;
    }
    else {
        std::cout << "[SOWMO]-Invalid arguments!\n";
        terminate();
    }
    const char* file = nullptr;
    if (mode == mode::mesh) {
        std::cout << "[SOWMO]-Used .mesh file to convert to .obj files: " << argv[1] << "\n";
        file = argv[1];
    }
    if (mode == mode::obj) {
        std::cout << "[SOWMO]-Used folder to convert .obj files to .mesh: " << argv[1] << "\n";
        std::cout << "[SOWMO]-Used file to receive .obj files: " << argv[3] << "\n";
        file = argv[3];
    }

    int objVertexIndex = 0;
    int objFaceIndex = 0;
    std::vector<std::vector<Vertex>> objVerts;
    std::vector<std::vector<Face>> objFaces;

    if (mode == mode::obj) {
        int index = 0;
        std::vector<fs::path> objFiles;
        std::copy(fs::directory_iterator(argv[1]), fs::directory_iterator(), std::back_inserter(objFiles));
        std::sort(objFiles.begin(), objFiles.end(), compareFileNames);
        for (const auto& entry : objFiles) {
            objVerts.push_back({});
            objFaces.push_back({});
            std::string buf;
            std::string word;
            std::stringstream ss;
            std::ifstream file(entry);
            ss << file.rdbuf();
            buf = ss.str();
            ss.str("");
            std::replace(buf.begin(), buf.end(), '\n', ' ');
            ss << buf;
            std::vector<std::string> words;
            while (std::getline(ss, word, ' ')) {
                if (word == "" || word == " ") continue;
                words.push_back(word);
            }
            for (int i = 0; i < words.size(); ++i) {
                if (words[i] == "v")
                    objVerts[index].push_back({ stof(words[i + 1]), stof(words[i + 2]), stof(words[i + 3]) });
                if (words[i] == "f")
                    objFaces[index].push_back({ (unsigned short)stoi(words[i + 1]), (unsigned short)stoi(words[i + 2]), (unsigned short)stoi(words[i + 3]) });
            }
            ++index;
        }
    }
    FILE* f = fopen(file, "rb+");
    fseek(f, 0x8, SEEK_SET);
    int FirstOff = readlong(f);
    int DataStride = readlong(f);

    fseek(f, FirstOff + 24, SEEK_SET);
    int MeshCount = readlong(f);
    int SkipSecCount = readlong(f);

    int BoneMapCount = readlong(f);
    int MeshCountTrue = readlong(f);

    fseek(f, 44, SEEK_CUR);
    int BoneCount = readlong(f);
    int BoneSecSize = readlong(f);


    for (int i = 0; i < MeshCount; i++) {
        readlong(f);
    }

    for (int i = 0; i < SkipSecCount; i++) {
        int getPos = ftell(f) + 16;
        fseek(f, getPos, SEEK_SET);
    }


    std::vector<int> VertSecSizeArray;
    std::vector<int> FaceSecSizeArray;
    std::vector<int> MapCountArray;
    std::vector<int> SubMeshCountArray;

    for (int i = 0; i < BoneMapCount; i++) {
        int getPos = ftell(f) + 48;
        int VertSecSize = readlong(f);
        int FaceSecSize = readlong(f);
        int MapCount = readlong(f);
        int SubMeshCount = readlong(f);
        fseek(f, getPos, SEEK_SET);

        VertSecSizeArray.push_back(VertSecSize);
        FaceSecSizeArray.push_back(FaceSecSize);
        MapCountArray.push_back(MapCount);
        SubMeshCountArray.push_back(SubMeshCount);
    }

    std::vector<MeshInfoStruct> Mesh_Info;
    long Pos = ftell(f);

    for (int i = 0; i < BoneMapCount; i++) {
        for (int j = 0; j < SubMeshCountArray[i]; j++) {
            long getPos = ftell(f) + 40;
            int UnkOff01 = readlong(f);
            int VertexStart = readlong(f);
            int VertexCount = readlong(f);
            int FaceStart = readlong(f);
            int FaceCount = readlong(f);
            int UsedBonesStart = readlong(f);
            int UsedBones = readlong(f);
            readlong(f);
            readlong(f);
            int VertexId = readlong(f);
            fseek(f, getPos, SEEK_SET);
            MeshInfoStruct meshInfo;
            meshInfo.VertexCount = VertexCount;
            meshInfo.VertexStart = VertexStart;
            meshInfo.FaceCount = FaceCount;
            meshInfo.FaceStart = FaceStart;
            meshInfo.VertexId = VertexId;
            meshInfo.UsedBones = UsedBones;
            meshInfo.UsedBonesStart = UsedBonesStart;
            meshInfo.VertexEnd = UnkOff01;
            Mesh_Info.push_back(meshInfo);
        }
    }

    fseek(f, Pos, SEEK_SET);

    fseek(f, (FirstOff + DataStride) + 24, SEEK_SET);

    std::vector<int> VertexStartArray;

    for (int i = 0; i < BoneMapCount; i++) {
        for (int x = 0; x < VertSecSizeArray[i]; x++) {
            long getPos = ftell(f) + 1;
            fseek(f, getPos, SEEK_SET);
        }
    }

    long FacePosition = ftell(f);

    std::vector<long> FacePosArray;

    for (int i = 0; i < MeshCountTrue; i++) {
        long FacePosStart = ftell(f);
        for (int x = 0; x < (Mesh_Info[i].FaceCount / 3); x++) {
            long getPos = ftell(f) + 6;
            fseek(f, getPos, SEEK_SET);
        }
        FacePosArray.push_back(FacePosStart);
    }

    std::vector<long> BoneIDPosArray;
    std::cout << "BoneIdposition " << ftell(f) << "\n";
    for (int i = 0; i < MeshCountTrue; i++) {
        long BoneIDPos = ftell(f);
        for (int x = 0; x < Mesh_Info[i].UsedBones; x++) {
            readbyte(f);
        }
        BoneIDPosArray.push_back(BoneIDPos);
    }

    fseek(f, (FirstOff + DataStride) + 24, SEEK_SET);

    std::cout << "[SOWMO]-Mesh count in .mesh file: " << MeshCountTrue << "\n";

    for (int i = 0; i < MeshCountTrue; i++) {
        int VertexSize = 0;
        long VertexStartPos = ftell(f);

        fseek(f, 0x6, SEEK_CUR);
        int CheckVert = readshort(f);
        if (CheckVert == 15360) {
            fseek(f, 0x1E, SEEK_CUR);
        }
        int CheckVert2 = readshort(f);
        if (CheckVert2 == 15360) {
            VertexSize = 16;
        }
        else {
            VertexSize = 24;
        }

        fseek(f, VertexStartPos, SEEK_SET);

        if (VertexSize == 16) {
            for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                long getPos = ftell(f) + 16;
                fseek(f, getPos, SEEK_SET);
            }
        }

        if (VertexSize == 24) {
            for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                long getPos = ftell(f) + 24;
                fseek(f, getPos, SEEK_SET);
            }
        }

        long PosVStart = ftell(f);

        long UvSize = 0;
        if (i != MeshCountTrue) {
            if (VertexSize == 16) {
                for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                    long getPos = ftell(f) + 44;
                    fseek(f, getPos, SEEK_SET);
                }
                long Pos0 = ftell(f);
                fseek(f, 0x6, SEEK_CUR);
                int CheckVert = readshort(f);
                if (CheckVert == 15360) {
                    fseek(f, 0x1E, SEEK_CUR);
                }
                int CheckVert2 = readshort(f);
                if (CheckVert2 == 15360) {
                    fseek(f, Pos0, SEEK_SET);
                }
                if (CheckVert2 != 15360) {
                    fseek(f, PosVStart, SEEK_SET);
                    for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                        long getPos = ftell(f) + 48;
                        fseek(f, getPos, SEEK_SET);
                    }
                    long Pos1 = ftell(f);
                    fseek(f, 0x6, SEEK_CUR);
                    int CheckVert = readshort(f);
                    if (CheckVert == 15360) {
                        fseek(f, 0x1E, SEEK_CUR);
                    }
                    int CheckVert2 = readshort(f);
                    if (CheckVert2 == 15360) {
                        fseek(f, Pos1, SEEK_SET);
                    }
                    if (CheckVert2 != 15360) {
                        fseek(f, PosVStart, SEEK_SET);
                        for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                            long getPos = ftell(f) + 52;
                            fseek(f, getPos, SEEK_SET);
                        }
                        long Pos2 = ftell(f);
                        fseek(f, 0x6, SEEK_CUR);
                        int CheckVert = readshort(f);
                        if (CheckVert == 15360) {
                            fseek(f, 0x1E, SEEK_CUR);
                        }
                        int CheckVert2 = readshort(f);
                        if (CheckVert2 == 15360) {
                            fseek(f, Pos2, SEEK_SET);
                        }
                        if (CheckVert2 != 15360) {
                            fseek(f, PosVStart, SEEK_SET);
                            for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                                long getPos = ftell(f) + 56;
                                fseek(f, getPos, SEEK_SET);
                            }
                            long Pos3 = ftell(f);
                            fseek(f, 0x6, SEEK_CUR);
                            int CheckVert = readshort(f);
                            if (CheckVert == 15360) {
                                fseek(f, 0x1E, SEEK_CUR);
                            }
                            int CheckVert2 = readshort(f);
                            if (CheckVert2 == 15360) {
                                fseek(f, Pos3, SEEK_SET);
                            }
                        }
                    }
                }
            }

            if (VertexSize == 24) {
                for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                    long getPos = ftell(f) + 44;
                    fseek(f, getPos, SEEK_SET);
                }
                long Pos0 = ftell(f);
                fseek(f, 0xC, SEEK_CUR);
                float CheckVert = readfloat(f);
                if (CheckVert == 1.0) {
                    fseek(f, 0x14, SEEK_CUR);
                }
                float CheckVert2 = readfloat(f);
                if (CheckVert2 == 1.0) {
                    fseek(f, Pos0, SEEK_SET);
                }
                if (CheckVert2 != 1.0) {
                    fseek(f, PosVStart, SEEK_SET);
                    for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                        long getPos = ftell(f) + 48;
                        fseek(f, getPos, SEEK_SET);
                    }
                    long Pos1 = ftell(f);
                    fseek(f, 0xC, SEEK_CUR);
                    float CheckVert = readfloat(f);
                    if (CheckVert == 1.0) {
                        fseek(f, 0x14, SEEK_CUR);
                    }
                    float CheckVert2 = readfloat(f);
                    if (CheckVert2 == 1.0) {
                        fseek(f, Pos1, SEEK_SET);
                    }
                    if (CheckVert2 != 1.0) {
                        fseek(f, PosVStart, SEEK_SET);
                        for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                            long getPos = ftell(f) + 52;
                            fseek(f, getPos, SEEK_SET);
                        }
                        long Pos2 = ftell(f);
                        fseek(f, 0xC, SEEK_CUR);
                        float CheckVert = readfloat(f);
                        if (CheckVert == 1.0) {
                            fseek(f, 0x14, SEEK_CUR);
                        }
                        float CheckVert2 = readfloat(f);
                        if (CheckVert2 == 1.0) {
                            fseek(f, Pos2, SEEK_SET);
                        }
                        if (CheckVert2 != 1.0) {
                            fseek(f, PosVStart, SEEK_SET);
                            for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                                long getPos = ftell(f) + 56;
                                fseek(f, getPos, SEEK_SET);
                            }
                            long Pos3 = ftell(f);
                            fseek(f, 0xC, SEEK_CUR);
                            float CheckVert = readfloat(f);
                            if (CheckVert == 1.0) {
                                fseek(f, 0x14, SEEK_CUR);
                            }
                            float CheckVert2 = readfloat(f);
                            if (CheckVert2 == 1.0) {
                                fseek(f, Pos3, SEEK_SET);
                            }
                        }
                    }
                }
            }
        }

        long PosVEnd = ftell(f);
        if (i != MeshCountTrue) {
            UvSize = (PosVEnd - PosVStart) / Mesh_Info[i].VertexCount;
        }
        else {
            UvSize = (FacePosition - PosVStart) / Mesh_Info[i].VertexCount;
        }



        fseek(f, PosVStart, SEEK_SET);

        for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
            long getPos = ftell(f) + UvSize;
            fseek(f, getPos, SEEK_SET);
        }

        VertexStartArray.push_back(VertexStartPos);
    }

    std::vector<Vertex> vertArray;
    std::vector<WeightData> Weight_array;
    std::vector<unsigned char> BoneIDArray;
    std::vector<Face> faceArray;

    for (int i = 0; i < MeshCountTrue; i++) {
        int BoneIdStart = BoneIDPosArray[i];

        fseek(f, BoneIdStart, SEEK_SET);


        for (int x = 0; x < Mesh_Info[i].UsedBones; x++) {
            unsigned char BoneId = readbyte(f) + 1;
            BoneIDArray.push_back(BoneId);
        }


        int VertexSize = 0;
        int VertexStart = VertexStartArray[i];

        fseek(f, VertexStart, SEEK_SET);

        fseek(f, 0x6, SEEK_CUR);
        short CheckVert = readshort(f);
        if (CheckVert == 15360)
            fseek(f, 0x1E, SEEK_CUR);
        short CheckVert2 = readshort(f);
        if (CheckVert2 == 15360)
            VertexSize = 16;
        else
            VertexSize = 24;

        fseek(f, VertexStart, SEEK_SET);


        int getPos, Null;
        if (VertexSize == 16) {
            std::cout << "[SOWMO]-Vertex count in mesh nr." << i << ": " << std::dec << Mesh_Info[i].VertexCount << "\n";
            std::cout << "Vertex Position: " << ftell(f) << "\n";
            std::cout << "Order: Float16, Float16, Float16, Short, Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte\n";
            for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                getPos = ftell(f) + 16;
                float16 vx = (float16)0;
                float16 vy = (float16)0;
                float16 vz = (float16)0;

                auto val = ftell(f);
                if (mode == mode::mesh) {
                    vx = readhalffloat(f);
                    vy = readhalffloat(f);
                    vz = readhalffloat(f);
                }
                if (mode == mode::obj) {
                    writehalffloat(f, -(float16)objVerts[i][objVertexIndex].x);
                    writehalffloat(f, (float16)objVerts[i][objVertexIndex].y);
                    writehalffloat(f, (float16)objVerts[i][objVertexIndex].z);
                    objVertexIndex++;
                }

                fseek(f, val, SEEK_SET);

                vx = readhalffloat(f);
                vy = readhalffloat(f);
                vz = readhalffloat(f);


                Null = readshort(f);

                unsigned char weight3 = readbyte(f);
                unsigned char weight2 = readbyte(f);
                unsigned char weight1 = readbyte(f);
                unsigned char weight4 = readbyte(f);

                unsigned char bone1 = readbyte(f);
                unsigned char bone2 = readbyte(f);
                unsigned char bone3 = readbyte(f);
                unsigned char bone4 = readbyte(f);

                WeightData w;
                float maxweight = 0;

                if (weight1 != 0)
                    maxweight = maxweight + weight1;
                if (weight2 != 0)
                    maxweight = maxweight + weight2;
                if (weight3 != 0)
                    maxweight = maxweight + weight3;
                if (weight4 != 0)
                    maxweight = maxweight + weight4;

                if (maxweight != 0) {
                    if (weight1 != 0) {
                        float w1 = static_cast<float>(weight1);
                        w.boneids.push_back(bone1 + 1);
                        w.weights.push_back(w1 / 255.0f);
                    }
                    if (weight2 != 0) {
                        float w2 = static_cast<float>(weight2);
                        w.boneids.push_back(bone2 + 1);
                        w.weights.push_back(w2 / 255.0f);
                    }
                    if (weight3 != 0) {
                        float w3 = static_cast<float>(weight3);
                        w.boneids.push_back(bone3 + 1);
                        w.weights.push_back(w3 / 255.0f);
                    }
                    if (weight4 != 0) {
                        float w4 = static_cast<float>(weight4);
                        w.boneids.push_back(bone4 + 1);
                        w.weights.push_back(w4 / 255.0f);
                    }
                }

                Weight_array.push_back(w);
                fseek(f, getPos, SEEK_SET);
                if (mode == mode::mesh)
                    vertArray.push_back(Vertex{ -vx, vy, vz });
            }
        }

        if (VertexSize == 24) {
            std::cout << "[SOWMO]-Vertex count in mesh nr." << i << ": " << std::dec << Mesh_Info[i].VertexCount << "\n";
            std::cout << "Vertex Position: " << ftell(f) << "\n";
            std::cout << "Order: Float, Float, Float, Long, Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte\n";
            for (int x = 0; x < Mesh_Info[i].VertexCount; x++) {
                getPos = ftell(f) + 24;

                // Read vertex position
                float vx = (float)0;
                float vy = (float)0;
                float vz = (float)0;

                auto val = ftell(f);

                if (mode == mode::mesh) {
                    vx = readfloat(f);
                    vy = readfloat(f);
                    vz = readfloat(f);
                }
                if (mode == mode::obj) {
                    writefloat(f, -(float)objVerts[i][objVertexIndex].x);
                    writefloat(f, (float)objVerts[i][objVertexIndex].y);
                    writefloat(f, (float)objVerts[i][objVertexIndex].z);
                    objVertexIndex++;
                }

                fseek(f, val, SEEK_SET);

                vx = readfloat(f);
                vy = readfloat(f);
                vz = readfloat(f);

                Null = readlong(f);

                // Read bone weights
                unsigned char weight3 = readbyte(f);
                unsigned char weight2 = readbyte(f);
                unsigned char weight1 = readbyte(f);
                unsigned char weight4 = readbyte(f);

                // Read bone indices
                unsigned char bone1 = readbyte(f);
                unsigned char bone2 = readbyte(f);
                unsigned char bone3 = readbyte(f);
                unsigned char bone4 = readbyte(f);

                WeightData w;
                float maxweight = 0;

                if (weight1 != 0)
                    maxweight = maxweight + weight1;
                if (weight2 != 0)
                    maxweight = maxweight + weight2;
                if (weight3 != 0)
                    maxweight = maxweight + weight3;
                if (weight4 != 0)
                    maxweight = maxweight + weight4;

                if (maxweight != 0) {
                    if (weight1 != 0) {
                        float w1 = static_cast<float>(weight1);
                        w.boneids.push_back(bone1 + 1);
                        w.weights.push_back(w1 / 255.0f);
                    }
                    if (weight2 != 0) {
                        float w2 = static_cast<float>(weight2);
                        w.boneids.push_back(bone2 + 1);
                        w.weights.push_back(w2 / 255.0f);
                    }
                    if (weight3 != 0) {
                        float w3 = static_cast<float>(weight3);
                        w.boneids.push_back(bone3 + 1);
                        w.weights.push_back(w3 / 255.0f);
                    }
                    if (weight4 != 0) {
                        float w4 = static_cast<float>(weight4);
                        w.boneids.push_back(bone4 + 1);
                        w.weights.push_back(w4 / 255.0f);
                    }
                }

                Weight_array.push_back(w);
                fseek(f, getPos, SEEK_SET);
                if (mode == mode::mesh)
                    vertArray.push_back(Vertex{ -vx, vy, vz });
            }
        }

        long PosVStart = ftell(f);

        if (i != MeshCountTrue) {
            if (VertexSize == 16) {
                for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                    long getPos = ftell(f) + 44;
                    fseek(f, getPos, SEEK_SET);
                }

                long Pos0 = ftell(f);
                fseek(f, 0x6, SEEK_CUR);
                short CheckVert = readshort(f);
                if (CheckVert == 15360) fseek(f, 0x1E, SEEK_CUR);

                short CheckVert2 = readshort(f);
                if (CheckVert2 == 15360) fseek(f, Pos0, SEEK_SET);

                if (CheckVert2 != 15360) {
                    fseek(f, PosVStart, SEEK_SET);
                    for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                        long getPos = ftell(f) + 48;
                        fseek(f, getPos, SEEK_SET);
                    }

                    long Pos1 = ftell(f);
                    fseek(f, 0x6, SEEK_CUR);
                    CheckVert = readshort(f);
                    if (CheckVert == 15360) fseek(f, 0x1E, SEEK_CUR);

                    CheckVert2 = readshort(f);
                    if (CheckVert2 == 15360) fseek(f, Pos1, SEEK_SET);

                    if (CheckVert2 != 15360) {
                        fseek(f, PosVStart, SEEK_SET);
                        for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                            long getPos = ftell(f) + 52;
                            fseek(f, getPos, SEEK_SET);
                        }

                        long Pos2 = ftell(f);
                        fseek(f, 0x6, SEEK_CUR);
                        CheckVert = readshort(f);
                        if (CheckVert == 15360) fseek(f, 0x1E, SEEK_CUR);

                        CheckVert2 = readshort(f);
                        if (CheckVert2 == 15360) fseek(f, Pos2, SEEK_SET);

                        if (CheckVert2 != 15360) {
                            fseek(f, PosVStart, SEEK_SET);
                            for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                                long getPos = ftell(f) + 56;
                                fseek(f, getPos, SEEK_SET);
                            }

                            long Pos3 = ftell(f);
                            fseek(f, 0x6, SEEK_CUR);
                            CheckVert = readshort(f);
                            if (CheckVert == 15360) fseek(f, 0x1E, SEEK_CUR);

                            CheckVert2 = readshort(f);
                            if (CheckVert2 == 15360) fseek(f, Pos3, SEEK_SET);
                        }
                    }
                }
            }
        }
        PosVStart = ftell(f);

        if (VertexSize == 24) {
            for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                long getPos = ftell(f) + 44;
                fseek(f, getPos, SEEK_SET);
            }

            long Pos0 = ftell(f);
            fseek(f, 0xC, SEEK_CUR);
            float CheckVert = readfloat(f);
            if (CheckVert == 1)
                fseek(f, 0x14, SEEK_CUR);

            float CheckVert2 = readfloat(f);
            if (CheckVert2 == 1)
                fseek(f, Pos0, SEEK_SET);
            if (CheckVert2 != 1) {
                fseek(f, PosVStart, SEEK_SET);
                for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                    long getPos = ftell(f) + 48;
                    fseek(f, getPos, SEEK_SET);
                }

                long Pos1 = ftell(f);
                fseek(f, 0xC, SEEK_CUR);
                CheckVert = readfloat(f);
                if (CheckVert == 1)
                    fseek(f, 0x14, SEEK_CUR);

                CheckVert2 = readfloat(f);
                if (CheckVert2 == 1)
                    fseek(f, Pos1, SEEK_SET);
                if (CheckVert2 != 1) {
                    fseek(f, PosVStart, SEEK_SET);
                    for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                        long getPos = ftell(f) + 52;
                        fseek(f, getPos, SEEK_SET);
                    }

                    long Pos2 = ftell(f);
                    fseek(f, 0xC, SEEK_CUR);
                    CheckVert = readfloat(f);
                    if (CheckVert == 1)
                        fseek(f, 0x14, SEEK_CUR);

                    CheckVert2 = readfloat(f);
                    if (CheckVert2 == readfloat(f))
                        fseek(f, Pos2, SEEK_SET);
                    if (CheckVert2 != 1) {
                        fseek(f, PosVStart, SEEK_SET);
                        for (int x = 1; x <= Mesh_Info[i].VertexCount; x++) {
                            long getPos = ftell(f) + 56;
                            fseek(f, getPos, SEEK_SET);
                        }

                        long Pos3 = ftell(f);
                        fseek(f, 0xC, SEEK_CUR);
                        CheckVert = readfloat(f);
                        if (CheckVert == 1)
                            fseek(f, 0x14, SEEK_CUR);

                        CheckVert2 = readfloat(f);
                        if (CheckVert2 == 1)
                            fseek(f, Pos3, SEEK_SET);
                    }
                }
            }
        }
        long PosVEnd = ftell(f);

        fseek(f, PosVStart, SEEK_SET);

        fseek(f, FacePosArray[i], SEEK_SET);
        std::cout << "Faces Position: " << ftell(f) << "\n";
        std::cout << "Order: Short, Short, Short\n";
        for (int x = 0; x < Mesh_Info[i].FaceCount / 3; ++x) {
            unsigned short fa;
            unsigned short fb;
            unsigned short fc;
            if (mode == mode::mesh) {
                auto val = ftell(f);
                fseek(f, val, SEEK_SET);
                fa = readshort(f, true) + 1;
                fb = readshort(f, true) + 1;
                fc = readshort(f, true) + 1;
            }
            if (mode == mode::obj) {
                auto val = ftell(f);
                fseek(f, val, SEEK_SET);
                writeshort(f, objFaces[i][objFaceIndex].fa - 1, true);
                writeshort(f, objFaces[i][objFaceIndex].fb - 1, true);
                writeshort(f, objFaces[i][objFaceIndex].fc - 1, true);
                objFaceIndex++;
            }
            if (mode == mode::mesh)
                faceArray.push_back(Face{ fa,fb,fc });
        }
        if (mode == mode::mesh) {
            std::cout << "[SOWMO]-Face count in mesh nr." << i << ": " << Mesh_Info[i].FaceCount << "\n";
            (void)_mkdir(".\\output");
            (void)_mkdir((".\\output\\" + remSuffix(outputFile(argv[1]))).c_str());
            std::ofstream of(".\\output\\" + remSuffix(outputFile(argv[1])) + "\\" + remSuffix(outputFile(argv[1])) + std::to_string(i) + ".obj");
            if (of.fail()) {
                std::cout << "[SOWMO]-Unable to create the output file for mesh " << i << "...\n";
                terminate();
            }
            info.open(".\\output\\" + remSuffix(outputFile(argv[1])) + ".txt", std::ios::app);
            info << "File: " << remSuffix(outputFile(argv[1])) + std::to_string(i) + ".obj:\n";
            info << "Vertices: " << vertArray.size() << "\n";
            info << "Faces: " << faceArray.size() << "\n";
            info << "--------------------------------\n";
            info.close();
            std::cout << "[SOWMO]-Writing vertices...\n";
            for (auto& iter : vertArray) {
                of << "v " << iter.x << " " << iter.y << " " << iter.z << "\n";
            }
            std::cout << "[SOWMO]-Writing faces...\n";
            for (auto& iter : faceArray) {
                of << "f " << iter.fa << "/ " << iter.fb << "/ " << iter.fc << "/\n";
            }
            of.close();
            vertArray.clear();
            faceArray.clear();
        }
        objVertexIndex = 0;
        objFaceIndex = 0;
    }
    std::cout << "[SOWMO]-File creation completed!\n";
}