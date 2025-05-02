# About .utoc Files

## General Information

.utoc (Unreal Table of Contents) files are part of Unreal Engine's IoStore container system introduced in later versions of the engine. They contain metadata and indexing information for the content stored in corresponding .ucas files. The IoStore system was introduced in Unreal Engine 5 as a replacement for the older .pak file system, offering improved performance and features.

.utoc files work in conjunction with .ucas (Unreal Content Archive Storage) files to form a complete IoStore container. While the .ucas file contains the actual asset data, the .utoc file provides the necessary indexing and metadata to locate and access that data efficiently.

Key characteristics of .utoc files include:
- Contain a table of contents for the IoStore container
- Store chunk IDs, offsets, and sizes for assets in the .ucas file
- Include compression information
- Provide directory indexing for efficient file lookup
- Support for encryption

## File Format

The .utoc file format consists of several key components:

### 1. Header
- Contains a magic number (`-==--==--==--==-`) to identify the file as a .utoc file
- Includes version information
- Contains information about the number of entries, compression methods, and other metadata

### 2. Chunk IDs
- Each chunk ID represents a piece of content in the .ucas file
- Chunk IDs are used to identify and locate specific assets
- Different types of chunks include:
  - ExportBundleData: Main asset data
  - BulkData: Large binary data
  - OptionalBulkData: Optional large binary data
  - MemoryMappedBulkData: Memory-mapped large binary data
  - ShaderCodeLibrary: Shader code libraries
  - ContainerHeader: Container metadata

### 3. Chunk Offsets and Lengths
- Store the location and size of each chunk in the .ucas file
- Allow for efficient access to specific chunks without scanning the entire file

### 4. Compression Blocks
- Information about how chunks are compressed
- Include compression method, block size, and other compression-related metadata

### 5. Directory Index
- A hierarchical representation of the file structure
- Maps file paths to chunk IDs for efficient lookup
- Supports mount points for virtual file paths

### 6. Container Header
- Contains information about packages in the container
- Includes package IDs, store entries, and other package-related metadata
- Supports localized packages and package redirects

## Version Differences

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

Similarly, the Container Header format has evolved:

| Version                 | UE Version | Key Features                     |
| ----------------------- | ---------- | -------------------------------- |
| PreInitial              | -          | Pre-initial version              |
| Initial                 | UE4.26-27  | Initial specification            |
| LocalizedPackages       | UE5.0      | Added localized packages support |
| OptionalSegmentPackages | UE5.1-5.2  | Added optional segment packages  |
| NoExportInfo            | UE5.3-5.4  | Removed export information       |
| SoftPackageReferences   | UE5.5+     | Added soft package references    |

## retoc Support

retoc provides comprehensive support for .utoc files:

1. **Reading Operations**:
   - Parse .utoc files to extract metadata and indexing information
   - Read chunk data from corresponding .ucas files
   - Support for all major .utoc file versions
   - Handle encrypted .utoc files with provided AES keys

2. **Writing Operations**:
   - Create new .utoc files
   - Add chunks to existing .utoc files
   - Generate proper directory indices
   - Support for various compression methods

3. **Utility Operations**:
   - Extract manifest from .utoc files
   - Show container information
   - List files in the directory index
   - Extract chunks to individual files
   - Extract and pack raw chunks

4. **Conversion Support**:
   - Convert Zen assets (IoStore) to Legacy assets (.pak)
   - Convert Legacy assets to Zen assets
   - Handle shader libraries during conversion

retoc's implementation of .utoc file handling is robust and supports all versions of the format from UE4.26 to the latest UE5.5, making it a versatile tool for working with Unreal Engine's IoStore containers.
