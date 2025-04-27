add_rules("mode.debug", "mode.release")

set_languages("c++23")

target("utoc_reader")
    set_kind("static")
    add_files("cpp_src/*.cpp")
    add_includedirs("cpp_include", { public = true })

target("example")
    set_kind("binary")
    add_files("cpp_example/*.cpp")
    add_deps("utoc_reader")
    add_includedirs("cpp_example", { public = true })
