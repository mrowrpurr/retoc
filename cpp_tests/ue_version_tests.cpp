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

TEST_CASE("UTOC Reader can parse UE4.27 container headers", "[utoc_reader][ue4.27]") {
    // Path to the UE4.27 container header file
    std::filesystem::path containerHeaderPath = "../../../../tests/UE4.27/ContainerHeader_1.bin";
    
    REQUIRE(std::filesystem::exists(containerHeaderPath));
    
    // Read the container header file
    std::vector<uint8_t> containerHeaderData = readFile(containerHeaderPath);
    REQUIRE_FALSE(containerHeaderData.empty());
    
    // TODO: Add actual parsing of the container header once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE4.27 container header file");
}

TEST_CASE("UTOC Reader can parse UE5.3 container headers", "[utoc_reader][ue5.3]") {
    // Path to the UE5.3 container header file
    std::filesystem::path containerHeaderPath = "../../../../tests/UE5.3/ContainerHeader_1.bin";
    
    REQUIRE(std::filesystem::exists(containerHeaderPath));
    
    // Read the container header file
    std::vector<uint8_t> containerHeaderData = readFile(containerHeaderPath);
    REQUIRE_FALSE(containerHeaderData.empty());
    
    // TODO: Add actual parsing of the container header once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE5.3 container header file");
}

TEST_CASE("UTOC Reader can parse UE4.27 assets", "[utoc_reader][ue4.27]") {
    // Path to the UE4.27 asset files
    std::filesystem::path assetPath = "../../../../tests/UE4.27/SPR_UI_Battle.uasset";
    
    REQUIRE(std::filesystem::exists(assetPath));
    
    // Read the asset file
    std::vector<uint8_t> assetData = readFile(assetPath);
    REQUIRE_FALSE(assetData.empty());
    
    // TODO: Add actual parsing of the asset once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE4.27 asset file");
}

TEST_CASE("UTOC Reader can parse UE5.4 assets", "[utoc_reader][ue5.4]") {
    // Path to the UE5.4 asset files
    std::filesystem::path assetPath = "../../../../tests/UE5.4/BP_Russian_pool_table.uasset";
    
    REQUIRE(std::filesystem::exists(assetPath));
    
    // Read the asset file
    std::vector<uint8_t> assetData = readFile(assetPath);
    REQUIRE_FALSE(assetData.empty());
    
    // TODO: Add actual parsing of the asset once implemented
    // For now, just verify that we can read the file
    SUCCEED("Successfully read UE5.4 asset file");
}

TEST_CASE("UTOC Reader can handle issue-specific test files", "[utoc_reader][issues]") {
    // Path to the issue-specific test files
    std::filesystem::path issue7Path = "../../../../tests/issues/issue7/header.bin";
    std::filesystem::path issue18Path = "../../../../tests/issues/issue18/header.bin";
    
    REQUIRE(std::filesystem::exists(issue7Path));
    REQUIRE(std::filesystem::exists(issue18Path));
    
    // Read the issue-specific test files
    std::vector<uint8_t> issue7Data = readFile(issue7Path);
    std::vector<uint8_t> issue18Data = readFile(issue18Path);
    
    REQUIRE_FALSE(issue7Data.empty());
    REQUIRE_FALSE(issue18Data.empty());
    
    // TODO: Add actual parsing of the issue-specific test files once implemented
    // For now, just verify that we can read the files
    SUCCEED("Successfully read issue-specific test files");
}
