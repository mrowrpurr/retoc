# Product Context: retoc

## Why This Project Exists
retoc exists to bridge the gap between different Unreal Engine asset formats, specifically addressing the challenges that arise when working with various versions of Unreal Engine and their corresponding asset containers. As Unreal Engine evolves, its asset formats change, creating compatibility issues for developers, modders, and researchers who need to work across different versions.

## Problems It Solves

### Asset Format Compatibility
- **Format Conversion**: Enables conversion between Zen assets (newer format) and Legacy assets (older format), allowing for backward and forward compatibility.
- **Version Bridging**: Helps bridge the gap between different Unreal Engine versions (particularly 5.3+ and earlier versions).

### Asset Extraction and Analysis
- **Container Unpacking**: Provides tools to extract and examine the contents of Unreal Engine's IoStore containers (.utoc/.ucas files).
- **Manifest Extraction**: Allows users to extract and analyze manifest data from .utoc files.
- **Directory Indexing**: Enables listing of files within .utoc containers for better visibility into game assets.

### Modding Support
- **Asset Modification**: Facilitates game modding by allowing assets to be extracted, potentially modified, and repacked.
- **Cross-Version Compatibility**: Helps modders work with games built on different Unreal Engine versions.

## How It Should Work
retoc is designed as a command-line tool with a straightforward interface that:

1. **Accepts Clear Commands**: Uses a simple command structure (e.g., `retoc [command] [arguments]`) for different operations.
2. **Processes Assets Efficiently**: Handles large asset files and containers with reasonable performance.
3. **Provides Feedback**: Gives clear progress indicators and results during operations.
4. **Maintains Data Integrity**: Ensures converted assets retain their functionality and properties.

## User Experience Goals

### For Developers
- **Seamless Integration**: Easy to incorporate into development workflows and pipelines.
- **Reliable Conversion**: Consistent and dependable asset conversion with minimal errors.
- **Detailed Information**: Access to comprehensive information about asset containers and their contents.

### For Modders
- **Accessibility**: Straightforward usage that doesn't require deep technical knowledge of Unreal Engine internals.
- **Flexibility**: Support for various modding scenarios and asset types.
- **Compatibility**: Ability to work with assets from games built on different Unreal Engine versions.

### For Researchers
- **Transparency**: Clear visibility into asset structures and relationships.
- **Analysis Capabilities**: Tools to examine and understand game assets and their organization.
- **Format Insights**: Better understanding of how Unreal Engine stores and manages assets across versions.
