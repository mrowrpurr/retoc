target("pak")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })
    
    -- Add dependencies on any other C++ static libraries in this folder:
    add_deps("liboodle")
    
    -- Add dependencies on any packages:
    add_packages("zlib", { public = true })
    add_packages("zstd", { public = true })
    add_packages("lz4", { public = true })
    add_packages("openssl", { public = true })
    add_packages("unordered_dense", { public = true })
