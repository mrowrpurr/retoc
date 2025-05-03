# Overview of retoc, repak, and Unreal Engine File Formats

## Introduction

This document provides an overview of the retoc and repak projects, as well as the Unreal Engine file formats they work with: .pak, .utoc, and .ucas. These projects and file formats are essential for working with Unreal Engine game assets, particularly for modding, asset extraction, and conversion between different Unreal Engine versions.

## Projects

### retoc

retoc is a CLI tool for working with Unreal Engine's IoStore containers (.utoc/.ucas) and converting between different asset formats. It serves as a comprehensive solution for handling Unreal Engine's newer asset storage system.

**Key Features:**
- Extract and manipulate IoStore containers (.utoc/.ucas files)
- Convert between Zen assets (used with IoStore) and Legacy assets (used with .pak files)
- Handle shader libraries during conversion
- Support a wide range of Unreal Engine versions (particularly UE 5.3+)

retoc is particularly valuable for:
- Game modders who need to work with UE5 games
- Developers migrating assets between different UE versions
- Tools developers building asset pipelines for UE games

### repak

repak is a library and CLI tool for working with Unreal Engine .pak files. It provides functionality for reading, writing, and manipulating .pak files across various Unreal Engine versions.

**Key Features:**
- Efficient .pak file handling with 2-30x faster unpacking than the official UnrealPak tool
- Support for all major .pak file versions (UE4.0 to UE5.3+)
- Comprehensive support for compression, encryption, and various index formats
- Clean API for integration with other applications

repak is integrated with retoc to provide complete support for both .pak files and IoStore containers, enabling seamless conversion between the two asset storage systems.

## Unreal Engine File Formats

### .pak Files

.pak files are Unreal Engine's traditional archive format used for packaging game assets in UE4 and early UE5 versions. They serve as containers for game files, allowing for efficient storage, loading, and distribution of game content.

**Key Characteristics:**
- Container format similar to ZIP or TAR, but tailored for Unreal Engine
- Support for compression to reduce file size
- Support for encryption to protect content
- Efficient indexing for fast asset lookup
- Mount point system for virtual file paths

**Structure:**
1. **File Header:** Contains a magic number and version information
2. **Data Blocks:** The actual file content, potentially compressed and/or encrypted
3. **Index:** A directory of all files in the .pak
4. **Footer:** Contains information about the index location and encryption

**Version Evolution:**
The .pak format has evolved from V1 (pre-UE4.0) to V11 (UE4.26-5.3+), with significant improvements in compression, encryption, and indexing along the way.

### .utoc Files

.utoc (Unreal Table of Contents) files are part of Unreal Engine's IoStore container system introduced in UE5. They contain metadata and indexing information for the content stored in corresponding .ucas files.

**Key Characteristics:**
- Contains a table of contents for the IoStore container
- Stores chunk IDs, offsets, and sizes for assets in the .ucas file
- Includes compression information
- Provides directory indexing for efficient file lookup
- Support for encryption

**Structure:**
1. **Header:** Contains a magic number and version information
2. **Chunk IDs:** Identifiers for chunks of data in the .ucas file
3. **Chunk Offsets and Lengths:** Location and size of each chunk
4. **Hash Map:** Perfect hash table for chunk lookup
5. **Compression Blocks:** Information about compressed blocks
6. **Directory Index:** Hierarchical file structure
7. **Chunk Metadata:** Additional information about each chunk

**Version Evolution:**
The .utoc format has evolved from Initial (UE4.26) to ReplaceIoChunkHashWithIoHash (UE5.5+), with improvements in hashing, indexing, and metadata handling.

### .ucas Files

.ucas (Unreal Content Archive Storage) files store the actual content/data of game assets, while the corresponding .utoc files contain the metadata and indexing information needed to access this content.

**Key Characteristics:**
- Store the actual asset data in chunks
- Support for compressed data
- Support for encrypted data
- Organized in blocks for efficient access
- Can be partitioned for better performance

**Structure:**
The .ucas file consists of a series of data blocks, each containing the raw data for a specific chunk. The location and size of each block are specified in the corresponding .utoc file.

**Compression Support:**
- Zlib
- Gzip
- Zstd
- LZ4
- Oodle

## Relationship Between Formats

In Unreal Engine's asset storage systems, there are two main approaches:

1. **Traditional .pak System (UE4 and early UE5):**
   - Assets are stored in .pak files
   - Each asset consists of multiple files (.uasset, .uexp, .ubulk)
   - Simple but less efficient for modern games

2. **IoStore System (UE5):**
   - Assets are stored in .utoc/.ucas file pairs
   - Assets are divided into "chunks" of data
   - More efficient for loading and streaming
   - Better memory management

retoc provides the ability to convert between these two systems, allowing for compatibility across different engine versions and configurations.

## Asset Formats

Unreal Engine has two primary asset storage formats:

1. **Legacy Assets:**
   - Used in Unreal Engine 4 and still supported in Unreal Engine 5
   - Stored in .pak files
   - Split into multiple files (.uasset, .uexp, .ubulk)
   - More compatible with older engine versions

2. **Zen Assets:**
   - Introduced in Unreal Engine 5
   - Stored in IoStore containers (.utoc/.ucas)
   - More efficient storage and loading
   - Better performance for modern platforms

The conversion between these formats involves:
- Asset parsing and reconstruction
- Chunk creation and extraction
- Dependency resolution
- Shader conversion

## Current Implementation Status

The retoc and repak projects are currently in development, with the following status:

1. **Ruby Implementation**: The original implementation in Ruby is functional and provides comprehensive support for working with .pak, .utoc, and .ucas files.

2. **C++ Port**: A C++ port is currently in progress, with the following components:
   - Static libraries for pak, utoc, and ucas file formats
   - A general-purpose utility CLI (pak_utoc_ucas.exe)
   - Integration with the Oodle compression library

3. **Current Challenges**:
   - The C++ implementation is still incomplete, with many stub/placeholder functions
   - The binary doesn't fully support reading real .utoc files yet
   - The .pak command has stability issues
   - Full binary format parsing according to the documentation needs to be implemented

## Conclusion

The retoc and repak projects provide powerful tools for working with Unreal Engine's asset storage systems. By understanding the .pak, .utoc, and .ucas file formats, developers and modders can effectively manipulate game assets across different Unreal Engine versions.

These tools and the knowledge of the underlying file formats are essential for:
- Game modding
- Asset extraction and conversion
- Cross-version compatibility
- Custom asset pipelines
- Game analysis and research

The ongoing C++ port will provide improved performance and better integration capabilities for C++ applications, making these tools even more valuable for developers and modders working with Unreal Engine assets.
