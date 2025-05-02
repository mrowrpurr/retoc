#include <catch2/catch_all.hpp>
#include "utoc_reader.h"
#include "ucas_reader.h"
#include <filesystem>
#include <fstream>
#include <vector>

// Helper function to read a file into a vector of bytes
static std::vector<uint8_t> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    REQUIRE(file.is_open());
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    
    return buffer;
}

TEST_CASE("UTOC Reader can parse UE5.4 shader archives", "[utoc_reader][ue5.4][shader]") {
    // Path to the UE5.4 shader archive files
    std::filesystem::path globalShaderPath = "../../../../tests/UE5.4/ShaderArchive-Global-PCD3D_SM6-PCD3D_SM6.ushaderbytecode";
    std::filesystem::path gameShaderPath = "../../../../tests/UE5.4/ShaderArchive-NuclearNightmare-PCD3D_SM6-PCD3D_SM6.ushaderbytecode";
    std::filesystem::path zenShaderPath = "../../../../tests/UE5.4/ShaderArchive-NuclearNightmare-PCD3D_SM6-PCD3D_SM6.uzenshaderbytecode";
    
    REQUIRE(std::filesystem::exists(globalShaderPath));
    REQUIRE(std::filesystem::exists(gameShaderPath));
    REQUIRE(std::filesystem::exists(zenShaderPath));
    
    // Read the shader archive files
    std::vector<uint8_t> globalShaderData = readFile(globalShaderPath);
    std::vector<uint8_t> gameShaderData = readFile(gameShaderPath);
    std::vector<uint8_t> zenShaderData = readFile(zenShaderPath);
    
    REQUIRE_FALSE(globalShaderData.empty());
    REQUIRE_FALSE(gameShaderData.empty());
    REQUIRE_FALSE(zenShaderData.empty());
    
    // TODO: Add actual parsing of the shader archives once implemented
    // For now, just verify that we can read the files
    SUCCEED("Successfully read UE5.4 shader archive files");
}

TEST_CASE("UTOC Reader can parse UE5.4 zen assets", "[utoc_reader][ue5.4][zen]") {
    // Path to the UE5.4 zen asset files
    std::filesystem::path zenAssetPath = "../../../../tests/UE5.4/BP_Table_Lamp.uzenasset";
    
    REQUIRE(std::filesystem::exists(zenAssetPath));
    
    // Read the zen asset file
    std::vector<uint8_t> zenAssetData = readFile(zenAssetPath);
    REQUIRE_FALSE(zenAssetData.empty());
    
    // TODO: Add actual parsing of the zen asset once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE5.4 zen asset file");
}

TEST_CASE("UTOC Reader can parse UE5.4 package store manifest", "[utoc_reader][ue5.4][manifest]") {
    // Path to the UE5.4 package store manifest file
    std::filesystem::path manifestPath = "../../../../tests/UE5.4/packagestore.manifest";
    
    REQUIRE(std::filesystem::exists(manifestPath));
    
    // Read the package store manifest file
    std::vector<uint8_t> manifestData = readFile(manifestPath);
    REQUIRE_FALSE(manifestData.empty());
    
    // TODO: Add actual parsing of the package store manifest once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE5.4 package store manifest file");
}

TEST_CASE("UTOC Reader can parse UE5.3 script objects", "[utoc_reader][ue5.3][script]") {
    // Path to the UE5.3 script objects file
    std::filesystem::path scriptObjectsPath = "../../../../tests/UE5.3/ScriptObjects.bin";
    
    REQUIRE(std::filesystem::exists(scriptObjectsPath));
    
    // Read the script objects file
    std::vector<uint8_t> scriptObjectsData = readFile(scriptObjectsPath);
    REQUIRE_FALSE(scriptObjectsData.empty());
    
    // TODO: Add actual parsing of the script objects once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE5.3 script objects file");
}
