target("pak_utoc_ucas")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include")
    
    -- Add dependencies on the static libraries
    add_deps("pak", "utoc", "ucas", "liboodle")
    
    -- Add dependencies on any packages:
    add_packages("cli11")      -- Command line argument parsing
    add_packages("osmanip")    -- Terminal output manipulation
    add_packages("indicators") -- Progress bars
    add_packages("rang")       -- Colored terminal output
    add_packages("spdlog")     -- Logging
    add_packages("nlohmann_json") -- JSON parsing
