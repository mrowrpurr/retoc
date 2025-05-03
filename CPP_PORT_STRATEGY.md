# C++ Port Strategy for retoc and repak

This document outlines a strategy for porting the Ruby libraries retoc and repak to C++ static libraries for handling Unreal Engine file formats (.pak, .utoc, and .ucas).

## Overview

The goal is to create three separate C++ static libraries:
1. **pak** - For handling .pak files
2. **utoc** - For handling .utoc files
3. **ucas** - For handling .ucas files

Additionally, a general-purpose utility CLI (`pak_utoc_ucas.exe`) will be created to provide a unified interface for working with these file formats.

## Dependencies

### Common Dependencies

These dependencies are required by all three libraries:

1. **Compression Libraries**:
   - **zlib/gzip** - For Zlib and Gzip compression
   - **zstd** - For Zstandard compression
   - **lz4** - For LZ4 compression
   - **Oodle** (optional but recommended) - For Oodle compression

2. **Cryptography**:
   - **OpenSSL** - For AES-256 encryption/decryption and SHA-1 hashing
   - Alternative: **Botan** or **Crypto++** for a more C++-oriented API

3. **Hashing**:
   - **BLAKE3** - For chunk hashing in newer formats
   - **CityHash** - For package ID generation from names

4. **Utility Libraries**:
   - **std::filesystem** (C++17) or **boost::filesystem** - For file system operations
   - **fmt** or **std::format** (C++20) - For string formatting

### Library-Specific Dependencies

#### pak Library

- **FNV64** hash implementation for path hashing

#### utoc Library

- **Perfect hash** implementation for chunk lookup
- **Compact binary format** utilities for handling specialized data structures

#### ucas Library

- **File pooling** system for efficient parallel access

### Optional Dependencies

1. **Logging Framework**:
   - **spdlog** - For structured logging

2. **Threading Library**:
   - **std::thread** (C++11) - For basic threading
   - **Intel TBB** - For more advanced parallel processing

3. **Command Line Parsing** (for the CLI utility):
   - **CLI11** - Modern C++11 command line parser
   - **cxxopts** - Lightweight C++ command line option parser

## Library Architecture

### Dependency Analysis

After analyzing the file formats and their relationships, we can determine that:

1. **pak** library can be completely independent
2. **utoc** library depends on functionality from **ucas** library for reading chunks
3. **ucas** library depends on **utoc** library for metadata

This creates a circular dependency between utoc and ucas. To resolve this, we have two options:

1. **Option 1**: Merge utoc and ucas into a single library
2. **Option 2**: Extract common functionality into a shared core library

**Recommendation**: Use Option 2 - Create a core library with shared functionality and keep the three libraries separate for better modularity.

### Proposed Architecture

```
+----------------+      +----------------+      +----------------+
|      pak       |      |      utoc      |      |      ucas      |
+----------------+      +----------------+      +----------------+
         |                      |                      |
         v                      v                      v
+-------------------------------------------------------+
|                       core_utils                       |
+-------------------------------------------------------+
```

Where `core_utils` contains:
- Compression utilities
- Encryption utilities
- Hashing utilities
- File I/O utilities
- Common data structures

This approach allows each library to be used independently while sharing common functionality.

## Library Design

### pak Library

#### Key Classes

```cpp
namespace Pak {

// Main class for reading .pak files
class PakReader {
public:
    PakReader(const std::filesystem::path& path, const std::optional<AesKey>& key = std::nullopt);
    
    // File operations
    bool fileExists(const std::string& path) const;
    std::vector<std::string> getFileList() const;
    std::vector<uint8_t> extractFile(const std::string& path);
    Result<size_t> extractAllFiles(const std::filesystem::path& outputDir);
    
    // Metadata
    PakVersion getVersion() const;
    bool isEncrypted() const;
    std::string getMountPoint() const;
    
    // Advanced operations
    Result<FileInfo> getFileInfo(const std::string& path) const;
};

// Class for creating .pak files
class PakWriter {
public:
    PakWriter(const std::filesystem::path& path, PakVersion version, 
              std::string_view mountPoint, bool useEncryption = false);
    
    // File operations
    Result<void> addFile(const std::string& path, const std::vector<uint8_t>& data, 
                        CompressionMethod compressionMethod = CompressionMethod::Zlib);
    Result<void> addFileFromDisk(const std::string& path, const std::filesystem::path& sourcePath,
                                CompressionMethod compressionMethod = CompressionMethod::Zlib);
    
    // Finalization
    Result<void> finalize();
};

// Supporting classes
class AesKey {
public:
    AesKey(const std::array<uint8_t, 32>& key);
    AesKey(std::string_view hexOrBase64Key);
    
    const std::array<uint8_t, 32>& getKey() const;
};

enum class PakVersion {
    V1, V2, V3, V4, V5, V6, V7, V8A, V8B, V9, V10, V11
};

enum class CompressionMethod {
    None, Zlib, Gzip, Oodle, LZ4, Zstd
};

enum class Error {
    None, FileNotFound, IoError, InvalidFormat, EncryptionError, CompressionError
};

template<typename T>
class Result {
public:
    Result(T value);
    Result(Error error, std::string_view errorMessage);
    
    bool isError() const;
    Error error() const;
    std::string_view errorMessage() const;
    const T& value() const;
};

} // namespace Pak
```

#### Implementation Strategy

1. **File Format Handling**:
   - Implement support for all .pak versions (V1-V11)
   - Handle version-specific features like path hash index, encryption, etc.

2. **Compression**:
   - Create a unified compression interface for all supported methods
   - Implement block-based compression for efficient random access

3. **Encryption**:
   - Implement AES-256 ECB encryption/decryption
   - Support for encrypted indices and encrypted file data

4. **Indexing**:
   - Implement efficient index structures for file lookup
   - Support for path hash index in V10+

### utoc Library

#### Key Classes

```cpp
namespace Utoc {

// Main class for reading .utoc files
class UtocReader {
public:
    UtocReader(const std::filesystem::path& utocPath, 
              const std::optional<std::filesystem::path>& ucasPath = std::nullopt,
              const std::optional<AesKey>& key = std::nullopt);
    
    // Chunk operations
    bool chunkExists(const ChunkId& chunkId) const;
    std::vector<ChunkId> getChunkList() const;
    Result<std::vector<uint8_t>> extractChunk(const ChunkId& chunkId) const;
    Result<size_t> extractAllChunks(const std::filesystem::path& outputDir) const;
    
    // Metadata
    Version getVersion() const;
    bool isEncrypted() const;
    std::string getMountPoint() const;
    
    // Advanced operations
    Result<ChunkInfo> getChunkInfo(const ChunkId& chunkId) const;
    const ContainerHeader& getContainerHeader() const;
    std::unordered_map<std::string, ChunkId> getFilePathMap() const;
};

// Class for creating .utoc files
class UtocWriter {
public:
    UtocWriter(const std::filesystem::path& utocPath, 
              const std::optional<std::filesystem::path>& ucasPath = std::nullopt,
              Version version = Version::PerfectHashWithOverflow,
              std::string_view mountPoint = "/");
    
    // Chunk operations
    Result<void> addChunk(const ChunkId& chunkId, const std::vector<uint8_t>& data,
                         CompressionMethod compressionMethod = CompressionMethod::Zlib,
                         ChunkType type = ChunkType::Raw);
    Result<void> addChunkFromDisk(const ChunkId& chunkId, const std::filesystem::path& sourcePath,
                                 CompressionMethod compressionMethod = CompressionMethod::Zlib,
                                 ChunkType type = ChunkType::Raw);
    
    // Finalization
    Result<void> finalize();
};

// Supporting classes
class ChunkId {
public:
    ChunkId(uint64_t id);
    
    uint32_t getHash() const;
    uint8_t getIndex() const;
    uint8_t getType() const;
    uint16_t getFlags() const;
    
    static ChunkId create(uint32_t hash, uint8_t index, uint8_t type, uint16_t flags);
    std::string toString() const;
};

class AesKey {
public:
    AesKey(const std::array<uint8_t, 32>& key);
    AesKey(std::string_view hexOrBase64Key);
    
    const std::array<uint8_t, 32>& getKey() const;
};

enum class Version {
    Invalid, Initial, DirectoryIndex, PartitionSize, PerfectHash, 
    PerfectHashWithOverflow, OnDemandMetaData, RemovedOnDemandMetaData,
    ReplaceIoChunkHashWithIoHash
};

enum class CompressionMethod {
    None, Zlib, Gzip, Zstd, LZ4, Oodle
};

enum class ChunkType {
    Raw, ExportBundleData, BulkData, OptionalBulkData, MemoryMappedBulkData,
    ShaderCodeLibrary, ShaderCode, ContainerHeader
};

enum class Error {
    None, FileNotFound, IoError, InvalidFormat, EncryptionError, CompressionError
};

template<typename T>
class Result {
public:
    Result(T value);
    Result(Error error, std::string_view errorMessage);
    
    bool isError() const;
    Error error() const;
    std::string_view errorMessage() const;
    const T& value() const;
};

} // namespace Utoc
```

#### Implementation Strategy

1. **Chunk ID System**:
   - Implement the 12-byte chunk ID structure
   - Handle version-dependent chunk type mappings

2. **Perfect Hash System**:
   - Implement the perfect hash table for efficient chunk lookup
   - Handle overflow for chunks that can't be perfectly hashed

3. **Directory Index**:
   - Implement the hierarchical directory structure
   - Support for file path to chunk ID mapping

4. **Container Header**:
   - Parse and create container headers
   - Support different container header versions

### ucas Library

#### Key Classes

```cpp
namespace Ucas {

// Main class for reading .ucas files
class UcasReader {
public:
    UcasReader(const std::filesystem::path& ucasPath, 
              const Utoc::UtocReader& utocReader);
    
    // Chunk operations
    Result<std::vector<uint8_t>> readChunk(const Utoc::ChunkId& chunkId);
    Result<std::vector<uint8_t>> readChunkRaw(uint64_t offset, uint64_t size);
    
    // Partitioning
    bool isPartitioned() const;
    uint32_t getPartitionCount() const;
    uint64_t getPartitionSize() const;
};

// Class for creating .ucas files
class UcasWriter {
public:
    UcasWriter(const std::filesystem::path& ucasPath, 
              Utoc::UtocWriter& utocWriter,
              uint64_t partitionSize = std::numeric_limits<uint64_t>::max());
    
    // Chunk operations
    Result<void> writeChunk(const Utoc::ChunkId& chunkId, const std::vector<uint8_t>& data,
                          Utoc::CompressionMethod compressionMethod = Utoc::CompressionMethod::Zlib,
                          Utoc::ChunkType type = Utoc::ChunkType::Raw);
    
    // Finalization
    Result<void> finalize();
};

// Supporting classes
class FilePool {
public:
    FilePool(const std::filesystem::path& path, size_t maxHandles = 8);
    
    class FileHandle {
    public:
        std::ifstream* get();
    };
    
    FileHandle acquireFile();
};

enum class Error {
    None, FileNotFound, IoError, InvalidFormat, EncryptionError, CompressionError
};

template<typename T>
class Result {
public:
    Result(T value);
    Result(Error error, std::string_view errorMessage);
    
    bool isError() const;
    Error error() const;
    std::string_view errorMessage() const;
    const T& value() const;
};

} // namespace Ucas
```

#### Implementation Strategy

1. **File Pooling**:
   - Implement a thread-safe file pool for efficient parallel access
   - Handle partitioned files

2. **Chunk Reading**:
   - Read compressed and/or encrypted chunks
   - Handle blocks that span multiple partitions

3. **Compression**:
   - Implement block-based compression and decompression
   - Support all required compression methods

4. **Partitioning**:
   - Support for reading and writing partitioned files
   - Determine which partition contains a chunk

## General Purpose Utility CLI

The `pak_utoc_ucas.exe` utility will provide a unified interface for working with all three file formats.

### Command Structure

```
pak_utoc_ucas <command> [options]
```

### Commands

1. **pak**:
   - `list` - List files in a .pak file
   - `extract` - Extract files from a .pak file
   - `create` - Create a new .pak file
   - `info` - Display information about a .pak file

2. **utoc**:
   - `list` - List chunks in a .utoc file
   - `extract` - Extract chunks from a .utoc file
   - `create` - Create a new .utoc file
   - `info` - Display information about a .utoc file

3. **ucas**:
   - `extract` - Extract a chunk from a .ucas file
   - `create` - Create a new .ucas file
   - `info` - Display information about a .ucas file

4. **convert**:
   - `pak-to-utoc` - Convert a .pak file to .utoc/.ucas files
   - `utoc-to-pak` - Convert .utoc/.ucas files to a .pak file

### Implementation

```cpp
// Main CLI implementation
int main(int argc, char** argv) {
    CLI::App app{"Unreal Engine File Format Utility"};
    
    // Add commands
    auto* pakCmd = app.add_subcommand("pak", "Work with .pak files");
    auto* utocCmd = app.add_subcommand("utoc", "Work with .utoc files");
    auto* ucasCmd = app.add_subcommand("ucas", "Work with .ucas files");
    auto* convertCmd = app.add_subcommand("convert", "Convert between file formats");
    
    // Configure commands
    configurePakCommand(pakCmd);
    configureUtocCommand(utocCmd);
    configureUcasCommand(ucasCmd);
    configureConvertCommand(convertCmd);
    
    // Parse command line
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    
    // Execute command
    if (pakCmd->parsed()) {
        return executePakCommand(pakCmd);
    } else if (utocCmd->parsed()) {
        return executeUtocCommand(utocCmd);
    } else if (ucasCmd->parsed()) {
        return executeUcasCommand(ucasCmd);
    } else if (convertCmd->parsed()) {
        return executeConvertCommand(convertCmd);
    }
    
    // No command specified
    app.help();
    return 0;
}
```

## Implementation Challenges

### 1. Circular Dependencies

The circular dependency between utoc and ucas libraries needs to be carefully managed. The recommended approach is to extract common functionality into a core library, but this requires careful API design to avoid tight coupling.

### 2. Oodle Compression

Oodle compression is proprietary and requires a license from Epic Games/RAD Game Tools. The recommended approach is to implement a dynamic loading mechanism similar to the one used in retoc, which allows the library to use Oodle if available but fall back to other compression methods if not.

### 3. Perfect Hash Implementation

The perfect hash system used in .utoc files is complex and requires careful implementation. The recommended approach is to use a minimal perfect hash function generator or implement a custom solution based on the retoc implementation.

### 4. Memory Management

Efficient memory management is crucial for handling large files. The recommended approach is to use buffer pooling, memory mapping for large files, and careful management of temporary buffers.

### 5. Thread Safety

Ensuring thread safety for parallel operations is important for performance. The recommended approach is to use thread-safe data structures and careful synchronization, especially for the file pool.

## File Format-Specific Recommendations

### pak

1. **Version Compatibility**:
   - Implement support for all versions (V1-V11)
   - Use a factory pattern to create version-specific readers/writers

2. **Index Structure**:
   - Implement efficient index structures for file lookup
   - Support for path hash index in V10+

3. **Mount Points**:
   - Implement proper handling of mount points
   - Support for virtual path mapping

### utoc

1. **Chunk ID System**:
   - Implement the 12-byte chunk ID structure
   - Handle version-dependent chunk type mappings

2. **Perfect Hash System**:
   - Implement the perfect hash table for efficient chunk lookup
   - Handle overflow for chunks that can't be perfectly hashed

3. **Directory Index**:
   - Implement the hierarchical directory structure
   - Support for file path to chunk ID mapping

### ucas

1. **File Pooling**:
   - Implement a thread-safe file pool for efficient parallel access
   - Handle partitioned files

2. **Block-Based Reading**:
   - Implement efficient block-based reading
   - Handle compression and encryption at the block level

3. **Partitioning**:
   - Support for reading and writing partitioned files
   - Determine which partition contains a chunk

## Conclusion

Porting the Ruby libraries retoc and repak to C++ static libraries is a complex but manageable task. By following the recommendations in this document, you can create a set of libraries that provide full support for Unreal Engine's file formats while maintaining good performance and usability.

The proposed architecture with three separate libraries (pak, utoc, ucas) and a shared core library provides a good balance between modularity and avoiding circular dependencies. The general-purpose utility CLI provides a unified interface for working with all three file formats, making it easy for users to work with Unreal Engine's file formats.
