# Overview of retoc, repak, and Unreal Engine File Formats

This document provides a comprehensive overview of the projects retoc and repak, as well as the Unreal Engine file formats they work with: .pak, .utoc, and .ucas.

## Unreal Engine File Formats

Unreal Engine uses different file formats for packaging and storing game assets, which have evolved across engine versions:

### .pak Files

The .pak file format is Unreal Engine's traditional archive format used for packaging game assets in Unreal Engine 4 and still supported in Unreal Engine 5.

**Key Characteristics:**
- Self-contained archive files similar to ZIP or TAR, but optimized for game assets
- Store "Legacy Assets" (.uasset, .uexp, .ubulk files)
- Support compression (Zlib, Gzip, Zstd, LZ4, Oodle)
- Support encryption (AES-256)
- Include an index for efficient file lookup
- Use mount points for virtual file paths

**Structure:**
1. **Data Blocks**: The actual file contents
2. **Index**: Directory of files in the archive
3. **Secondary Indices** (V10+): Additional indices for improved performance
4. **Footer**: Metadata about the archive

**Version Evolution:**
- Evolved from V1 (pre-UE4.0) to V11 (UE4.26-5.3+)
- Added features like compression, encryption, path hash index, and more
- Latest version (V11) fixed an FNV64 hash bug and is used in modern games

### .utoc and .ucas Files (IoStore)

The IoStore container system was introduced in Unreal Engine 5 as a replacement for the .pak file system, offering improved performance and features. It uses a split approach with two file types:

#### .utoc Files (Unreal Table of Contents)

**Key Characteristics:**
- Contain metadata and indexing information
- Store chunk IDs, offsets, and sizes
- Include compression information
- Provide directory indexing
- Support encryption
- Store container header information

**Structure:**
1. **Header**: Magic number, version info, container flags
2. **Chunk IDs**: Identifiers for chunks in the .ucas file
3. **Chunk Offsets and Lengths**: Location and size of chunks
4. **Hash Map**: Perfect hash table for chunk lookup
5. **Compression Blocks**: Information about compressed blocks
6. **Compression Methods**: Names of compression methods used
7. **Directory Index**: Hierarchical file structure
8. **Chunk Metadata**: Additional information about chunks

**Version Evolution:**
- Evolved from Initial (UE4.26) to ReplaceIoChunkHashWithIoHash (UE5.5+)
- Added features like directory index, partition size, perfect hash, and more

#### .ucas Files (Unreal Content Archive Storage)

**Key Characteristics:**
- Store the actual content/data of game assets
- Support compressed data
- Support encrypted data
- Organized in blocks for efficient access
- Can be partitioned for better performance

**Structure:**
- Series of data blocks containing the raw asset data
- No file header or metadata (all metadata is in the .utoc file)
- Blocks may be compressed and/or encrypted

**Partitioning:**
- .ucas files can be split into multiple partitions (e.g., global.ucas.0, global.ucas.1)
- Partitioning allows for better parallel loading and reduced file size
- Partition information is stored in the .utoc file

## Asset Formats

Unreal Engine uses two primary asset formats:

### Legacy Assets

Used in Unreal Engine 4 and still supported in Unreal Engine 5:
- Stored in .pak files
- Split into multiple files (.uasset, .uexp, .ubulk)
- More compatible with older engine versions

### Zen Assets

Introduced in Unreal Engine 5:
- Stored in IoStore containers (.utoc/.ucas)
- More efficient storage and loading
- Better performance for modern platforms

## Projects

### retoc

retoc is a CLI tool for packing/unpacking Unreal Engine IoStore containers (.utoc/.ucas) as well as converting between Zen assets and Legacy assets.

**Key Features:**
- Extract manifest from .utoc files
- Show container information
- List files in .utoc (directory index)
- Extract chunks (files) from .utoc
- Convert assets from Zen format to Legacy format
- Convert assets from Legacy format to Zen format
- Handle shader libraries during conversion

**Usage Examples:**
```console
# Converting Zen to Legacy
$ retoc to-legacy AbioticFactor/Content/Paks legacy_P.pak

# Converting Legacy to Zen
$ retoc to-zen legacy_P.pak iostore.utoc --version UE5_4
```

### repak

repak is a library and CLI tool for working with Unreal Engine .pak files, providing functionality for reading, writing, and manipulating .pak files across various Unreal Engine versions.

**Key Features:**
- Support for all major .pak file versions (V2-V11)
- Efficient reading and writing of .pak files
- Support for compression (Zlib, Gzip, Zstd, LZ4, Oodle)
- Reading of AES-encrypted .pak files
- 2-30x faster unpacking speeds compared to UnrealPak

**Usage Examples:**
```console
# Packing files
$ repak pack -v mod

# Unpacking files
$ repak --aes-key 0x12345678 unpack MyEncryptedGame.pak
```

## Relationship Between Formats and Projects

The relationship between these file formats and projects can be summarized as follows:

1. **Legacy Assets in .pak Files**:
   - Traditional asset format used in UE4
   - Managed by repak for packing/unpacking

2. **Zen Assets in IoStore Containers**:
   - Newer asset format introduced in UE5
   - Stored in .utoc/.ucas files
   - Managed by retoc for packing/unpacking

3. **Conversion Between Formats**:
   - retoc provides conversion between Zen and Legacy assets
   - Allows for compatibility across different engine versions
   - Handles shader libraries during conversion

4. **Integration Between Projects**:
   - retoc uses repak for .pak file handling during conversion
   - This integration provides a complete solution for working with both asset formats

## Conclusion

Understanding these file formats and projects is essential for working with Unreal Engine assets, especially when dealing with different engine versions or converting between formats. The retoc and repak projects provide powerful tools for manipulating these formats, enabling efficient asset management and conversion for Unreal Engine games.
