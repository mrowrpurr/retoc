# UTOC Chunk ID Format

## Overview

The Chunk ID is a fundamental concept in Unreal Engine's IoStore system. It uniquely identifies chunks of data within the IoStore containers. This document provides a detailed explanation of the Chunk ID format, its structure, and how it's used in the .utoc file format.

## Binary Structure

A Chunk ID (`FIoChunkId`) is a 12-byte structure with the following layout:

```
Offset  Size    Description
------  ------  -----------
0x00    8       Chunk ID (Package ID for most chunks)
0x08    2       Chunk Index
0x0A    1       Chunk Type
0x0B    1       Version Bits
```

### Chunk ID (8 bytes)

The first 8 bytes of the Chunk ID structure contain the chunk's primary identifier:

- For asset chunks (ExportBundleData, BulkData, etc.), this is the Package ID, which is a 64-bit hash of the package name
- For shader chunks, this is derived from the shader hash
- For other chunk types, this may have different meanings

The Package ID is computed using a CityHash64 function on the lowercase UTF-16 encoded package name.

### Chunk Index (2 bytes)

The chunk index is a 16-bit value that distinguishes between multiple chunks of the same type within a package:

- For most chunks, this is typically 0
- For assets with multiple chunks of the same type, this is incremented for each additional chunk

### Chunk Type (1 byte)

The chunk type indicates what kind of data the chunk contains. The lower 6 bits of this byte are used for the chunk type value:

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

Note: The chunk type values are different for versions before PerfectHash (UE5.0). In older versions, the mapping is:

```
Value   Name                    Description
------  ----------------------  -----------
0       Invalid                 Invalid chunk type
1       InstallManifest         Install manifest
2       ExportBundleData        Main asset data
3       BulkData                Large binary data
4       OptionalBulkData        Optional large binary data
5       MemoryMappedBulkData    Memory-mapped large binary data
6       LoaderGlobalMeta        Loader global metadata
7       LoaderInitialLoadMeta   Loader initial load metadata
8       LoaderGlobalNames       Loader global names
9       LoaderGlobalNameHashes  Loader global name hashes
10      ContainerHeader         Container metadata
11      ShaderCodeLibrary       Shader code library
12      ShaderCode              Shader code
```

### Version Bits (1 byte)

The version bits byte contains flags related to the chunk ID's version:

- Bit 6 (0x40): Has Version - Indicates that the chunk ID has version information
- Bit 7 (0x80): Is New - Indicates whether the chunk uses the new (UE5.0+) or old chunk type mapping

The lower 6 bits of this byte are not used.

## Creating Chunk IDs

### Asset Chunks

For asset chunks, the Chunk ID is created from the package name:

1. Convert the package name to lowercase
2. Encode the name as UTF-16
3. Compute the CityHash64 of the encoded name to get the Package ID
4. Combine the Package ID with the chunk index and chunk type

Example:
```
Package Name: "/Game/Characters/Hero"
Package ID: CityHash64(lowercase_utf16("/game/characters/hero"))
Chunk ID for ExportBundleData: PackageID + ChunkIndex(0) + ChunkType(ExportBundleData)
```

### Shader Chunks

For shader chunks, the Chunk ID is created from the shader hash:

1. Take the first 11 bytes of the shader hash
2. Set the chunk type to ShaderCode

### Shader Library Chunks

For shader library chunks, the Chunk ID is created from the library name and shader format:

1. Combine the shader library name and shader format name with a hyphen
2. Compute the CityHash64 of the combined name
3. Set the chunk index to 0
4. Set the chunk type to ShaderCodeLibrary

Example:
```
Shader Library Name: "Global"
Shader Format Name: "PCD3D_SM6"
Combined Name: "Global-PCD3D_SM6"
Chunk ID: CityHash64("global-pcd3d_sm6") + ChunkIndex(0) + ChunkType(ShaderCodeLibrary)
```

## Chunk ID Lookup

In the .utoc file, chunk IDs are used to locate chunks in the .ucas file:

1. The .utoc file contains an array of chunk IDs
2. Each chunk ID corresponds to an entry in the chunk offsets and lengths array
3. The chunk offset and length specify where to find the chunk data in the .ucas file

In versions >= PerfectHash (UE5.0), a perfect hash table is used for efficient chunk lookup:

1. The hash table uses the chunk ID to compute a hash value
2. The hash value is used to index into the chunk array
3. If the chunk can't be perfectly hashed, it's stored in an overflow table

## Version Compatibility

When working with chunk IDs, it's important to handle version differences correctly:

1. Check the version bits to determine if the chunk uses the new or old chunk type mapping
2. Use the appropriate chunk type mapping based on the version
3. When creating chunk IDs, set the version bits according to the target version

## Conclusion

The Chunk ID format is a key component of the IoStore system, providing a way to uniquely identify and efficiently locate chunks of data. Understanding its structure and how it's used is essential for working with .utoc and .ucas files.
