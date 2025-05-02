# About .ucas Files

## General Information

.ucas (Unreal Content Archive Storage) files are part of Unreal Engine's IoStore container system. They store the actual content/data of game assets, while the corresponding .utoc files contain the metadata and indexing information needed to access this content. The IoStore system was introduced in Unreal Engine 5 as a replacement for the older .pak file system.

.ucas files work in conjunction with .utoc files to form a complete IoStore container. While the .utoc file provides the indexing and metadata, the .ucas file contains the raw asset data itself. This separation of concerns allows for efficient access to specific assets without having to scan the entire archive.

Key characteristics of .ucas files include:
- Store the actual asset data in chunks
- Support for compressed data
- Support for encrypted data
- Organized in blocks for efficient access
- Can be partitioned for better performance

## File Format

The .ucas file format is relatively straightforward compared to the .utoc file:

### 1. Data Blocks
- The file consists of a series of data blocks
- Each block contains the raw data for a specific chunk
- Blocks may be compressed and/or encrypted
- The location and size of each block are specified in the corresponding .utoc file

### 2. Compression
- Data blocks can be compressed using various methods:
  - Zlib
  - Gzip
  - Zstd
  - LZ4
  - Oodle
- Compression information is stored in the .utoc file
- Different chunks can use different compression methods

### 3. Encryption
- Data blocks can be encrypted using AES-256
- Encryption information is stored in the .utoc file
- Encrypted blocks are aligned to AES block boundaries (16 bytes)

### 4. Partitioning
- In newer versions, .ucas files can be partitioned
- Each partition is a separate file (e.g., global.ucas.0, global.ucas.1)
- Partitioning allows for better parallel loading and reduced file size
- Partition information is stored in the .utoc file

## Version Differences

The .ucas file format itself has remained relatively stable across Unreal Engine versions, with most of the version-specific changes being handled in the .utoc file. However, there are some differences in how .ucas files are used:

1. **UE4.26-UE4.27**:
   - Basic .ucas file format
   - No partitioning support
   - Limited compression options

2. **UE5.0-UE5.3**:
   - Added support for partitioning
   - Improved compression options
   - Better integration with the engine's asset loading system

3. **UE5.4-UE5.5**:
   - Enhanced partitioning capabilities
   - Optimized for on-demand loading
   - Improved handling of large assets

## retoc Support

retoc provides comprehensive support for .ucas files:

1. **Reading Operations**:
   - Read data blocks from .ucas files based on information in the .utoc file
   - Handle compressed and encrypted data
   - Support for partitioned .ucas files
   - Efficient memory management for large files

2. **Writing Operations**:
   - Create new .ucas files
   - Add data blocks to existing .ucas files
   - Support for various compression methods
   - Generate proper partitioning if needed

3. **Utility Operations**:
   - Extract specific chunks from .ucas files
   - Verify the integrity of .ucas files
   - Handle partitioned .ucas files seamlessly

4. **Conversion Support**:
   - When converting from Zen to Legacy format, read asset data from .ucas files
   - When converting from Legacy to Zen format, write asset data to .ucas files

retoc's implementation of .ucas file handling is designed to work seamlessly with its .utoc file handling, providing a complete solution for working with Unreal Engine's IoStore containers. The library efficiently manages memory usage by only loading the specific chunks needed, rather than loading the entire .ucas file into memory.
