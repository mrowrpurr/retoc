# Progress: retoc

## What Works

Based on the README and project structure, the following functionality appears to be implemented and working:

### Core Functionality
- **Manifest Extraction**: Extracting manifests from .utoc files
- **Container Information**: Displaying information about IoStore containers
- **Directory Listing**: Listing files contained within .utoc containers
- **Asset Unpacking**: Extracting chunks/files from .utoc containers
- **Zen to Legacy Conversion**: Converting assets from Zen format to Legacy format
- **Legacy to Zen Conversion**: Converting assets from Legacy format to Zen format

### Supporting Features
- **Progress Reporting**: Visual progress indicators for long-running operations
- **Shader Library Handling**: Processing of shader libraries during conversion
- **Version Detection**: Automatic detection of Unreal Engine versions
- **AES Key Support**: Handling of encrypted assets with provided AES keys

## What's Left to Build

As this is an initial exploration of an existing project, we don't have a complete understanding of what might be missing or planned. However, based on the README and project structure, potential areas for enhancement might include:

- **Additional Engine Version Support**: Improving support for earlier Unreal Engine versions
- **Dependency Handling**: Better handling of asset dependencies for pre-5.3 versions
- **Additional Asset Types**: Support for more specialized asset types
- **Performance Optimizations**: Further optimizations for large-scale conversions
- **User Interface Improvements**: Potentially a more user-friendly interface beyond CLI

## Current Status

The project appears to be functional and capable of performing its core tasks. Based on the README, it can successfully:

- Extract information from IoStore containers
- Convert between Zen and Legacy asset formats
- Handle shader libraries during conversion
- Process assets from different Unreal Engine versions

The codebase is organized into logical modules that handle different aspects of the functionality, and the project includes test data for various Unreal Engine versions.

## Known Issues

From the README, one known limitation is mentioned:

- **Pre-5.3 Dependency Information**: Lack of dependency information in versions prior to Unreal Engine 5.3 may result in games failing to load some assets when converted to .pak format, though this should not be an issue for modding purposes.

Additional issues might be documented in the code or issue tracker, which would require further investigation.

## Evolution of Project Decisions

Without access to the project's history or development discussions, it's difficult to trace the evolution of project decisions. However, some inferences can be made:

### Architecture Decisions
- **Modular Design**: The project is organized into focused modules, suggesting a deliberate decision to maintain separation of concerns
- **Rust Implementation**: The choice of Rust likely reflects a focus on performance and memory safety for handling complex binary formats
- **Command-Based Structure**: The CLI uses a command-based structure, providing a clear and extensible interface

### Version Support Strategy
- **Focus on UE 5.3+**: The project prioritizes support for newer Unreal Engine versions (5.3+)
- **Limited Support for Earlier Versions**: Earlier versions are supported but with known limitations
- **Test Data Organization**: Test data is organized by engine version, suggesting a systematic approach to version compatibility

### Feature Prioritization
- **Core Conversion Functionality**: The primary focus appears to be on reliable asset conversion
- **Shader Library Handling**: Special attention is given to shader libraries, suggesting their importance in the conversion process
- **Progress Reporting**: Implementation of progress indicators shows consideration for user experience during long operations

## Next Development Priorities

Based on the current state, potential priorities for future development might include:

1. **Improved Pre-5.3 Support**: Addressing limitations with earlier Unreal Engine versions
2. **Performance Optimization**: Further optimizing for large-scale conversions
3. **Additional Asset Type Support**: Expanding support for specialized asset types
4. **Enhanced Error Handling**: Improving error reporting and recovery
5. **Documentation Expansion**: More comprehensive documentation of usage patterns and limitations

## Milestone Tracking

As this is an initial exploration, we don't have information about specific milestones or their completion status. This section will be updated as we learn more about the project's roadmap and progress.
