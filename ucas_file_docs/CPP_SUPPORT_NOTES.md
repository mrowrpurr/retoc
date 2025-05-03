# C++ Implementation Notes for UCAS File Format Support

This document provides guidance for implementing a C++ static library for reading and writing UCAS files. It covers dependencies, implementation challenges, and design considerations based on analysis of the retoc codebase.

## Dependencies

### Essential Third-Party Libraries

1. **Compression Libraries**:
   - **zlib** - For Zlib compression
     ```cpp
     // Example installation
     // Ubuntu: sudo apt-get install zlib1g-dev
     // Windows with vcpkg: vcpkg install zlib
     #include <zlib.h>
     ```
   - **zstd** - For Zstandard compression
     ```cpp
     // Example installation
     // Ubuntu: sudo apt-get install libzstd-dev
     // Windows with vcpkg: vcpkg install zstd
     #include <zstd.h>
     ```
   - **lz4** - For LZ4 compression
     ```cpp
     // Example installation
     // Ubuntu: sudo apt-get install liblz4-dev
     // Windows with vcpkg: vcpkg install lz4
     #include <lz4.h>
     ```
   - **Oodle** (optional but recommended) - For Oodle compression
     ```cpp
     // Note: Oodle is proprietary and requires a license from Epic Games/RAD Game Tools
     // You'll need to implement a dynamic loading mechanism similar to retoc's oodle_loader
     ```

2. **Cryptography Library**:
   - An **AES-256** implementation for encryption/decryption
     ```cpp
     // Example with OpenSSL
     // Ubuntu: sudo apt-get install libssl-dev
     // Windows with vcpkg: vcpkg install openssl
     #include <openssl/aes.h>
     #include <openssl/evp.h>
     ```

3. **Hashing Library**:
   - **BLAKE3** - For chunk verification
     ```cpp
     // Example installation
     // Using the C implementation: https://github.com/BLAKE3-team/BLAKE3/tree/master/c
     // Or a C++ wrapper
     #include "blake3.h"
     ```

### Optional Libraries

1. **Logging Framework**:
   ```cpp
   // Example with spdlog
   // Ubuntu: sudo apt-get install libspdlog-dev
   // Windows with vcpkg: vcpkg install spdlog
   #include <spdlog/spdlog.h>
   ```

2. **Threading Library**:
   ```cpp
   // Standard C++ threading
   #include <thread>
   #include <mutex>
   #include <condition_variable>
   
   // Or using a library like Intel TBB for more advanced parallel processing
   // Ubuntu: sudo apt-get install libtbb-dev
   // Windows with vcpkg: vcpkg install tbb
   #include <tbb/parallel_for.h>
   ```

## Implementation Challenges

### 1. File Pooling

The retoc implementation uses a file pool to manage file handles efficiently for parallel processing. Implementing this in C++ requires careful thread synchronization:

```cpp
class FilePool {
private:
    std::string path;
    std::mutex mutex;
    std::condition_variable condVar;
    std::deque<std::unique_ptr<std::ifstream>> availableFiles;
    size_t maxHandles;
    size_t activeCount;

public:
    FilePool(const std::string& path, size_t maxHandles)
        : path(path), maxHandles(maxHandles), activeCount(0) {
        // Verify the file can be opened
        std::ifstream testFile(path, std::ios::binary);
        if (!testFile.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }
    }

    class FileHandle {
    private:
        std::unique_ptr<std::ifstream> file;
        FilePool* pool;

    public:
        FileHandle(std::unique_ptr<std::ifstream> file, FilePool* pool)
            : file(std::move(file)), pool(pool) {}

        ~FileHandle() {
            // Return the file to the pool
            if (file && pool) {
                pool->returnFile(std::move(file));
            }
        }

        std::ifstream* get() { return file.get(); }
    };

    FileHandle acquireFile() {
        std::unique_lock<std::mutex> lock(mutex);
        
        while (true) {
            // Check if there's an available file
            if (!availableFiles.empty()) {
                auto file = std::move(availableFiles.front());
                availableFiles.pop_front();
                return FileHandle(std::move(file), this);
            }
            
            // Check if we can create a new file
            if (activeCount < maxHandles) {
                activeCount++;
                lock.unlock();
                
                auto file = std::make_unique<std::ifstream>(path, std::ios::binary);
                if (!file->is_open()) {
                    throw std::runtime_error("Failed to open file: " + path);
                }
                
                return FileHandle(std::move(file), this);
            }
            
            // Wait for a file to become available
            condVar.wait(lock);
        }
    }

private:
    void returnFile(std::unique_ptr<std::ifstream> file) {
        std::lock_guard<std::mutex> lock(mutex);
        availableFiles.push_back(std::move(file));
        condVar.notify_one();
    }
};
```

### 2. Chunk Reading

Reading chunks from a UCAS file involves several steps:

```cpp
std::vector<uint8_t> readChunk(std::ifstream& casStream, uint32_t tocEntryIndex) {
    // Get offset and length from UTOC
    const auto& offsetAndLength = chunkOffsetLengths[tocEntryIndex];
    uint64_t offset = offsetAndLength.getOffset();
    uint64_t size = offsetAndLength.getLength();

    // Calculate block indices
    uint32_t compressionBlockSize = 0x10000; // 65536
    size_t firstBlockIndex = offset / compressionBlockSize;
    size_t lastBlockIndex = ((alignUp(offset + size, compressionBlockSize) - 1) 
                            / compressionBlockSize);

    // Get the blocks
    const auto& blocks = compressionBlocks.subspan(firstBlockIndex, 
                                                 lastBlockIndex - firstBlockIndex + 1);

    // Check if encryption is needed
    std::unique_ptr<AES_KEY> aesKey;
    if (containerFlags & EIoContainerFlags::Encrypted) {
        // Get the encryption key
        aesKey = getAesKey(encryptionKeyGuid);
    }

    // Read and process the data
    std::vector<uint8_t> data(size);
    size_t currentPos = 0;

    for (const auto& block : blocks) {
        uint32_t compressedSize = block.getCompressedSize();
        uint32_t uncompressedSize = block.getUncompressedSize();
        uint8_t compressionMethodIndex = block.getCompressionMethodIndex();

        // Seek to the block position
        casStream.seekg(block.getOffset());

        // Read the block data
        std::vector<uint8_t> blockData(compressedSize);
        casStream.read(reinterpret_cast<char*>(blockData.data()), compressedSize);

        // Decrypt if needed
        if (aesKey) {
            // Align to AES block size (16 bytes)
            size_t alignedSize = alignUp(compressedSize, 16);
            blockData.resize(alignedSize);
            
            // Decrypt in-place
            for (size_t i = 0; i < alignedSize; i += 16) {
                AES_decrypt(blockData.data() + i, blockData.data() + i, aesKey.get());
            }
        }

        // Decompress if needed
        if (compressionMethodIndex > 0) {
            const auto& compressionMethod = compressionMethods[compressionMethodIndex - 1];
            std::vector<uint8_t> decompressedData(uncompressedSize);
            
            switch (compressionMethod) {
                case CompressionMethod::Zlib:
                    decompressZlib(blockData, decompressedData);
                    break;
                case CompressionMethod::Zstd:
                    decompressZstd(blockData, decompressedData);
                    break;
                case CompressionMethod::LZ4:
                    decompressLz4(blockData, decompressedData);
                    break;
                case CompressionMethod::Oodle:
                    decompressOodle(blockData, decompressedData);
                    break;
            }
            
            // Copy to the output buffer
            size_t copySize = std::min(uncompressedSize, size - currentPos);
            std::memcpy(data.data() + currentPos, decompressedData.data(), copySize);
        } else {
            // No compression, just copy the data
            size_t copySize = std::min(uncompressedSize, size - currentPos);
            std::memcpy(data.data() + currentPos, blockData.data(), copySize);
        }
        
        currentPos += uncompressedSize;
    }

    return data;
}
```

### 3. Partitioning Support

When dealing with partitioned UCAS files, you need to determine which partition contains the data:

```cpp
std::ifstream openPartitionFile(const std::string& basePath, int32_t partitionIndex) {
    std::string partitionPath;
    
    if (partitionIndex < 0) {
        partitionPath = basePath;
    } else {
        partitionPath = basePath + "." + std::to_string(partitionIndex);
    }
    
    std::ifstream file(partitionPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open partition file: " + partitionPath);
    }
    
    return file;
}

int32_t getPartitionIndex(const CompressionBlock& block) {
    if (partitionSize == std::numeric_limits<uint64_t>::max()) {
        return -1; // No partitioning
    }
    
    return static_cast<int32_t>(block.getOffset() / partitionSize);
}
```

### 4. Oodle Compression

Handling Oodle compression requires dynamic loading of the Oodle library:

```cpp
class OodleLoader {
private:
    void* libraryHandle;
    
    // Function pointers for Oodle functions
    typedef int (*OodleDecompressFunc)(const void* compBuf, int compBufSize, 
                                      void* rawBuf, int rawLen);
    OodleDecompressFunc oodleDecompress;

public:
    OodleLoader() : libraryHandle(nullptr), oodleDecompress(nullptr) {
        // Try to load the Oodle library
        #ifdef _WIN32
        libraryHandle = LoadLibraryA("oo2core_9_win64.dll");
        #else
        libraryHandle = dlopen("liboo2core_9_linux64.so", RTLD_LAZY);
        #endif
        
        if (!libraryHandle) {
            throw std::runtime_error("Failed to load Oodle library");
        }
        
        // Get function pointers
        #ifdef _WIN32
        oodleDecompress = reinterpret_cast<OodleDecompressFunc>(
            GetProcAddress(static_cast<HMODULE>(libraryHandle), "OodleLZ_Decompress"));
        #else
        oodleDecompress = reinterpret_cast<OodleDecompressFunc>(
            dlsym(libraryHandle, "OodleLZ_Decompress"));
        #endif
        
        if (!oodleDecompress) {
            throw std::runtime_error("Failed to get Oodle function pointers");
        }
    }
    
    ~OodleLoader() {
        if (libraryHandle) {
            #ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(libraryHandle));
            #else
            dlclose(libraryHandle);
            #endif
        }
    }
    
    int decompress(const void* compBuf, int compBufSize, void* rawBuf, int rawLen) {
        return oodleDecompress(compBuf, compBufSize, rawBuf, rawLen);
    }
};
```

## Design Considerations

### 1. Class Structure

A possible class structure for a C++ UCAS library:

```cpp
// Forward declarations
class Toc;
class FilePool;

// Main IoStore container class
class IoStoreContainer {
private:
    std::string name;
    std::filesystem::path path;
    std::unique_ptr<Toc> toc;
    std::unique_ptr<FilePool> cas;
    std::unique_ptr<ContainerHeader> containerHeader;

public:
    // Constructor
    IoStoreContainer(const std::filesystem::path& tocPath, const Config& config);
    
    // Accessors
    const std::string& getName() const;
    const std::filesystem::path& getPath() const;
    
    // Operations
    std::vector<uint8_t> readChunk(const ChunkId& chunkId);
    bool hasChunk(const ChunkId& chunkId) const;
    std::vector<ChunkInfo> getChunks() const;
    std::optional<std::string> getChunkPath(const ChunkId& chunkId) const;
};

// Writer class for creating UCAS files
class IoStoreWriter {
private:
    std::filesystem::path tocPath;
    std::unique_ptr<std::ofstream> tocStream;
    std::unique_ptr<std::ofstream> casStream;
    std::unique_ptr<Toc> toc;
    std::unique_ptr<ContainerHeader> containerHeader;

public:
    // Constructor
    IoStoreWriter(const std::filesystem::path& tocPath, 
                 EIoStoreTocVersion tocVersion,
                 std::optional<EIoContainerHeaderVersion> containerHeaderVersion,
                 const std::string& mountPoint);
    
    // Operations
    void writeChunk(const ChunkId& chunkId, 
                   std::optional<const std::string&> path,
                   const std::vector<uint8_t>& data);
    
    void finalize();
};
```

### 2. Memory Management

Efficient memory management is crucial for handling large UCAS files:

```cpp
// A simple buffer pool for reusing memory buffers
class BufferPool {
private:
    std::mutex mutex;
    std::vector<std::vector<uint8_t>> availableBuffers;
    size_t bufferSize;

public:
    BufferPool(size_t bufferSize, size_t initialCount = 0)
        : bufferSize(bufferSize) {
        for (size_t i = 0; i < initialCount; ++i) {
            availableBuffers.emplace_back(bufferSize);
        }
    }
    
    std::vector<uint8_t> acquireBuffer() {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (!availableBuffers.empty()) {
            auto buffer = std::move(availableBuffers.back());
            availableBuffers.pop_back();
            return buffer;
        }
        
        return std::vector<uint8_t>(bufferSize);
    }
    
    void releaseBuffer(std::vector<uint8_t>&& buffer) {
        if (buffer.capacity() >= bufferSize) {
            buffer.resize(bufferSize);
            
            std::lock_guard<std::mutex> lock(mutex);
            availableBuffers.push_back(std::move(buffer));
        }
    }
};
```

### 3. Error Handling

Robust error handling is important for a library that deals with file I/O and complex formats:

```cpp
// Error codes
enum class UcasErrorCode {
    Success,
    FileNotFound,
    InvalidFormat,
    CompressionError,
    EncryptionError,
    ChunkNotFound,
    PartitionError,
    OutOfMemory,
    UnknownError
};

// Exception class
class UcasException : public std::exception {
private:
    UcasErrorCode code;
    std::string message;

public:
    UcasException(UcasErrorCode code, const std::string& message)
        : code(code), message(message) {}
    
    UcasErrorCode getCode() const { return code; }
    const char* what() const noexcept override { return message.c_str(); }
};

// Result class for operations that can fail
template<typename T>
class Result {
private:
    std::variant<T, UcasException> value;

public:
    Result(const T& value) : value(value) {}
    Result(UcasException error) : value(error) {}
    
    bool isSuccess() const { return std::holds_alternative<T>(value); }
    bool isError() const { return std::holds_alternative<UcasException>(value); }
    
    const T& getValue() const {
        if (isError()) {
            throw std::get<UcasException>(value);
        }
        return std::get<T>(value);
    }
    
    const UcasException& getError() const {
        if (isSuccess()) {
            throw std::runtime_error("Result does not contain an error");
        }
        return std::get<UcasException>(value);
    }
};
```

### 4. Thread Safety

Ensuring thread safety for parallel operations:

```cpp
class ThreadSafeContainer {
private:
    IoStoreContainer container;
    std::shared_mutex mutex;

public:
    ThreadSafeContainer(const std::filesystem::path& tocPath, const Config& config)
        : container(tocPath, config) {}
    
    std::vector<uint8_t> readChunk(const ChunkId& chunkId) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return container.readChunk(chunkId);
    }
    
    bool hasChunk(const ChunkId& chunkId) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return container.hasChunk(chunkId);
    }
    
    // Other operations...
};
```

## Performance Optimization

### 1. Parallel Processing

Using parallel processing for better performance:

```cpp
std::vector<std::vector<uint8_t>> extractChunks(const std::vector<ChunkId>& chunkIds) {
    std::vector<std::vector<uint8_t>> results(chunkIds.size());
    
    #pragma omp parallel for
    for (size_t i = 0; i < chunkIds.size(); ++i) {
        results[i] = readChunk(chunkIds[i]);
    }
    
    return results;
}
```

### 2. Memory Mapping

Using memory mapping for large files:

```cpp
class MemoryMappedFile {
private:
    #ifdef _WIN32
    HANDLE fileHandle;
    HANDLE mappingHandle;
    #else
    int fileDescriptor;
    #endif
    
    void* mappedData;
    size_t fileSize;

public:
    MemoryMappedFile(const std::string& path) {
        // Implementation depends on the platform
        // Windows: CreateFile, CreateFileMapping, MapViewOfFile
        // Unix: open, mmap
    }
    
    ~MemoryMappedFile() {
        // Cleanup
        // Windows: UnmapViewOfFile, CloseHandle
        // Unix: munmap, close
    }
    
    const void* getData() const { return mappedData; }
    size_t getSize() const { return fileSize; }
};
```

## Conclusion

Implementing a C++ static library for UCAS file support requires careful consideration of dependencies, memory management, thread safety, and error handling. The retoc codebase provides a good reference implementation, but adapting it to C++ requires addressing language-specific challenges.

The most complex aspects are handling compression (especially Oodle), encryption, and partitioning. A well-designed C++ library should provide a clean API that abstracts these complexities while maintaining good performance.
