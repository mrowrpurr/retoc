# C++ Implementation Notes for UTOC Support

This document provides guidance for implementing a C++ static library with full support for the .utoc file format, based on analysis of the retoc Rust implementation.

## Dependencies

### Core Dependencies

1. **Compression Libraries**:
   - **zlib/gzip**: For Zlib and Gzip compression methods
     - [zlib](https://www.zlib.net/) is widely available and easy to integrate
   - **zstd**: For Zstandard compression
     - [zstd](https://github.com/facebook/zstd) provides a C API that works well from C++
   - **lz4**: For LZ4 compression
     - [lz4](https://github.com/lz4/lz4) is lightweight and has a simple C API
   - **Oodle**: For Oodle compression (optional but important for full compatibility)
     - Proprietary, requires licensing from Epic Games/RAD Game Tools
     - Consider implementing a dynamic loading mechanism similar to retoc's approach

2. **Cryptography**:
   - **AES**: For AES-256 encryption/decryption (in CBC mode)
     - [OpenSSL](https://www.openssl.org/) or [Botan](https://botan.randombit.net/) are good options
     - [Crypto++](https://www.cryptopp.com/) is another comprehensive option
   - **GUID handling**: For encryption key identification
     - Standard library `<uuid>` in C++17 or boost::uuid for earlier versions

3. **Hashing**:
   - **BLAKE3**: For chunk hashing (in newer versions)
     - [BLAKE3 C implementation](https://github.com/BLAKE3-team/BLAKE3) can be used
   - **SHA-1**: For older hash verification
     - Available in most crypto libraries like OpenSSL
   - **CityHash**: For package ID generation from names
     - [CityHash](https://github.com/google/cityhash) has a C++ implementation

4. **Data Structures**:
   - A library for perfect hashing or implement your own
     - [cmph](http://cmph.sourceforge.net/) is a possibility
     - [tsl::hopscotch_map](https://github.com/Tessil/hopscotch-map) for efficient hash tables

### Optional Dependencies

1. **Serialization**:
   - JSON library if you want to support manifest operations
     - [nlohmann/json](https://github.com/nlohmann/json) is a popular choice
     - [rapidjson](https://github.com/Tencent/rapidjson) for performance-critical code

2. **Filesystem**:
   - `<filesystem>` in C++17 or boost::filesystem for earlier versions
   - Consider a cross-platform abstraction if targeting multiple platforms

## Implementation Challenges

### 1. Compact Binary Formats

Several structures in the .utoc format use compact binary representations:

- **FIoOffsetAndLength**: 10-byte structure (5 bytes for offset, 5 bytes for length)
- **FIoStoreTocCompressedBlockEntry**: 12-byte structure with bit-packed fields

**Example Implementation for FIoOffsetAndLength**:

```cpp
class FIoOffsetAndLength {
private:
    std::array<uint8_t, 10> data;

public:
    FIoOffsetAndLength() : data{} {}
    
    uint64_t GetOffset() const {
        uint64_t result = 0;
        // Big-endian format with 5 bytes
        result |= static_cast<uint64_t>(data[0]) << 32;
        result |= static_cast<uint64_t>(data[1]) << 24;
        result |= static_cast<uint64_t>(data[2]) << 16;
        result |= static_cast<uint64_t>(data[3]) << 8;
        result |= static_cast<uint64_t>(data[4]);
        return result;
    }
    
    void SetOffset(uint64_t offset) {
        data[0] = (offset >> 32) & 0xFF;
        data[1] = (offset >> 24) & 0xFF;
        data[2] = (offset >> 16) & 0xFF;
        data[3] = (offset >> 8) & 0xFF;
        data[4] = offset & 0xFF;
    }
    
    uint64_t GetLength() const {
        uint64_t result = 0;
        // Big-endian format with 5 bytes
        result |= static_cast<uint64_t>(data[5]) << 32;
        result |= static_cast<uint64_t>(data[6]) << 24;
        result |= static_cast<uint64_t>(data[7]) << 16;
        result |= static_cast<uint64_t>(data[8]) << 8;
        result |= static_cast<uint64_t>(data[9]);
        return result;
    }
    
    void SetLength(uint64_t length) {
        data[5] = (length >> 32) & 0xFF;
        data[6] = (length >> 24) & 0xFF;
        data[7] = (length >> 16) & 0xFF;
        data[8] = (length >> 8) & 0xFF;
        data[9] = length & 0xFF;
    }
};
```

### 2. Perfect Hash Implementation

The perfect hash system is crucial for efficient chunk lookup in newer versions:

- Implement a minimal perfect hash function generator
- Handle the overflow table for chunks that can't be perfectly hashed

**Pseudocode for Chunk Lookup**:

```cpp
ChunkData* LookupChunk(const FIoChunkId& chunkId) {
    if (tocVersion >= EIoStoreTocVersion::PerfectHash) {
        // Use perfect hash
        uint32_t hash = ComputeHash(chunkId, perfectHashSeeds);
        if (hash < chunkIds.size()) {
            if (chunkIds[hash] == chunkId) {
                return &chunks[hash];
            }
        }
        
        // Check overflow table if using PerfectHashWithOverflow
        if (tocVersion >= EIoStoreTocVersion::PerfectHashWithOverflow) {
            for (uint32_t index : chunkIndicesWithoutPerfectHash) {
                if (chunkIds[index] == chunkId) {
                    return &chunks[index];
                }
            }
        }
    } else {
        // Linear search for older versions
        for (size_t i = 0; i < chunkIds.size(); ++i) {
            if (chunkIds[i] == chunkId) {
                return &chunks[i];
            }
        }
    }
    
    return nullptr; // Chunk not found
}
```

### 3. Directory Index Structure

The directory index is a complex hierarchical structure:

- Implement a tree-like structure for directories and files
- Handle serialization and deserialization of the index
- Support encryption of the directory index

**Suggested Class Structure**:

```cpp
class FIoDirectoryIndexResource {
private:
    std::string mountPoint;
    std::vector<FIoDirectoryIndexEntry> directoryEntries;
    std::vector<FIoFileIndexEntry> fileEntries;
    std::vector<std::string> stringTable;
    
    // Helper methods
    uint32_t GetOrCreateName(const std::string& name);
    uint32_t GetOrCreateDirectory(uint32_t parentIndex, const std::string& name);
    uint32_t GetOrCreateFile(uint32_t dirIndex, const std::string& name);

public:
    FIoDirectoryIndexResource();
    
    // Add a file to the directory index
    void AddFile(const std::string& path, uint32_t userData);
    
    // Iterate through all files in the index
    void IterateFiles(std::function<void(uint32_t userData, const std::string& path)> visitor) const;
    
    // Serialization
    void Serialize(std::ostream& stream) const;
    void Deserialize(std::istream& stream);
    
    // Encryption support
    std::vector<uint8_t> SerializeAndEncrypt(const AesKey& key) const;
    void DecryptAndDeserialize(const std::vector<uint8_t>& data, const AesKey& key);
};
```

### 4. Chunk ID Handling

The chunk ID system is complex with version-dependent behavior:

- Implement proper handling of the 12-byte chunk ID structure
- Support different chunk type mappings based on version
- Handle version bits correctly

**Example Implementation**:

```cpp
class FIoChunkId {
private:
    std::array<uint8_t, 12> id;

public:
    FIoChunkId() : id{} {}
    
    // Create from raw bytes
    static FIoChunkId FromRaw(const std::array<uint8_t, 12>& rawId, EIoStoreTocVersion version) {
        FIoChunkId result;
        result.id = rawId;
        
        // Set version bits
        bool isNew = version > EIoStoreTocVersion::PerfectHash;
        result.id[11] = GetChunkType(result.id[11], isNew);
        result.id[11] |= (isNew ? 0x80 : 0x00); // Set is_new bit
        result.id[11] |= 0x40; // Set has_version bit
        
        return result;
    }
    
    // Create from package ID
    static FIoChunkId FromPackageId(uint64_t packageId, uint16_t chunkIndex, EIoChunkType chunkType) {
        FIoChunkId result;
        
        // Set package ID (first 8 bytes)
        for (int i = 0; i < 8; ++i) {
            result.id[i] = (packageId >> (i * 8)) & 0xFF;
        }
        
        // Set chunk index (next 2 bytes)
        result.id[8] = chunkIndex & 0xFF;
        result.id[9] = (chunkIndex >> 8) & 0xFF;
        
        // Set chunk type and version bits
        result.id[10] = 0;
        result.id[11] = static_cast<uint8_t>(chunkType);
        result.id[11] |= 0x40; // Set has_version bit
        
        return result;
    }
    
    // Get raw chunk ID
    std::array<uint8_t, 12> GetRaw() const {
        std::array<uint8_t, 12> result = id;
        if ((id[11] & 0x40) == 0) {
            throw std::runtime_error("No version info, cannot convert to raw");
        }
        
        bool isNew = (id[11] & 0x80) != 0;
        result[11] = GetRawChunkType(GetChunkType(), isNew);
        return result;
    }
    
    // Get chunk type
    EIoChunkType GetChunkType() const {
        return static_cast<EIoChunkType>(id[11] & 0x3F);
    }
    
    // Get package ID
    uint64_t GetPackageId() const {
        uint64_t result = 0;
        for (int i = 0; i < 8; ++i) {
            result |= static_cast<uint64_t>(id[i]) << (i * 8);
        }
        return result;
    }
    
private:
    // Convert between raw chunk type and enum based on version
    static uint8_t GetChunkType(uint8_t rawType, bool isNew);
    static uint8_t GetRawChunkType(EIoChunkType chunkType, bool isNew);
};
```

### 5. Compression System

The compression system needs to handle multiple methods and block-based compression:

- Implement a unified interface for different compression methods
- Handle block-based compression and decompression
- Support encryption of compressed blocks

**Suggested Interface**:

```cpp
// Compression method enum
enum class CompressionMethod {
    None,
    Zlib,
    Gzip,
    Zstd,
    LZ4,
    Oodle
};

// Compression interface
class CompressionSystem {
public:
    // Compress a buffer using the specified method
    static std::vector<uint8_t> Compress(
        CompressionMethod method,
        const std::vector<uint8_t>& input);
    
    // Decompress a buffer using the specified method
    static void Decompress(
        CompressionMethod method,
        const std::vector<uint8_t>& input,
        std::vector<uint8_t>& output,
        size_t uncompressedSize);
    
    // Read a chunk from the .ucas file, handling compression and encryption
    static std::vector<uint8_t> ReadChunk(
        std::istream& casStream,
        const std::vector<FIoStoreTocCompressedBlockEntry>& compressionBlocks,
        const std::vector<CompressionMethod>& compressionMethods,
        uint64_t offset,
        uint64_t size,
        uint32_t compressionBlockSize,
        const AesKey* encryptionKey = nullptr);
};
```

### 6. AES Encryption

Implement proper AES-256 encryption in CBC mode:

- Support for key management via GUIDs
- Handle block alignment (16 bytes) for AES
- Implement encryption and decryption of both directory index and chunk data

**Example Implementation**:

```cpp
class AesEncryption {
private:
    std::unordered_map<FGuid, std::array<uint8_t, 32>> keys;

public:
    // Add a key with its GUID
    void AddKey(const FGuid& guid, const std::array<uint8_t, 32>& key) {
        keys[guid] = key;
    }
    
    // Get a key by GUID
    const std::array<uint8_t, 32>* GetKey(const FGuid& guid) const {
        auto it = keys.find(guid);
        return (it != keys.end()) ? &it->second : nullptr;
    }
    
    // Encrypt a buffer
    std::vector<uint8_t> Encrypt(
        const std::vector<uint8_t>& input,
        const std::array<uint8_t, 32>& key) const;
    
    // Decrypt a buffer
    std::vector<uint8_t> Decrypt(
        const std::vector<uint8_t>& input,
        const std::array<uint8_t, 32>& key) const;
};
```

### 7. Partitioning Support

Handle partitioned .ucas files:

- Implement logic to determine which partition contains a chunk
- Support opening and reading from multiple partition files
- Handle chunks that span multiple partitions

**Pseudocode for Partition Handling**:

```cpp
class IoStoreReader {
private:
    std::vector<std::unique_ptr<std::ifstream>> partitionStreams;
    uint64_t partitionSize;
    
public:
    // Open all partition files
    void OpenPartitions(const std::string& basePath, uint32_t partitionCount) {
        for (uint32_t i = 0; i < partitionCount; ++i) {
            std::string partitionPath = basePath + ".ucas";
            if (i > 0) {
                partitionPath += "." + std::to_string(i);
            }
            partitionStreams.push_back(std::make_unique<std::ifstream>(partitionPath, std::ios::binary));
        }
    }
    
    // Read data from the appropriate partition
    std::vector<uint8_t> ReadFromPartition(uint64_t offset, uint64_t size) {
        uint32_t partitionIndex = static_cast<uint32_t>(offset / partitionSize);
        uint64_t offsetInPartition = offset % partitionSize;
        
        if (partitionIndex >= partitionStreams.size()) {
            throw std::runtime_error("Invalid partition index");
        }
        
        std::vector<uint8_t> result(size);
        auto& stream = partitionStreams[partitionIndex];
        
        stream->seekg(offsetInPartition);
        stream->read(reinterpret_cast<char*>(result.data()), size);
        
        return result;
    }
};
```

## Architecture Recommendations

### 1. Class Structure

Consider organizing your library with these key classes:

- **IoStoreReader**: Main class for reading .utoc and .ucas files
- **IoStoreWriter**: Class for creating and writing .utoc and .ucas files
- **FIoChunkId**: Class for handling chunk IDs
- **FIoDirectoryIndex**: Class for managing the directory structure
- **CompressionSystem**: Utility class for compression/decompression
- **AesEncryption**: Utility class for encryption/decryption

### 2. Memory Management

- Use smart pointers for resource management
- Consider memory mapping for large .ucas files
- Implement buffer pooling for compression operations

### 3. Error Handling

- Use exceptions for error conditions
- Provide detailed error messages
- Consider a result-based error handling approach as an alternative

### 4. Threading Model

- Implement thread-safe access to shared resources
- Consider parallel decompression of multiple blocks
- Use thread pools for batch operations

### 5. API Design

- Provide both high-level and low-level APIs
- Support streaming operations for large files
- Design for extensibility to handle future versions

## Example Usage

```cpp
// Reading a .utoc file
IoStoreReader reader;
reader.Open("path/to/file.utoc");

// Get information about the container
auto version = reader.GetVersion();
auto containerFlags = reader.GetContainerFlags();

// Read a chunk by ID
FIoChunkId chunkId = FIoChunkId::FromPackageId(
    packageId, 0, EIoChunkType::ExportBundleData);
std::vector<uint8_t> chunkData = reader.ReadChunk(chunkId);

// Extract all files
reader.ExtractAllFiles("output/directory");

// Writing a .utoc file
IoStoreWriter writer;
writer.Create("path/to/file.utoc", EIoStoreTocVersion::PerfectHashWithOverflow);
writer.SetMountPoint("../../../");

// Add a chunk
FIoChunkId chunkId = FIoChunkId::FromPackageId(
    packageId, 0, EIoChunkType::ExportBundleData);
writer.AddChunk(chunkId, "Game/Content/MyAsset.uasset", chunkData);

// Finalize and write the file
writer.Finalize();
```

## Performance Considerations

1. **Chunk Lookup**: The perfect hash system is critical for performance with large containers
2. **Compression**: Consider using hardware-accelerated compression when available
3. **Memory Usage**: Minimize copying of large buffers
4. **File I/O**: Use buffered I/O and consider memory mapping for large files
5. **Parallelism**: Implement parallel processing for batch operations

## Testing Strategy

1. **Unit Tests**: Test individual components (chunk ID, compression, etc.)
2. **Integration Tests**: Test the full read/write pipeline
3. **Compatibility Tests**: Test with .utoc files from different UE versions
4. **Performance Tests**: Benchmark critical operations
5. **Stress Tests**: Test with large containers and many chunks

## Conclusion

Implementing a C++ static library for .utoc support is a complex but manageable task. The key challenges are handling the compact binary formats, implementing the perfect hash system, and supporting the various compression and encryption methods. By following the recommendations in this document, you can create a robust and efficient implementation that provides full compatibility with Unreal Engine's IoStore container system.
