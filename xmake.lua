add_rules("mode.debug", "mode.release")

set_languages("c++23")

add_requires("catch2")
add_requires("zlib")
add_requires("zstd")
add_requires("lz4")

target("utoc_reader")
    set_kind("static")
    add_files("cpp_src/*.cpp")
    add_includedirs("cpp_include", { public = true })
    add_packages("zlib", "zstd", "lz4")

target("example")
    set_kind("binary")
    add_files("cpp_example/*.cpp")
    add_deps("utoc_reader")
    add_includedirs("cpp_example", { public = true })

target("tests")
    set_kind("binary")
    add_files("cpp_tests/*.cpp")
    add_deps("utoc_reader")
    add_packages("catch2", "zlib", "zstd", "lz4")
    add_includedirs("cpp_tests", { public = true })
