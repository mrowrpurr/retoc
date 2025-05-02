# About IoStore

## Overview

IoStore is Unreal Engine's newer asset storage system introduced in later versions of the engine. It replaces the older .pak file system with a more efficient and feature-rich approach to storing and accessing game assets. Designed to address the limitations of the .pak system, IoStore offers improved performance, better memory management, and enhanced features for modern game development.

## General Information

IoStore (Input/Output Store) is a container system that was introduced in Unreal Engine 5 to replace the older .pak file system. It was designed to address several key challenges in modern game development:

1. **Performance**: Faster loading times, especially for large games with many assets
2. **Scalability**: Better handling of large game projects with thousands of assets
3. **Memory Efficiency**: Improved memory usage during asset loading
4. **Streaming**: Enhanced support for asset streaming and on-demand loading
5. **Modularity**: Better organization of game content for easier updates and patching

The IoStore system uses a split approach to storing game assets:
- Metadata and indexing information is stored in .utoc files (Unreal Table of Contents)
- The actual asset data is stored in .ucas files (Unreal Content Archive Storage)

This separation allows for more efficient asset lookup and loading, as the engine can quickly scan the smaller .utoc files to locate assets without having to parse the entire content archive.

## Architecture

The IoStore architecture is built around several key concepts:

### 1. Chunk-Based Storage
- Game assets are divided into "chunks" of data
- Each chunk has a unique ID and can be accessed independently
- Different types of chunks exist for different types of data (e.g., export data, bulk data, shader code)

### 2. Container System
- Assets are stored in containers, each with its own .utoc and .ucas files
- Containers can be prioritized, allowing for patch containers to override base game content
- Multiple containers can be mounted simultaneously

### 3. Directory Index
- A hierarchical representation of the file structure
- Maps virtual file paths to chunk IDs
- Supports mount points for organizing content

### 4. Package Store
- Manages package metadata and dependencies
- Handles package redirects and localization
- Tracks relationships between packages

### 5. Asynchronous Loading
- Designed for efficient multi-threaded loading
- Supports prioritization of critical assets
- Enables streaming of content during gameplay

## Components

The IoStore system consists of several key components:

### .utoc Files (Table of Contents)
- Contain metadata and indexing information
- Store chunk IDs, offsets, and sizes
- Include compression information
- Provide directory indexing
- Support for encryption
- Store container header information

### .ucas Files (Content Archive Storage)
- Store the actual asset data in chunks
- Support for compressed and encrypted data
- Can be partitioned for better performance
- Organized in blocks for efficient access

### Container Header
- Contains information about packages in the container
- Includes package IDs, store entries, and other package-related metadata
- Supports localized packages and package redirects
- Stored within the .utoc file

### Chunk System
- Each chunk represents a piece of content
- Different chunk types for different data:
  - ExportBundleData: Main asset data
  - BulkData: Large binary data
  - OptionalBulkData: Optional large binary data
  - MemoryMappedBulkData: Memory-mapped large binary data
  - ShaderCodeLibrary: Shader code libraries
  - ContainerHeader: Container metadata

### Asset Formats
- Zen assets: The native asset format for IoStore
- Legacy assets: The older asset format used with .pak files
- Conversion systems for compatibility between formats

## Implementation Details

The IoStore system is implemented with several sophisticated features:

### 1. Chunk Identification and Lookup
- Chunks are identified by a 12-byte ID
- The ID encodes the package ID, chunk index, and chunk type
- Perfect hash tables are used for efficient chunk lookup
- Overflow tables handle hash collisions

### 2. Compression System
- Multiple compression methods are supported:
  - Zlib
  - Gzip
  - Zstd
  - LZ4
  - Oodle
- Compression is applied at the block level
- Different chunks can use different compression methods
- Compression information is stored in the .utoc file

### 3. Encryption
- AES-256 encryption is supported
- Encryption can be applied to both data and directory index
- Encryption keys are identified by GUID
- Encrypted blocks are aligned to AES block boundaries

### 4. Partitioning
- .ucas files can be split into multiple partitions
- Each partition is a separate file
- Partitioning allows for better parallel loading
- Partition information is stored in the .utoc file

### 5. Package Store Entries
- Track package dependencies
- Store export information
- Reference shader maps
- Support localization and redirects

## Version Differences

The IoStore system has evolved across Unreal Engine versions:

### UE4.26-UE4.27
- Initial implementation of IoStore
- Basic .utoc and .ucas file formats
- Limited features compared to later versions
- Transitional period with both .pak and IoStore support

### UE5.0-UE5.3
- Full adoption of IoStore as the primary container system
- Improved performance and features
- Enhanced support for Zen assets
- Better integration with the engine's asset loading system

### UE5.4-UE5.5
- Advanced features like on-demand metadata
- Optimized for next-generation consoles and PCs
- Improved handling of large assets and shader libraries
- Enhanced partitioning capabilities

## Integration with Unreal Engine

IoStore is deeply integrated with Unreal Engine's asset loading and management systems:

### 1. Asset Loading
- The engine uses IoStore to load assets during gameplay
- Asynchronous loading is supported for better performance
- Streaming systems use IoStore for on-demand content

### 2. Build Pipeline
- The Unreal Engine build system generates IoStore containers
- Cook processes create optimized IoStore containers for different platforms
- Packaging tools manage IoStore containers for game distribution

### 3. Editor Integration
- The Unreal Editor can work with IoStore containers
- Development workflows support IoStore for testing and iteration
- Tools for analyzing and optimizing IoStore containers

### 4. Platform Support
- IoStore is supported across all platforms that Unreal Engine targets
- Platform-specific optimizations are applied for best performance
- Mobile platforms benefit from IoStore's efficient loading

### 5. Modding Support
- IoStore containers can be used for game mods
- Patch containers can override base game content
- Tools like retoc enable modders to work with IoStore containers
