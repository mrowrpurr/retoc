# About repak

## Overview

repak is a library and CLI tool for working with Unreal Engine .pak files. It provides functionality for reading, writing, and manipulating .pak files across various Unreal Engine versions. The project is designed to be both a standalone tool for working with .pak files and a library that can be integrated into other applications, such as retoc.

## Project Goals

The primary goals of the repak project are:

1. **Efficient .pak File Handling**: Provide a fast and efficient way to read, write, and manipulate Unreal Engine .pak files.

2. **Version Compatibility**: Support a wide range of Unreal Engine versions, from early UE4 versions to the latest UE5 releases.

3. **Feature Completeness**: Support all major features of .pak files, including compression, encryption, and various index formats.

4. **Performance**: Offer significantly better performance than the official UnrealPak tool, with 2-30x faster unpacking speeds.

5. **Library Integration**: Provide a clean API that can be used by other applications to work with .pak files.

## File/Folder Structure

The repak project is organized as follows:

```
repak/
├── repak/                    # Core library
│   ├── src/                  # Library source code
│   │   ├── lib.rs            # Library entry point
│   │   ├── pak.rs            # Main .pak file handling
│   │   ├── entry.rs          # File entry handling
│   │   ├── data.rs           # Data handling and compression
│   │   ├── footer.rs         # .pak file footer handling
│   │   ├── error.rs          # Error handling
│   │   └── ext.rs            # Extension traits
│   └── tests/                # Library tests
├── repak_cli/                # CLI application
│   ├── src/                  # CLI source code
│   │   └── main.rs           # CLI entry point
│   └── tests/                # CLI tests
└── liboodle/             # Optional Oodle compression support
    └── src/                  # Oodle loader source code
        └── lib.rs            # Oodle loader entry point
```

## Features

repak offers a comprehensive set of features for working with .pak files:

1. **Version Support**:
   - Supports all major .pak file versions from UE4.0 to UE5.3+
   - Handles version-specific features and format differences

2. **Reading Operations**:
   - Efficient reading of .pak file indices
   - On-demand loading of file data
   - Support for encrypted indices and data
   - Support for compressed files

3. **Writing Operations**:
   - Creation of new .pak files
   - Addition of files to existing .pak files
   - Support for various compression methods

4. **Compression Support**:
   - Zlib compression
   - Gzip compression
   - Zstd compression
   - LZ4 compression
   - Oodle compression (with optional feature)

5. **Security Features**:
   - Reading of AES-encrypted .pak files
   - Protection against path traversal attacks during extraction

6. **Performance Optimizations**:
   - Efficient memory usage
   - Fast extraction speeds
   - Deterministic index writing

## Usage

repak provides a command-line interface with several subcommands:

```console
$ repak --help
Usage: repak [OPTIONS] <COMMAND>

Commands:
  info       Print .pak info
  list       List .pak files
  hash-list  List .pak files and the SHA256 of their contents. Useful for finding differences between paks
  unpack     Unpack .pak file
  pack       Pack directory into .pak file
  get        Reads a single file to stdout
  help       Print this message or the help of the given subcommand(s)

Options:
  -a, --aes-key <AES_KEY>  256 bit AES encryption key as base64 or hex string if the pak is encrypted
  -h, --help               Print help
  -V, --version            Print version
```

### Packing Files

```console
$ find mod
mod
mod/assets
mod/assets/AssetA.uasset
mod/assets/AssetA.uexp

$ repak pack -v mod
packing assets/AssetA.uasset
packing assets/AssetA.uexp
Packed 4 files to mod.pak
```

### Unpacking Files

```console
$ repak --aes-key 0x12345678 unpack MyEncryptedGame.pak
Unpacked 12345 files to MyEncryptedGame from MyEncryptedGame.pak
```

## Integration with retoc

repak is integrated into the retoc project to provide .pak file handling capabilities:

1. When retoc converts assets from Zen format to Legacy format, it uses repak to create .pak files that contain the converted Legacy assets.

2. When retoc converts assets from Legacy format to Zen format, it uses repak to read from existing .pak files to access the Legacy assets for conversion.

The integration is primarily through the repak library API, which provides a clean interface for reading and writing .pak files. This allows retoc to focus on asset conversion while delegating .pak file handling to repak.
