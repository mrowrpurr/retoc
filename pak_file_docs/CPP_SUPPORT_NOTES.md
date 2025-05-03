# C++ Implementation Notes for .pak File Support

## Overview

This document provides guidance for implementing a C++ static library with full support for Unreal Engine's .pak file format. It covers required dependencies, implementation challenges, architectural considerations, and provides pseudocode examples for key components.

## Dependencies

### Core Dependencies

1. **Cryptography Library** - For AES-256 encryption/decryption:
   - **OpenSSL** (`libssl`, `libcrypto`) - Comprehensive but large
   - **Botan** - Modern C++ crypto library, more lightweight than OpenSSL
   - **Tiny AES** - Minimal implementation if you only need AES-256 ECB

2. **Compression Libraries** - For supporting all compression methods:
   - **zlib** - For Zlib and Gzip compression
   - **zstd** - For Zstandard compression
   - **lz4** - For LZ4 compression
   - **Oodle** (optional) - Proprietary compression from RAD Game Tools/Epic

3. **Hashing Library** - For SHA-1 hashing used in index verification:
   - Part of OpenSSL or Botan if you're already using them
   - **picosha2** - Header-only SHA-1 implementation
   - **Crypto++** - Comprehensive but larger

### Optional Dependencies

1. **String Encoding** - For proper UTF-16/UTF-8 handling:
   - **ICU** - Comprehensive Unicode support
   - **utf8cpp** - Lightweight UTF-8 handling
   - C++11's `std::codecvt` (deprecated in C++17)

2. **Endianness Handling** - For cross-platform compatibility:
   - **Boost.Endian** - If you're already using Boost
   - Custom implementation using compiler-specific intrinsics

3. **Command Line Parsing** (if building a CLI tool):
   - **CLI11** - Modern C++11 command line parser
   - **cxxopts** - Lightweight C++ command line option parser
   - **argparse** - Argument Parser for Modern C++

## Implementation Challenges

### 1. Oodle Compression Support

Oodle compression is proprietary and requires a license from Epic Games/RAD Game Tools. Approaches to handle this:

```cpp
// Option 1: Dynamic loading with optional support
class OodleCompression {
private:
    void* libraryHandle = nullptr;
    // Function pointers to Oodle API
    typedef int (*OodleCompressFunc)(void* buffer, int bufferSize, void* output, int level);
    OodleCompressFunc oodleCompress = nullptr;

public:
    bool initialize() {
        // Try to load the Oodle library dynamically
        #ifdef _WIN32
        libraryHandle = LoadLibrary("oo2core_9_win64.dll");
        #else
        libraryHandle = dlopen("liboo2core_9_linux64.so", RTLD_LAZY);
        #endif

        if (!libraryHandle) {
            return false;
        }

        // Get function pointers
        #ifdef _WIN32
        oodleCompress = (OodleCompressFunc)GetProcAddress(libraryHandle, "OodleLZ_Compress");
        #else
        oodleCompress = (OodleCompressFunc)dlsym(libraryHandle, "OodleLZ_Compress");
        #endif

        return oodleCompress != nullptr;
    }

    bool isAvailable() const {
        return oodleCompress != nullptr;
    }

    // Compression methods...
};

// Option 2: Compile-time optional support with feature flag
#ifdef ENABLE_OODLE_COMPRESSION
#include "oodle/oodle.h"
// Oodle implementation
#endif
```

### 2. Cross-Platform Compatibility

Ensuring consistent behavior across different platforms:

```cpp
// Endianness handling
template<typename T>
T swapEndian(T value) {
    static_assert(std::is_integral<T>::value, "Integral required.");
    
    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        #if defined(_MSC_VER)
        return _byteswap_ushort(value);
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(value);
        #else
        return (value >> 8) | (value << 8);
        #endif
    } else if constexpr (sizeof(T) == 4) {
        #if defined(_MSC_VER)
        return _byteswap_ulong(value);
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(value);
        #else
        return ((value & 0xFF) << 24) | 
               ((value & 0xFF00) << 8) | 
               ((value & 0xFF0000) >> 8) | 
               ((value >> 24) & 0xFF);
        #endif
    } else if constexpr (sizeof(T) == 8) {
        #if defined(_MSC_VER)
        return _byteswap_uint64(value);
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(value);
        #else
        // Manual implementation for 64-bit
        #endif
    }
}

// Platform-specific file handling
class FileIO {
public:
    static std::vector<uint8_t> readFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + path);
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            throw std::runtime_error("Failed to read file: " + path);
        }
        
        return buffer;
    }
    
    // Other file operations...
};
```

### 3. Memory Management

Efficient handling of large files and indices:

```cpp
// Memory-mapped file access for large files
class MemoryMappedFile {
private:
    #ifdef _WIN32
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    HANDLE mappingHandle = NULL;
    #else
    int fileDescriptor = -1;
    #endif
    void* mappedData = nullptr;
    size_t fileSize = 0;

public:
    MemoryMappedFile(const std::string& path) {
        // Platform-specific memory mapping implementation
        #ifdef _WIN32
        // Windows implementation using CreateFileA, CreateFileMapping, MapViewOfFile
        #else
        // POSIX implementation using open, mmap
        #endif
    }
    
    ~MemoryMappedFile() {
        // Cleanup
    }
    
    const uint8_t* data() const {
        return static_cast<const uint8_t*>(mappedData);
    }
    
    size_t size() const {
        return fileSize;
    }
};
```

### 4. Version Compatibility

Handling different .pak file versions:

```cpp
enum class PakVersion {
    V1, V2, V3, V4, V5, V6, V7, V8A, V8B, V9, V10, V11
};

class PakReader {
private:
    PakVersion version;
    
    // Version-specific implementations
    void readIndexV1to9(std::istream& stream);
    void readIndexV10Plus(std::istream& stream);
    
public:
    PakReader(const std::string& path) {
        // Detect version and initialize accordingly
        std::ifstream file(path, std::ios::binary);
        
        // Read footer to determine version
        file.seekg(-4, std::ios::end); // Position to version field
        uint32_t versionValue;
        file.read(reinterpret_cast<char*>(&versionValue), sizeof(versionValue));
        
        // Map version value to enum
        version = mapVersionValue(versionValue);
        
        // Initialize based on version
        if (version >= PakVersion::V10) {
            readIndexV10Plus(file);
        } else {
            readIndexV1to9(file);
        }
    }
    
    // Version-specific operations...
};
```

## Architectural Considerations

### 1. Class Structure

A well-designed class hierarchy for .pak file handling:

```cpp
// Core classes
class PakFile {
public:
    // Factory method to create appropriate reader/writer
    static std::unique_ptr<PakReader> createReader(const std::string& path);
    static std::unique_ptr<PakWriter> createWriter(const std::string& path, PakVersion version);
};

class PakReader {
public:
    virtual ~PakReader() = default;
    virtual std::vector<std::string> getFileList() const = 0;
    virtual bool fileExists(const std::string& path) const = 0;
    virtual std::vector<uint8_t> extractFile(const std::string& path) = 0;
    virtual PakVersion getVersion() const = 0;
    virtual bool isEncrypted() const = 0;
    
    // Factory method for version-specific implementation
    static std::unique_ptr<PakReader> create(const std::string& path);
};

class PakWriter {
public:
    virtual ~PakWriter() = default;
    virtual void addFile(const std::string& path, const std::vector<uint8_t>& data, 
                        bool compress = true) = 0;
    virtual void finalize() = 0;
    
    // Factory method for version-specific implementation
    static std::unique_ptr<PakWriter> create(const std::string& path, 
                                           PakVersion version, 
                                           const std::string& mountPoint);
};

// Version-specific implementations
class PakReaderV11 : public PakReader {
    // Implementation for V11
};

class PakWriterV11 : public PakWriter {
    // Implementation for V11
};
```

### 2. Feature Flags

Using compile-time feature flags to control library size and dependencies:

```cpp
// In CMakeLists.txt or build configuration
option(PAK_ENABLE_ENCRYPTION "Enable encryption support" ON)
option(PAK_ENABLE_OODLE "Enable Oodle compression support" OFF)
option(PAK_ENABLE_ZSTD "Enable Zstd compression support" ON)
option(PAK_ENABLE_LZ4 "Enable LZ4 compression support" ON)

// In code
#ifdef PAK_ENABLE_ENCRYPTION
// Encryption implementation
#endif

#ifdef PAK_ENABLE_OODLE
// Oodle compression implementation
#endif
```

### 3. Error Handling

Robust error handling strategy:

```cpp
// Error handling with exceptions
class PakException : public std::runtime_error {
public:
    enum class ErrorCode {
        FileNotFound,
        InvalidFormat,
        EncryptionError,
        CompressionError,
        VersionNotSupported,
        // ...
    };
    
    PakException(ErrorCode code, const std::string& message)
        : std::runtime_error(message), errorCode(code) {}
    
    ErrorCode getErrorCode() const { return errorCode; }
    
private:
    ErrorCode errorCode;
};

// Usage
try {
    auto reader = PakFile::createReader("game.pak");
    auto data = reader->extractFile("Content/Textures/texture.png");
} catch (const PakException& e) {
    if (e.getErrorCode() == PakException::ErrorCode::EncryptionError) {
        // Handle encryption error
    } else {
        // Handle other errors
    }
}
```

### 4. Thread Safety

Considerations for multi-threaded access:

```cpp
class ThreadSafePakReader {
private:
    std::unique_ptr<PakReader> reader;
    mutable std::mutex mutex;
    
public:
    ThreadSafePakReader(const std::string& path)
        : reader(PakFile::createReader(path)) {}
    
    std::vector<uint8_t> extractFile(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex);
        return reader->extractFile(path);
    }
    
    // Other thread-safe methods...
};

// Parallel extraction
void extractFilesParallel(PakReader& reader, const std::vector<std::string>& files) {
    std::vector<std::future<void>> futures;
    std::mutex outputMutex;
    
    for (const auto& file : files) {
        futures.push_back(std::async(std::launch::async, [&reader, &file, &outputMutex]() {
            auto data = reader->extractFile(file);
            
            // Thread-safe output
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                std::ofstream outFile(file, std::ios::binary);
                outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
            }
        }));
    }
    
    // Wait for all extractions to complete
    for (auto& future : futures) {
        future.wait();
    }
}
```

## Implementation Strategy

### 1. Core Components

Key components to implement:

1. **PakHeader/Footer** - Structures for the file header and footer
2. **PakEntry** - Structure for file entries
3. **PakIndex** - Class for managing the file index
4. **PakReader** - Class for reading .pak files
5. **PakWriter** - Class for creating .pak files
6. **Compression** - Wrapper for compression algorithms
7. **Encryption** - Wrapper for encryption algorithms
8. **PathHashIndex** - Implementation of the path hash index (V10+)
9. **DirectoryIndex** - Implementation of the directory index (V10+)

### 2. Phased Implementation Approach

A suggested implementation order:

1. **Phase 1**: Basic file structure and reading
   - Implement footer parsing
   - Implement basic index parsing
   - Support for reading uncompressed, unencrypted files

2. **Phase 2**: Compression support
   - Implement Zlib/Gzip compression
   - Add LZ4 and Zstd support
   - Optional Oodle support

3. **Phase 3**: Encryption support
   - Implement AES-256 ECB encryption/decryption
   - Support for encrypted indices
   - Support for encrypted file data

4. **Phase 4**: Advanced features
   - Path hash index implementation
   - Full directory index implementation
   - Support for all version-specific features

5. **Phase 5**: Optimization and robustness
   - Performance optimization
   - Memory usage optimization
   - Error handling improvements
   - Cross-platform testing

### 3. Testing Strategy

Comprehensive testing approach:

1. **Unit Tests** - Test individual components
   - Test compression/decompression
   - Test encryption/decryption
   - Test path hashing
   - Test index parsing

2. **Integration Tests** - Test complete workflows
   - Test reading various .pak file versions
   - Test creating .pak files
   - Test extracting files

3. **Compatibility Tests** - Test with real-world files
   - Test with .pak files from different Unreal Engine versions
   - Test with encrypted and compressed files
   - Test with large files

4. **Performance Tests** - Measure and optimize performance
   - Benchmark file extraction
   - Benchmark index parsing
   - Compare with UnrealPak and repak

## Example Implementation Snippets

### 1. Reading a .pak File

```cpp
// Basic .pak file reader
class PakReader {
private:
    struct Footer {
        std::optional<std::array<uint8_t, 16>> encryptionGuid;
        bool encryptedIndex = false;
        uint32_t magic;
        uint32_t version;
        uint64_t indexOffset;
        uint64_t indexSize;
        std::array<uint8_t, 20> indexHash;
        bool frozenIndex = false;
        std::vector<std::string> compressionMethods;
    };
    
    struct Entry {
        uint64_t offset;
        uint64_t compressedSize;
        uint64_t uncompressedSize;
        uint32_t compressionMethod;
        std::array<uint8_t, 20> hash;
        std::vector<std::pair<uint64_t, uint64_t>> blocks;
        uint8_t flags;
        uint32_t compressionBlockSize;
        
        bool isEncrypted() const { return (flags & 1) != 0; }
        bool isDeleted() const { return (flags & 2) != 0; }
    };
    
    std::ifstream file;
    Footer footer;
    std::unordered_map<std::string, Entry> entries;
    std::string mountPoint;
    
    void readFooter() {
        // Seek to end of file minus potential footer size
        file.seekg(0, std::ios::end);
        auto fileSize = file.tellg();
        
        // Try different footer sizes based on version
        std::vector<int64_t> footerSizes = {36, 37, 53, 181, 213, 214};
        
        for (auto size : footerSizes) {
            if (fileSize < size) continue;
            
            file.seekg(fileSize - size);
            
            // Read potential footer
            if (size >= 53) {
                // V7+ has encryption GUID
                footer.encryptionGuid = std::array<uint8_t, 16>();
                file.read(reinterpret_cast<char*>(footer.encryptionGuid->data()), 16);
            }
            
            if (size >= 37) {
                // V4+ has encrypted index flag
                uint8_t encrypted;
                file.read(reinterpret_cast<char*>(&encrypted), 1);
                footer.encryptedIndex = (encrypted != 0);
            }
            
            // Read magic number
            file.read(reinterpret_cast<char*>(&footer.magic), 4);
            
            // Check if valid magic number
            if (footer.magic == 0x5A6F12E1) {
                // Valid footer found, read remaining fields
                file.read(reinterpret_cast<char*>(&footer.version), 4);
                file.read(reinterpret_cast<char*>(&footer.indexOffset), 8);
                file.read(reinterpret_cast<char*>(&footer.indexSize), 8);
                file.read(reinterpret_cast<char*>(footer.indexHash.data()), 20);
                
                if (footer.version == 9) {
                    // V9 has frozen index flag
                    uint8_t frozen;
                    file.read(reinterpret_cast<char*>(&frozen), 1);
                    footer.frozenIndex = (frozen != 0);
                }
                
                if (footer.version >= 8) {
                    // V8+ has compression method names
                    int compressionCount = (footer.version == 8 && size < 213) ? 4 : 5;
                    footer.compressionMethods.resize(compressionCount);
                    
                    for (int i = 0; i < compressionCount; i++) {
                        char methodName[32] = {0};
                        file.read(methodName, 32);
                        footer.compressionMethods[i] = methodName;
                    }
                }
                
                break;
            }
        }
        
        if (footer.magic != 0x5A6F12E1) {
            throw PakException(PakException::ErrorCode::InvalidFormat, 
                              "Invalid .pak file: Magic number not found");
        }
    }
    
    void readIndex() {
        // Seek to index
        file.seekg(footer.indexOffset);
        
        // Read index data
        std::vector<uint8_t> indexData(footer.indexSize);
        file.read(reinterpret_cast<char*>(indexData.data()), footer.indexSize);
        
        // Decrypt index if needed
        if (footer.encryptedIndex) {
            // Decrypt index data
            // ...
        }
        
        // Parse index
        std::istringstream indexStream(std::string(
            reinterpret_cast<char*>(indexData.data()), 
            indexData.size()));
        
        // Read mount point
        // ...
        
        // Read entries
        // ...
    }
    
public:
    PakReader(const std::string& path) : file(path, std::ios::binary) {
        if (!file) {
            throw PakException(PakException::ErrorCode::FileNotFound, 
                              "Failed to open .pak file: " + path);
        }
        
        readFooter();
        readIndex();
    }
    
    std::vector<std::string> getFileList() const {
        std::vector<std::string> files;
        files.reserve(entries.size());
        
        for (const auto& entry : entries) {
            files.push_back(entry.first);
        }
        
        return files;
    }
    
    bool fileExists(const std::string& path) const {
        return entries.find(path) != entries.end();
    }
    
    std::vector<uint8_t> extractFile(const std::string& path) {
        auto it = entries.find(path);
        if (it == entries.end()) {
            throw PakException(PakException::ErrorCode::FileNotFound, 
                              "File not found in .pak: " + path);
        }
        
        const auto& entry = it->second;
        
        // Seek to file data
        file.seekg(entry.offset);
        
        // Read file data
        std::vector<uint8_t> data(entry.compressedSize);
        file.read(reinterpret_cast<char*>(data.data()), entry.compressedSize);
        
        // Decrypt if needed
        if (entry.isEncrypted()) {
            // Decrypt data
            // ...
        }
        
        // Decompress if needed
        if (entry.compressionMethod != 0) {
            // Decompress data
            // ...
        }
        
        return data;
    }
};
```

### 2. Creating a .pak File

```cpp
// Basic .pak file writer
class PakWriter {
private:
    struct Entry {
        std::string path;
        uint64_t offset;
        uint64_t compressedSize;
        uint64_t uncompressedSize;
        uint32_t compressionMethod;
        std::array<uint8_t, 20> hash;
        std::vector<std::pair<uint64_t, uint64_t>> blocks;
        uint8_t flags;
        uint32_t compressionBlockSize;
    };
    
    std::ofstream file;
    std::vector<Entry> entries;
    std::string mountPoint;
    uint32_t version;
    bool useEncryption;
    std::vector<std::string> compressionMethods;
    
public:
    PakWriter(const std::string& path, uint32_t version, 
             const std::string& mountPoint, bool useEncryption = false)
        : file(path, std::ios::binary), 
          mountPoint(mountPoint), 
          version(version),
          useEncryption(useEncryption) {
        if (!file) {
            throw PakException(PakException::ErrorCode::FileNotFound, 
                              "Failed to create .pak file: " + path);
        }
        
        // Initialize compression methods based on version
        if (version >= 8) {
            compressionMethods = {"Zlib", "Gzip", "Oodle", "LZ4", "Zstd"};
            if (version == 8 && compressionMethods.size() > 4) {
                compressionMethods.resize(4);
            }
        }
    }
    
    void addFile(const std::string& path, const std::vector<uint8_t>& data, 
                bool compress = true) {
        Entry entry;
        entry.path = path;
        entry.uncompressedSize = data.size();
        
        // Calculate hash
        // ...
        
        // Compress if requested
        std::vector<uint8_t> processedData;
        if (compress) {
            // Compress data
            // ...
            entry.compressedSize = processedData.size();
            entry.compressionMethod = 1; // Zlib
        } else {
            processedData = data;
            entry.compressedSize = data.size();
            entry.compressionMethod = 0; // None
        }
        
        // Encrypt if requested
        if (useEncryption) {
            // Encrypt data
            // ...
            entry.flags |= 1; // Set encrypted flag
        }
        
        // Write file data
        entry.offset = file.tellp();
        file.write(reinterpret_cast<const char*>(processedData.data()), 
                  processedData.size());
        
        // Add entry to index
        entries.push_back(entry);
    }
    
    void finalize() {
        // Write index
        uint64_t indexOffset = file.tellp();
        
        // Write mount point
        // ...
        
        // Write entries
        // ...
        
        uint64_t indexSize = file.tellp() - indexOffset;
        
        // Calculate index hash
        std::array<uint8_t, 20> indexHash = {0};
        // ...
        
        // Write footer
        if (version >= 7) {
            // Write encryption GUID (zeros if not using encryption)
            std::array<uint8_t, 16> guid = {0};
            file.write(reinterpret_cast<const char*>(guid.data()), guid.size());
        }
        
        if (version >= 4) {
            // Write encrypted index flag
            uint8_t encryptedIndex = 0;
            file.write(reinterpret_cast<const char*>(&encryptedIndex), 1);
        }
        
        // Write magic number
        uint32_t magic = 0x5A6F12E1;
        file.write(reinterpret_cast<const char*>(&magic), 4);
        
        // Write version
        file.write(reinterpret_cast<const char*>(&version), 4);
        
        // Write index offset and size
        file.write(reinterpret_cast<const char*>(&indexOffset), 8);
        file.write(reinterpret_cast<const char*>(&indexSize), 8);
        
        // Write index hash
        file.write(reinterpret_cast<const char*>(indexHash.data()), indexHash.size());
        
        if (version == 9) {
            // Write frozen index flag
            uint8_t frozenIndex = 0;
            file.write(reinterpret_cast<const char*>(&frozenIndex), 1);
        }
        
        if (version >= 8) {
            // Write compression method names
            for (const auto& method : compressionMethods) {
                char methodName[32] = {0};
                std::strncpy(methodName, method.c_str(), 31);
                file.write(methodName, 32);
            }
        }
        
        file.close();
    }
};
```

## Performance Considerations

### 1. Memory Usage

Strategies for efficient memory usage:

- Use memory-mapped files for large .pak files
- Stream file data instead of loading everything into memory
- Use move semantics to avoid unnecessary copies
- Consider custom allocators for large buffers

### 2. Parallelization

Opportunities for parallelization:

- Parallel decompression of multiple files
- Parallel processing of index entries
- Multi-threaded .pak creation
- Thread pool for handling multiple requests

### 3. Caching

Caching strategies for improved performance:

- Cache frequently accessed index entries
- Cache decompressed file data
- LRU cache for file contents
- Cache path hash lookups

## Conclusion

Implementing a C++ static library for .pak file support is a substantial but manageable project. By understanding the file format details, carefully managing dependencies, and addressing the implementation challenges outlined in this document, you can create a robust and efficient library that provides full support for Unreal Engine's .pak file format.

The phased implementation approach allows for incremental development and testing, while the architectural considerations ensure a well-designed and maintainable codebase. The example implementation snippets provide a starting point for key components, and the performance considerations help ensure the library meets the demands of game development scenarios.
