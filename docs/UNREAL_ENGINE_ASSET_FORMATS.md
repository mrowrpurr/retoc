# Unreal Engine Asset Formats

## Overview of Unreal Engine Asset Formats

Unreal Engine uses different asset formats depending on the engine version and configuration. This document provides an overview of these formats and how they relate to each other.

Unreal Engine has two primary asset storage formats:

1. **Legacy Assets**: Used in Unreal Engine 4 and still supported in Unreal Engine 5
   - Stored in .pak files
   - Split into multiple files (.uasset, .uexp, .ubulk)
   - More compatible with older engine versions

2. **Zen Assets**: Introduced in Unreal Engine 5
   - Stored in IoStore containers (.utoc/.ucas)
   - More efficient storage and loading
   - Better performance for modern platforms

The retoc tool provides functionality to convert between these formats, allowing for compatibility across different engine versions and configurations.

## Legacy Assets (.uasset, .uexp, .ubulk)

Legacy assets are the traditional asset format used in Unreal Engine 4 and still supported in Unreal Engine 5. They are typically stored in .pak files and consist of multiple files for each asset:

### File Types

1. **`.uasset`**: Contains the asset header and metadata
   - Package summary
   - Name table
   - Import table
   - Export table
   - Dependency information
   - Asset registry data

2. **`.uexp`**: Contains the serialized export data
   - Object properties
   - Structured data
   - References to other assets

3. **`.ubulk`**: Contains large binary data
   - Texture data
   - Mesh data
   - Other large binary resources

4. **`.uptnl`**: Optional bulk data (introduced in UE5)
   - Contains optional resources that can be loaded on demand

5. **`.m.ubulk`**: Memory-mapped bulk data (introduced in UE5)
   - Designed for efficient memory mapping

### Structure

Legacy assets follow a structured format:

1. **Package Summary**:
   - File version information
   - Package flags
   - Name, import, and export counts and offsets
   - GUID and other identifiers

2. **Name Table**:
   - List of all names used in the asset
   - Each name has an index and optional number suffix

3. **Import Table**:
   - References to external objects
   - Each import includes class, package, and object name

4. **Export Table**:
   - Objects contained within the asset
   - Each export includes class, super, template, and object name
   - Serialization information (offset, size)
   - Dependency information

5. **Export Data**:
   - Serialized properties for each exported object
   - Structured according to the object's class

### Version Differences

The legacy asset format has evolved across Unreal Engine versions:

- **UE4.0-4.19**: Basic asset format with various improvements
- **UE4.20-4.25**: Added support for additional features like world composition
- **UE4.26-4.27**: Enhanced support for large worlds and optimizations
- **UE5.0+**: Added support for new features like World Partition, Nanite, and Lumen

## Zen Assets

Zen assets are the newer asset format introduced in Unreal Engine 5, designed to improve loading performance and efficiency. They are stored in IoStore containers (.utoc/.ucas files).

### Key Characteristics

1. **Chunk-Based Storage**:
   - Assets are divided into "chunks" of data
   - Each chunk has a unique ID and can be accessed independently
   - Different types of chunks for different types of data

2. **Package Structure**:
   - Package header with versioning information
   - Name map for efficient name storage
   - Import and export maps
   - Export bundle entries
   - Dependency information

3. **Optimized Loading**:
   - Designed for asynchronous loading
   - Better memory management
   - Improved streaming capabilities

### Components

1. **Package Header**:
   - Contains summary information
   - Versioning data
   - Offsets to other sections

2. **Name Map**:
   - Efficient storage of names used in the package
   - Mapped names for quick lookup

3. **Import Map**:
   - References to external objects
   - Uses package object indices for efficient lookup

4. **Export Map**:
   - Objects contained within the package
   - Includes class, super, template information
   - Object flags and serialization data

5. **Export Bundles**:
   - Groups of exports that should be loaded together
   - Commands for creating and serializing objects

6. **Dependency Information**:
   - Tracks dependencies between objects
   - Ensures correct loading order

### Version Differences

The Zen asset format has evolved across Unreal Engine 5 versions:

- **UE5.0**: Initial implementation with basic features
- **UE5.1-5.2**: Added support for optional segment packages and localized packages
- **UE5.3-5.4**: Removed export information and improved dependency handling
- **UE5.5+**: Added support for soft package references and other optimizations

## Conversion Between Formats

retoc provides functionality to convert between Legacy and Zen asset formats:

### Legacy to Zen Conversion

When converting from Legacy to Zen format:

1. **Asset Parsing**:
   - Parse .uasset, .uexp, and .ubulk files
   - Extract metadata and content

2. **Chunk Creation**:
   - Create appropriate chunks for different types of data
   - Generate chunk IDs and metadata

3. **Package Store Entry Creation**:
   - Create package store entries with dependency information
   - Set up import and export maps

4. **IoStore Container Creation**:
   - Write chunks to .ucas file
   - Create directory index and metadata in .utoc file

5. **Shader Conversion**:
   - Convert shader libraries to the appropriate format
   - Maintain shader references and dependencies

### Zen to Legacy Conversion

When converting from Zen to Legacy format:

1. **Chunk Extraction**:
   - Extract chunks from IoStore containers
   - Identify different types of data

2. **Asset Reconstruction**:
   - Reconstruct asset header and metadata
   - Set up name, import, and export tables

3. **File Creation**:
   - Create .uasset file with header information
   - Create .uexp file with export data
   - Create .ubulk file with bulk data if needed

4. **Dependency Resolution**:
   - Resolve dependencies between assets
   - Ensure correct references

5. **Shader Conversion**:
   - Convert shader libraries to the appropriate format
   - Maintain shader references and dependencies

### Compatibility Considerations

When converting between formats, several factors need to be considered:

1. **Engine Version Compatibility**:
   - Different engine versions support different features
   - Some features may not be available in older versions

2. **Asset Dependencies**:
   - Assets often depend on other assets
   - All dependencies must be properly converted

3. **Shader Support**:
   - Shader libraries require special handling
   - Different platforms may use different shader formats

4. **Platform Specifics**:
   - Some assets may be platform-specific
   - Conversion may need to account for platform differences

retoc handles these considerations to ensure the most compatible conversion possible between the different asset formats.
