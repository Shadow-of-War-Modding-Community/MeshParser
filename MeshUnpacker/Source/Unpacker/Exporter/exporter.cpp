#include "../Internal/includes-types-defs.h"

std::shared_ptr<aiScene> mesh_to_assimp(std::shared_ptr<MESH_UNPACKER::Mesh> mesh) {
    auto scene = std::make_shared<aiScene>();

    aiMaterial* material = new aiMaterial();

    aiColor3D diffuseColor(1.0f, 1.0f, 1.0f);
    material->AddProperty(&diffuseColor, 1, AI_MATKEY_COLOR_DIFFUSE);

    aiColor3D specularColor(1.0f, 1.0f, 1.0f);
    material->AddProperty(&specularColor, 1, AI_MATKEY_COLOR_SPECULAR);

    aiColor3D ambientColor(1.0f, 1.0f, 1.0f);
    material->AddProperty(&ambientColor, 1, AI_MATKEY_COLOR_AMBIENT);

    aiColor3D emissiveColor(1.0f, 1.0f, 1.0f);
    material->AddProperty(&emissiveColor, 1, AI_MATKEY_COLOR_EMISSIVE);

    float shininess = 20.0f;
    material->AddProperty(&shininess, 1, AI_MATKEY_SHININESS);

    // Convert a bone's transformation to a 4x4 matrix
    auto bone_transforms_to_mat4x4 = [](const MESH_UNPACKER::INTERNAL::MESH::TYPES::Bone& bone) {
        aiQuaternion quat(bone.rotation.x, bone.rotation.y, bone.rotation.z, bone.rotation.w);
        aiMatrix4x4 rotationMatrix = aiMatrix4x4(quat.GetMatrix());

        // Apply the translation to the matrix
        rotationMatrix.a4 = bone.translation.x;
        rotationMatrix.b4 = bone.translation.y;
        rotationMatrix.c4 = bone.translation.z;

        return rotationMatrix;
        };

    // Compute the inverse of the bone's transformation (bind-pose offset matrix)
    auto compute_bone_offset_matrix = [&](const MESH_UNPACKER::INTERNAL::MESH::TYPES::Bone& bone) {
        aiMatrix4x4 transform = bone_transforms_to_mat4x4(bone);
        return transform;  // Inverse matrix represents the offset to the bone space
        };

    int mesh_counter = 0;
    aiMesh** mesh_buffer = new aiMesh * [mesh->meshInfoSection.meshCount];

    // Loop through LODs and meshes
    for (int lodCounter = 0; lodCounter < mesh->lodBuffers.size(); ++lodCounter) {
        for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
            aiMesh* ai_mesh = new aiMesh;
            auto current_mesh_vertex_count = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size();
            aiVector3D* positionBuffer = new aiVector3D[current_mesh_vertex_count];
            aiVector3D* normalsBuffer = new aiVector3D[current_mesh_vertex_count];
            aiVector3D* tangentsBuffer = new aiVector3D[current_mesh_vertex_count];
            aiVector3D* bitangentsBuffer = new aiVector3D[current_mesh_vertex_count];
            aiColor4D* colorsBuffer = new aiColor4D[current_mesh_vertex_count];
            auto uvChannelCount = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][0].uvs.size();
            aiVector3D** uvsBuffers = new aiVector3D * [uvChannelCount];
            for (int i = 0; i < uvChannelCount; ++i)
                uvsBuffers[i] = new aiVector3D[current_mesh_vertex_count];

            // Allocate bones for the mesh
            aiBone** bones = new aiBone * [mesh->skeleton->header.boneCount1];
            if (mesh->has_skeleton) {
                for (int boneCounter = 0; boneCounter < mesh->skeleton->header.boneCount1; ++boneCounter) {
                    auto bone = mesh->skeleton->boneSection.bones[boneCounter];
                    bones[boneCounter] = new aiBone();
                    bones[boneCounter]->mName = bone.boneName;
                    bones[boneCounter]->mOffsetMatrix = compute_bone_offset_matrix(mesh->meshInfoSection.boneSection.bones[boneCounter]);
                }
            }

            // Process vertices and attributes
            for (int vertexCounter = 0; vertexCounter < current_mesh_vertex_count; ++vertexCounter) {
                const auto& vertex = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][vertexCounter];

                positionBuffer[vertexCounter].x = vertex.position.x;
                positionBuffer[vertexCounter].y = vertex.position.y;
                positionBuffer[vertexCounter].z = vertex.position.z;

                normalsBuffer[vertexCounter].x = vertex.normal.x;
                normalsBuffer[vertexCounter].y = vertex.normal.y;
                normalsBuffer[vertexCounter].z = vertex.normal.z;

                tangentsBuffer[vertexCounter].x = vertex.tangent.x;
                tangentsBuffer[vertexCounter].y = vertex.tangent.y;
                tangentsBuffer[vertexCounter].z = vertex.tangent.z;

                bitangentsBuffer[vertexCounter].x = vertex.bitangent.x;
                bitangentsBuffer[vertexCounter].y = vertex.bitangent.y;
                bitangentsBuffer[vertexCounter].z = vertex.bitangent.z;

                colorsBuffer[vertexCounter].r = (float)vertex.color.r / 255.f;
                colorsBuffer[vertexCounter].g = (float)vertex.color.g / 255.f;
                colorsBuffer[vertexCounter].b = (float)vertex.color.b / 255.f;
                colorsBuffer[vertexCounter].a = (float)vertex.color.a / 255.f;

                for (int i = 0; i < uvChannelCount; ++i) {
                    if (vertex.uvs[i].second) { // true = short
                        uvsBuffers[i][vertexCounter].x = vertex.uvs[i].first.u / 32767.f;
                        uvsBuffers[i][vertexCounter].y = 1.f - vertex.uvs[i].first.v / 32767.f;
                        uvsBuffers[i][vertexCounter].z = 0.f;
                    }
                    else { // false = float
                        uvsBuffers[i][vertexCounter].x = vertex.uvs[i].first.u;
                        uvsBuffers[i][vertexCounter].y = 1.f - vertex.uvs[i].first.v;
                        uvsBuffers[i][vertexCounter].z = 0.f;
                    }
                }
            }

            // Assign mesh data
            ai_mesh->mNumVertices = current_mesh_vertex_count;
            ai_mesh->mVertices = positionBuffer;
            ai_mesh->mNormals = normalsBuffer;
            ai_mesh->mTangents = tangentsBuffer;
            ai_mesh->mBitangents = bitangentsBuffer;
            ai_mesh->mColors[0] = colorsBuffer;
            for (int i = 0; i < uvChannelCount; ++i) {
                ai_mesh->mTextureCoords[i] = uvsBuffers[i];
                ai_mesh->mNumUVComponents[i] = 2;
            }

            // Process faces
            auto current_mesh_face_count = mesh->lodBuffers[lodCounter].meshFaceContainers[meshCounter].size();
            aiFace* faceBuffer = new aiFace[current_mesh_face_count];
            for (int faceCounter = 0; faceCounter < current_mesh_face_count; ++faceCounter) {
                const auto& face = mesh->lodBuffers[lodCounter].meshFaceContainers[meshCounter][faceCounter];
                faceBuffer[faceCounter].mNumIndices = 3;
                faceBuffer[faceCounter].mIndices = new unsigned int[3];
                faceBuffer[faceCounter].mIndices[0] = face.index1;
                faceBuffer[faceCounter].mIndices[1] = face.index2;
                faceBuffer[faceCounter].mIndices[2] = face.index3;
            }
            ai_mesh->mNumFaces = current_mesh_face_count;
            ai_mesh->mFaces = faceBuffer;

            // Process bone weights
            if (mesh->has_skeleton) {
                for (int vertexGroupCounter = 0; vertexGroupCounter < mesh->lodBuffers[lodCounter].meshVertexGroupContainers[meshCounter].size(); ++vertexGroupCounter) {
                    std::vector<aiVertexWeight> weights;
                    auto currentVertexGroup = mesh->lodBuffers[lodCounter].meshVertexGroupContainers[meshCounter][vertexGroupCounter];
                    for (int vertexCounter = 0; vertexCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter].size(); ++vertexCounter) {
                        aiVertexWeight weight;
                        auto& vertex = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers[meshCounter][vertexCounter];

                        int sum = vertex.weights.weight1 + vertex.weights.weight2 + vertex.weights.weight3 + vertex.weights.weight4;
                        if (sum > 0) {
                            if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex1) {
                                weight.mVertexId = vertexCounter;
                                weight.mWeight = (float)vertex.weights.weight1 / sum;
                                weights.push_back(weight);
                            }
                            else if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex2) {
                                weight.mVertexId = vertexCounter;
                                weight.mWeight = (float)vertex.weights.weight2 / sum;
                                weights.push_back(weight);
                            }
                            else if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex3) {
                                weight.mVertexId = vertexCounter;
                                weight.mWeight = (float)vertex.weights.weight3 / sum;
                                weights.push_back(weight);
                            }
                            else if (currentVertexGroup == vertex.vertexGroups.vertexGroupIndex4) {
                                weight.mVertexId = vertexCounter;
                                weight.mWeight = (float)vertex.weights.weight4 / sum;
                                weights.push_back(weight);
                            }
                        }
                    }
                    bones[currentVertexGroup]->mNumWeights = weights.size();
                    bones[currentVertexGroup]->mWeights = new aiVertexWeight[weights.size()];
                    std::copy(weights.begin(), weights.end(), bones[currentVertexGroup]->mWeights);
                }
            }
            ai_mesh->mBones = bones;
            ai_mesh->mNumBones = mesh->skeleton->header.boneCount1;

            ai_mesh->mMaterialIndex = 0;
            mesh_buffer[mesh_counter] = ai_mesh;
            mesh_counter++;
        }
    }
    scene->mNumMeshes = mesh->meshInfoSection.meshCount;
    scene->mMeshes = mesh_buffer;

    // Set up the root node and bone hierarchy
    aiNode* rootNode = new aiNode;
    rootNode->mNumMeshes = mesh->meshInfoSection.meshCount;
    rootNode->mName = "RootNode";

    std::vector<aiNode*> nodes(mesh->skeleton->header.boneCount1);
    for (int i = 0; i < mesh->skeleton->header.boneCount1; ++i) {
        aiNode* node = new aiNode();
        node->mName = aiString(mesh->skeleton->boneSection.bones[i].boneName);
        node->mTransformation = bone_transforms_to_mat4x4(mesh->meshInfoSection.boneSection.bones[i]);
        nodes[i] = node;
    }

    for (int i = 0; i < mesh->skeleton->header.boneCount1; ++i) {
        auto parent_index = mesh->meshInfoSection.boneSection.bones[i].parentIndex;
        if (parent_index == -1) {
            rootNode->addChildren(1, &nodes[i]);
        }
        else {
            nodes[parent_index]->addChildren(1, &nodes[i]);
        }
    }

    // Set LOD nodes as children of the root node
    //mesh_counter = 0;
    //aiNode** lodNodes = new aiNode * [mesh->lodBuffers.size()];
    //for (int lodCounter = 0; lodCounter < mesh->lodBuffers.size(); ++lodCounter) {
    //    lodNodes[lodCounter] = new aiNode;
    //    lodNodes[lodCounter]->mName = aiString("LOD " + std::to_string(lodCounter + 1));
    //    lodNodes[lodCounter]->mNumMeshes = mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size();
    //    unsigned int* meshes = new unsigned int[mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size()];
    //    for (int meshCounter = 0; meshCounter < mesh->lodBuffers[lodCounter].meshVertexAttributeContainers.size(); ++meshCounter) {
    //        meshes[meshCounter] = mesh_counter;
    //        mesh_counter++;
    //    }
    //    lodNodes[lodCounter]->mMeshes = meshes;
    //}
    //rootNode->addChildren(mesh->lodBuffers.size(), lodNodes);
    unsigned int* meshNr = new unsigned int[mesh->meshInfoSection.meshCount];
    for (int i = 0; i < mesh->meshInfoSection.meshCount; ++i)
        meshNr[i] = i;

    rootNode->mMeshes = meshNr;
    scene->mRootNode = rootNode;
    scene->mNumMaterials = 1;
    scene->mMaterials = new aiMaterial * [1] {material};

    return scene;
}
