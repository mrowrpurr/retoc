# Active Context: retoc

## Current Work Focus
The current focus is on understanding the retoc project, which is a CLI tool for working with Unreal Engine asset containers. We are examining the project structure, functionality, and purpose to build a comprehensive understanding of how it works and what it does.

## Recent Changes
As this is the initial exploration of the project, there are no recent changes to document yet. This file will be updated as we make progress in understanding and potentially modifying the codebase.

## Next Steps
1. **Deeper Code Exploration**: Examine key source files to understand the implementation details
2. **Functionality Testing**: Test the various commands to see how they work in practice
3. **Documentation Review**: Look for any additional documentation that might provide more insights
4. **Dependency Analysis**: Analyze the project dependencies in Cargo.toml to understand external libraries used

## Active Decisions and Considerations
- **Project Scope Understanding**: Determining the exact scope and capabilities of the tool
- **Architecture Analysis**: Understanding the architectural decisions made in the codebase
- **Version Support**: Identifying which Unreal Engine versions are supported and how version-specific code is organized
- **Performance Characteristics**: Considering how the tool handles large asset files and performance optimizations

## Important Patterns and Preferences
Based on the initial examination of the project:

- **Command-Line Interface**: The project uses a command-based CLI structure
- **Modular Design**: Code is organized into focused modules for different aspects of functionality
- **Rust Idioms**: The project likely follows standard Rust idioms and patterns
- **Binary Processing**: Custom binary processing is a core aspect of the codebase
- **Progress Reporting**: The tool provides progress indicators for long-running operations

## Learnings and Project Insights
Initial insights from examining the README and project structure:

- The project serves two main purposes: working with IoStore containers and converting between asset formats
- It has good support for Unreal Engine 5.3+ but limited support for earlier versions
- The tool handles complex binary formats and requires deep understanding of Unreal Engine asset structures
- The project includes test data for different Unreal Engine versions, suggesting a focus on compatibility
- The codebase is organized into logical modules that reflect different aspects of asset processing

## Current Understanding of Key Components

### Asset Conversion
- Handles conversion between Zen and Legacy asset formats
- Implemented in `asset_conversion.rs` and related files
- Requires understanding of both format specifications

### Container Handling
- Manages reading from and writing to IoStore containers (.utoc/.ucas)
- Implemented in `iostore.rs`, `iostore_writer.rs`, and related files
- Includes directory indexing and chunk extraction

### Shader Library Processing
- Special handling for shader libraries during conversion
- Implemented in `shader_library.rs`
- Includes compression and optimization of shader code

### Version Management
- Detects and handles different Unreal Engine versions
- Implemented in `version.rs` and `version_heuristics.rs`
- Critical for ensuring compatibility across engine versions

## Open Questions
- How does the tool handle encrypted assets?
- What are the specific limitations when working with pre-5.3 Unreal Engine versions?
- How are dependencies between assets tracked and maintained during conversion?
- What is the performance profile for large-scale asset conversion?
