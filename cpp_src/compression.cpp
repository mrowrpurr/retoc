#include "compression.h"
#include <algorithm>
#include <cctype>
#include <iostream>

// Include compression libraries
#include <zlib.h>
#include <zstd.h>
#include <lz4.h>

namespace utoc {

CompressionMethod CompressionMethodFromString(const std::string& method) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerMethod = method;
    std::transform(lowerMethod.begin(), lowerMethod.end(), lowerMethod.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    if (lowerMethod.empty() || lowerMethod == "none") {
        return CompressionMethod::None;
    } else if (lowerMethod == "zlib") {
        return CompressionMethod::Zlib;
    } else if (lowerMethod == "zstd") {
        return CompressionMethod::Zstd;
    } else if (lowerMethod == "lz4") {
        return CompressionMethod::LZ4;
    } else if (lowerMethod == "oodle") {
        return CompressionMethod::Oodle;
    } else {
        throw CompressionError("Unknown compression method: " + method);
    }
}

std::string CompressionMethodToString(CompressionMethod method) {
    switch (method) {
        case CompressionMethod::None:
            return "None";
        case CompressionMethod::Zlib:
            return "Zlib";
        case CompressionMethod::Zstd:
            return "Zstd";
        case CompressionMethod::LZ4:
            return "LZ4";
        case CompressionMethod::Oodle:
            return "Oodle";
        default:
            throw CompressionError("Invalid compression method enum value");
    }
}

std::vector<uint8_t> DecompressData(
    const std::vector<uint8_t>& compressedData,
    const std::string& method,
    uint32_t uncompressedSize) {
    return DecompressData(compressedData, CompressionMethodFromString(method), uncompressedSize);
}

std::vector<uint8_t> DecompressData(
    const std::vector<uint8_t>& compressedData,
    CompressionMethod method,
    uint32_t uncompressedSize) {
    
    // If no compression or empty method, return the data as is
    if (method == CompressionMethod::None) {
        return compressedData;
    }
    
    // Prepare output buffer
    std::vector<uint8_t> decompressedData(uncompressedSize);
    
    switch (method) {
        case CompressionMethod::Zlib: {
            // Zlib decompression
            uLongf destLen = uncompressedSize;
            int result = uncompress(
                decompressedData.data(),
                &destLen,
                compressedData.data(),
                compressedData.size()
            );
            
            if (result != Z_OK) {
                std::string errorMsg = "Zlib decompression failed with error code: " + std::to_string(result);
                throw CompressionError(errorMsg);
            }
            
            if (destLen != uncompressedSize) {
                std::cerr << "Warning: Zlib decompression produced " << destLen 
                          << " bytes, expected " << uncompressedSize << " bytes" << std::endl;
                decompressedData.resize(destLen);
            }
            break;
        }
        
        case CompressionMethod::Zstd: {
            // Zstd decompression
            size_t result = ZSTD_decompress(
                decompressedData.data(),
                uncompressedSize,
                compressedData.data(),
                compressedData.size()
            );
            
            if (ZSTD_isError(result)) {
                std::string errorMsg = "Zstd decompression failed: " + std::string(ZSTD_getErrorName(result));
                throw CompressionError(errorMsg);
            }
            
            if (result != uncompressedSize) {
                std::cerr << "Warning: Zstd decompression produced " << result 
                          << " bytes, expected " << uncompressedSize << " bytes" << std::endl;
                decompressedData.resize(result);
            }
            break;
        }
        
        case CompressionMethod::LZ4: {
            // LZ4 decompression
            int result = LZ4_decompress_safe(
                reinterpret_cast<const char*>(compressedData.data()),
                reinterpret_cast<char*>(decompressedData.data()),
                compressedData.size(),
                uncompressedSize
            );
            
            if (result < 0) {
                std::string errorMsg = "LZ4 decompression failed with error code: " + std::to_string(result);
                throw CompressionError(errorMsg);
            }
            
            if (static_cast<uint32_t>(result) != uncompressedSize) {
                std::cerr << "Warning: LZ4 decompression produced " << result 
                          << " bytes, expected " << uncompressedSize << " bytes" << std::endl;
                decompressedData.resize(result);
            }
            break;
        }
        
        case CompressionMethod::Oodle: {
            // Oodle decompression (not implemented yet)
            throw CompressionError("Oodle decompression not implemented yet");
        }
        
        default:
            throw CompressionError("Unknown compression method");
    }
    
    return decompressedData;
}

} // namespace utoc
