{
    depfiles_format = "cl_json",
    values = {
        [[C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\bin\HostX64\x64\cl.exe]],
        {
            "-nologo",
            "-Zi",
            "-FS",
            [[-Fdbuild\windows\x64\debug\utoc_reader.pdb]],
            "-Od",
            "-std:c++latest",
            "-Icpp_include",
            "/EHsc"
        }
    },
    files = {
        [[cpp_src\example_file.cpp]]
    },
    depfiles = "{\
    \"Version\": \"1.2\",\
    \"Data\": {\
        \"Source\": \"d:\\\\code\\\\other\\\\retoc\\\\cpp_src\\\\example_file.cpp\",\
        \"ProvidedModule\": \"\",\
        \"Includes\": [],\
        \"ImportedModules\": [],\
        \"ImportedHeaderUnits\": []\
    }\
}"
}