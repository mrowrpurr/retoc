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

TEST_CASE("UTOC Reader can parse UTOC files", "[utoc_reader]") {
    // Path to the example UTOC file
    std::filesystem::path utocPath = "../../../../example_files/000_BetterHUD_P.utoc";
    
    REQUIRE(std::filesystem::exists(utocPath));
    
    utoc::UtocReader reader;
    REQUIRE(reader.Open(utocPath));
    
    // Check header information
    const auto& header = reader.GetHeader();
    REQUIRE(header.IsValid());
    REQUIRE(header.version == utoc::EIoStoreTocVersion::PerfectHashWithOverflow);
    REQUIRE(header.toc_entry_count == 23);
    REQUIRE_FALSE(header.IsCompressed());
    REQUIRE_FALSE(header.IsEncrypted());
    REQUIRE_FALSE(header.IsSigned());
    REQUIRE(header.IsIndexed());
    
    // Check file paths
    std::vector<std::string> filePaths = reader.GetAllFilePaths();
    REQUIRE(filePaths.size() == 22);
    
    // Check that the first file path is what we expect
    REQUIRE(filePaths[0] == "../../../WBP_ModernHud_EffectIcons.uasset");
    
    // Check chunk offset and length for the first chunk
    const auto& offsetLength = reader.GetChunkOffsetLengths()[0];
    REQUIRE(offsetLength.GetOffset() == 0);
    REQUIRE(offsetLength.GetLength() == 6620);
}

TEST_CASE("UTOC Reader can read chunks from UCAS files", "[utoc_reader][ucas_reader]") {
    // Path to the example UTOC file
    std::filesystem::path utocPath = "../../../../example_files/000_BetterHUD_P.utoc";
    std::filesystem::path ucasPath = "../../../../example_files/000_BetterHUD_P.ucas";
    
    REQUIRE(std::filesystem::exists(utocPath));
    REQUIRE(std::filesystem::exists(ucasPath));
    
    utoc::UtocReader reader;
    REQUIRE(reader.Open(utocPath));
    REQUIRE(reader.OpenUcas());
    
    // Try to read the first chunk
    REQUIRE_NOTHROW([&]() {
        std::vector<uint8_t> chunkData = reader.ReadChunkByIndex(0);
        REQUIRE(chunkData.size() == 6620);
        
        // Check the first few bytes of the chunk
        REQUIRE(chunkData[0] == 0x00);
        REQUIRE(chunkData[1] == 0x00);
        REQUIRE(chunkData[2] == 0x00);
        REQUIRE(chunkData[3] == 0x00);
    }());
}

TEST_CASE("UTOC Reader handles errors gracefully", "[utoc_reader][error_handling]") {
    // Test with a non-existent file
    std::filesystem::path nonExistentPath = "non_existent_file.utoc";
    
    utoc::UtocReader reader;
    REQUIRE_FALSE(reader.Open(nonExistentPath));
    
    // Test with an invalid file
    std::filesystem::path invalidPath = "../../../../cpp_tests/utoc_reader_tests.cpp"; // Using this file as an invalid UTOC file
    REQUIRE(std::filesystem::exists(invalidPath));
    
    REQUIRE_FALSE(reader.Open(invalidPath));
    
    // Test reading a chunk with an invalid index
    std::filesystem::path validPath = "../../../../example_files/000_BetterHUD_P.utoc";
    REQUIRE(reader.Open(validPath));
    
    REQUIRE_THROWS_AS(reader.ReadChunkByIndex(9999), std::out_of_range);
    
    // Test reading a chunk with a non-existent path
    REQUIRE_THROWS_AS(reader.ReadChunkByPath("non_existent_path"), std::runtime_error);
}
