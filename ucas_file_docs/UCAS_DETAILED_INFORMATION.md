# UCAS File Format: Detailed Information

## Introduction

The UCAS (Unreal Content Archive Storage) file format is a key component of Unreal Engine's IoStore container system, introduced in Unreal Engine 5 as a replacement for the older .pak file system. This document provides detailed information about the UCAS file format, its structure, and how it works with the UTOC (Unreal Table of Contents) file to form a complete IoStore container.

## Purpose and Role in Unreal Engine

UCAS files serve as the storage component of the IoStore container system. While the UTOC file contains metadata and indexing information, the UCAS file stores the actual content/data of game assets. This separation allows for efficient access to specific assets without having to scan the entire archive.

The IoStore system was designed to address several key challenges in modern game development:

1. **Performance**: Faster loading times, especially for large games with many assets
2. **Scalability**: Better handling of large game projects with thousands of assets
3. **Memory Efficiency**: Improved memory usage during asset loading
4. **Streaming**: Enhanced support for asset streaming and on-demand loading
5. **Modularity**: Better organization of game content for easier updates and patching

## Relationship with UTOC Files

UCAS files work in conjunction with UTOC files to form a complete IoStore container:

- **UTOC File**: Contains metadata, indexing information, and directory structure
- **UCAS File**: Contains the actual binary data of the assets

This separation allows the engine to quickly scan the smaller UTOC file to locate assets without having to parse the entire content archive. When an asset is needed, the engine uses the information from the UTOC file to directly access the relevant portion of the UCAS file.

## Detailed Structure

### Data Organization

UCAS files are organized as a series of data blocks. Each block corresponds to a chunk of data referenced by the UTOC file. The UTOC file contains metadata about these chunks, including:

- Chunk IDs
- Offsets and lengths within the UCAS file
- Compression information
- Hash values for verification

### Chunk System

The IoStore system uses a chunk-based approach to data storage:

1. **Chunk ID**: Each chunk has a unique 12-byte ID that identifies it
2. **Chunk Types**: Different types of chunks exist for different types of data:
   - ExportBundleData: Main asset data
   - BulkData: Large binary data
   - OptionalBulkData: Optional large binary data
   - MemoryMappedBulkData: Memory-mapped large binary data
   - ShaderCodeLibrary: Shader code libraries
   - ShaderCode: Individual shader code
   - ContainerHeader: Container metadata

### Block Size and Alignment

Each data block in the UCAS file has the following characteristics:

1. **Size**: The size of each block is determined by the `compression_block_size` value in the UTOC file (typically 0x10000 or 65536 bytes)
2. **Alignment**: Blocks may be aligned for AES encryption (16-byte alignment)
3. **Content**: The actual binary data of the asset chunk

## Technical Features

### Compression

UCAS files support several compression methods to reduce file size:

1. **Zlib**: A widely used compression algorithm
2. **Zstd**: A newer compression algorithm with better compression ratios and speed
3. **LZ4**: A fast compression algorithm with lower compression ratios
4. **Oodle**: A proprietary compression algorithm developed by RAD Game Tools, known for its high performance in game assets

The compression method is specified in the UTOC file for each block. Different chunks can use different compression methods based on the type of data and the desired trade-off between file size and decompression speed.

### Encryption

UCAS files can be encrypted using AES-256 in ECB mode to protect content. The encryption key is specified in the UTOC file and is identified by a GUID. Encrypted blocks are aligned to AES block boundaries (16 bytes).

The encryption process is as follows:

1. Read the encrypted data from the UCAS file
2. Divide the data into 16-byte blocks
3. Decrypt each block using the AES key

### Partitioning

UCAS files can be partitioned, which means the content is split across multiple files. This is particularly useful for large games where a single UCAS file might be too large to handle efficiently.

When a UCAS file is partitioned, the partitions are named using a numeric suffix:

- `global.ucas.0`
- `global.ucas.1`
- `global.ucas.2`
- etc.

Partitioning provides several benefits:

1. **Improved Loading Performance**: Smaller files can be loaded more efficiently, especially on platforms with limited memory
2. **Parallel Loading**: Multiple partitions can be loaded in parallel, improving loading times
3. **Reduced Memory Usage**: The system can load only the partitions it needs, rather than the entire UCAS file
4. **Better Caching**: Smaller files are more likely to be cached by the operating system

## Version Evolution

The UCAS file format has evolved across different versions of Unreal Engine:

### UE4.26-UE4.27

- Initial implementation of the UCAS file format
- Basic structure with data blocks
- Added support for directory indexing
- Added support for partitioning

### UE5.0-UE5.3

- Improved chunk lookup with perfect hashing
- Added support for overflow in the perfect hash table
- Better integration with the engine's asset loading system
- Enhanced compression options

### UE5.4-UE5.5

- Added and later removed on-demand metadata
- Replaced chunk hash with IO hash for better performance
- Enhanced partitioning capabilities
- Optimized for on-demand loading
- Improved handling of large assets

## Implementation Considerations

When working with UCAS files, consider the following implementation details:

### Reading UCAS Files

1. **File Pooling**: Use a file pool to manage file handles efficiently, especially for parallel processing
2. **Chunk Reading**: Reading chunks involves finding the chunk in the UTOC file, determining the offset and length, reading the data, and processing it
3. **Partitioning**: If the UCAS file is partitioned, determine which partition contains the data and open the appropriate file

### Writing UCAS Files

1. **Block Size**: Use the standard block size of 0x10000 (65536) bytes
2. **Chunk Writing**: Divide the data into blocks, write each block, and record the metadata in the UTOC file
3. **Hashing**: Hash each chunk using Blake3 for verification

### Performance Optimization

1. **Parallel Processing**: Use parallel processing for better performance with large files
2. **Memory Management**: Use a buffer pool to manage memory efficiently
3. **Chunk Lookup**: Use the perfect hash table in the UTOC file for efficient chunk lookup

### Error Handling

1. **Missing Files**: Check if the UCAS file exists before trying to open it
2. **Encryption Errors**: Check if the UCAS file is encrypted and if the encryption key is available
3. **Compression Errors**: Check if the compression method is supported and if the decompression was successful

## Conclusion

The UCAS file format is a key component of Unreal Engine's IoStore container system. It provides efficient storage for game assets, with support for compression, encryption, and partitioning. Understanding the UCAS file format is essential for working with Unreal Engine's asset system, especially when developing tools for asset management or game modding.

By separating metadata (in UTOC files) from content (in UCAS files), the IoStore system achieves better performance and scalability compared to the older .pak file system. This design allows for efficient asset lookup and loading, which is crucial for modern games with thousands of assets.
