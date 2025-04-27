# Technical Context: retoc

## Technologies Used

### Primary Language
- **Rust**: The entire application is written in Rust, leveraging its memory safety, performance, and robust error handling capabilities.

### Binary Processing
- **Custom Binary Parsers**: Hand-crafted binary parsers for Unreal Engine asset formats
- **Serialization/Deserialization**: Custom implementations for handling Unreal Engine's binary formats

### Compression
- **Compression Algorithms**: Support for various compression methods used by Unreal Engine (likely including zlib, oodle, etc.)
- **Decompression Utilities**: Tools for extracting compressed asset data

### File I/O
- **File Handling**: Rust standard library and custom file I/O implementations
- **Memory Mapping**: Possible use of memory mapping for efficient file access

## Development Setup

### Build System
- **Cargo**: Rust's package manager and build system
- **Workspace Structure**: The project appears to use a workspace with at least one sub-crate (load_logger)

### Project Structure
```
retoc/
├── src/                  # Main source code
│   ├── asset_conversion.rs
│   ├── compact_binary.rs
│   ├── compression.rs
│   ├── container_header.rs
│   ├── file_pool.rs
│   ├── iostore_writer.rs
│   ├── iostore.rs
│   ├── legacy_asset.rs
│   ├── logging.rs
│   ├── main.rs           # Entry point
│   ├── manifest.rs
│   ├── name_map.rs
│   ├── script_objects.rs
│   ├── ser.rs
│   ├── shader_library.rs
│   ├── version_heuristics.rs
│   ├── version.rs
│   ├── zen_asset_conversion.rs
│   └── zen.rs
├── load_logger/          # Separate crate for logging functionality
│   ├── src/
│   │   ├── lib.rs
│   │   ├── log.rs
│   │   └── resolvers.rs
│   ├── Cargo.toml
│   └── Cargo.lock
├── tests/                # Test files and test data
│   ├── issues/           # Test cases for specific issues
│   ├── UE4.27/           # Test data for Unreal Engine 4.27
│   ├── UE5.3/            # Test data for Unreal Engine 5.3
│   └── UE5.4/            # Test data for Unreal Engine 5.4
├── Cargo.toml            # Project manifest
└── Cargo.lock            # Dependency lock file
```

## Technical Constraints

### Unreal Engine Version Compatibility
- **Version Support**: Primary focus on UE 5.3+, with limited support for earlier versions
- **Format Differences**: Need to handle differences in asset formats between engine versions
- **Dependency Information**: Limited dependency information in versions prior to 5.3 may cause issues

### Binary Format Complexity
- **Proprietary Formats**: Working with proprietary and undocumented binary formats
- **Version Variations**: Handling variations in formats across engine versions
- **Reverse Engineering**: Likely requires reverse engineering of formats

### Performance Considerations
- **Large File Handling**: Need to efficiently process potentially very large asset files
- **Memory Management**: Careful memory management required for processing large assets
- **Processing Speed**: Balance between processing speed and memory usage

## Dependencies

The project likely has minimal external dependencies, focusing on Rust standard library and possibly:

- **clap**: Command-line argument parsing (based on CLI help output)
- **byteorder**: For handling endianness in binary data
- **compression libraries**: For supporting various compression algorithms
- **indicatif**: For progress bars (based on CLI output showing progress)

## Tool Usage Patterns

### Command-Line Interface
```
retoc [OPTIONS] <COMMAND>
```

#### Global Options
- `-a, --aes-key <AES_KEY>`: For encrypted assets
- `-h, --help`: Print help information

#### Commands
- **manifest**: Extract manifest from .utoc
  ```
  retoc manifest <utoc_file>
  ```

- **info**: Show container info
  ```
  retoc info <utoc_file>
  ```

- **list**: List files in .utoc (directory index)
  ```
  retoc list <utoc_file>
  ```

- **unpack**: Extracts chunks (files) from .utoc
  ```
  retoc unpack <utoc_file> <output_directory>
  ```

- **to-legacy**: Converts assets from Zen to Legacy format
  ```
  retoc to-legacy <input_directory> <output_pak_file>
  ```

- **to-zen**: Converts assets from Legacy to Zen format
  ```
  retoc to-zen <input_pak_file> <output_utoc_file> --version <UE_VERSION>
  ```

### Workflow Examples

#### Converting Zen Assets to Legacy Format
```console
$ retoc to-legacy AbioticFactor/Content/Paks legacy_P.pak
```
This command:
1. Processes all IoStore containers in the specified directory
2. Converts Zen assets to Legacy format
3. Extracts shader libraries
4. Writes everything to a .pak file

#### Converting Legacy Assets to Zen Format
```console
$ retoc to-zen legacy_P.pak iostore.utoc --version UE5_4
```
This command:
1. Reads assets from the specified .pak file
2. Converts them to Zen format
3. Processes shader libraries
4. Creates IoStore container files (.utoc/.ucas)

## Development Practices

### Testing
- **Test Data**: Includes test data for different Unreal Engine versions (UE4.27, UE5.3, UE5.4)
- **Issue Reproduction**: Contains specific test cases for reported issues

### Error Handling
- **Robust Error Reporting**: Likely uses Rust's Result type for error handling
- **Progress Tracking**: Implements progress reporting for long-running operations

### Performance Optimization
- **Efficient Binary Processing**: Optimized for handling large binary files
- **Memory Efficiency**: Careful management of memory for large assets
- **Parallel Processing**: Possible use of parallelism for processing multiple assets
