#include <iostream>
#include <string>
#include <vector>
#include "utoc_reader.h"

int main() {
    // Since we're running from build/windows/x64/debug, we need to adjust the path
    const std::string exampleFilesPath = "../../../../example_files";
    
    // Hardcode the UTOC file path since we know it exists
    std::vector<std::string> utocFiles = {
        exampleFilesPath + "/000_BetterHUD_P.utoc"
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
        
        // Get header information
        const auto& header = reader.GetHeader();
        std::cout << "UTOC Version: " << static_cast<int>(header.version) << std::endl;
        std::cout << "Entry Count: " << header.toc_entry_count << std::endl;
        std::cout << "Compressed: " << (header.IsCompressed() ? "Yes" : "No") << std::endl;
        std::cout << "Encrypted: " << (header.IsEncrypted() ? "Yes" : "No") << std::endl;
        std::cout << "Signed: " << (header.IsSigned() ? "Yes" : "No") << std::endl;
        std::cout << "Indexed: " << (header.IsIndexed() ? "Yes" : "No") << std::endl;
        
        // Get all file paths
        std::vector<std::string> filePaths = reader.GetAllFilePaths();
        
        std::cout << "\nFiles in UTOC (" << filePaths.size() << " files):" << std::endl;
        for (const auto& filePath : filePaths) {
            std::cout << "  " << filePath << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    return 0;
}
