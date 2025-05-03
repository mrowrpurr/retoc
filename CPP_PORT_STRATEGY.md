# C++ Port Strategy for retoc and repak

This document outlines a strategy for porting the Ruby libraries retoc and repak to C++ static libraries. The goal is to create three separate C++ static libraries (`pak`, `utoc`, and `ucas`) along with a general-purpose utility CLI (`pak_utoc_ucas.exe`) for working with these file formats.

## Table of Contents

1. [Overview](#overview)
2. [Library Architecture](#library-architecture)
3. [Dependencies](#dependencies)
4. [PAK Library](#pak-library)
5. [UTOC Library](#utoc-library)
6. [UCAS Library](#ucas-library)
7. [General Purpose Utility](#general-purpose-utility)
8. [Implementation Strategy](#implementation-strategy)
9. [Testing Strategy](#testing-strategy)
10. [Conclusion](#conclusion)

## Overview

The current Ruby implementations (retoc and repak) provide functionality for working with Unreal Engine's file formats:
- **repak**: Handles .pak files (Legacy assets)
- **retoc**: Handles .utoc/.ucas files (IoStore containers) and provides conversion between Legacy and Zen assets

The C++ port will reorganize this functionality into three static libraries:
- **pak**: For working with .pak files
- **utoc**: For working with .utoc files
- **ucas**: For working with .ucas files

Additionally, a general-purpose utility CLI (`pak_utoc_ucas.exe`) will provide a unified interface for working with all three file formats.

## Library Architecture

After analyzing the dependencies and relationships between the file formats, I recommend the following architecture:

```
+-------------------+      +-------------------+      +-------------------+
|                   |      |                   |      |                   |
|  pak (static lib) |      | utoc (static lib) |<---->| ucas (static lib) |
|                   |      |                   |      |                   |
+-------------------+      +-------------------+      +-------------------+
          ^                        ^                          ^
          |                        |                          |
          |                        |                          |
          v                        v                          v
+---------------------------------------------------------------+
|                                                               |
|                pak_utoc_ucas.exe (CLI utility)                |
|                                                               |
+---------------------------------------------------------------+
```

### Separation vs. Integration

Based on the analysis of the file formats and their relationships:

- **pak** can be completely separate from utoc/ucas, as it uses a different file format and doesn't directly interact with .utoc/.ucas files.
- **utoc** and **ucas** are tightly coupled, as .utoc files contain metadata for .ucas files. However, they can still be separated into distinct libraries with well-defined interfaces between them.

This architecture allows for:
1. Independent development and testing of each library
2. Flexibility in using only the libraries needed for specific tasks
3. Clear separation of concerns between different file formats
4. Easier maintenance and updates for each file format

## Dependencies

### Common Dependencies

These dependencies are required by all three libraries:

1. **Compression Libraries**:
   - **zlib**: For Zlib/Gzip compression
   - **zstd**: For Zstandard compression
   - **lz4**: For LZ4 compression
   - **Oodle** (optional): For Oodle compression (requires licensing from Epic Games/RAD Game Tools)

2. **Cryptography**:
   - **OpenSSL** or equivalent: For AES-256 encryption/decryption
   - **SHA-1/SHA-256**: For hash verification

3. **Utility Libraries**:
   - **fmt** or **std::format** (C++20): For string formatting
   - **std::filesystem** (C++17) or **boost::filesystem**: For file system operations

### Library-Specific Dependencies

#### pak Library
- **FNV-64 Hash**: For path hashing in .pak files
- **CityHash64**: For package ID generation

#### utoc Library
- **BLAKE3**: For chunk hashing in newer versions
- **Perfect Hash**: For efficient chunk lookup

#### ucas Library
- No additional specific dependencies beyond the common ones

## PAK Library

### Core Functionality

The `pak` library should provide the following functionality:

1. **Reading Operations**:
   - Parse .pak file headers and footers
   - Read file entries and indices
   - Extract files from .pak archives
   - Support for all major .pak file versions (V2-V11)
   - Handle encrypted indices and data

2. **Writing Operations**:
   - Create new .pak files
   - Add files to existing .pak files
   - Support various compression methods
   - Generate proper indices and metadata

### Key Components

```cpp
// Key classes for the pak library
class PakReader {
public:
    PakReader(const std::string& path, const std::optional<AesKey>& key = std::nullopt);
    
    // File operations
    bool fileExists(const std::string& path) const;
    std::vector<std::string> getFileList() const;
    std::vector<uint8_t> extractFile(const std::string& path);
    
    // Metadata operations
    uint32_t getVersion() const;
    bool isEncrypted() const;
    std::string getMountPoint() const;
    
    // Advanced operations
    void extractAllFiles(const std::string& outputDir);
    std::vector<FileEntry> getEntries() const;
};

class PakWriter {
public:
    PakWriter(const std::string& path, uint32_t version, const std::string& mountPoint);
    
    // File operations
    void addFile(const std::string& path, const std::vector<uint8_t>& data, 
                CompressionMethod compression = CompressionMethod::Zlib);
    void addFileFromDisk(const std::string& sourcePath, const std::string& destPath,
                        CompressionMethod compression = CompressionMethod::Zlib);
    
    // Finalization
    void finalize();
};

// Supporting structures
struct PakEntry {
    std::string path;
    uint64_t offset;
    uint64_t compressedSize;
    uint64_t uncompressedSize;
    uint32_t compressionMethod;
    std::array<uint8_t, 20> hash;
    std::vector<CompressionBlock> blocks;
    uint8_t flags;
    uint32_t compressionBlockSize;
};

enum class CompressionMethod {
    None,
    Zlib,
    Gzip,
    Zstd,
    LZ4,
    Oodle
};
```

### Dependencies

The `pak` library requires:
- All common dependencies
- FNV-64 hash implementation for path hashing
- Support for various .pak file versions

## UTOC Library

### Core Functionality

The `utoc` library should provide the following functionality:

1. **Reading Operations**:
   - Parse .utoc file headers
   - Read chunk IDs, offsets, and lengths
   - Read compression blocks and methods
   - Read directory index
   - Support for all major .utoc file versions

2. **Writing Operations**:
   - Create new .utoc files
   - Add chunks to .utoc files
   - Generate proper directory indices
   - Support various compression methods

3. **Utility Operations**:
   - Extract manifest from .utoc files
   - Show container information
   - List files in the directory index

### Key Components

```cpp
// Key classes for the utoc library
class UtocReader {
public:
    UtocReader(const std::string& path, const std::optional<AesKey>& key = std::nullopt);
    
    // Chunk operations
    bool hasChunk(const ChunkId& chunkId) const;
    ChunkInfo getChunkInfo(const ChunkId& chunkId) const;
    std::vector<ChunkId> getAllChunks() const;
    
    // Directory operations
    std::vector<std::string> getFileList() const;
    std::optional<ChunkId> getChunkIdForPath(const std::string& path) const;
    
    // Metadata operations
    EIoStoreTocVersion getVersion() const;
    bool isEncrypted() const;
    std::string getMountPoint() const;
    
    // Container header operations
    std::optional<ContainerHeader> getContainerHeader() const;
};

class UtocWriter {
public:
    UtocWriter(const std::string& path, EIoStoreTocVersion version, 
              const std::string& mountPoint);
    
    // Chunk operations
    void addChunk(const ChunkId& chunkId, const ChunkInfo& chunkInfo);
    void addChunkWithPath(const ChunkId& chunkId, const std::string& path, 
                         const ChunkInfo& chunkInfo);
    
    // Directory operations
    void addDirectoryEntry(const std::string& path);
    
    // Container header operations
    void setContainerHeader(const ContainerHeader& header);
    
    // Finalization
    void finalize();
};

// Supporting structures
struct ChunkId {
    uint64_t id;
    uint16_t index;
    uint8_t type;
    uint8_t flags;
    
    static ChunkId create(uint64_t id, uint16_t index, EIoChunkType type);
};

struct ChunkInfo {
    uint64_t offset;
    uint64_t size;
    CompressionMethod compressionMethod;
    std::vector<CompressionBlock> blocks;
    bool encrypted;
};

enum class EIoStoreTocVersion {
    Invalid,
    Initial,
    DirectoryIndex,
    PartitionSize,
    PerfectHash,
    PerfectHashWithOverflow,
    OnDemandMetaData,
    RemovedOnDemandMetaData,
    ReplaceIoChunkHashWithIoHash
};
```

### Dependencies

The `utoc` library requires:
- All common dependencies
- BLAKE3 for chunk hashing
- Perfect hash implementation for chunk lookup
- Interface with the `ucas` library for reading/writing chunk data

## UCAS Library

### Core Functionality

The `ucas` library should provide the following functionality:

1. **Reading Operations**:
   - Read chunk data from .ucas files
   - Handle compressed and encrypted data
   - Support for partitioned .ucas files

2. **Writing Operations**:
   - Write chunk data to .ucas files
   - Support various compression methods
   - Generate proper partitioning if needed

### Key Components

```cpp
// Key classes for the ucas library
class UcasReader {
public:
    UcasReader(const std::string& path, uint64_t partitionSize = 0, 
              uint32_t partitionCount = 0);
    
    // Chunk operations
    std::vector<uint8_t> readChunk(uint64_t offset, uint64_t size, 
                                 const std::vector<CompressionBlock>& blocks,
                                 const std::optional<AesKey>& key = std::nullopt);
    
    // Partition operations
    int32_t getPartitionIndex(uint64_t offset) const;
};

class UcasWriter {
public:
    UcasWriter(const std::string& path, uint64_t partitionSize = 0);
    
    // Chunk operations
    ChunkInfo writeChunk(const std::vector<uint8_t>& data, 
                       CompressionMethod compressionMethod = CompressionMethod::Zlib,
                       uint32_t compressionBlockSize = 0x10000,
                       const std::optional<AesKey>& key = std::nullopt);
    
    // Finalization
    void finalize();
    
    // Partition information
    uint32_t getPartitionCount() const;
};

// Supporting structures
struct CompressionBlock {
    uint64_t offset;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint8_t compressionMethodIndex;
};
```

### Dependencies

The `ucas` library requires:
- All common dependencies
- Interface with the `utoc` library for metadata

## General Purpose Utility

The `pak_utoc_ucas.exe` CLI utility should provide a unified interface for working with all three file formats.

### Core Functionality

1. **PAK Operations**:
   - List files in .pak archives
   - Extract files from .pak archives
   - Create new .pak archives
   - Add files to existing .pak archives

2. **UTOC/UCAS Operations**:
   - List files in .utoc files
   - Extract chunks from .utoc/.ucas files
   - Create new .utoc/.ucas files
   - Add chunks to existing .utoc/.ucas files

3. **Conversion Operations**:
   - Convert between Legacy and Zen assets
   - Handle shader libraries during conversion

### Command-Line Interface

```
Usage: pak_utoc_ucas [OPTIONS] <COMMAND>

Commands:
  pak:
    info       Print .pak info
    list       List files in .pak
    extract    Extract files from .pak
    create     Create new .pak
    add        Add files to existing .pak

  utoc:
    info       Print .utoc info
    list       List files in .utoc
    extract    Extract chunks from .utoc
    create     Create new .utoc
    add        Add chunks to existing .utoc

  convert:
    to-legacy  Convert Zen assets to Legacy assets
    to-zen     Convert Legacy assets to Zen assets

Options:
  -a, --aes-key <KEY>  AES key for encrypted files
  -v, --verbose        Enable verbose output
  -h, --help           Print help
  --version            Print version
```

## Implementation Strategy

### Phase 1: Core Library Structure

1. **Set up project structure**:
   - Create the three static library projects
   - Set up build system (xmake)
   - Define common interfaces and data structures

2. **Implement common utilities**:
   - Compression/decompression functions
   - Encryption/decryption functions
   - File I/O utilities
   - Logging and error handling

### Phase 2: PAK Library Implementation

1. **Implement PAK reading**:
   - Parse .pak file headers and footers
   - Read file entries and indices
   - Extract files from .pak archives

2. **Implement PAK writing**:
   - Create new .pak files
   - Add files to .pak files
   - Generate proper indices and metadata

3. **Test PAK library**:
   - Unit tests for reading/writing
   - Integration tests with real .pak files

### Phase 3: UTOC/UCAS Library Implementation

1. **Implement UTOC reading**:
   - Parse .utoc file headers
   - Read chunk IDs, offsets, and lengths
   - Read directory index

2. **Implement UCAS reading**:
   - Read chunk data from .ucas files
   - Handle compressed and encrypted data
   - Support for partitioned .ucas files

3. **Implement UTOC/UCAS writing**:
   - Create new .utoc/.ucas files
   - Add chunks to .utoc/.ucas files
   - Generate proper directory indices

4. **Test UTOC/UCAS libraries**:
   - Unit tests for reading/writing
   - Integration tests with real .utoc/.ucas files

### Phase 4: Conversion Implementation

1. **Implement Legacy to Zen conversion**:
   - Parse Legacy assets
   - Convert to Zen format
   - Write to .utoc/.ucas files

2. **Implement Zen to Legacy conversion**:
   - Parse Zen assets
   - Convert to Legacy format
   - Write to .pak files

3. **Test conversion**:
   - Unit tests for conversion
   - Integration tests with real assets

### Phase 5: CLI Utility Implementation

1. **Implement command-line interface**:
   - Parse command-line arguments
   - Dispatch to appropriate library functions
   - Handle errors and output

2. **Test CLI utility**:
   - Integration tests for all commands
   - End-to-end tests for common workflows

## Dependencies

### Required Dependencies

1. **Compression Libraries**:
   - **zlib**: For Zlib/Gzip compression
     - License: zlib License
     - Integration: Can be linked statically or dynamically
   - **zstd**: For Zstandard compression
     - License: BSD License
     - Integration: Can be linked statically or dynamically
   - **lz4**: For LZ4 compression
     - License: BSD License
     - Integration: Can be linked statically or dynamically

2. **Cryptography**:
   - **OpenSSL** or **Botan**: For AES-256 encryption/decryption
     - License: OpenSSL License (OpenSSL) or BSD License (Botan)
     - Integration: Can be linked statically or dynamically
   - **BLAKE3**: For chunk hashing
     - License: CC0 or Apache 2.0
     - Integration: Can be included directly as source

3. **Utility Libraries**:
   - **fmt**: For string formatting
     - License: MIT License
     - Integration: Can be included directly as source or linked statically
   - **CLI11**: For command-line parsing
     - License: BSD License
     - Integration: Header-only library

### Optional Dependencies

1. **Oodle Compression**:
   - License: Proprietary (requires licensing from Epic Games/RAD Game Tools)
   - Integration: Dynamic loading at runtime (similar to the existing oodle_loader)

2. **Threading Library**:
   - **TBB**: For parallel processing
     - License: Apache 2.0
     - Integration: Can be linked statically or dynamically

## Testing Strategy

1. **Unit Tests**:
   - Test individual components (e.g., compression, encryption, file parsing)
   - Use a framework like Google Test or Catch2
   - Aim for high code coverage

2. **Integration Tests**:
   - Test interactions between components
   - Test with real .pak, .utoc, and .ucas files
   - Verify compatibility with different versions

3. **Performance Tests**:
   - Benchmark reading and writing operations
   - Compare performance with original Ruby implementations
   - Identify and optimize bottlenecks

4. **Compatibility Tests**:
   - Test with files from different Unreal Engine versions
   - Verify compatibility with the original Ruby implementations
   - Test on different platforms (Windows, Linux, macOS)

## Conclusion

Porting the Ruby libraries retoc and repak to C++ static libraries is a significant undertaking, but it offers several benefits:

1. **Performance**: C++ implementations can be significantly faster than Ruby
2. **Integration**: Static libraries can be easily integrated into other C++ projects
3. **Portability**: C++ code can be compiled for various platforms
4. **Maintenance**: Separate libraries for each file format simplify maintenance

The proposed architecture with three separate libraries (`pak`, `utoc`, and `ucas`) provides a clean separation of concerns while allowing for efficient integration through well-defined interfaces. The general-purpose utility CLI (`pak_utoc_ucas.exe`) provides a unified interface for working with all three file formats.

By following the implementation strategy outlined in this document, the port can be completed in a systematic and efficient manner, ensuring compatibility with the original Ruby implementations while leveraging the benefits of C++.
