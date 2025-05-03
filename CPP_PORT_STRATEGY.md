# C++ Port Strategy for retoc and repak

## Introduction

This document outlines a strategy for porting the Ruby-based retoc and repak libraries to C++. The goal is to create three separate static libraries (`pak`, `utoc`, and `ucas`) along with a general-purpose utility CLI (`pak_utoc_ucas.exe`) that provides functionality for working with Unreal Engine's .pak, .utoc, and .ucas file formats.

## Current Implementation Status and Issues

The current C++ implementation has several limitations that need to be addressed:

1. **Incomplete File Format Parsing**: The current implementations of the libraries are mostly stubs/placeholders that don't properly parse the actual binary formats of .pak, .utoc, and .ucas files. This results in:
   - The binary not being able to read real .utoc files successfully
   - The .pak command crashing when trying to run with --help
   - Limited functionality for real-world use cases

2. **Missing Binary Format Implementation**: While the header files define comprehensive interfaces, the actual implementation of the binary format parsing and manipulation is incomplete. The libraries need to be updated to properly handle:
   - Reading and parsing the binary layouts according to the documentation
   - Proper error handling for malformed or corrupted files
   - Support for all versions of the file formats

3. **Stub Implementations**: Many methods return empty vectors or default values instead of actually processing the files. These need to be replaced with proper implementations that:
   - Read and parse the actual file formats
   - Handle compression and encryption correctly
   - Properly extract and manipulate the file contents

## Overall Architecture

The proposed architecture consists of:

1. **Three Static Libraries:**
   - `pak`: For working with .pak files
   - `utoc`: For working with .utoc files
   - `ucas`: For working with .ucas files

2. **Existing Library:**
   - `oodle_loader`: For Oodle compression support (already implemented)

3. **General Purpose Utility:**
   - `pak_utoc_ucas.exe`: A CLI tool that leverages the static libraries to provide comprehensive functionality for working with all three file formats

### Library Interdependencies

After analyzing the file formats and their relationships, here's the proposed dependency structure:

```
                  +----------------+
                  | pak_utoc_ucas  |
                  | (executable)   |
                  +----------------+
                          |
                          v
+----------------+  +----------------+  +----------------+
|      pak       |  |      utoc      |  |      ucas      |
| (static lib)   |  | (static lib)   |  | (static lib)   |
+----------------+  +----------------+  +----------------+
                          |                     ^
                          v                     |
                  +----------------+            |
                  |  oodle_loader  |------------+
                  | (static lib)   |
                  +----------------+
```

- `pak` can be independent of the other libraries
- `utoc` depends on `oodle_loader` for compression support
- `ucas` depends on `oodle_loader` for compression support
- `pak_utoc_ucas` depends on all three libraries

This structure avoids circular dependencies while allowing each library to focus on its specific file format.

## Common Dependencies and Utilities

All libraries will share some common dependencies and utilities:

1. **Standard Library Dependencies:**
   - `<vector>`, `<string>`, `<unordered_map>`, `<memory>`, etc.

2. **File I/O:**
   - `<fstream>` for file operations
   - Custom memory-mapped file implementation for efficient reading

3. **Compression Libraries:**
   - Zlib
   - Zstd
   - LZ4
   - Oodle (via `oodle_loader`)

4. **Cryptography:**
   - OpenSSL or Botan for AES encryption/decryption

5. **Common Utilities:**
   - Logging system
   - Error handling
   - Memory management utilities
   - Endianness conversion

## PAK Library

### Overview

The `pak` library will provide functionality for reading, writing, and manipulating .pak files. It will support all major .pak file versions from UE4.0 to UE5.3+.

### Dependencies

- **Standard Library:** `<vector>`, `<string>`, `<unordered_map>`, `<memory>`, etc.
- **File I/O:** `<fstream>`, memory-mapped file implementation
- **Compression:** Zlib, Zstd, LZ4, Oodle (via `oodle_loader`)
- **Cryptography:** OpenSSL or Botan for AES encryption/decryption
- **Hashing:** FNV-64 implementation for path hashing

### Key Classes and Functions

#### PakFile Class

```cpp
namespace Pak {

class PakFile {
    // Private members
    std::string _filePath;
    PakHeader _header;
    PakIndex _index;
    PakFooter _footer;
    std::vector<PakEntry> _entries;
    
public:
    // Constructors
    PakFile();
    explicit PakFile(const std::string& filePath);
    
    // Open/close operations
    bool open(const std::string& filePath);
    void close();
    
    // Information retrieval
    PakInfo getInfo() const;
    std::vector<std::string> listFiles() const;
    bool hasFile(const std::string& path) const;
    
    // File operations
    std::vector<uint8_t> extractFile(const std::string& path);
    bool extractFile(const std::string& path, const std::string& outputPath);
    bool extractAllFiles(const std::string& outputDir);
    
    // Creation operations
    bool create(const std::string& filePath, const std::string& mountPoint);
    bool addFile(const std::string& path, const std::vector<uint8_t>& data, CompressionMethod method = CompressionMethod::None);
    bool addFile(const std::string& path, const std::string& inputPath, CompressionMethod method = CompressionMethod::None);
    bool save();
    
    // Encryption operations
    bool setEncryptionKey(const std::string& key);
    bool isEncrypted() const;
};

} // namespace Pak
```

#### PakEntry Class

```cpp
namespace Pak {

class PakEntry {
    // Private members
    std::string _path;
    uint64_t _offset;
    uint64_t _compressedSize;
    uint64_t _uncompressedSize;
    CompressionMethod _compressionMethod;
    std::vector<uint8_t> _hash;
    bool _encrypted;
    std::vector<PakBlock> _blocks;
    
public:
    // Constructors
    PakEntry();
    PakEntry(const std::string& path, uint64_t offset, uint64_t compressedSize, uint64_t uncompressedSize);
    
    // Getters
    const std::string& getPath() const;
    uint64_t getOffset() const;
    uint64_t getCompressedSize() const;
    uint64_t getUncompressedSize() const;
    CompressionMethod getCompressionMethod() const;
    const std::vector<uint8_t>& getHash() const;
    bool isEncrypted() const;
    const std::vector<PakBlock>& getBlocks() const;
    
    // Setters
    void setPath(const std::string& path);
    void setOffset(uint64_t offset);
    void setCompressedSize(uint64_t size);
    void setUncompressedSize(uint64_t size);
    void setCompressionMethod(CompressionMethod method);
    void setHash(const std::vector<uint8_t>& hash);
    void setEncrypted(bool encrypted);
    void addBlock(const PakBlock& block);
};

} // namespace Pak
```

#### Utility Functions

```cpp
namespace Pak {

// Path hashing
uint64_t fnv64Hash(const std::string& path, uint64_t seed);

// Compression
std::vector<uint8_t> compressData(const std::vector<uint8_t>& data, CompressionMethod method);
std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data, CompressionMethod method, size_t uncompressedSize);

// Encryption
std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);

// Version detection
PakVersion detectVersion(const PakFooter& footer);

} // namespace Pak
```

## UTOC Library

### Overview

The `utoc` library will provide functionality for reading, writing, and manipulating .utoc files. It will support all major .utoc file versions from UE4.26 to UE5.5+.

### Dependencies

- **Standard Library:** `<vector>`, `<string>`, `<unordered_map>`, `<memory>`, etc.
- **File I/O:** `<fstream>`, memory-mapped file implementation
- **Compression:** Zlib, Zstd, LZ4, Oodle (via `oodle_loader`)
- **Cryptography:** OpenSSL or Botan for AES encryption/decryption
- **Hashing:** SHA-1 or SHA-256 for chunk hashing

### Key Classes and Functions

#### UtocFile Class

```cpp
namespace Utoc {

class UtocFile {
    // Private members
    std::string _filePath;
    UtocHeader _header;
    std::vector<ChunkId> _chunkIds;
    std::vector<OffsetAndLength> _offsetsAndLengths;
    std::vector<CompressionBlock> _compressionBlocks;
    std::vector<std::string> _compressionMethods;
    DirectoryIndex _directoryIndex;
    std::vector<ChunkMetadata> _chunkMetadata;
    
public:
    // Constructors
    UtocFile();
    explicit UtocFile(const std::string& filePath);
    
    // Open/close operations
    bool open(const std::string& filePath);
    void close();
    
    // Information retrieval
    UtocInfo getInfo() const;
    std::vector<std::string> listFiles() const;
    bool hasFile(const std::string& path) const;
    
    // Chunk operations
    ChunkId getChunkId(const std::string& path) const;
    OffsetAndLength getOffsetAndLength(const ChunkId& chunkId) const;
    CompressionBlock getCompressionBlock(uint32_t blockIndex) const;
    
    // Directory operations
    const DirectoryIndex& getDirectoryIndex() const;
    
    // Creation operations
    bool create(const std::string& filePath, UtocVersion version = UtocVersion::Latest);
    bool addChunk(const ChunkId& chunkId, const OffsetAndLength& offsetAndLength, const ChunkMetadata& metadata);
    bool addFile(const std::string& path, const ChunkId& chunkId);
    bool save();
    
    // Encryption operations
    bool setEncryptionKey(const std::string& key);
    bool isEncrypted() const;
};

} // namespace Utoc
```

#### ChunkId Class

```cpp
namespace Utoc {

class ChunkId {
    // Private members
    uint64_t _id;
    uint16_t _index;
    uint8_t _type;
    uint8_t _flags;
    
public:
    // Constructors
    ChunkId();
    ChunkId(uint64_t id, uint16_t index, ChunkType type);
    
    // Getters
    uint64_t getId() const;
    uint16_t getIndex() const;
    ChunkType getType() const;
    bool isNew() const;
    bool hasVersion() const;
    
    // Setters
    void setId(uint64_t id);
    void setIndex(uint16_t index);
    void setType(ChunkType type);
    void setNew(bool isNew);
    void setHasVersion(bool hasVersion);
    
    // Conversion
    uint8_t* toBytes(uint8_t* buffer) const;
    static ChunkId fromBytes(const uint8_t* buffer);
};

} // namespace Utoc
```

#### DirectoryIndex Class

```cpp
namespace Utoc {

class DirectoryIndex {
    // Private members
    std::string _mountPoint;
    std::vector<DirectoryEntry> _directories;
    std::vector<FileEntry> _files;
    std::vector<std::string> _stringTable;
    
public:
    // Constructors
    DirectoryIndex();
    
    // Getters
    const std::string& getMountPoint() const;
    const std::vector<DirectoryEntry>& getDirectories() const;
    const std::vector<FileEntry>& getFiles() const;
    const std::vector<std::string>& getStringTable() const;
    
    // Operations
    void addDirectory(const DirectoryEntry& directory);
    void addFile(const FileEntry& file);
    void addString(const std::string& str);
    
    // Lookup
    ChunkId findChunkId(const std::string& path) const;
    std::vector<std::string> listFiles() const;
};

} // namespace Utoc
```

#### Utility Functions

```cpp
namespace Utoc {

// Chunk ID operations
ChunkId createChunkId(uint64_t packageId, uint16_t chunkIndex, ChunkType type);
bool isValidChunkId(const ChunkId& chunkId);

// Perfect hash operations
uint32_t computePerfectHash(const ChunkId& chunkId, const std::vector<uint32_t>& seeds);
std::vector<uint32_t> generatePerfectHashSeeds(const std::vector<ChunkId>& chunkIds);

// Compression
std::vector<uint8_t> compressData(const std::vector<uint8_t>& data, CompressionMethod method);
std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data, CompressionMethod method, size_t uncompressedSize);

// Encryption
std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);

// Version detection
UtocVersion detectVersion(const UtocHeader& header);

} // namespace Utoc
```

## UCAS Library

### Overview

The `ucas` library will provide functionality for reading, writing, and manipulating .ucas files. It will work in conjunction with the `utoc` library to provide a complete solution for working with IoStore containers.

### Dependencies

- **Standard Library:** `<vector>`, `<string>`, `<unordered_map>`, `<memory>`, etc.
- **File I/O:** `<fstream>`, memory-mapped file implementation
- **Compression:** Zlib, Zstd, LZ4, Oodle (via `oodle_loader`)
- **Cryptography:** OpenSSL or Botan for AES encryption/decryption

### Key Classes and Functions

#### UcasFile Class

```cpp
namespace Ucas {

class UcasFile {
    // Private members
    std::string _filePath;
    std::vector<std::string> _partitionPaths;
    std::vector<std::fstream> _partitionStreams;
    uint64_t _partitionSize;
    
public:
    // Constructors
    UcasFile();
    explicit UcasFile(const std::string& filePath);
    
    // Open/close operations
    bool open(const std::string& filePath);
    void close();
    
    // Partition operations
    bool openPartition(uint32_t partitionIndex);
    void closePartition(uint32_t partitionIndex);
    uint32_t getPartitionCount() const;
    uint64_t getPartitionSize() const;
    
    // Data operations
    std::vector<uint8_t> readData(uint64_t offset, uint64_t size, uint32_t partitionIndex = 0);
    bool writeData(uint64_t offset, const std::vector<uint8_t>& data, uint32_t partitionIndex = 0);
    
    // Creation operations
    bool create(const std::string& filePath, uint64_t partitionSize = 0);
    bool createPartition(uint32_t partitionIndex);
    bool save();
};

} // namespace Ucas
```

#### DataBlock Class

```cpp
namespace Ucas {

class DataBlock {
    // Private members
    uint64_t _offset;
    uint64_t _compressedSize;
    uint64_t _uncompressedSize;
    CompressionMethod _compressionMethod;
    bool _encrypted;
    
public:
    // Constructors
    DataBlock();
    DataBlock(uint64_t offset, uint64_t compressedSize, uint64_t uncompressedSize, CompressionMethod method = CompressionMethod::None);
    
    // Getters
    uint64_t getOffset() const;
    uint64_t getCompressedSize() const;
    uint64_t getUncompressedSize() const;
    CompressionMethod getCompressionMethod() const;
    bool isEncrypted() const;
    
    // Setters
    void setOffset(uint64_t offset);
    void setCompressedSize(uint64_t size);
    void setUncompressedSize(uint64_t size);
    void setCompressionMethod(CompressionMethod method);
    void setEncrypted(bool encrypted);
};

} // namespace Ucas
```

#### Utility Functions

```cpp
namespace Ucas {

// Partition operations
uint32_t calculatePartitionIndex(uint64_t offset, uint64_t partitionSize);
uint64_t calculatePartitionOffset(uint64_t offset, uint64_t partitionSize);

// Compression
std::vector<uint8_t> compressData(const std::vector<uint8_t>& data, CompressionMethod method);
std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data, CompressionMethod method, size_t uncompressedSize);

// Encryption
std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);

// Block alignment
uint64_t alignToBlockSize(uint64_t offset, uint64_t blockSize);

} // namespace Ucas
```

## General Purpose Utility (pak_utoc_ucas.exe)

### Overview

The `pak_utoc_ucas.exe` utility will provide a command-line interface for working with .pak, .utoc, and .ucas files. It will leverage the functionality provided by the three static libraries to offer a comprehensive solution for Unreal Engine asset manipulation.

### Dependencies

- **All Three Static Libraries:** `pak`, `utoc`, `ucas`
- **Command-Line Parsing:** CLI11 or similar library
- **Logging:** spdlog or similar library
- **Progress Reporting:** Custom implementation or third-party library

### Command Structure

The utility will use a subcommand structure:

```
pak_utoc_ucas [OPTIONS] SUBCOMMAND

OPTIONS:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit

SUBCOMMANDS:
  pak                         PAK file operations
  utoc                        UTOC file operations
  ucas                        UCAS file operations
```

Each subcommand will have its own set of operations:

```
pak_utoc_ucas pak [OPTIONS] SUBCOMMAND

SUBCOMMANDS:
  info                        Display information about a PAK file
  list                        List files in a PAK file
  extract                     Extract files from a PAK file
  create                      Create a new PAK file
  add                         Add files to a PAK file
```

```
pak_utoc_ucas utoc [OPTIONS] SUBCOMMAND

SUBCOMMANDS:
  info                        Display information about a UTOC file
  list                        List files in a UTOC file
  extract                     Extract chunks from a UTOC/UCAS file pair
  create                      Create a new UTOC file
  add                         Add chunks to a UTOC file
```

```
pak_utoc_ucas ucas [OPTIONS] SUBCOMMAND

SUBCOMMANDS:
  create                      Create a new UCAS file
  add                         Add data to a UCAS file
```

### Implementation

The utility will be implemented as a thin wrapper around the static libraries, with each subcommand delegating to the appropriate library functions.

```cpp
// Main entry point
int main(int argc, char** argv) {
    CLI::App app{"Unreal Engine PAK/UTOC/UCAS File Utility"};
    
    // Version flag
    app.set_version_flag("--version", "1.0.0");
    
    // PAK subcommand
    auto* pakCmd = app.add_subcommand("pak", "PAK file operations");
    setupPakCommands(pakCmd);
    
    // UTOC subcommand
    auto* utocCmd = app.add_subcommand("utoc", "UTOC file operations");
    setupUtocCommands(utocCmd);
    
    // UCAS subcommand
    auto* ucasCmd = app.add_subcommand("ucas", "UCAS file operations");
    setupUcasCommands(ucasCmd);
    
    // Require a subcommand
    app.require_subcommand(1);
    
    // Parse command line
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    
    return 0;
}
```

## Dependencies

### Required External Libraries

1. **Compression Libraries:**
   - Zlib: For zlib and gzip compression
   - Zstd: For Zstandard compression
   - LZ4: For LZ4 compression
   - Oodle: Via the existing `oodle_loader` library

2. **Cryptography:**
   - OpenSSL or Botan: For AES encryption/decryption
   - SHA-1 or SHA-256 hashing

3. **Command-Line Parsing:**
   - CLI11: Modern C++11 command-line parser

4. **Logging:**
   - spdlog: Fast C++ logging library

### Optional Dependencies

1. **Memory Mapping:**
   - Custom implementation or platform-specific APIs

2. **Progress Reporting:**
   - Custom implementation or third-party library

3. **Testing:**
   - Catch2 or Google Test for unit testing

## Implementation Strategy

### Phase 1: Core Infrastructure (Completed)

1. ✅ Set up the project structure with xmake
2. ✅ Implement common utilities and dependencies
3. ✅ Create basic class skeletons for all three libraries
4. ✅ Implement file I/O and memory mapping

### Phase 2: PAK Library (Needs Completion)

1. **Binary Format Parsing**: Implement proper parsing of the PAK file format according to the binary layout documentation
   - Parse the footer to determine the version and index location
   - Read and parse the index according to the version-specific format
   - Handle different versions of the PAK format (V2-V11)
   - Implement proper error handling for malformed or corrupted files

2. **Reading Support**:
   - Implement proper file entry lookup using the index
   - Add support for reading compressed and encrypted data
   - Handle different compression methods (Zlib, Gzip, Zstd, LZ4, Oodle)
   - Implement proper AES decryption for encrypted files

3. **Writing Support**:
   - Implement proper index creation and writing
   - Add support for compressing and encrypting data
   - Handle different compression methods
   - Implement proper AES encryption for encrypted files

4. **Testing**:
   - Test with various PAK file versions from different Unreal Engine games
   - Verify compatibility with the official UnrealPak tool
   - Test edge cases like large files, many small files, etc.

### Phase 3: UTOC Library (Needs Completion)

1. **Binary Format Parsing**: Implement proper parsing of the UTOC file format according to the binary layout documentation
   - Parse the header to determine the version and container information
   - Read and parse the chunk IDs, offsets, and lengths
   - Handle the perfect hash table for chunk lookup
   - Parse the compression blocks and methods
   - Read and parse the directory index
   - Handle different versions of the UTOC format

2. **Reading Support**:
   - Implement proper chunk lookup using the perfect hash table
   - Add support for reading the directory index
   - Handle different compression methods
   - Implement proper AES decryption for encrypted files

3. **Writing Support**:
   - Implement proper header and chunk table creation
   - Generate perfect hash seeds for chunk lookup
   - Create and write the directory index
   - Handle different compression methods
   - Implement proper AES encryption for encrypted files

4. **Testing**:
   - Test with various UTOC file versions from different Unreal Engine games
   - Verify compatibility with the Unreal Engine IoStore system
   - Test edge cases like large chunks, many small chunks, etc.

### Phase 4: UCAS Library (Needs Completion)

1. **Binary Format Handling**: Implement proper handling of the UCAS file format
   - Handle reading and writing of data blocks
   - Support for partitioned UCAS files
   - Implement proper alignment for encrypted blocks

2. **Reading Support**:
   - Implement proper chunk data reading based on offsets from UTOC
   - Add support for decompressing chunk data
   - Handle different compression methods
   - Implement proper AES decryption for encrypted chunks

3. **Writing Support**:
   - Implement proper chunk data writing
   - Add support for compressing chunk data
   - Handle different compression methods
   - Implement proper AES encryption for encrypted chunks
   - Support for creating partitioned UCAS files

4. **Testing**:
   - Test with various UCAS file versions from different Unreal Engine games
   - Verify compatibility with the Unreal Engine IoStore system
   - Test edge cases like large chunks, many small chunks, etc.

### Phase 5: Command-Line Utility (Needs Improvement)

1. **Robust Command-Line Parsing**:
   - Fix issues with the current implementation
   - Ensure proper error handling for invalid arguments
   - Add comprehensive help messages for all commands

2. **PAK Subcommands**:
   - Fix the current issues with the PAK subcommands
   - Implement proper error handling
   - Add support for all PAK operations (info, list, extract, create, add)

3. **UTOC Subcommands**:
   - Improve the current implementation of UTOC subcommands
   - Implement proper error handling
   - Add support for all UTOC operations (info, list, extract, create, add)

4. **UCAS Subcommands**:
   - Improve the current implementation of UCAS subcommands
   - Implement proper error handling
   - Add support for all UCAS operations (create, add)

5. **Progress Reporting and Logging**:
   - Implement proper progress reporting for long-running operations
   - Add comprehensive logging for debugging and information
   - Handle errors gracefully with informative error messages

6. **Testing**:
   - Test all commands with various file formats and versions
   - Verify compatibility with real-world game files
   - Test edge cases and error conditions

### Phase 6: Integration and Testing (Final Phase)

1. **Integration**:
   - Ensure all libraries work together correctly
   - Verify that the command-line utility properly uses the libraries
   - Check for any circular dependencies or integration issues

2. **Real-World Testing**:
   - Test with real-world game files from various Unreal Engine versions
   - Verify compatibility with the original Ruby implementations
   - Test with large files and complex directory structures

3. **Performance Optimization**:
   - Profile the code to identify bottlenecks
   - Optimize critical paths for better performance
   - Implement memory-efficient algorithms for large files

4. **Bug Fixing**:
   - Address any issues or bugs found during testing
   - Fix edge cases and error conditions
   - Ensure robust error handling throughout the codebase

5. **Documentation**:
   - Document the API and usage of all libraries
   - Provide examples for common use cases
   - Create comprehensive documentation for the command-line utility

## Conclusion

The proposed strategy for porting the Ruby-based retoc and repak libraries to C++ involves creating three separate static libraries (`pak`, `utoc`, and `ucas`) along with a general-purpose utility CLI (`pak_utoc_ucas.exe`). This approach allows for modular development and usage while avoiding circular dependencies.

Each library will focus on its specific file format, with the `pak_utoc_ucas.exe` utility providing a unified interface for working with all three formats. The implementation will leverage modern C++ features and external libraries for compression, encryption, and other functionality.

By following this strategy, the C++ port will maintain the functionality of the original Ruby libraries while providing improved performance and integration capabilities for C++ applications.
