# UTOC Binary File Layout

## Overview

The .utoc (Unreal Table of Contents) file is a binary file that contains metadata and indexing information for the content stored in corresponding .ucas files. This document describes the binary layout of the .utoc file format.

## File Structure

The .utoc file consists of the following sections in order:

```
+---------------------------+
| FIoStoreTocHeader         | Header with magic number and version info
+---------------------------+
| Chunk IDs                 | Array of FIoChunkId
+---------------------------+
| Chunk Offsets and Lengths | Array of FIoOffsetAndLength
+---------------------------+
| Hash Map                  | Perfect hash table for chunk lookup
+---------------------------+
| Compression Blocks        | Array of FIoStoreTocCompressedBlockEntry
+---------------------------+
| Compression Methods       | Array of compression method names
+---------------------------+
| Signatures                | Optional signatures if container is signed
+---------------------------+
| Directory Index           | Hierarchical file structure
+---------------------------+
| Chunk Metadata            | Array of FIoStoreTocEntryMeta
+---------------------------+
```

## Detailed Structure

### FIoStoreTocHeader (144 bytes)

The header contains information about the .utoc file and its contents.

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    16      byte[16]            Magic number ("-==--==--==--==-")
0x10    1       EIoStoreTocVersion  Version
0x11    1       byte                Reserved0
0x12    2       uint16              Reserved1
0x14    4       uint32              TocHeaderSize (0x90 = 144 bytes)
0x18    4       uint32              TocEntryCount
0x1C    4       uint32              TocCompressedBlockEntryCount
0x20    4       uint32              TocCompressedBlockEntrySize (12 bytes)
0x24    4       uint32              CompressionMethodNameCount
0x28    4       uint32              CompressionMethodNameLength (32 bytes)
0x2C    4       uint32              CompressionBlockSize
0x30    4       uint32              DirectoryIndexSize
0x34    4       uint32              PartitionCount
0x38    8       uint64              ContainerId
0x40    16      FGuid               EncryptionKeyGuid
0x50    1       EIoContainerFlags   ContainerFlags
0x51    1       byte                Reserved3
0x52    2       uint16              Reserved4
0x54    4       uint32              TocChunkPerfectHashSeedsCount
0x58    8       uint64              PartitionSize
0x60    4       uint32              TocChunksWithoutPerfectHashCount
0x64    4       uint32              Reserved7
0x68    40      uint64[5]           Reserved8
```

#### EIoStoreTocVersion

```
Value   Name                        Description
------  --------------------------  -----------
0       Invalid                     Invalid version
1       Initial                     Initial specification
2       DirectoryIndex              Added directory index
3       PartitionSize               Added partition size
4       PerfectHash                 Added perfect hash for chunk lookup
5       PerfectHashWithOverflow     Extended perfect hash with overflow
6       OnDemandMetaData            Added on-demand metadata
7       RemovedOnDemandMetaData     Removed on-demand metadata
8       ReplaceIoChunkHashWithIoHash Replaced chunk hash with IO hash
```

#### EIoContainerFlags (bitflags)

```
Bit     Flag        Description
------  ----------  -----------
0       Compressed  Container has compressed data
1       Encrypted   Container has encrypted data
2       Signed      Container has signatures
3       Indexed     Container has directory index
```

### Chunk IDs

An array of `TocEntryCount` FIoChunkId structures. Each FIoChunkId is 12 bytes and identifies a chunk of data in the .ucas file.

```
Offset  Size    Type        Description
------  ------  ----------  -----------
0x00    8       uint64      ChunkId (Package ID for most chunks)
0x08    2       uint16      ChunkIndex
0x0A    1       uint8       ChunkType
0x0B    1       uint8       Version bits (bit 7: is_new, bit 6: has_version)
```

#### EIoChunkType

```
Value   Name                    Description
------  ----------------------  -----------
0       Invalid                 Invalid chunk type
1       ExportBundleData        Main asset data
2       BulkData                Large binary data
3       OptionalBulkData        Optional large binary data
4       MemoryMappedBulkData    Memory-mapped large binary data
5       ScriptObjects           Script objects
6       ContainerHeader         Container metadata
7       ExternalFile            External file
8       ShaderCodeLibrary       Shader code library
9       ShaderCode              Shader code
10      PackageStoreEntry       Package store entry
11      DerivedData             Derived data
12      EditorDerivedData       Editor derived data
13      PackageResource         Package resource
```

Note: The chunk type values are different for versions before PerfectHash (UE5.0).

### Chunk Offsets and Lengths

An array of `TocEntryCount` FIoOffsetAndLength structures. Each FIoOffsetAndLength is 10 bytes and specifies the location and size of a chunk in the .ucas file.

```
Offset  Size    Description
------  ------  -----------
0x00    5       Offset (40 bits)
0x05    5       Length (40 bits)
```

### Hash Map

The hash map consists of two arrays:
1. An array of `TocChunkPerfectHashSeedsCount` 32-bit integers for the perfect hash seeds
2. An array of `TocChunksWithoutPerfectHashCount` 32-bit integers for chunks without perfect hash

This is only present in versions >= PerfectHash.

### Compression Blocks

An array of `TocCompressedBlockEntryCount` FIoStoreTocCompressedBlockEntry structures. Each FIoStoreTocCompressedBlockEntry is 12 bytes and describes a compressed block in the .ucas file.

```
Offset  Size    Description
------  ------  -----------
0x00    5       Offset (40 bits)
0x05    3       CompressedSize (24 bits)
0x08    3       UncompressedSize (24 bits)
0x0B    1       CompressionMethodIndex
```

### Compression Methods

An array of `CompressionMethodNameCount` compression method names. Each name is `CompressionMethodNameLength` bytes long (padded with zeros).

Common compression methods:
- Zlib
- Gzip
- Zstd
- LZ4
- Oodle

### Signatures (Optional)

If the container is signed (ContainerFlags has the Signed bit set), this section contains:
1. A 32-bit size value
2. A TOC signature of the specified size
3. A block signature of the specified size
4. An array of `TocCompressedBlockEntryCount` FSHAHash structures (20 bytes each)

### Directory Index

The directory index contains a hierarchical representation of the file structure. It consists of:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    varies  String              MountPoint
varies  varies  FIoDirectoryIndexEntry[] DirectoryEntries
varies  varies  FIoFileIndexEntry[] FileEntries
varies  varies  String[]            StringTable
```

If the container is encrypted (ContainerFlags has the Encrypted bit set), the directory index is encrypted using AES-256 with the key specified by EncryptionKeyGuid.

### Chunk Metadata

An array of `TocEntryCount` FIoStoreTocEntryMeta structures. Each FIoStoreTocEntryMeta contains:

```
Offset  Size    Type                    Description
------  ------  ----------------------  -----------
0x00    32      FIoChunkHash            ChunkHash (or first 20 bytes in newer versions)
0x20    1       FIoStoreTocEntryMetaFlags Flags
```

In versions >= ReplaceIoChunkHashWithIoHash, only the first 20 bytes of the hash are stored, followed by the flags and 3 bytes of padding.

#### FIoStoreTocEntryMetaFlags (bitflags)

```
Bit     Flag            Description
------  --------------  -----------
0       Compressed      Chunk is compressed
1       MemoryMapped    Chunk is memory-mapped
```

## Alignment and Padding

- Offsets in the .ucas file are aligned to the compression block size
- Encrypted blocks are aligned to AES block boundaries (16 bytes)

## Version Differences

The .utoc file format has evolved across Unreal Engine versions:

### UE4.26-UE4.27 (Initial, DirectoryIndex, PartitionSize)
- Basic .utoc file format
- Added directory index
- Added partition size

### UE5.0-UE5.3 (PerfectHash, PerfectHashWithOverflow)
- Added perfect hash for chunk lookup
- Extended perfect hash with overflow

### UE5.4-UE5.5 (OnDemandMetaData, RemovedOnDemandMetaData, ReplaceIoChunkHashWithIoHash)
- Added on-demand metadata
- Removed on-demand metadata
- Replaced chunk hash with IO hash

## Binary Visualization

```
+---------------------------+
| FIoStoreTocHeader         |
| Magic: -==--==--==--==-   |
| Version: X                |
| TocEntryCount: N          |
| ...                       |
+---------------------------+
| Chunk IDs (N entries)     |
| +---------------------+   |
| | ChunkId: 0x...      |   |
| | ChunkIndex: X       |   |
| | ChunkType: Y        |   |
| +---------------------+   |
| | ...                 |   |
+---------------------------+
| Offsets & Lengths (N)     |
| +---------------------+   |
| | Offset: 0x...       |   |
| | Length: 0x...       |   |
| +---------------------+   |
| | ...                 |   |
+---------------------------+
| Hash Map                  |
| +---------------------+   |
| | PerfectHashSeeds    |   |
| | ChunksWithoutHash   |   |
+---------------------------+
| Compression Blocks        |
| +---------------------+   |
| | Offset: 0x...       |   |
| | CompressedSize: X   |   |
| | UncompressedSize: Y |   |
| | MethodIndex: Z      |   |
| +---------------------+   |
| | ...                 |   |
+---------------------------+
| Compression Methods       |
| +---------------------+   |
| | "Zlib\0\0\0..."     |   |
| | "Oodle\0\0\0..."    |   |
| | ...                 |   |
+---------------------------+
| Directory Index           |
| +---------------------+   |
| | MountPoint          |   |
| | DirectoryEntries    |   |
| | FileEntries         |   |
| | StringTable         |   |
+---------------------------+
| Chunk Metadata (N)        |
| +---------------------+   |
| | ChunkHash: 0x...    |   |
| | Flags: 0x...        |   |
| +---------------------+   |
| | ...                 |   |
+---------------------------+
