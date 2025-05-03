# Unreal Engine File Formats and Projects Overview

## Introduction

This document provides an overview of the Unreal Engine file formats (.pak, .utoc, and .ucas) and the associated projects (retoc and repak) that work with these formats. Understanding these file formats and tools is essential for game modding, asset extraction, and custom content creation for Unreal Engine games.

## Unreal Engine File Formats

### .pak Files

The `.pak` file format is Unreal Engine's traditional archive format used for packaging game assets, similar to ZIP or TAR files but with specific features tailored for Unreal Engine.

**Key Characteristics:**
- Used primarily in Unreal Engine 4, but still supported in UE5 for backward compatibility
- Contains both asset data and metadata in a single file
- Supports multiple versions (V1-V11) with different capabilities
- Supports compression (Zlib, Gzip, Oodle, Zstd, LZ4)
- Supports encryption (AES-256 in ECB mode)
- Uses mount points for organizing content in a virtual file system
- Includes efficient indexing for fast file lookup

**Structure:**
1. **Data Blocks**: The actual file contents
2. **Index**: Directory of files in the archive
3. **Secondary Indices** (V10+): Additional indices for improved performance
4. **Footer**: Metadata about the archive

### .utoc Files (Unreal Table of Contents)

The `.utoc` file format is part of Unreal Engine's newer IoStore container system introduced in UE5. It contains metadata and indexing information for game assets.

**Key Characteristics:**
- Introduced in Unreal Engine 5 as part of the IoStore container system
- Contains metadata, indexing information, and directory structure
- Works in conjunction with .ucas files
- Supports multiple versions with evolving features
- Uses a chunk-based system for organizing data
- Implements a perfect hash system for efficient chunk lookup
- Supports encryption and compression information

**Structure:**
1. **Header**: Basic information about the container
2. **Chunk IDs**: Array of unique identifiers for chunks
3. **Chunk Offsets and Lengths**: Location and size of each chunk in the .ucas file
4. **Hash Map**: Perfect hash table for efficient chunk lookup
5. **Compression Blocks**: Information about compressed blocks
6. **Compression Methods**: List of compression methods used
7. **Directory Index**: Hierarchical representation of the file structure
8. **Chunk Metadata**: Hash and flags for each chunk

### .ucas Files (Unreal Content Archive Storage)

The `.ucas` file format is the storage component of the IoStore container system, containing the actual binary data of game assets.

**Key Characteristics:**
- Stores the actual content/data of game assets
- Works in conjunction with .utoc files
- Organized as a series of data blocks
- Supports compression (Zlib, Zstd, LZ4, Oodle)
- Supports encryption (AES-256)
- Can be partitioned for better performance

**Structure:**
- A series of data blocks, each corresponding to a chunk referenced by the .utoc file
- Each block may be compressed and/or encrypted
- Blocks are typically 65536 bytes (0x10000) in size
- Partitioned .ucas files are split into multiple files with numeric suffixes

## Projects

### retoc

**retoc** is a tool for working with Unreal Engine's IoStore container system (.utoc/.ucas files). It provides functionality for reading, writing, and converting these files.

**Key Features:**
- Reading and extracting content from .utoc/.ucas files
- Creating new .utoc/.ucas files
- Converting between Zen assets (IoStore) and Legacy assets (.pak)
- Support for compression and encryption
- Directory index manipulation
- Container header management

**Components:**
- IoStore reader and writer
- Compression system
- Encryption system
- Directory index handling
- Container header parsing and creation
- Asset conversion utilities

### repak

**repak** is a high-performance library for working with Unreal Engine's .pak files. It is significantly faster than the official UnrealPak tool (2-30x faster).

**Key Features:**
- Reading and extracting content from .pak files
- Creating new .pak files
- Support for all major .pak file versions (V1-V11)
- Support for compression and encryption
- Efficient indexing and file lookup
- Mount point management

**Components:**
- Pak reader and writer
- Compression system
- Encryption system
- Index management
- Mount point handling
- Hash calculation utilities

## Relationship Between Formats

The relationship between these file formats reflects the evolution of Unreal Engine's asset storage system:

1. **UE4 Era**: .pak files were the primary asset container format
2. **UE5 Era**: IoStore (.utoc/.ucas) became the preferred format, with .pak files supported for backward compatibility

The IoStore system offers several advantages over the .pak system:
- Better performance through separation of metadata and content
- More efficient asset lookup with the perfect hash system
- Enhanced support for streaming and on-demand loading
- Improved memory usage during asset loading
- Better scalability for large game projects

Tools like retoc provide functionality to convert between these formats:
- Converting from Zen assets (IoStore) to Legacy assets (.pak)
- Converting from Legacy assets (.pak) to Zen assets (IoStore)

## Conclusion

Understanding the .pak, .utoc, and .ucas file formats is essential for working with Unreal Engine games, whether for modding, asset extraction, or custom content creation. The retoc and repak projects provide powerful tools for manipulating these formats, offering capabilities beyond what's available in the official Unreal Engine tools.

As Unreal Engine continues to evolve, the IoStore system (.utoc/.ucas) is becoming increasingly important, while the .pak system remains relevant for backward compatibility and working with older games. Having a solid understanding of both systems and the tools to work with them is valuable for anyone involved in Unreal Engine game development or modding.
