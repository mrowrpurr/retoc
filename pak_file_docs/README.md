# Unreal Engine .pak File Format Documentation

## Overview

This repository contains comprehensive documentation for the Unreal Engine .pak file format. The .pak file format is Unreal Engine's archive format used for packaging game assets, similar to ZIP or TAR files but with specific features tailored for Unreal Engine.

## Documentation Files

This documentation is organized into several files:

1. **[PAK_BINARY_FILE_LAYOUT.md](PAK_BINARY_FILE_LAYOUT.md)** - Detailed description of the binary structure of .pak files, including offsets, field types, and version differences.

2. **[PAK_DETAILED_INFORMATION.md](PAK_DETAILED_INFORMATION.md)** - Implementation details, best practices, and usage patterns for working with .pak files.

3. **[PAK_FNV64_HASH_ALGORITHM.md](PAK_FNV64_HASH_ALGORITHM.md)** - Detailed explanation of the FNV-64 hash algorithm used in .pak files for path hashing.

4. **[PAK_ENCRYPTION_GUIDE.md](PAK_ENCRYPTION_GUIDE.md)** - Comprehensive guide to encryption in .pak files, including implementation details and best practices.

5. **[CPP_SUPPORT_NOTES.md](CPP_SUPPORT_NOTES.md)** - Implementation notes for creating a C++ static library with full .pak file support, including dependencies, challenges, and code examples.

6. **[PAK_010_EDITOR_TEMPLATE.bt](PAK_010_EDITOR_TEMPLATE.bt)** - A template for the 010 Editor hex editor that allows for visual inspection and parsing of .pak files.

7. **[PAK_RESEARCH_NOTES_SCRATCHPAD.md](PAK_RESEARCH_NOTES_SCRATCHPAD.md)** - Research notes and findings from analyzing the .pak file format.

## Key Features of .pak Files

- **Versioning**: Multiple versions (V1-V11) with different capabilities
- **Compression**: Support for multiple compression algorithms (Zlib, Gzip, Oodle, Zstd, LZ4)
- **Encryption**: AES-256 encryption for content protection
- **Mount Points**: Virtual path system for organizing content
- **Efficient Indexing**: Fast file lookup through various index structures
- **Compatibility**: Support across all Unreal Engine versions from 4.0 to 5.3+

## File Structure

A .pak file consists of four main sections:

1. **Data Blocks**: The actual file contents
2. **Index**: Directory of files in the archive
3. **Secondary Indices** (V10+): Additional indices for improved performance
4. **Footer**: Metadata about the archive

## Version History

| Version | UE Version   | Key Features                                 |
| ------- | ------------ | -------------------------------------------- |
| V1      | Pre-4.0      | Initial specification with timestamps        |
| V2      | UE 4.0-4.2   | Removed timestamps                           |
| V3      | UE 4.3-4.15  | Added compression and encryption support     |
| V4      | UE 4.16-4.19 | Added index encryption support               |
| V5      | UE 4.20      | Changed to relative chunk offsets            |
| V6      | -            | Added delete records support                 |
| V7      | UE 4.21      | Added encryption key GUID                    |
| V8A     | UE 4.22      | Added FName-based compression (4 methods)    |
| V8B     | UE 4.23-4.24 | Extended FName-based compression (5 methods) |
| V9      | UE 4.25      | Added frozen index support                   |
| V10     | -            | Added path hash index                        |
| V11     | UE 4.26-5.3+ | Fixed FNV64 hash bug                         |

## Tools for Working with .pak Files

- **UnrealPak**: Official tool included with Unreal Engine
- **repak**: High-performance third-party library (2-30x faster than UnrealPak)
- **UEViewer (umodel)**: Tool for viewing and extracting assets
- **FModel**: Advanced viewer for Unreal Engine assets
- **010 Editor with PAK Template**: For binary analysis of .pak files

## Relationship to IoStore (.utoc/.ucas)

While .pak files were the primary asset container format in Unreal Engine 4, Unreal Engine 5 introduced the newer IoStore container system (.utoc/.ucas) as the preferred format. However, .pak files are still supported in UE5 for backward compatibility.

Tools like retoc provide functionality to convert between these formats:
- Converting from Zen assets (IoStore) to Legacy assets (.pak)
- Converting from Legacy assets (.pak) to Zen assets (IoStore)

## References

This documentation is based on analysis of the repak library source code, which provides comprehensive support for reading and writing .pak files across all major versions.
