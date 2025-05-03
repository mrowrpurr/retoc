target("liboodle")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })
    if is_plat("windows") then
        add_syslinks("advapi32")
    end
    
    -- Add dependencies on any packages:
    --
    -- Example:
    -- add_packages("spdlog")
    --
    -- See xmake.lua in the root folder for a list of available packages.
    --
    -- If you want a package which isn't available, please just stop and ask :)