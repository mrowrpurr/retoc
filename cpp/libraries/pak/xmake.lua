target("pak")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })
    
    -- Add dependencies on any other C++ static libraries in this folder:
    add_deps("liboodle")
    
    -- If this should rely, for example, on "utoc" and/or "ucas" then use add_deps("utoc", "ucas")
    
    -- Add dependencies on any packages:
    --
    -- Example:
    -- add_packages("spdlog")
    --
    -- See xmake.lua in the root folder for a list of available packages.
    --
    -- If you want a package which isn't available, please just stop and ask :)