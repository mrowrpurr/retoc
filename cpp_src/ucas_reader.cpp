#include "ucas_reader.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace utoc {

bool UcasReader::Open(const std::filesystem::path& path) {
    file_path_ = path;
    
    // Close any previously opened file
    if (file_.is_open()) {
        file_.close();
    }
    
    // Open the file in binary mode
    file_.open(path, std::ios::binary);
    if (!file_) {
        std::cerr << "Failed to open UCAS file: " << path.string() << std::endl;
        return false;
    }
    
    return true;
}

std::vector<uint8_t> UcasReader::ReadRawData(uint64_t offset, uint64_t size) {
    if (!file_.is_open()) {
        throw std::runtime_error("UCAS file is not open");
    }
    
    // Check for unreasonable size
    const uint64_t MAX_REASONABLE_SIZE = 100 * 1024 * 1024; // 100MB
    if (size > MAX_REASONABLE_SIZE) {
        throw std::runtime_error("Requested size is too large: " + std::to_string(size) + " bytes");
    }
    
    // Get file size
    std::streampos currentPos = file_.tellg();
    file_.seekg(0, std::ios::end);
    std::streampos fileSize = file_.tellg();
    
    // Check if offset is within file bounds
    if (offset >= static_cast<uint64_t>(fileSize)) {
        throw std::runtime_error("Offset is beyond end of file: " + std::to_string(offset) + " >= " + std::to_string(fileSize));
    }
    
    // Check if size is within file bounds
    if (offset + size > static_cast<uint64_t>(fileSize)) {
        throw std::runtime_error("Requested data extends beyond end of file");
    }
    
    // Seek to the offset
    file_.seekg(offset, std::ios::beg);
    if (!file_) {
        throw std::runtime_error("Failed to seek to offset in UCAS file");
    }
    
    // Read the data
    std::vector<uint8_t> data(size);
    file_.read(reinterpret_cast<char*>(data.data()), size);
    
    // Check if we read the expected number of bytes
    if (file_.gcount() != static_cast<std::streamsize>(size)) {
        throw std::runtime_error("Failed to read expected number of bytes from UCAS file");
    }
    
    // Restore original position
    file_.seekg(currentPos);
    
    return data;
}

std::vector<uint8_t> UcasReader::ReadChunk(const FIoOffsetAndLength& offsetLength, 
                                         const std::vector<FIoStoreTocCompressedBlockEntry>& compressionBlocks,
                                         const std::vector<std::string>& compressionMethods,
                                         bool isCompressed) {
    if (!file_.is_open()) {
        throw std::runtime_error("UCAS file is not open");
    }
    
    uint64_t offset = offsetLength.GetOffset();
    uint64_t length = offsetLength.GetLength();
    
    // Check for unreasonable chunk size
    const uint64_t MAX_REASONABLE_CHUNK_SIZE = 100 * 1024 * 1024; // 100MB
    if (length > MAX_REASONABLE_CHUNK_SIZE) {
        throw std::runtime_error("Chunk size is too large: " + std::to_string(length) + " bytes");
    }
    
    if (!isCompressed) {
        // If the chunk is not compressed, simply read the raw data
        return ReadRawData(offset, length);
    } else {
        // If the chunk is compressed, we need to read and decompress each block
        std::vector<uint8_t> decompressedData;
        decompressedData.reserve(length); // Reserve space for the decompressed data
        
        // Check if there are any compression blocks
        if (compressionBlocks.empty()) {
            // If there are no compression blocks, treat it as uncompressed
            std::cerr << "Warning: Compressed chunk has no compression blocks, treating as uncompressed" << std::endl;
            return ReadRawData(offset, length);
        }
        
        // Check if there are any compression methods
        if (compressionMethods.empty()) {
            // If there are no compression methods, treat it as uncompressed
            std::cerr << "Warning: Compressed chunk has no compression methods, treating as uncompressed" << std::endl;
            return ReadRawData(offset, length);
        }
        
        for (const auto& block : compressionBlocks) {
            // Get block information
            uint64_t blockOffset = block.GetOffset();
            uint32_t compressedSize = block.GetCompressedSize();
            uint32_t uncompressedSize = block.GetUncompressedSize();
            uint8_t compressionMethodIndex = block.GetCompressionMethodIndex();
            
            // Check for unreasonable block sizes
            const uint32_t MAX_REASONABLE_BLOCK_SIZE = 10 * 1024 * 1024; // 10MB
            if (compressedSize > MAX_REASONABLE_BLOCK_SIZE || uncompressedSize > MAX_REASONABLE_BLOCK_SIZE) {
                throw std::runtime_error("Block size is too large: compressed=" + 
                                        std::to_string(compressedSize) + 
                                        ", uncompressed=" + 
                                        std::to_string(uncompressedSize));
            }
            
            // Check if the compression method index is valid
            if (compressionMethodIndex >= compressionMethods.size()) {
                throw std::runtime_error("Invalid compression method index: " + 
                                        std::to_string(compressionMethodIndex) + 
                                        " >= " + 
                                        std::to_string(compressionMethods.size()));
            }
            
            // Get the compression method
            const std::string& compressionMethod = compressionMethods[compressionMethodIndex];
            
            try {
                // Read the compressed data
                std::vector<uint8_t> compressedData = ReadRawData(blockOffset, compressedSize);
                
                // Decompress the data
                std::vector<uint8_t> blockDecompressedData = DecompressData(compressedData, compressionMethod, uncompressedSize);
                
                // Append the decompressed data to the result
                decompressedData.insert(decompressedData.end(), blockDecompressedData.begin(), blockDecompressedData.end());
            } catch (const std::exception& e) {
                throw std::runtime_error("Error processing compression block: " + std::string(e.what()));
            }
        }
        
        return decompressedData;
    }
}

std::vector<uint8_t> UcasReader::DecompressData(const std::vector<uint8_t>& compressedData, 
                                              const std::string& compressionMethod,
                                              uint32_t uncompressedSize) {
    // For now, we'll just implement a simple pass-through for uncompressed data
    // In a real implementation, you would need to handle different compression methods
    
    if (compressionMethod == "None" || compressionMethod.empty()) {
        // No compression, return the data as is
        return compressedData;
    } else if (compressionMethod == "Zlib") {
        // TODO: Implement Zlib decompression
        std::cerr << "Zlib decompression not implemented yet" << std::endl;
    } else if (compressionMethod == "Gzip") {
        // TODO: Implement Gzip decompression
        std::cerr << "Gzip decompression not implemented yet" << std::endl;
    } else if (compressionMethod == "Oodle") {
        // TODO: Implement Oodle decompression
        std::cerr << "Oodle decompression not implemented yet" << std::endl;
    } else {
        std::cerr << "Unknown compression method: " << compressionMethod << std::endl;
    }
    
    // For now, just return an empty vector for unsupported compression methods
    return std::vector<uint8_t>(uncompressedSize);
}

} // namespace utoc
