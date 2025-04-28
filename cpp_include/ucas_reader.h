#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <optional>
#include "utoc_reader.h"

namespace utoc {

// Forward declarations
class UcasReader;

// Main UCAS reader class
class UcasReader {
public:
    UcasReader() = default;
    ~UcasReader() = default;

    // Disable copy
    UcasReader(const UcasReader&) = delete;
    UcasReader& operator=(const UcasReader&) = delete;

    // Enable move
    UcasReader(UcasReader&&) = default;
    UcasReader& operator=(UcasReader&&) = default;

    // Open a UCAS file
    bool Open(const std::filesystem::path& path);

    // Read a chunk from the UCAS file
    std::vector<uint8_t> ReadChunk(const FIoOffsetAndLength& offsetLength, 
                                  const std::vector<FIoStoreTocCompressedBlockEntry>& compressionBlocks,
                                  const std::vector<std::string>& compressionMethods,
                                  bool isCompressed);

    // Read raw data from the UCAS file at a specific offset and size
    std::vector<uint8_t> ReadRawData(uint64_t offset, uint64_t size);

    // Decompress data using the specified compression method
    std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& compressedData, 
                                       const std::string& compressionMethod,
                                       uint32_t uncompressedSize);

private:
    std::filesystem::path file_path_;
    std::ifstream file_;
};

} // namespace utoc
