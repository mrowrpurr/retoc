add_rules("mode.debug", "mode.release")

-- Reminder: this means you have access to std::format, std::filesystem, std::string_view, etc.
set_languages("c++23")

add_requires(
    "catch2",
    "spdlog",
    "reproc",
    "trompeloeil",
    "xxhash",
    "unordered_dense",
    "indicators",
    "thread-pool",
    "nlohmann_json",
    "boost_di",
    "cli11",
    "stb"
)

-- Add Windows-specific settings
if is_plat("windows") then
    add_defines("_CRT_SECURE_NO_WARNINGS")
    add_defines("NOMINMAX")
    add_defines("WIN32_LEAN_AND_MEAN")
end

includes("cpp/*/xmake.lua")
includes("cpp/examples/*/xmake.lua")

-- TODO: we'll make a cpp/tests/ folder too! with catch2 etc, but notyet.
 