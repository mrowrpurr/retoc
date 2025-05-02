# UTOC Research Notes

## Overview
The .utoc (Unreal Table of Contents) file format is part of Unreal Engine's IoStore container system. It contains metadata and indexing information for the content stored in corresponding .ucas files. The IoStore system was introduced in Unreal Engine 5 as a replacement for the older .pak file system, offering improved performance and features.

## Key Components Found in Source Code

### FIoStoreTocHeader
- Magic number: `-==--==--==--==-`
- Version information
- Container flags
- Compression information
- Directory index size
- Partition information
- Encryption key GUID

### EIoStoreTocVersion
Versions of the UTOC file format:
- Invalid
- Initial
- DirectoryIndex
- PartitionSize
- PerfectHash
- PerfectHashWithOverflow
- OnDemandMetaData
- RemovedOnDemandMetaData
- ReplaceIoChunkHashWithIoHash

### EIoContainerFlags
Flags that indicate container properties:
- Compressed
- Encrypted
- Signed
- Indexed

### FIoChunkId
Represents a chunk of data in the IoStore system. Contains:
- Chunk ID (8 bytes)
- Chunk index (2 bytes)
- Chunk type (1 byte)
- Version information (1 bit)

### EIoChunkType
Types of chunks in the IoStore system:
- ExportBundleData: Main asset data
- BulkData: Large binary data
- OptionalBulkData: Optional large binary data
- MemoryMappedBulkData: Memory-mapped large binary data
- ScriptObjects: Script objects
- ContainerHeader: Container metadata
- ShaderCodeLibrary: Shader code libraries
- ShaderCode: Shader code
- And others...

### FIoOffsetAndLength
Stores the offset and length of a chunk in the .ucas file:
- Offset (5 bytes)
- Length (5 bytes)

### FIoStoreTocCompressedBlockEntry
Information about a compressed block:
- Offset (5 bytes)
- Compressed size (3 bytes)
- Uncompressed size (3 bytes)
- Compression method index (1 byte)

### FIoChunkHash
Hash of a chunk's data (32 bytes)

### FIoDirectoryIndexResource
Directory structure for file lookup:
- Mount point
- Directory entries
- File entries
- String table

## Reading Process
1. Read the header
2. Read chunk IDs
3. Read chunk offsets and lengths
4. Read hash map
5. Read compression blocks
6. Read compression methods
7. Read chunk block signatures (if signed)
8. Read directory index
9. Read chunk metadata

## Writing Process
1. Write the header
2. Write chunk IDs
3. Write chunk offsets and lengths
4. Write compression blocks
5. Write compression methods
6. Write directory index
7. Write chunk metadata

## Questions to Investigate
- How are chunks organized in the .ucas file?
- How does the perfect hash table work for chunk lookup?
- How is encryption implemented?
- How are partitioned .ucas files handled?
- What's the relationship between chunk IDs and file paths?
