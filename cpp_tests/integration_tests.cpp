#include <catch2/catch_all.hpp>
#include "utoc_reader.h"
#include "ucas_reader.h"
#include <filesystem>
#include <fstream>
#include <vector>

TEST_CASE("UTOC and UCAS readers work together", "[integration]") {
    // Path to the example files
    std::filesystem::path utocPath = "../../../../example_files/000_BetterHUD_P.utoc";
    std::filesystem::path ucasPath = "../../../../example_files/000_BetterHUD_P.ucas";
    
    REQUIRE(std::filesystem::exists(utocPath));
    REQUIRE(std::filesystem::exists(ucasPath));
    
    // Open the UTOC file
    utoc::UtocReader utocReader;
    REQUIRE(utocReader.Open(utocPath));
    
    // Open the UCAS file
    REQUIRE(utocReader.OpenUcas());
    
    // Get all file paths from the UTOC
    std::vector<std::string> filePaths = utocReader.GetAllFilePaths();
    REQUIRE_FALSE(filePaths.empty());
    
    // Try to read the first chunk by index
    REQUIRE_NOTHROW([&]() {
        std::vector<uint8_t> chunkData = utocReader.ReadChunkByIndex(0);
        REQUIRE_FALSE(chunkData.empty());
    }());
    
    // Try to read the first file by path
    REQUIRE_NOTHROW([&]() {
        std::vector<uint8_t> chunkData = utocReader.ReadChunkByPath(filePaths[0]);
        REQUIRE_FALSE(chunkData.empty());
    }());
}

TEST_CASE("UTOC reader can handle different file paths", "[integration][file_paths]") {
    // Path to the example UTOC file
    std::filesystem::path utocPath = "../../../../example_files/000_BetterHUD_P.utoc";
    
    REQUIRE(std::filesystem::exists(utocPath));
    
    utoc::UtocReader reader;
    REQUIRE(reader.Open(utocPath));
    
    // Get all file paths
    std::vector<std::string> filePaths = reader.GetAllFilePaths();
    REQUIRE_FALSE(filePaths.empty());
    
    // Check that file paths are properly formatted
    for (const auto& path : filePaths) {
        REQUIRE_FALSE(path.empty());
        
        // Check that the path has a file extension
        auto lastDot = path.find_last_of('.');
        REQUIRE(lastDot != std::string::npos);
        
        // Check that the file extension is valid
        std::string extension = path.substr(lastDot);
        bool isValidExtension = (extension == ".uasset" || extension == ".umap" || extension == ".uexp" || extension == ".ubulk");
        REQUIRE(isValidExtension);
    }
}

TEST_CASE("UTOC reader can map file paths to chunk indices", "[integration][mapping]") {
    // Path to the example UTOC file
    std::filesystem::path utocPath = "../../../../example_files/000_BetterHUD_P.utoc";
    
    REQUIRE(std::filesystem::exists(utocPath));
    
    utoc::UtocReader reader;
    REQUIRE(reader.Open(utocPath));
    
    // Get all file paths
    std::vector<std::string> filePaths = reader.GetAllFilePaths();
    REQUIRE_FALSE(filePaths.empty());
    
    // Check that each file path can be mapped to a chunk index
    for (const auto& path : filePaths) {
        auto chunkIndex = reader.GetChunkIndexByPath(path);
        REQUIRE(chunkIndex.has_value());
        
        // Check that the chunk index is valid
        REQUIRE(*chunkIndex < reader.GetHeader().toc_entry_count);
    }
}
