# About .pak Files

## General Information

.pak files are Unreal Engine's archive format used for packaging game assets. They serve as containers for game files, allowing for efficient storage, loading, and distribution of game content. These files are essentially archives that bundle multiple game assets together, similar to ZIP or TAR files, but with specific features tailored for Unreal Engine.

.pak files were the primary asset container format in Unreal Engine 4 and are still supported in Unreal Engine 5, although UE5 introduced the newer IoStore container system (.utoc/.ucas) as the preferred format.

Key characteristics of .pak files include:
- Support for compression to reduce file size
- Support for encryption to protect content
- Efficient indexing for fast asset lookup
- Mount point system for virtual file paths
- Version-specific features that evolved with Unreal Engine

## File Format

The .pak file format consists of several key components:

### 1. File Header
- Contains a magic number (0x5A6F12E1) to identify the file as a .pak file
- Includes version information

### 2. File Entries
- Each entry represents a file within the .pak
- Contains metadata such as:
  - File path
  - Offset within the .pak
  - Size (compressed and uncompressed)
  - Compression method (if used)
  - Hash for verification

### 3. Index
- A directory of all files in the .pak
- Allows for quick lookup of files without scanning the entire archive
- In newer versions, includes a path hash index for faster lookups

### 4. Footer
- Contains information about the index location
- Includes encryption information if applicable
- Contains hash of the index for verification

### 5. Data Blocks
- The actual file content, potentially compressed and/or encrypted
- Organized in blocks for efficient access

## Version Differences

The .pak file format has evolved significantly across Unreal Engine versions:

| Version | UE Version   | Key Features                                               |
| ------- | ------------ | ---------------------------------------------------------- |
| V1      | Pre-4.0      | Initial specification                                      |
| V2      | UE 4.0-4.2   | Removed timestamps                                         |
| V3      | UE 4.3-4.15  | Added compression and encryption support                   |
| V4      | UE 4.16-4.19 | Added index encryption support                             |
| V5      | UE 4.20      | Changed to relative chunk offsets                          |
| V6      | -            | Added delete records support                               |
| V7      | UE 4.21      | Added encryption key GUID                                  |
| V8A     | UE 4.22      | Added FName-based compression                              |
| V8B     | UE 4.23-4.24 | Extended FName-based compression                           |
| V9      | UE 4.25      | Added frozen index support                                 |
| V10     | -            | Added path hash index                                      |
| V11     | UE 4.26-5.3+ | Fixed FNV64 hash bug, current version used in modern games |

Key changes across versions:
- **V3**: Added support for compression and encryption, significantly improving file size and security
- **V4**: Added the ability to encrypt just the index, improving security while maintaining performance
- **V7**: Added encryption key GUID to support multiple encryption keys
- **V8**: Improved compression with named compression methods
- **V9**: Added frozen index for optimized loading
- **V10**: Added path hash index for faster file lookups
- **V11**: Fixed hashing bugs and is the current standard version

## retoc Support

retoc interacts with .pak files primarily through its integration with the repak library. While retoc itself focuses on IoStore containers (.utoc/.ucas), it uses .pak files in the following ways:

1. **Asset Conversion**: When converting from Zen assets (IoStore) to Legacy assets, retoc creates .pak files to store the converted assets.

2. **Reading Legacy Assets**: When converting from Legacy assets to Zen assets, retoc reads from .pak files to access the Legacy assets.

The conversion process handles:
- Asset data conversion
- Shader library conversion
- Maintaining references between assets
- Preserving metadata

## repak Support

repak provides comprehensive support for .pak files:

1. **Reading Support**:
   - Supports all major .pak file versions (V2-V11)
   - Handles encrypted indices and data
   - Supports all compression methods used in .pak files
   - Efficiently reads file data on demand

2. **Writing Support**:
   - Creates new .pak files
   - Adds files to existing .pak files
   - Supports various compression methods
   - Writes deterministic indices for reproducible builds

3. **Feature Support**:
   - Compression: Zlib, Gzip, Zstd, LZ4, Oodle (with optional feature)
   - Encryption: Reading AES-encrypted .pak files
   - Index formats: Standard index, path hash index
   - Mount points: Configurable mount point paths

4. **Limitations**:
   - Writing does not currently support encryption
   - Limited support for frozen index compression (UE 4.25 only)
   - Not all compression algorithms are available in all games

repak is designed to be significantly faster than the official UnrealPak tool, with 2-30x faster unpacking speeds, making it an efficient solution for working with .pak files.
