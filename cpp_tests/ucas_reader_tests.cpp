#include <catch2/catch_all.hpp>
#include "ucas_reader.h"
#include <filesystem>
#include <fstream>
#include <vector>

TEST_CASE("UCAS Reader can open UCAS files", "[ucas_reader]") {
    // Path to the example UCAS file
    std::filesystem::path ucasPath = "../../../../example_files/000_BetterHUD_P.ucas";
    
    REQUIRE(std::filesystem::exists(ucasPath));
    
    utoc::UcasReader reader;
    REQUIRE(reader.Open(ucasPath));
}

TEST_CASE("UCAS Reader can read raw data", "[ucas_reader]") {
    // Path to the example UCAS file
    std::filesystem::path ucasPath = "../../../../example_files/000_BetterHUD_P.ucas";
    
    REQUIRE(std::filesystem::exists(ucasPath));
    
    utoc::UcasReader reader;
    REQUIRE(reader.Open(ucasPath));
    
    // Read the first 100 bytes of the file
    REQUIRE_NOTHROW([&]() {
        std::vector<uint8_t> data = reader.ReadRawData(0, 100);
        REQUIRE(data.size() == 100);
    }());
}

TEST_CASE("UCAS Reader handles errors gracefully", "[ucas_reader][error_handling]") {
    // Test with a non-existent file
    std::filesystem::path nonExistentPath = "non_existent_file.ucas";
    
    utoc::UcasReader reader;
    REQUIRE_FALSE(reader.Open(nonExistentPath));
    
    // Test with a valid file but invalid offset/size
    std::filesystem::path validPath = "../../../../example_files/000_BetterHUD_P.ucas";
    REQUIRE(reader.Open(validPath));
    
    // Test reading beyond the end of the file
    REQUIRE_THROWS([&]() {
        reader.ReadRawData(1000000000, 100);
    }());
    
    // Test reading with an unreasonably large size
    REQUIRE_THROWS([&]() {
        reader.ReadRawData(0, 1000000000);
    }());
}

TEST_CASE("UCAS Reader can decompress data", "[ucas_reader][decompression]") {
    // This test is a placeholder for future decompression tests
    // Currently, our DecompressData method only supports uncompressed data
    
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    std::string compressionMethod = "None";
    uint32_t uncompressedSize = testData.size();
    
    utoc::UcasReader reader;
    std::vector<uint8_t> decompressedData = reader.DecompressData(testData, compressionMethod, uncompressedSize);
    
    REQUIRE(decompressedData.size() == testData.size());
    REQUIRE(decompressedData == testData);
}
