# UCAS File Format Documentation

This folder contains comprehensive documentation for the UCAS (Unreal Content Archive Storage) file format used in Unreal Engine's IoStore container system.

## Overview

The UCAS file format is part of Unreal Engine's IoStore container system, introduced in Unreal Engine 5 as a replacement for the older .pak file system. UCAS files work in conjunction with UTOC (Unreal Table of Contents) files to form a complete IoStore container.

While UTOC files contain metadata and indexing information, UCAS files store the actual content/data of game assets. This separation allows for efficient access to specific assets without having to scan the entire archive.

## Documentation Files

This folder contains the following documentation files:

### 1. [UCAS_BINARY_FILE_LAYOUT.md](UCAS_BINARY_FILE_LAYOUT.md)

This document describes the binary layout of UCAS files, including:
- File structure
- Data blocks
- Accessing data
- Compression
- Encryption
- Partitioning
- Version differences
- Relationship with UTOC files

### 2. [UCAS_DETAILED_INFORMATION.md](UCAS_DETAILED_INFORMATION.md)

This document provides detailed information about the UCAS file format, including:
- Purpose and role in Unreal Engine
- Relationship with UTOC files
- Detailed structure
- Technical features
- Version evolution
- Implementation considerations

### 3. [ucas_RESEARCH_NOTES_SCRATCHPAD.md](ucas_RESEARCH_NOTES_SCRATCHPAD.md)

This document contains research notes and findings from analyzing the retoc codebase, including:
- Source code findings
- Binary file layout analysis
- Compression and encryption mechanisms
- Partitioning mechanisms
- Version differences
- Implementation notes

### 4. [ucas.bt](ucas.bt) and [ucas_advanced.bt](ucas_advanced.bt)

These are 010 Editor binary templates for parsing UCAS files:
- `ucas.bt`: A basic template that shows the raw data in the UCAS file
- `ucas_advanced.bt`: An advanced template that can work with metadata from a UTOC file

## Key Characteristics

From the documentation, the key characteristics of UCAS files are:

- Store the actual asset data in chunks
- Support compressed data using various methods (Zlib, Gzip, Zstd, LZ4, Oodle)
- Support encrypted data (AES-256)
- Are organized in blocks for efficient access
- Can be partitioned for better performance (e.g., global.ucas.0, global.ucas.1)

## Implementation Considerations

When implementing a reader or writer for UCAS files, consider the following:

### Reading UCAS Files

1. **File Pooling**: Use a file pool to manage file handles efficiently, especially for parallel processing
2. **Chunk Reading**: Reading chunks involves finding the chunk in the UTOC file, determining the offset and length, reading the data, and processing it
3. **Partitioning**: If the UCAS file is partitioned, determine which partition contains the data and open the appropriate file

### Writing UCAS Files

1. **Block Size**: Use the standard block size of 0x10000 (65536) bytes
2. **Chunk Writing**: Divide the data into blocks, write each block, and record the metadata in the UTOC file
3. **Hashing**: Hash each chunk using Blake3 for verification

## Conclusion

The UCAS file format is a key component of Unreal Engine's IoStore container system. It provides efficient storage for game assets, with support for compression, encryption, and partitioning. Understanding the UCAS file format is essential for working with Unreal Engine's asset system, especially when developing tools for asset management or game modding.
