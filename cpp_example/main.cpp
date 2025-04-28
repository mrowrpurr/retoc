#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "utoc_reader.h"
#include "ucas_reader.h"

// Helper function to print a hex dump of binary data
void printHexDump(const std::vector<uint8_t>& data, size_t maxBytes = 256) {
    const size_t bytesToShow = std::min(data.size(), maxBytes);
    
    std::cout << "Data size: " << data.size() << " bytes" << std::endl;
    std::cout << "Hex dump (first " << bytesToShow << " bytes):" << std::endl;
    
    for (size_t i = 0; i < bytesToShow; i += 16) {
        // Print offset
        std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
        
        // Print hex values
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < bytesToShow) {
                std::cout << std::setw(2) << std::setfill('0') << std::hex 
                          << static_cast<int>(data[i + j]) << " ";
            } else {
                std::cout << "   ";
            }
            
            // Add extra space in the middle
            if (j == 7) {
                std::cout << " ";
            }
        }
        
        // Print ASCII representation
        std::cout << " | ";
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < bytesToShow) {
                char c = static_cast<char>(data[i + j]);
                std::cout << (c >= 32 && c <= 126 ? c : '.');
            } else {
                std::cout << " ";
            }
        }
        
        std::cout << std::endl;
    }
    
    // Reset to decimal output
    std::cout << std::dec;
    
    if (data.size() > maxBytes) {
        std::cout << "... (" << (data.size() - maxBytes) << " more bytes)" << std::endl;
    }
    
    std::cout << std::endl;
}

// Helper function to save binary data to a file
bool saveToFile(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << path.string() << std::endl;
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

int main() {
    // Since we're running from build/windows/x64/debug, we need to adjust the path
    const std::filesystem::path exampleFilesPath = "../../../../example_files";
    const std::filesystem::path outputPath = "../../../../output";
    
    // Create output directory if it doesn't exist
    std::filesystem::create_directories(outputPath);
    
    // Hardcode the UTOC file path since we know it exists
    std::vector<std::filesystem::path> utocFiles = {
        exampleFilesPath / "000_BetterHUD_P.utoc"
    };
    
    std::cout << "Processing UTOC files from: " << exampleFilesPath << std::endl;
    
    // Process each .utoc file
    for (const auto& utocFile : utocFiles) {
        std::cout << "\nProcessing UTOC file: " << utocFile << std::endl;
        
        utoc::UtocReader reader;
        if (!reader.Open(utocFile)) {
            std::cerr << "Failed to open UTOC file: " << utocFile << std::endl;
            continue;
        }
        
        // Open the corresponding UCAS file
        if (!reader.OpenUcas()) {
            std::cerr << "Failed to open UCAS file for: " << utocFile << std::endl;
            continue;
        }
        
        // Get header information
        const auto& header = reader.GetHeader();
        std::cout << "UTOC Version: " << static_cast<int>(header.version) << std::endl;
        std::cout << "Entry Count: " << header.toc_entry_count << std::endl;
        std::cout << "Compressed: " << (header.IsCompressed() ? "Yes" : "No") << std::endl;
        std::cout << "Encrypted: " << (header.IsEncrypted() ? "Yes" : "No") << std::endl;
        std::cout << "Signed: " << (header.IsSigned() ? "Yes" : "No") << std::endl;
        std::cout << "Indexed: " << (header.IsIndexed() ? "Yes" : "No") << std::endl;
        
        // Get compression methods
        const auto& compressionMethods = reader.GetCompressionMethods();
        std::cout << "\nCompression Methods:" << std::endl;
        for (size_t i = 0; i < compressionMethods.size(); ++i) {
            std::cout << "  " << i << ": " << compressionMethods[i] << std::endl;
        }
        
        // Get all file paths
        std::vector<std::string> filePaths = reader.GetAllFilePaths();
        
        std::cout << "\nFiles in UTOC (" << filePaths.size() << " files):" << std::endl;
        
        // Limit the number of files to process to avoid excessive output
        const size_t maxFilesToProcess = 5;
        size_t processedFiles = 0;
        
        for (const auto& filePath : filePaths) {
            std::cout << "  " << filePath << std::endl;
            
            // Try to read the chunk data for this file
            try {
                if (processedFiles < maxFilesToProcess) {
                    std::cout << "\nReading chunk data for: " << filePath << std::endl;
                    
                    // Read the chunk data
                    std::vector<uint8_t> chunkData = reader.ReadChunkByPath(filePath);
                    
                    // Print a hex dump of the data
                    printHexDump(chunkData);
                    
                    // Save the chunk data to a file
                    std::filesystem::path outputFilePath = outputPath / std::filesystem::path(filePath).filename();
                    if (saveToFile(outputFilePath, chunkData)) {
                        std::cout << "Saved chunk data to: " << outputFilePath << std::endl;
                    }
                    
                    processedFiles++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error reading chunk data: " << e.what() << std::endl;
            }
        }
        
        // Read a specific chunk by index
        if (header.toc_entry_count > 0) {
            std::cout << "\nReading first chunk by index (0):" << std::endl;
            try {
                std::vector<uint8_t> chunkData = reader.ReadChunkByIndex(0);
                printHexDump(chunkData);
            } catch (const std::exception& e) {
                std::cerr << "Error reading chunk by index: " << e.what() << std::endl;
            }
        }
        
        std::cout << std::endl;
    }
    
    return 0;
}
