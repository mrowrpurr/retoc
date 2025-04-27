# System Patterns: retoc

## System Architecture
retoc is structured as a Rust-based command-line application with a modular architecture that separates concerns between different aspects of Unreal Engine asset handling:

1. **Command Processing Layer**: Handles CLI arguments and dispatches to appropriate functionality
2. **Asset Processing Core**: Contains the logic for asset conversion and manipulation
3. **Container Handling**: Manages reading from and writing to Unreal Engine container formats
4. **Utility Components**: Provides shared functionality like compression, serialization, and logging

## Key Technical Decisions

### Rust as Implementation Language
- **Memory Safety**: Leverages Rust's memory safety guarantees for robust handling of complex binary formats
- **Performance**: Utilizes Rust's performance characteristics for efficient processing of large asset files
- **Error Handling**: Takes advantage of Rust's strong error handling patterns

### Modular File Format Handling
- **Format Abstraction**: Abstracts the differences between Zen and Legacy asset formats
- **Version-Specific Logic**: Implements version-specific handling for different Unreal Engine versions
- **Extensible Design**: Allows for adding support for new versions or format variations

### Binary Processing Approach
- **Direct Binary Manipulation**: Works directly with binary formats rather than using intermediate representations
- **Streaming Processing**: Where possible, processes data in a streaming fashion to handle large files efficiently
- **Minimal Dependencies**: Minimizes external dependencies to maintain control over the binary processing logic

## Design Patterns in Use

### Command Pattern
- Used to implement the various CLI commands (manifest, info, list, unpack, to-legacy, to-zen)
- Each command is encapsulated in its own module with a consistent interface

### Factory Pattern
- Employed for creating appropriate asset handlers based on detected versions and formats
- Allows for runtime selection of the correct processing logic

### Strategy Pattern
- Applied to compression and decompression algorithms that may vary based on asset type and version
- Enables swapping different processing strategies without changing the client code

### Builder Pattern
- Used for constructing complex asset structures during conversion processes
- Provides a clear and consistent way to assemble multi-part assets

## Component Relationships

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  CLI Interface  │────▶│ Command Handlers │────▶│ Asset Processors│
└─────────────────┘     └─────────────────┘     └─────────────────┘
                                                         │
                                                         ▼
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  Serialization  │◀────│ Format Handlers  │◀────│ Container I/O   │
└─────────────────┘     └─────────────────┘     └─────────────────┘
        │                        │                       │
        └────────────────┬───────┘                       │
                         ▼                               │
                ┌─────────────────┐                      │
                │  Compression    │◀─────────────────────┘
                └─────────────────┘
```

## Critical Implementation Paths

### Asset Format Detection
1. Examine file headers and metadata
2. Determine Unreal Engine version
3. Select appropriate format handlers
4. Initialize conversion context with version-specific parameters

### Zen to Legacy Conversion
1. Parse Zen asset format
2. Extract asset data and metadata
3. Transform to Legacy format structure
4. Handle shader libraries and dependencies
5. Write to Legacy container format (.pak)

### Legacy to Zen Conversion
1. Parse Legacy asset format
2. Extract asset data and metadata
3. Transform to Zen format structure
4. Process shader libraries
5. Write to IoStore container format (.utoc/.ucas)

### Container Unpacking
1. Parse container header and directory structure
2. Locate requested assets or chunks
3. Handle decompression if needed
4. Extract to specified output location

## Key Abstractions

### Asset Representation
- **Abstract Asset Interface**: Common interface for working with assets regardless of format
- **Format-Specific Implementations**: Concrete implementations for Zen and Legacy formats
- **Version-Specific Adaptations**: Adjustments for different Unreal Engine versions

### Container Formats
- **Container Reader/Writer**: Abstractions for reading from and writing to different container formats
- **Directory Structure**: Representation of the internal organization of asset containers
- **Chunk Management**: Handling of data chunks within containers

### Serialization System
- **Binary Serialization**: Tools for reading and writing binary data with proper endianness handling
- **Type Conversion**: Utilities for converting between different data representations
- **Version Compatibility**: Logic to handle differences in serialization across engine versions
