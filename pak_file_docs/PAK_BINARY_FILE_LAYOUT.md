# PAK File Binary Layout

## Overview

The .pak file format is Unreal Engine's archive format used for packaging game assets. This document details the binary layout of .pak files, focusing on the structure and organization of data within these files.

## File Structure

A .pak file consists of four main sections:

```
+------------------+
| Data Blocks      |
|                  |
| (File Contents)  |
+------------------+
| Index            |
|                  |
| (File Directory) |
+------------------+
| Secondary Indices|
| (V10+ only)      |
+------------------+
| Footer           |
|                  |
| (Metadata)       |
+------------------+
```

### 1. Data Blocks

The data blocks section contains the actual file data for all files stored in the .pak archive. Each file's data is stored at the offset specified in its corresponding entry in the index.

- Files may be stored as raw data or compressed using various algorithms
- Files may be encrypted using AES encryption
- Files may be divided into blocks for more efficient compression and access

### 2. Index

The index section contains information about all files stored in the .pak archive. It serves as a directory, allowing the engine to locate files within the archive.

- Contains a mount point (virtual path prefix)
- Contains entries for each file in the archive
- Each entry includes path, offset, size, and other metadata
- In newer versions (V10+), the index structure is more complex with additional indices

### 3. Secondary Indices (V10+)

In version 10 and above, additional indices are included for improved performance:

- **Path Hash Index**: Maps file path hashes to entry offsets for faster lookups
- **Full Directory Index**: Provides a directory structure for more efficient directory traversal

### 4. Footer

The footer is located at the end of the file and contains metadata about the .pak file itself, including:

- Magic number to identify the file as a .pak file
- Version information
- Index location and size
- Encryption information
- Hash of the index for verification

## Detailed Binary Layout

### Footer Structure

The footer is located at the end of the file and its size varies depending on the version. The footer contains:

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Encryption UUID   | uint128        | UUID for encryption key (V7+)            |
| Encrypted Index   | bool (uint8)   | Whether index is encrypted (V4+)         |
| Magic             | uint32         | Magic number (0x5A6F12E1)                |
| Version           | uint32         | Version number                           |
| Index Offset      | uint64         | Offset to index from start of file       |
| Index Size        | uint64         | Size of index in bytes                   |
| Index Hash        | uint8[20]      | SHA1 hash of index                       |
| Frozen Index      | bool (uint8)   | Whether index is frozen (V9 only)        |
| Compression Names | char[32][4-5]  | Names of compression methods (V8+)       |
+-------------------+----------------+------------------------------------------+
```

ASCII Diagram of Footer (V11):
```
                                                                  Footer
+----------------+----------------+----------------+----------------+----------------+----------------+
|                |                |                |                |                |                |
| Encryption UUID (16 bytes)      | Encrypted      | Magic Number (0x5A6F12E1)      | Version Major  |
| (V7+)                           | Index (1 byte) | (4 bytes)                      | (4 bytes)      |
|                |                | (V4+)          |                |                |                |
+----------------+----------------+----------------+----------------+----------------+----------------+
|                                                  |                                                  |
| Index Offset (8 bytes)                           | Index Size (8 bytes)                             |
|                                                  |                                                  |
+--------------------------------------------------+--------------------------------------------------+
|                                                                                                     |
| Index Hash (20 bytes)                                                                               |
|                                                  |                                                  |
+--------------------------------------------------+--------------------------------------------------+
|                                                                                                     |
| Compression Method Names (32 bytes each, 4-5 entries for V8+)                                       |
|                                                                                                     |
+-----------------------------------------------------------------------------------------------------+
```

### Index Structure

The index structure varies significantly between versions, especially from V10 onwards.

#### Basic Index Structure (V1-V9)

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Mount Point       | String         | Virtual path prefix                      |
| Entry Count       | uint32         | Number of file entries                   |
| Entries           | Entry[]        | Array of file entries                    |
+-------------------+----------------+------------------------------------------+
```

#### Enhanced Index Structure (V10+)

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Mount Point       | String         | Virtual path prefix                      |
| Entry Count       | uint32         | Number of file entries                   |
| Path Hash Seed    | uint64         | Seed for path hash calculation           |
| Path Hash Index   | PHI            | Path hash index structure                |
| Directory Index   | DI             | Full directory index structure           |
| Encoded Entries   | EncodedEntry[] | Array of encoded file entries            |
+-------------------+----------------+------------------------------------------+
```

ASCII Diagram of Index (V10+):
```
                                      Index
+--------------------------------------------------+
| Mount Point (variable length string)             |
+--------------------------------------------------+
| Entry Count (4 bytes)                            |
+--------------------------------------------------+
| Path Hash Seed (8 bytes)                         |
+--------------------------------------------------+
| Has Path Hash Index (4 bytes)                    |
+--------------------------------------------------+
| Path Hash Index Offset (8 bytes)                 |
+--------------------------------------------------+
| Path Hash Index Size (8 bytes)                   |
+--------------------------------------------------+
| Path Hash Index Hash (20 bytes)                  |
+--------------------------------------------------+
| Has Full Directory Index (4 bytes)               |
+--------------------------------------------------+
| Full Directory Index Offset (8 bytes)            |
+--------------------------------------------------+
| Full Directory Index Size (8 bytes)              |
+--------------------------------------------------+
| Full Directory Index Hash (20 bytes)             |
+--------------------------------------------------+
| Encoded Entries Size (4 bytes)                   |
+--------------------------------------------------+
| Encoded Entries (variable length)                |
+--------------------------------------------------+
| Unused File Count (4 bytes)                      |
+--------------------------------------------------+
```

### Entry Structure

The entry structure also varies by version, with more fields added in later versions.

#### Basic Entry Structure (V1-V2)

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Offset            | uint64         | Offset to file data                      |
| Size              | uint64         | Size of file data                        |
| Uncompressed Size | uint64         | Uncompressed size (same as Size)         |
| Compression       | uint32/uint8   | Compression method (0 = none)            |
| Timestamp         | uint64         | File timestamp (V1 only)                 |
| Hash              | uint8[20]      | SHA1 hash of file data                   |
+-------------------+----------------+------------------------------------------+
```

#### Enhanced Entry Structure (V3+)

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Offset            | uint64         | Offset to file data                      |
| Compressed Size   | uint64         | Size of compressed file data             |
| Uncompressed Size | uint64         | Size of uncompressed file data           |
| Compression       | uint32/uint8   | Compression method (0 = none)            |
| Hash              | uint8[20]      | SHA1 hash of file data                   |
| Blocks            | Block[]        | Array of data blocks (if compressed)     |
| Flags             | uint8          | Flags (bit 0 = encrypted, bit 1 = deleted)|
| Block Size        | uint32         | Compression block size                   |
+-------------------+----------------+------------------------------------------+
```

ASCII Diagram of Entry (V11):
```
                                      Entry
+--------------------------------------------------+
| Offset (8 bytes)                                 |
+--------------------------------------------------+
| Compressed Size (8 bytes)                        |
+--------------------------------------------------+
| Uncompressed Size (8 bytes)                      |
+--------------------------------------------------+
| Compression Method (4 bytes)                     |
+--------------------------------------------------+
| Hash (20 bytes)                                  |
+--------------------------------------------------+
| Block Count (4 bytes) - if compressed            |
+--------------------------------------------------+
| Blocks (16 bytes each) - if compressed           |
+--------------------------------------------------+
| Flags (1 byte)                                   |
+--------------------------------------------------+
| Compression Block Size (4 bytes)                 |
+--------------------------------------------------+
```

### Encoded Entry Structure (V10+)

In V10+, entries are encoded in a more compact format:

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Flags             | uint32         | Bit flags for entry properties           |
| Offset            | uint32/uint64  | Offset to file data                      |
| Uncompressed Size | uint32/uint64  | Size of uncompressed file data           |
| Compressed Size   | uint32/uint64  | Size of compressed file data (if compr.) |
| Block Sizes       | uint32[]       | Sizes of blocks (if multiple blocks)     |
+-------------------+----------------+------------------------------------------+
```

The flags field in encoded entries contains:
- Bits 0-5: Compression block size (shifted left by 11)
- Bits 6-21: Compression block count
- Bit 22: Encrypted flag
- Bits 23-28: Compression method
- Bit 29: Compressed size is 32-bit
- Bit 30: Uncompressed size is 32-bit
- Bit 31: Offset is 32-bit

### Path Hash Index (V10+)

The path hash index provides a mapping from file path hashes to entry offsets:

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Entry Count       | uint32         | Number of entries                        |
| Entries           | PHIEntry[]     | Array of path hash index entries         |
| Terminator        | uint32         | Always 0                                 |
+-------------------+----------------+------------------------------------------+
```

Each path hash index entry:
```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Path Hash         | uint64         | FNV-64 hash of file path                 |
| Entry Offset      | uint32         | Offset to encoded entry                  |
+-------------------+----------------+------------------------------------------+
```

### Full Directory Index (V10+)

The full directory index provides a directory structure:

```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Directory Count   | uint32         | Number of directories                    |
| Directories       | Directory[]    | Array of directory entries               |
+-------------------+----------------+------------------------------------------+
```

Each directory entry:
```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| Directory Name    | String         | Directory path                           |
| File Count        | uint32         | Number of files in directory             |
| Files             | File[]         | Array of file entries                    |
+-------------------+----------------+------------------------------------------+
```

Each file entry:
```
+-------------------+----------------+------------------------------------------+
| Field             | Type           | Description                              |
+-------------------+----------------+------------------------------------------+
| File Name         | String         | File name                                |
| Entry Offset      | uint32         | Offset to encoded entry                  |
+-------------------+----------------+------------------------------------------+
```

## String Encoding

Strings in .pak files are encoded in one of two ways:

1. **ASCII Strings**:
   - Length (uint32) - includes null terminator
   - Characters (uint8[]) - ASCII characters
   - Null terminator (uint8) - always 0

2. **UTF-16 Strings**:
   - Length (int32) - negative value, includes null terminator
   - Characters (uint16[]) - UTF-16 characters
   - Null terminator (uint16) - always 0

## Version Differences

The .pak file format has evolved significantly across different versions of Unreal Engine. The table below summarizes the key differences:

| Version | UE Version   | Footer Size | Key Features                                 |
| ------- | ------------ | ----------- | -------------------------------------------- |
| V1      | Pre-4.0      | 36 bytes    | Initial specification with timestamps        |
| V2      | UE 4.0-4.2   | 36 bytes    | Removed timestamps                           |
| V3      | UE 4.3-4.15  | 36 bytes    | Added compression and encryption support     |
| V4      | UE 4.16-4.19 | 37 bytes    | Added index encryption support               |
| V5      | UE 4.20      | 37 bytes    | Changed to relative chunk offsets            |
| V6      | -            | 37 bytes    | Added delete records support                 |
| V7      | UE 4.21      | 53 bytes    | Added encryption key GUID                    |
| V8A     | UE 4.22      | 181 bytes   | Added FName-based compression (4 methods)    |
| V8B     | UE 4.23-4.24 | 213 bytes   | Extended FName-based compression (5 methods) |
| V9      | UE 4.25      | 214 bytes   | Added frozen index support                   |
| V10     | -            | 213 bytes   | Added path hash index                        |
| V11     | UE 4.26-5.3+ | 213 bytes   | Fixed FNV64 hash bug                         |

## Compression

The .pak file format supports multiple compression methods:

1. **Zlib**: Standard zlib compression
2. **Gzip**: Standard gzip compression
3. **Oodle**: Proprietary compression algorithm by RAD Game Tools
4. **Zstd**: Zstandard compression algorithm
5. **LZ4**: LZ4 compression algorithm

Files can be compressed in blocks, with each block compressed independently. This allows for more efficient random access to compressed data.

## Encryption

The .pak file format supports AES encryption:

1. **Index Encryption**: Only the index is encrypted
2. **Data Encryption**: File data is encrypted
3. **Encryption Key GUID**: A GUID that identifies which encryption key to use

Encryption is performed using AES-256 in ECB mode, with each 16-byte block encrypted independently.

## Path Hashing

In V10+, file paths are hashed using the FNV-64 algorithm:

1. Convert path to lowercase
2. Convert to UTF-16
3. Apply FNV-64 hash algorithm with the path hash seed

The resulting hash is used in the path hash index for faster file lookups.
