add_rules("mode.debug", "mode.release")

-- Reminder: this means you have access to std::format, std::filesystem, std::string_view, etc.
set_languages("c++23")

add_requires(
    -- Common dependencies for pak + utoc + ucas.
    "zlib",
    "zstd",
    "lz4",
    "openssl",
    
    -- utoc
    "blake3",
    
    -- General dependencies
    "unordered_dense",
    "spdlog",
    "nlohmann_json",
    
    -- Command Line utilities
    "cli11",
    "osmanip",
    "indicators",
    "rang"
)

-- Add Windows-specific settings
if is_plat("windows") then
    add_defines("_CRT_SECURE_NO_WARNINGS")
    add_defines("NOMINMAX")
    add_defines("WIN32_LEAN_AND_MEAN")
end

includes("cpp/libraries/*/xmake.lua")
includes("cpp/programs/*/xmake.lua")
includes("cpp/examples/*/xmake.lua")

-- TODO: we'll make a cpp/tests/ folder too! with catch2 etc, but notyet.
