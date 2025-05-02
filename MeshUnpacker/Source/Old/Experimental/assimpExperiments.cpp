#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <vector>
#include <windows.h>

int main() {
	auto outfilepath = "F:\\cube.fbx";

	Assimp::Importer mimporter;
	const aiScene* testscene = mimporter.ReadFile("C:\\Users\\gerbd\\Desktop\\Cube.fbx", 0);
	aiScene* scene = new aiScene;
	scene->mNumMeshes = testscene->mNumMeshes;
	scene->mMeshes = testscene->mMeshes;
	scene->mNumMaterials = testscene->mNumMaterials;
	scene->mMaterials = testscene->mMaterials;
	scene->mRootNode = testscene->mRootNode;

	Assimp::Exporter exporter;
	exporter.Export(scene, "fbxa", outfilepath);
	return 0;
}

//#include <assimp/Importer.hpp>
//#include <assimp/Exporter.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <iostream>
//
//
//int main() {
//  
//    aiScene* scene = new aiScene();
//
//    aiMesh* cubeMesh = new aiMesh();
//    cubeMesh->mNumVertices = 8;
//    cubeMesh->mNumFaces = 12;
//
//    cubeMesh->mVertices = new aiVector3D[8]{
//        aiVector3D(-1.0f, -1.0f, -1.0f), aiVector3D(1.0f, -1.0f, -1.0f),
//        aiVector3D(1.0f,  1.0f, -1.0f), aiVector3D(-1.0f,  1.0f, -1.0f),
//        aiVector3D(-1.0f, -1.0f,  1.0f), aiVector3D(1.0f, -1.0f,  1.0f),
//        aiVector3D(1.0f,  1.0f,  1.0f), aiVector3D(-1.0f,  1.0f,  1.0f)
//    };
//
//    cubeMesh->mFaces = new aiFace[12];
//    unsigned int faceIndices[12][3] = {
//        {0, 1, 2}, {0, 2, 3}, {4, 5, 6}, {4, 6, 7},
//        {0, 1, 5}, {0, 5, 4}, {1, 2, 6}, {1, 6, 5},
//        {2, 3, 7}, {2, 7, 6}, {3, 0, 4}, {3, 4, 7}
//    };
//
//    for (unsigned int i = 0; i < 12; ++i) {
//        cubeMesh->mFaces[i].mIndices = new unsigned int[3];
//        std::copy(faceIndices[i], faceIndices[i] + 3, cubeMesh->mFaces[i].mIndices);
//        cubeMesh->mFaces[i].mNumIndices = 3;
//    }
//
//    aiMaterial* material = new aiMaterial();
//    aiColor4D color(1.0f, 0.0f, 0.0f, 1.0f);
//    material->AddProperty(&color, 2, AI_MATKEY_BASE_COLOR);
//    cubeMesh->mMaterialIndex = 0;
//
//    scene->mNumMeshes = 1;
//    scene->mMeshes = new aiMesh * [1] {cubeMesh};
//
//    scene->mNumMaterials = 1;
//    scene->mMaterials = new aiMaterial * [1] {material};
//
//    aiNode* rootNode = new aiNode();
//    rootNode->mName = aiString("RootNode");
//    rootNode->mNumMeshes = 1;
//    rootNode->mMeshes = new unsigned int[1] {0};
//    rootNode->mNumChildren = 0;
//    rootNode->mChildren = nullptr;
//
//    scene->mRootNode = rootNode;
//
//    Assimp::Exporter exporter;
//    if (exporter.Export(scene, "fbxa", "F:\\cube.fbx") != aiReturn_SUCCESS) {
//        std::cerr << "Fehler beim Exportieren der FBX-Datei" << std::endl;
//    }
//
//
//    return 0;
//}