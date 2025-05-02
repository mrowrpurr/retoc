# UTOC Directory Index

## Overview

The Directory Index is a critical component of the .utoc file format that provides a hierarchical representation of the file structure. It maps virtual file paths to chunk IDs, allowing the engine to quickly locate assets without having to scan the entire container. This document provides a detailed explanation of the Directory Index structure and how it's used in the .utoc file format.

## Purpose

The Directory Index serves several important purposes:

1. **File Path Mapping**: Maps virtual file paths to chunk IDs
2. **Hierarchical Organization**: Provides a tree-like structure of directories and files
3. **Efficient Lookup**: Allows for quick lookup of assets by path
4. **Mount Point Support**: Supports mounting containers at specific paths

## Binary Structure

The Directory Index is stored in the .utoc file as a binary blob. If the container is encrypted, this blob is encrypted using AES-256 with the key specified by the EncryptionKeyGuid in the header.

When deserialized, the Directory Index consists of the following components:

```
Offset  Size    Type                Description
------  ------  ------------------  -----------
0x00    varies  String              MountPoint
varies  varies  DirectoryEntry[]    DirectoryEntries
varies  varies  FileEntry[]         FileEntries
varies  varies  String[]            StringTable
```

### Mount Point

The mount point is a string that specifies the base path where the container is mounted in the virtual file system. Typically, this is "../../../" for game content.

### Directory Entries

Directory entries form a tree structure representing the directories in the container. Each directory entry contains:

```
Offset  Size    Type            Description
------  ------  --------------  -----------
0x00    4       Option<IdName>  Name (index into StringTable, or None for root)
0x04    4       Option<IdDir>   FirstChildEntry (index of first child directory)
0x08    4       Option<IdDir>   NextSiblingEntry (index of next sibling directory)
0x0C    4       Option<IdFile>  FirstFileEntry (index of first file in this directory)
```

The directory entries are organized as a tree:
- The root directory (index 0) has no name
- Each directory can have child directories (FirstChildEntry)
- Siblings are linked in a list (NextSiblingEntry)
- Each directory can contain files (FirstFileEntry)

### File Entries

File entries represent the files in the container. Each file entry contains:

```
Offset  Size    Type            Description
------  ------  --------------  -----------
0x00    4       IdName          Name (index into StringTable)
0x04    4       Option<IdFile>  NextFileEntry (index of next file in the same directory)
0x08    4       uint32          UserData (typically the chunk index)
```

Files in the same directory are linked in a list (NextFileEntry).

### String Table

The string table is an array of strings used for directory and file names. Indices into this table are used in the directory and file entries.

## Traversal Algorithm

To traverse the Directory Index and build a complete file path to chunk ID mapping:

1. Start at the root directory (index 0)
2. For each directory:
   a. Process all files in the directory
   b. Recursively process all child directories
3. For each file:
   a. Build the full path by concatenating the mount point, all parent directory names, and the file name
   b. Map the full path to the file's UserData (chunk index)

## Example

Consider a simple directory structure:

```
/Game/
  ├── Characters/
  │     ├── Hero.uasset
  │     └── Enemy.uasset
  └── Weapons/
        └── Sword.uasset
```

This would be represented in the Directory Index as:

```
MountPoint: "../../../"

DirectoryEntries:
  [0] Root:
      Name: None
      FirstChildEntry: 1
      NextSiblingEntry: None
      FirstFileEntry: None
  [1] Game:
      Name: "Game" (StringTable[0])
      FirstChildEntry: 2
      NextSiblingEntry: None
      FirstFileEntry: None
  [2] Characters:
      Name: "Characters" (StringTable[1])
      FirstChildEntry: None
      NextSiblingEntry: 3
      FirstFileEntry: 0
  [3] Weapons:
      Name: "Weapons" (StringTable[2])
      FirstChildEntry: None
      NextSiblingEntry: None
      FirstFileEntry: 2

FileEntries:
  [0] Hero.uasset:
      Name: "Hero.uasset" (StringTable[3])
      NextFileEntry: 1
      UserData: 0 (Chunk index for Hero.uasset)
  [1] Enemy.uasset:
      Name: "Enemy.uasset" (StringTable[4])
      NextFileEntry: None
      UserData: 1 (Chunk index for Enemy.uasset)
  [2] Sword.uasset:
      Name: "Sword.uasset" (StringTable[5])
      NextFileEntry: None
      UserData: 2 (Chunk index for Sword.uasset)

StringTable:
  [0] "Game"
  [1] "Characters"
  [2] "Weapons"
  [3] "Hero.uasset"
  [4] "Enemy.uasset"
  [5] "Sword.uasset"
```

## File Path Lookup

To look up a file by path:

1. Split the path into components (e.g., "/Game/Characters/Hero.uasset" -> ["Game", "Characters", "Hero.uasset"])
2. Start at the root directory
3. For each path component:
   a. If it's the last component, search for a file with that name in the current directory
   b. Otherwise, search for a child directory with that name and continue the search there
4. If found, return the file's UserData (chunk index)

## Adding Files

To add a file to the Directory Index:

1. Split the path into components
2. Start at the root directory
3. For each directory component:
   a. If the directory doesn't exist, create it
   b. Navigate to that directory
4. Add the file to the current directory with the appropriate UserData

## Version Differences

The Directory Index was introduced in the DirectoryIndex version (UE4.26) of the .utoc file format. In earlier versions, there was no hierarchical file structure, and files were identified solely by their chunk IDs.

## Implementation Considerations

When implementing a reader or writer for the Directory Index:

1. **Encryption**: Handle encryption if the container is encrypted
2. **Memory Management**: Use efficient data structures to represent the directory tree
3. **Path Normalization**: Normalize paths (e.g., convert to lowercase, use forward slashes) for consistent lookup
4. **Error Handling**: Handle missing files or directories gracefully

## Conclusion

The Directory Index is a powerful feature of the .utoc file format that provides a hierarchical view of the container's contents. By understanding its structure and how to traverse it, developers can efficiently locate and access assets within IoStore containers.
