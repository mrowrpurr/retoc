# UTOC Container Header

## Overview

The Container Header is a special chunk in the IoStore system that contains metadata about the packages stored in the container. It provides information about package IDs, store entries, redirects, and other package-related metadata. This document explains the structure and purpose of the Container Header in the .utoc file format.

## Purpose

The Container Header serves several important purposes:

1. **Package Metadata**: Stores metadata about packages in the container
2. **Store Entries**: Contains information about package exports, imports, and dependencies
3. **Package Redirects**: Maps old package IDs to new package IDs for compatibility
4. **Localized Packages**: Supports localized versions of packages

## Location in the UTOC File

The Container Header is stored as a special chunk in the .utoc file with the following characteristics:

- Chunk Type: `ContainerHeader` (value 6 in UE5.0+, value 10 in pre-UE5.0)
- Chunk ID: 0
- Chunk Index: 0

To access the Container Header, you would look for a chunk with the ID `FIoChunkId::create(0, 0, EIoChunkType::ContainerHeader)`.

## Version Evolution

The Container Header has its own version system that has evolved across Unreal Engine versions:

| Version                 | UE Version | Key Features                     |
| ----------------------- | ---------- | -------------------------------- |
| PreInitial              | -          | Pre-initial version              |
| Initial                 | UE4.26-27  | Initial specification            |
| LocalizedPackages       | UE5.0      | Added localized packages support |
| OptionalSegmentPackages | UE5.1-5.2  | Added optional segment packages  |
| NoExportInfo            | UE5.3-5.4  | Removed export information       |
| SoftPackageReferences   | UE5.5+     | Added soft package references    |

## Binary Structure

The Container Header has a complex structure that varies depending on its version. The basic structure is as follows:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    4       EIoContainerHeaderVersion  Version
varies  varies  PackageStoreEntry[] StoreEntries
varies  varies  PackageRedirect[]   PackageRedirects
varies  varies  LocalizedPackage[]  LocalizedPackages (if version >= LocalizedPackages)
varies  varies  OptionalSegmentPackage[] OptionalSegmentPackages (if version >= OptionalSegmentPackages)
```

### Store Entries

Store entries contain metadata about packages in the container. Each store entry includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    8       FPackageId          PackageId
varies  varies  ExportBundleEntry[] ExportBundleEntries (if version < NoExportInfo)
varies  varies  ImportedPackage[]   ImportedPackages
varies  varies  ShaderMapEntry[]    ShaderMapEntries
```

#### Export Bundle Entries

Export bundle entries describe the exports in a package. Each export bundle entry includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    4       uint32              ExportBundleCount
varies  varies  ExportBundleHeader[] ExportBundleHeaders
```

#### Imported Packages

Imported packages list the packages that are imported by a package. Each imported package entry includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    8       FPackageId          ImportedPackageId
```

#### Shader Map Entries

Shader map entries describe the shader maps used by a package. Each shader map entry includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    20      FSHAHash            ShaderMapHash
```

### Package Redirects

Package redirects map old package IDs to new package IDs for compatibility. Each package redirect includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    8       FPackageId          SourcePackageId
0x08    8       FPackageId          TargetPackageId
```

### Localized Packages

Localized packages provide information about localized versions of packages. Each localized package entry includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    8       FPackageId          SourcePackageId
varies  varies  LocalizedPackageEntry[] LocalizedPackageEntries
```

### Optional Segment Packages

Optional segment packages provide information about packages that are part of optional segments. Each optional segment package entry includes:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    8       FPackageId          PackageId
0x08    4       uint32              SegmentIndex
```

## Usage

The Container Header is used by the engine to:

1. **Locate Packages**: Find packages by their ID
2. **Resolve Dependencies**: Determine which packages depend on each other
3. **Handle Redirects**: Redirect requests for old packages to new packages
4. **Support Localization**: Provide localized versions of packages
5. **Manage Optional Content**: Handle optional segments of content

## Reading the Container Header

To read the Container Header:

1. Find the Container Header chunk in the .utoc file
2. Read the chunk data
3. Parse the data according to the Container Header version
4. Extract the store entries, redirects, and other metadata

## Writing the Container Header

To write the Container Header:

1. Determine the appropriate Container Header version
2. Create store entries for each package
3. Add package redirects if needed
4. Add localized package information if needed
5. Add optional segment package information if needed
6. Serialize the data according to the Container Header version
7. Write the data as a chunk in the .utoc file

## Implementation Considerations

When implementing a reader or writer for the Container Header:

1. **Version Handling**: Handle different versions of the Container Header format
2. **Memory Management**: Use efficient data structures to represent the Container Header
3. **Package ID Generation**: Generate consistent package IDs for packages
4. **Dependency Tracking**: Properly track and represent package dependencies

## Example

Here's a simplified example of a Container Header for a container with two packages:

```
Version: Initial

StoreEntries:
  [0] PackageId: 0x1234567890ABCDEF
      ExportBundleEntries:
        Count: 1
        Headers:
          [0] ExportCount: 3
      ImportedPackages:
        [0] ImportedPackageId: 0x0987654321FEDCBA
      ShaderMapEntries:
        [0] ShaderMapHash: 0x0123456789ABCDEF0123456789ABCDEF01234567

  [1] PackageId: 0x0987654321FEDCBA
      ExportBundleEntries:
        Count: 1
        Headers:
          [0] ExportCount: 2
      ImportedPackages: []
      ShaderMapEntries: []

PackageRedirects:
  [0] SourcePackageId: 0xAAAAAAAAAAAAAAAA
      TargetPackageId: 0x1234567890ABCDEF
```

## Conclusion

The Container Header is a critical component of the IoStore system that provides metadata about the packages in a container. By understanding its structure and how to work with it, developers can effectively manage and access packages within IoStore containers.
