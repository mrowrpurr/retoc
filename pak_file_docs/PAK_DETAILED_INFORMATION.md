# PAK File Format - Detailed Information

## Introduction

The .pak file format is Unreal Engine's archive format used for packaging game assets. This document provides detailed information about the .pak file format, focusing on implementation details, best practices, and usage patterns for developers working with these files.

## Core Concepts

### Mount Points

Mount points are virtual path prefixes that define how files within a .pak archive are mapped to the game's virtual file system. When a .pak file is mounted, its contents become accessible through these virtual paths.

```
Example:
Mount Point: "/Game/"
File in .pak: "Textures/Landscape/grass.uasset"
Virtual Path: "/Game/Textures/Landscape/grass.uasset"
```

Mount points allow multiple .pak files to be mounted simultaneously, with their contents organized in a unified virtual file system. This is particularly useful for:

- Base game content
- DLC and expansion packs
- Mods and user-generated content
- Localization files

### File Entries

Each file in a .pak archive is represented by a file entry, which contains metadata about the file:

- Virtual path within the archive
- Offset to the file data
- Size of the file data (compressed and uncompressed)
- Compression method used (if any)
- Hash for verification
- Additional flags and metadata

File entries are stored in the index section of the .pak file, allowing the engine to quickly locate and access files without scanning the entire archive.

### Compression

The .pak file format supports multiple compression methods to reduce file size:

1. **Zlib**: General-purpose compression with good balance of speed and compression ratio
2. **Gzip**: Similar to Zlib but with different header format
3. **Oodle**: High-performance proprietary compression by RAD Game Tools
4. **Zstd**: Modern compression algorithm with excellent compression ratio and speed
5. **LZ4**: Very fast compression algorithm with moderate compression ratio

Compression can be applied at the file level, with each file potentially using a different compression method. Files can also be divided into blocks, with each block compressed independently, allowing for more efficient random access to compressed data.

### Encryption

The .pak file format supports AES-256 encryption in ECB mode for protecting content:

1. **Index Encryption**: Only the index is encrypted, protecting file names and metadata
2. **Data Encryption**: File data is encrypted, protecting the actual content
3. **Encryption Key GUID**: A GUID that identifies which encryption key to use

Encryption is particularly useful for:
- Protecting proprietary assets
- Preventing unauthorized access to content
- DRM implementation
- Securing pre-release content

## Implementation Details

### Reading .pak Files

The process of reading a .pak file involves several steps:

1. **Read Footer**:
   - Seek to the end of the file minus the footer size
   - Read the footer to get the index location, size, and other metadata
   - Verify the magic number to confirm it's a valid .pak file

2. **Read Index**:
   - Seek to the index offset
   - Read the index data
   - If the index is encrypted, decrypt it using the provided AES key

3. **Parse Index**:
   - Read the mount point
   - Read the number of entries
   - Parse each entry to build a directory of files
   - In V10+, parse additional indices (path hash index, full directory index)

4. **Access Files**:
   - To read a file, find its entry in the index
   - Seek to the file data offset
   - Read the file data
   - If the file is compressed, decompress it
   - If the file is encrypted, decrypt it

### Writing .pak Files

Creating a .pak file involves these steps:

1. **Initialize Writer**:
   - Create a new file
   - Set the mount point
   - Initialize the index structure

2. **Add Files**:
   - For each file to add:
     - Determine if compression should be used
     - Write the file data to the archive
     - Create an entry in the index with the file's metadata

3. **Write Index**:
   - Write the index structure with all file entries
   - In V10+, generate and write additional indices
   - Calculate the SHA1 hash of the index

4. **Write Footer**:
   - Write the footer with the index offset, size, and hash
   - Write the magic number and version information

### Path Hash Index (V10+)

The path hash index provides a fast way to look up files by their path hash:

1. **Hash Calculation**:
   - Convert the path to lowercase
   - Convert to UTF-16
   - Apply the FNV-64 hash algorithm with the path hash seed

2. **Index Structure**:
   - Each entry maps a path hash to an offset in the encoded entries section
   - Binary search can be used for efficient lookups

3. **Lookup Process**:
   - Calculate the hash of the requested path
   - Search the path hash index for the hash
   - If found, read the encoded entry at the specified offset

### Full Directory Index (V10+)

The full directory index provides a hierarchical view of the files in the archive:

1. **Structure**:
   - Directories are organized in a tree structure
   - Each directory contains a list of files and subdirectories
   - Each file entry points to an encoded entry in the encoded entries section

2. **Usage**:
   - Efficient directory listing
   - Path traversal operations
   - Finding files in specific directories

## Version-Specific Features

### V1-V2: Basic Functionality

- Simple index structure with file entries
- No compression or encryption
- V1 includes timestamps, removed in V2

### V3: Compression and Encryption

- Added support for compression (Zlib, Gzip, Oodle)
- Added support for encryption
- Introduced compression blocks for random access

### V4: Index Encryption

- Added ability to encrypt just the index
- Improved security while maintaining performance

### V5: Relative Chunk Offsets

- Changed to relative chunk offsets for better compatibility
- Improved handling of large files

### V6: Delete Records

- Added support for marking files as deleted
- Useful for patching and updates

### V7: Encryption Key GUID

- Added GUID for identifying encryption keys
- Supports multiple encryption keys

### V8: FName-Based Compression

- Added named compression methods
- V8A supports 4 compression methods
- V8B extends to 5 compression methods

### V9: Frozen Index

- Added support for frozen indices
- Optimizes loading performance

### V10: Path Hash Index

- Added path hash index for faster lookups
- Added full directory index for better directory traversal
- Introduced encoded entries for more compact storage

### V11: FNV64 Bug Fix

- Fixed a bug in the FNV-64 hash calculation
- Current version used in modern games

## Best Practices

### Organizing Content

When creating .pak files, consider these organizational strategies:

1. **Logical Grouping**:
   - Group related assets together in the same .pak file
   - Consider access patterns when organizing files

2. **Chunking Strategy**:
   - Split content into multiple .pak files for better loading and patching
   - Common approaches:
     - By level or map
     - By asset type (textures, sounds, models)
     - By content module (UI, gameplay, cinematics)

3. **Patch-Friendly Structure**:
   - Organize content to minimize patch sizes
   - Keep frequently updated content separate from stable content

### Compression Strategies

Optimize compression based on content type and access patterns:

1. **By Content Type**:
   - Textures: Already compressed, use light compression or none
   - Audio: Already compressed, use light compression or none
   - Text/JSON: Benefits from strong compression
   - Meshes: Moderate compression works well

2. **By Access Pattern**:
   - Frequently accessed files: Use faster decompression (LZ4)
   - Rarely accessed files: Use stronger compression (Zstd, Oodle)
   - Streaming content: Consider block-based compression

3. **Block Size Considerations**:
   - Smaller blocks: Better random access, worse compression ratio
   - Larger blocks: Better compression ratio, worse random access
   - Typical block sizes: 64KB - 1MB

### Encryption Considerations

When using encryption, consider these factors:

1. **Performance Impact**:
   - Encryption adds processing overhead
   - Consider encrypting only sensitive content

2. **Key Management**:
   - Secure storage of encryption keys
   - Key rotation strategies
   - Using the encryption key GUID for key identification

3. **Selective Encryption**:
   - Index encryption protects file names and structure
   - Data encryption protects the actual content
   - Consider which level of protection is needed

### Mounting Strategies

Optimize how .pak files are mounted in the engine:

1. **Priority Order**:
   - .pak files are mounted in priority order
   - Higher priority files override lower priority files with the same path
   - Useful for mods and patches

2. **On-Demand Mounting**:
   - Mount .pak files only when needed
   - Unmount when no longer required
   - Reduces memory usage and improves performance

3. **Patch Layering**:
   - Base game in lower priority .pak files
   - Patches and updates in higher priority .pak files
   - Allows for efficient patching without replacing the entire base content

## Common Challenges and Solutions

### Large File Handling

Working with very large .pak files presents challenges:

1. **Memory Constraints**:
   - Reading the entire index into memory may be problematic
   - Solution: Stream the index or use memory-mapped files

2. **Seek Performance**:
   - Random access in large files can be slow
   - Solution: Optimize file organization, use SSD storage

3. **Build Times**:
   - Creating large .pak files can be time-consuming
   - Solution: Parallel processing, incremental builds

### Patching and Updates

Efficiently updating .pak files:

1. **Differential Patching**:
   - Create patch .pak files containing only changed files
   - Mount patch .pak files with higher priority

2. **Content Chunking**:
   - Split content into logical chunks
   - Update only the chunks that have changed

3. **Version Management**:
   - Track content versions within .pak files
   - Use version information for compatibility checks

### Cross-Platform Considerations

Supporting multiple platforms:

1. **Endianness**:
   - .pak files use little-endian byte order
   - Ensure correct byte order handling on all platforms

2. **Path Separators**:
   - Standardize on forward slashes (/) for paths
   - Handle platform-specific path conventions

3. **Platform-Specific Optimization**:
   - Consider different compression strategies per platform
   - Adjust block sizes based on platform capabilities

## Tools and Libraries

### Official Tools

1. **UnrealPak**:
   - Official tool included with Unreal Engine
   - Supports creating, extracting, and listing .pak files
   - Command-line interface

2. **Unreal Editor**:
   - Can create .pak files as part of the build process
   - Integrated with the engine's asset system

### Third-Party Tools

1. **repak**:
   - High-performance library for working with .pak files
   - Significantly faster than UnrealPak (2-30x)
   - Supports all major .pak file versions

2. **UEViewer (umodel)**:
   - Tool for viewing and extracting assets from .pak files
   - Useful for content inspection and analysis

3. **FModel**:
   - Advanced viewer for Unreal Engine assets
   - Supports .pak file browsing and extraction

### Programming Interfaces

When implementing .pak file handling in custom tools:

1. **File Format Compliance**:
   - Follow the binary format exactly
   - Handle all version differences correctly

2. **Error Handling**:
   - Validate magic numbers and checksums
   - Gracefully handle corrupted or incomplete files

3. **Performance Optimization**:
   - Use memory mapping for large files
   - Implement parallel processing where appropriate
   - Cache frequently accessed metadata

## Future Considerations

### IoStore Integration

As Unreal Engine transitions to the IoStore container system (.utoc/.ucas), consider:

1. **Migration Path**:
   - Tools like retoc for converting between formats
   - Supporting both formats during transition

2. **Feature Parity**:
   - Understanding the differences between .pak and IoStore
   - Ensuring all required functionality is available

3. **Performance Comparison**:
   - Benchmarking .pak vs. IoStore for specific use cases
   - Making informed decisions about which format to use

### Custom Extensions

The .pak format can be extended for specific needs:

1. **Custom Metadata**:
   - Adding additional information to the index
   - Storing project-specific data

2. **Alternative Compression**:
   - Implementing custom compression algorithms
   - Optimizing for specific content types

3. **Enhanced Security**:
   - Implementing additional security measures
   - Custom encryption or obfuscation techniques

## Conclusion

The .pak file format is a powerful and flexible system for packaging game assets in Unreal Engine. Understanding its structure, capabilities, and best practices allows developers to optimize content delivery, improve performance, and enhance security in their games.

By leveraging the features of the .pak file format and following the guidelines in this document, developers can create efficient, secure, and maintainable asset packaging solutions for their Unreal Engine projects.
