#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <filesystem>
#include <string>

std::string g_assetsDir;
std::filesystem::path g_exeDir;

int main(int argc, char* argv[]) {
	// Get the executable directory and construct the path to the assets directory
	std::filesystem::path exePath = argv[0];
	g_exeDir = exePath.parent_path();
	g_assetsDir = (g_exeDir / "assets").string();

	return Catch::Session().run(argc, argv);
}
