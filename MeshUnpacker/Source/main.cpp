#include "Parser/includes-types.h"

#include <filesystem>

int main(int argc, char* argv[]) {
	using namespace PARSER;
	if (argc < 2) return 0;

	if (std::string(argv[1]) == "unpack") {
		auto mesh = Parser::Import(argv[2]);
		Parser::Export(mesh, std::filesystem::path(std::string(argv[2])).parent_path().string() + "\\out.fbx", "fbx");
	}
	if (std::string(argv[1]) == "repack") {
		auto ref = Parser::Import(argv[3]);
		auto mesh = Parser::Import(argv[2], &ref);
		Parser::Export(mesh, std::filesystem::path(std::string(argv[2])).parent_path().string() + "\\out.mesh", "mesh");
	}
}