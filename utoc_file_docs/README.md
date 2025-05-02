# UTOC File Format Documentation

## Overview

This documentation provides a comprehensive guide to the .utoc (Unreal Table of Contents) file format used in Unreal Engine's IoStore container system. It covers the binary structure, key components, and implementation details of the format, aimed at software engineers who need to write readers and writers for this file format.

## Table of Contents

1. [UTOC Detailed Information](UTOC_DETAILED_INFORMATION.md) - Comprehensive overview of the UTOC file format
2. [UTOC Binary File Layout](UTOC_BINARY_FILE_LAYOUT.md) - Detailed description of the binary structure
3. [UTOC Chunk ID Format](UTOC_CHUNK_ID_FORMAT.md) - Explanation of the chunk ID system
4. [UTOC Directory Index](UTOC_DIRECTORY_INDEX.md) - Details on the hierarchical file structure
5. [UTOC Perfect Hash](UTOC_PERFECT_HASH.md) - Information on the perfect hash system for chunk lookup
6. [UTOC Container Header](UTOC_CONTAINER_HEADER.md) - Details on the container header structure
7. [UTOC Compression System](UTOC_COMPRESSION_SYSTEM.md) - Explanation of the compression system
8. [UTOC Encryption](UTOC_ENCRYPTION.md) - Details on the encryption system
9. [UTOC Partitioning](UTOC_PARTITIONING.md) - Information on the partitioning system
10. [UTOC UCAS Relationship](UTOC_UCAS_RELATIONSHIP.md) - How UTOC and UCAS files work together
11. [UTOC Research Notes](UTOC_RESEARCH_NOTES_SCRATCHPAD.md) - Research notes and findings
12. [UTOC 010 Editor Template](utoc.bt) - Binary template for 010 Editor

## Key Concepts

### IoStore Container System

The IoStore container system is Unreal Engine's newer asset storage system introduced in later versions of the engine. It replaces the older .pak file system with a more efficient and feature-rich approach to storing and accessing game assets.

### UTOC and UCAS Files

The IoStore system uses a split approach to storing game assets:
- Metadata and indexing information is stored in .utoc files (Unreal Table of Contents)
- The actual asset data is stored in .ucas files (Unreal Content Archive Storage)

This separation allows for more efficient asset lookup and loading, as the engine can quickly scan the smaller .utoc files to locate assets without having to parse the entire content archive.

### Chunk-Based Storage

The IoStore system organizes data into "chunks," which are the basic units of storage. Each chunk:
- Has a unique 12-byte ID
- Belongs to a specific type (e.g., ExportBundleData, BulkData, ShaderCodeLibrary)
- Can be compressed and/or encrypted
- Has a hash for verification

### Version Evolution

The .utoc file format has evolved across Unreal Engine versions:

| Version                      | UE Version | Key Features                        |
| ---------------------------- | ---------- | ----------------------------------- |
| Invalid                      | -          | Invalid version                     |
| Initial                      | UE4.26     | Initial specification               |
| DirectoryIndex               | UE4.26     | Added directory index               |
| PartitionSize                | UE4.27     | Added partition size                |
| PerfectHash                  | UE5.0      | Added perfect hash for chunk lookup |
| PerfectHashWithOverflow      | UE5.0-5.3  | Extended perfect hash with overflow |
| OnDemandMetaData             | UE5.4      | Added on-demand metadata            |
| RemovedOnDemandMetaData      | -          | Removed on-demand metadata          |
| ReplaceIoChunkHashWithIoHash | UE5.5      | Replaced chunk hash with IO hash    |

## Implementation Guide

### Reading a UTOC File

To read a .utoc file:

1. Open the file and read the header
2. Parse the chunk IDs, offsets, and lengths
3. Read the hash map (if present)
4. Read the compression blocks
5. Read the compression methods
6. Read the directory index
7. Read the chunk metadata

### Writing a UTOC File

To write a .utoc file:

1. Determine the appropriate version
2. Create the header
3. Generate chunk IDs for each chunk
4. Calculate offsets and lengths for each chunk
5. Create the hash map (if needed)
6. Create compression blocks
7. Record compression methods
8. Build the directory index
9. Generate chunk metadata
10. Write all components to the file

### Reading a Chunk

To read a chunk from an IoStore container:

1. Look up the chunk ID in the .utoc file
2. Get the chunk's offset, size, and compression information
3. Open the appropriate .ucas file
4. Read the compressed data from the .ucas file
5. Decompress the data using the specified compression method

### Writing a Chunk

To write a chunk to an IoStore container:

1. Compress the chunk data if desired
2. Write the compressed data to the .ucas file
3. Record the chunk's offset, size, and compression information in the .utoc file
4. Update the directory index if necessary

## Best Practices

1. **Version Compatibility**: Handle different versions of the .utoc format appropriately
2. **Efficient Chunk Lookup**: Use the perfect hash system for fast chunk access
3. **Memory Management**: Minimize memory usage by only loading necessary data
4. **Error Handling**: Gracefully handle missing or corrupted data
5. **Partitioning Support**: Handle partitioned .ucas files correctly

## Tools

The [utoc.bt](utoc.bt) file provides a binary template for 010 Editor, which can be used to analyze and inspect .utoc files. This template helps visualize the structure of the file and navigate its components.

## References

- [ABOUT_IOSTORE.md](../docs/ABOUT_IOSTORE.md) - Overview of the IoStore system
- [ABOUT_UTOC_FILES.md](../docs/ABOUT_UTOC_FILES.md) - General information about .utoc files
- [ABOUT_UCAS_FILES.md](../docs/ABOUT_UCAS_FILES.md) - General information about .ucas files
- [ABOUT_RETOC.md](../docs/ABOUT_RETOC.md) - Information about the retoc tool

## Conclusion

This documentation provides a comprehensive guide to the .utoc file format. By understanding its structure and how to work with it, developers can effectively read and write IoStore containers for Unreal Engine games.
