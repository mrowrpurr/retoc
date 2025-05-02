#include <catch2/catch_all.hpp>
#include "compression.h"
#include <vector>
#include <string>
#include <random>
#include <algorithm>

// Include compression libraries
#include <zlib.h>
#include <zstd.h>
#include <lz4.h>

// Helper function to generate random data
std::vector<uint8_t> generateRandomData(size_t size, unsigned int seed = 42) {
    std::vector<uint8_t> data(size);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    
    std::generate(data.begin(), data.end(), [&]() { return static_cast<uint8_t>(dist(rng)); });
    
    return data;
}

// Helper function to compress data using zlib
std::vector<uint8_t> compressZlib(const std::vector<uint8_t>& data) {
    // Calculate the upper bound for the compressed data size
    uLongf compressedSize = compressBound(data.size());
    std::vector<uint8_t> compressedData(compressedSize);
    
    // Compress the data
    int result = compress(
        compressedData.data(),
        &compressedSize,
        data.data(),
        data.size()
    );
    
    REQUIRE(result == Z_OK);
    
    // Resize the vector to the actual compressed size
    compressedData.resize(compressedSize);
    
    return compressedData;
}

// Helper function to compress data using zstd
std::vector<uint8_t> compressZstd(const std::vector<uint8_t>& data) {
    // Calculate the upper bound for the compressed data size
    size_t compressedSize = ZSTD_compressBound(data.size());
    std::vector<uint8_t> compressedData(compressedSize);
    
    // Compress the data
    size_t result = ZSTD_compress(
        compressedData.data(),
        compressedSize,
        data.data(),
        data.size(),
        1  // Compression level
    );
    
    REQUIRE_FALSE(ZSTD_isError(result));
    
    // Resize the vector to the actual compressed size
    compressedData.resize(result);
    
    return compressedData;
}

// Helper function to compress data using LZ4
std::vector<uint8_t> compressLZ4(const std::vector<uint8_t>& data) {
    // Calculate the upper bound for the compressed data size
    int maxCompressedSize = LZ4_compressBound(data.size());
    std::vector<uint8_t> compressedData(maxCompressedSize);
    
    // Compress the data
    int compressedSize = LZ4_compress_default(
        reinterpret_cast<const char*>(data.data()),
        reinterpret_cast<char*>(compressedData.data()),
        data.size(),
        maxCompressedSize
    );
    
    REQUIRE(compressedSize > 0);
    
    // Resize the vector to the actual compressed size
    compressedData.resize(compressedSize);
    
    return compressedData;
}

TEST_CASE("CompressionMethod enum conversion", "[compression]") {
    SECTION("String to enum conversion") {
        REQUIRE(utoc::CompressionMethodFromString("None") == utoc::CompressionMethod::None);
        REQUIRE(utoc::CompressionMethodFromString("none") == utoc::CompressionMethod::None);
        REQUIRE(utoc::CompressionMethodFromString("NONE") == utoc::CompressionMethod::None);
        REQUIRE(utoc::CompressionMethodFromString("") == utoc::CompressionMethod::None);
        
        REQUIRE(utoc::CompressionMethodFromString("Zlib") == utoc::CompressionMethod::Zlib);
        REQUIRE(utoc::CompressionMethodFromString("zlib") == utoc::CompressionMethod::Zlib);
        REQUIRE(utoc::CompressionMethodFromString("ZLIB") == utoc::CompressionMethod::Zlib);
        
        REQUIRE(utoc::CompressionMethodFromString("Zstd") == utoc::CompressionMethod::Zstd);
        REQUIRE(utoc::CompressionMethodFromString("zstd") == utoc::CompressionMethod::Zstd);
        REQUIRE(utoc::CompressionMethodFromString("ZSTD") == utoc::CompressionMethod::Zstd);
        
        REQUIRE(utoc::CompressionMethodFromString("LZ4") == utoc::CompressionMethod::LZ4);
        REQUIRE(utoc::CompressionMethodFromString("lz4") == utoc::CompressionMethod::LZ4);
        REQUIRE(utoc::CompressionMethodFromString("Lz4") == utoc::CompressionMethod::LZ4);
        
        REQUIRE(utoc::CompressionMethodFromString("Oodle") == utoc::CompressionMethod::Oodle);
        REQUIRE(utoc::CompressionMethodFromString("oodle") == utoc::CompressionMethod::Oodle);
        REQUIRE(utoc::CompressionMethodFromString("OODLE") == utoc::CompressionMethod::Oodle);
        
        REQUIRE_THROWS_AS(utoc::CompressionMethodFromString("Unknown"), utoc::CompressionError);
    }
    
    SECTION("Enum to string conversion") {
        REQUIRE(utoc::CompressionMethodToString(utoc::CompressionMethod::None) == "None");
        REQUIRE(utoc::CompressionMethodToString(utoc::CompressionMethod::Zlib) == "Zlib");
        REQUIRE(utoc::CompressionMethodToString(utoc::CompressionMethod::Zstd) == "Zstd");
        REQUIRE(utoc::CompressionMethodToString(utoc::CompressionMethod::LZ4) == "LZ4");
        REQUIRE(utoc::CompressionMethodToString(utoc::CompressionMethod::Oodle) == "Oodle");
    }
}

TEST_CASE("Decompression with no compression", "[compression]") {
    // Test with empty data
    std::vector<uint8_t> emptyData;
    auto result = utoc::DecompressData(emptyData, utoc::CompressionMethod::None, 0);
    REQUIRE(result.empty());
    
    // Test with non-empty data
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    result = utoc::DecompressData(testData, utoc::CompressionMethod::None, testData.size());
    REQUIRE(result == testData);
    
    // Test with string method
    result = utoc::DecompressData(testData, "None", testData.size());
    REQUIRE(result == testData);
    
    // Test with empty string method
    result = utoc::DecompressData(testData, "", testData.size());
    REQUIRE(result == testData);
}

TEST_CASE("Zlib decompression", "[compression][zlib]") {
    // Test with small data
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    std::vector<uint8_t> compressedData = compressZlib(testData);
    
    auto result = utoc::DecompressData(compressedData, utoc::CompressionMethod::Zlib, testData.size());
    REQUIRE(result == testData);
    
    // Test with string method
    result = utoc::DecompressData(compressedData, "Zlib", testData.size());
    REQUIRE(result == testData);
    
    // Test with larger random data
    std::vector<uint8_t> largeData = generateRandomData(10000);
    std::vector<uint8_t> largeCompressedData = compressZlib(largeData);
    
    result = utoc::DecompressData(largeCompressedData, utoc::CompressionMethod::Zlib, largeData.size());
    REQUIRE(result == largeData);
}

TEST_CASE("Zstd decompression", "[compression][zstd]") {
    // Test with small data
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    std::vector<uint8_t> compressedData = compressZstd(testData);
    
    auto result = utoc::DecompressData(compressedData, utoc::CompressionMethod::Zstd, testData.size());
    REQUIRE(result == testData);
    
    // Test with string method
    result = utoc::DecompressData(compressedData, "Zstd", testData.size());
    REQUIRE(result == testData);
    
    // Test with larger random data
    std::vector<uint8_t> largeData = generateRandomData(10000);
    std::vector<uint8_t> largeCompressedData = compressZstd(largeData);
    
    result = utoc::DecompressData(largeCompressedData, utoc::CompressionMethod::Zstd, largeData.size());
    REQUIRE(result == largeData);
}

TEST_CASE("LZ4 decompression", "[compression][lz4]") {
    // Test with small data
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    std::vector<uint8_t> compressedData = compressLZ4(testData);
    
    auto result = utoc::DecompressData(compressedData, utoc::CompressionMethod::LZ4, testData.size());
    REQUIRE(result == testData);
    
    // Test with string method
    result = utoc::DecompressData(compressedData, "LZ4", testData.size());
    REQUIRE(result == testData);
    
    // Test with larger random data
    std::vector<uint8_t> largeData = generateRandomData(10000);
    std::vector<uint8_t> largeCompressedData = compressLZ4(largeData);
    
    result = utoc::DecompressData(largeCompressedData, utoc::CompressionMethod::LZ4, largeData.size());
    REQUIRE(result == largeData);
}

TEST_CASE("Oodle decompression throws", "[compression][oodle]") {
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    
    REQUIRE_THROWS_AS(
        utoc::DecompressData(testData, utoc::CompressionMethod::Oodle, testData.size()),
        utoc::CompressionError
    );
    
    REQUIRE_THROWS_AS(
        utoc::DecompressData(testData, "Oodle", testData.size()),
        utoc::CompressionError
    );
}

TEST_CASE("Error handling", "[compression][errors]") {
    // Test with invalid compression method
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    
    REQUIRE_THROWS_AS(
        utoc::DecompressData(testData, "InvalidMethod", testData.size()),
        utoc::CompressionError
    );
    
    // Test with corrupted compressed data
    std::vector<uint8_t> compressedData = compressZlib(testData);
    // Corrupt the data by changing a byte
    if (!compressedData.empty()) {
        compressedData[0] = ~compressedData[0];
    }
    
    REQUIRE_THROWS_AS(
        utoc::DecompressData(compressedData, utoc::CompressionMethod::Zlib, testData.size()),
        utoc::CompressionError
    );
}
