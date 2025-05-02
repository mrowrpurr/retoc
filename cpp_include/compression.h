#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

namespace utoc {

// Enumeration of supported compression methods
enum class CompressionMethod {
    None,
    Zlib,
    Zstd,
    LZ4,
    Oodle
};

// Convert string to CompressionMethod
CompressionMethod CompressionMethodFromString(const std::string& method);

// Convert CompressionMethod to string
std::string CompressionMethodToString(CompressionMethod method);

// Decompress data using the specified compression method
std::vector<uint8_t> DecompressData(
    const std::vector<uint8_t>& compressedData,
    CompressionMethod method,
    uint32_t uncompressedSize);

// Decompress data using the specified compression method (string version)
std::vector<uint8_t> DecompressData(
    const std::vector<uint8_t>& compressedData,
    const std::string& method,
    uint32_t uncompressedSize);

// Exception class for compression errors
class CompressionError : public std::runtime_error {
public:
    explicit CompressionError(const std::string& message) : std::runtime_error(message) {}
};

} // namespace utoc
