# About retoc

## Overview

retoc is a CLI tool for packing/unpacking Unreal Engine IoStore containers (.utoc/.ucas) as well as converting between Zen assets and Legacy assets (found in .pak containers). It provides a comprehensive solution for working with Unreal Engine's newer asset storage system (IoStore) and facilitates conversion between different asset formats.

## Project Goals

The primary goals of the retoc project are:

1. **IoStore Container Manipulation**: Provide tools for working with Unreal Engine's IoStore containers (.utoc/.ucas files), including extracting, listing, and creating these containers.

2. **Asset Format Conversion**: Enable conversion between Zen assets (used with IoStore) and Legacy assets (used with .pak files), allowing for compatibility across different Unreal Engine versions.

3. **Shader Library Support**: Handle shader libraries during conversion, ensuring that shaders are properly processed when converting between formats.

4. **Version Compatibility**: Support a wide range of Unreal Engine versions, with particular focus on UE 5.3+ for full compatibility.

## File/Folder Structure

The retoc project is organized as follows:

```
retoc/
├── src/                      # Main source code
│   ├── main.rs               # CLI entry point and command implementations
│   ├── iostore.rs            # IoStore container handling
│   ├── iostore_writer.rs     # IoStore container creation
│   ├── container_header.rs   # IoStore container header parsing
│   ├── asset_conversion.rs   # Asset conversion logic
│   ├── zen_asset_conversion.rs # Zen asset conversion
│   ├── legacy_asset.rs       # Legacy asset handling
│   ├── zen.rs                # Zen asset format handling
│   ├── shader_library.rs     # Shader library processing
│   └── ...                   # Other supporting modules
├── load_logger/              # Logging utility
├── repak/                    # Integrated repak library for .pak file handling
└── tests/                    # Test files and test data
```

## Features

retoc offers a range of features for working with Unreal Engine assets:

1. **IoStore Container Operations**:
   - Extract manifest from .utoc files
   - Show container information
   - List files in .utoc (directory index)
   - Extract chunks (files) from .utoc
   - Extract and pack raw chunks

2. **Asset Conversion**:
   - Convert assets from Zen format to Legacy format
   - Convert assets from Legacy format to Zen format
   - Handle shader libraries during conversion

3. **Shader Support**:
   - Convert shader libraries between formats
   - Maintain shader references and dependencies

4. **Flexible Configuration**:
   - Support for AES encryption keys
   - Engine version overrides
   - Filtering options for specific assets

## Usage

retoc provides a command-line interface with several subcommands:

```console
$ retoc --help
Usage: retoc [OPTIONS] <COMMAND>

Commands:
  manifest    Extract manifest from .utoc
  info        Show container info
  list        List fils in .utoc (directory index)
  unpack      Extracts chunks (files) from .utoc
  to-legacy   Converts asests and shaders from Zen to Legacy
  to-zen      Converts assets and shaders from Legacy to Zen
  help        Print this message or the help of the given subcommand(s)

Options:
  -a, --aes-key <AES_KEY>
  -h, --help               Print help
```

### Converting Zen to Legacy

```console
$ retoc to-legacy AbioticFactor/Content/Paks legacy_P.pak
Detected package version: FPackageFileVersion(UE4: 522, UE5: 1012), EZenPackageVersion: 3
[00:00:06] ########################################   22522/22522
Extracted 22522 (0 failed) legacy assets to "legacy_P.pak"
```

### Converting Legacy to Zen

```console
$ retoc to-zen legacy_P.pak iostore.utoc --version UE5_4
[00:00:02] ########################################   22522/22522
```

## Integration with repak

retoc integrates with the repak library to handle .pak files during the conversion process:

1. When converting from Zen to Legacy format, retoc uses repak to create .pak files that contain the converted Legacy assets.

2. When converting from Legacy to Zen format, retoc uses repak to read from existing .pak files to access the Legacy assets for conversion.

This integration allows retoc to provide a complete solution for working with both IoStore containers (.utoc/.ucas) and traditional .pak files, facilitating seamless conversion between the two asset storage systems.
