# Project Brief: retoc

## Overview
retoc is a command-line interface (CLI) tool designed for working with Unreal Engine asset containers. It serves two primary functions:
1. Packing/unpacking Unreal Engine IoStore containers (.utoc/.ucas files)
2. Converting between Zen assets and Legacy assets (found in .pak containers)

## Core Functionality
- Extract manifests from .utoc files
- Display container information
- List files in .utoc (directory index)
- Unpack chunks (files) from .utoc
- Convert assets from Zen format to Legacy format
- Convert assets from Legacy format to Zen format

## Target Users
- Game modders
- Unreal Engine developers
- Game asset researchers and analysts

## Technical Requirements
- Support for Unreal Engine versions 5.3+ (well supported)
- Limited support for earlier versions (may have issues with dependency information)
- Ability to handle shader libraries and complex asset conversions

## Project Goals
- Provide a reliable tool for converting between different Unreal Engine asset formats
- Enable modding and analysis of Unreal Engine games
- Support the game development and modding communities with robust asset conversion capabilities
