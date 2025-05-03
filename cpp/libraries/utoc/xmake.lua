target("utoc")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })
    
    -- Add dependencies on any other C++ static libraries in this folder:
    add_deps("liboodle")
    
    -- Add dependencies on any packages:
    --
    -- Example:
    -- add_packages("spdlog")
    --
    -- See xmake.lua in the root folder for a list of available packages.
    --
    -- If you want a package which isn't available, please just stop and ask :)