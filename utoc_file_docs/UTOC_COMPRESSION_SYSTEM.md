# UTOC Compression System

## Overview

The Compression System is a key feature of the .utoc file format that allows for efficient storage of game assets. It provides information about how chunks in the .ucas file are compressed, enabling the engine to decompress them when needed. This document explains how the compression system works and how it's implemented in the .utoc file format.

## Purpose

The compression system serves several important purposes:

1. **Reduced File Size**: Compresses chunks to reduce the overall size of the .ucas file
2. **Efficient Storage**: Allows for more content to be stored in the same amount of space
3. **Flexible Compression**: Supports multiple compression methods for different types of data
4. **Block-Based Compression**: Divides chunks into blocks for more efficient compression and decompression

## Binary Structure

The compression system in the .utoc file consists of two main components:

1. **Compression Blocks**: An array of `FIoStoreTocCompressedBlockEntry` structures that describe compressed blocks in the .ucas file
2. **Compression Methods**: An array of compression method names that identify the compression algorithms used

### Compression Blocks

Each compression block is described by a 12-byte `FIoStoreTocCompressedBlockEntry` structure:

```
Offset  Size    Description
------  ------  -----------
0x00    5       Offset (40 bits)
0x05    3       CompressedSize (24 bits)
0x08    3       UncompressedSize (24 bits)
0x0B    1       CompressionMethodIndex
```

- **Offset**: The offset of the block in the .ucas file
- **CompressedSize**: The size of the compressed block in bytes
- **UncompressedSize**: The size of the uncompressed block in bytes
- **CompressionMethodIndex**: An index into the compression methods array (0 = uncompressed)

### Compression Methods

The compression methods are stored as an array of fixed-length strings (typically 32 bytes each), padded with zeros:

```
Offset  Size    Description
------  ------  -----------
0x00    32      CompressionMethodName[0]
0x20    32      CompressionMethodName[1]
...     ...     ...
```

Common compression methods include:
- Zlib
- Gzip
- Zstd
- LZ4
- Oodle

## How It Works

### Compression Process

The compression process works as follows:

1. **Block Division**: Chunks are divided into blocks of a fixed size (specified by `CompressionBlockSize` in the header)
2. **Compression Application**: Each block is compressed using a selected compression method
3. **Block Recording**: Information about each compressed block is recorded in the .utoc file
4. **Method Recording**: The compression methods used are recorded in the .utoc file

### Decompression Process

The decompression process works as follows:

1. **Block Identification**: Identify which blocks contain the requested chunk data
2. **Block Reading**: Read the compressed blocks from the .ucas file
3. **Method Identification**: Determine the compression method used for each block
4. **Decompression Application**: Apply the appropriate decompression algorithm to each block
5. **Data Assembly**: Assemble the decompressed blocks into the complete chunk data

## Compression Block Size

The `CompressionBlockSize` field in the .utoc header specifies the size of uncompressed blocks. This value is typically a power of 2, such as 65536 (64 KB).

The block size affects several aspects of the compression system:

1. **Compression Efficiency**: Larger blocks can achieve better compression ratios
2. **Memory Usage**: Larger blocks require more memory during decompression
3. **Random Access**: Smaller blocks allow for more efficient random access to parts of a chunk
4. **Parallelism**: Smaller blocks can be decompressed in parallel more effectively

## Alignment and Padding

Compressed blocks in the .ucas file are subject to alignment and padding requirements:

1. **Compression Alignment**: Blocks are aligned to the compression block size
2. **Encryption Alignment**: If encryption is used, blocks are aligned to AES block boundaries (16 bytes)
3. **Padding**: Blocks may be padded to meet alignment requirements

## Chunk to Block Mapping

A single chunk may span multiple compression blocks:

1. **Starting Block**: The block containing the start of the chunk
2. **Ending Block**: The block containing the end of the chunk
3. **Intermediate Blocks**: Any blocks between the starting and ending blocks

To read a chunk, all blocks containing any part of the chunk must be read and decompressed.

## Compression Method Selection

The selection of compression methods depends on several factors:

1. **Data Type**: Different types of data compress better with different methods
2. **Performance Requirements**: Some methods are faster but achieve lower compression ratios
3. **Platform Constraints**: Some platforms may have limited support for certain compression methods
4. **License Considerations**: Some compression methods (like Oodle) require licensing

## Implementation Details

### Compressed Block Entry

The `FIoStoreTocCompressedBlockEntry` structure uses a compact representation to save space:

1. **Offset**: 40 bits (5 bytes) allowing for .ucas files up to 1 TB
2. **Compressed Size**: 24 bits (3 bytes) allowing for blocks up to 16 MB
3. **Uncompressed Size**: 24 bits (3 bytes) allowing for blocks up to 16 MB
4. **Compression Method Index**: 8 bits (1 byte) allowing for up to 256 compression methods

### Reading Compressed Blocks

To read a compressed block:

1. Seek to the block's offset in the .ucas file
2. Read the compressed data (size = CompressedSize)
3. If the block is encrypted, decrypt it
4. If CompressionMethodIndex is not 0, decompress the data using the specified method
5. Verify that the decompressed size matches UncompressedSize

### Writing Compressed Blocks

To write a compressed block:

1. Compress the data using the selected method
2. If encryption is used, encrypt the compressed data
3. Write the compressed data to the .ucas file
4. Record the offset, compressed size, uncompressed size, and compression method index in the .utoc file

## Version Differences

The compression system has remained relatively stable across versions of the .utoc file format, with most changes affecting other aspects of the format.

## Performance Considerations

The compression system affects performance in several ways:

1. **Load Time**: Decompression adds overhead to loading times
2. **Disk Space**: Compression reduces the amount of disk space required
3. **Memory Usage**: Decompression requires additional memory for buffers
4. **CPU Usage**: Decompression requires CPU resources

## Implementation Considerations

When implementing a reader or writer for the compression system:

1. **Compression Library Support**: Implement or integrate libraries for all supported compression methods
2. **Efficient Memory Management**: Minimize memory allocations and copies during decompression
3. **Parallel Decompression**: Consider decompressing multiple blocks in parallel for better performance
4. **Error Handling**: Handle compression and decompression errors gracefully

## Conclusion

The compression system is a vital component of the .utoc file format that significantly reduces the size of game assets while maintaining efficient access. By understanding how it works and how to implement it, developers can effectively work with compressed data in IoStore containers.
