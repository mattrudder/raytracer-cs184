
local common = {
    Env = {
        CPPPATH = { "include" },
        CCOPTS = {
            { "/W4", "/WX", "/wd4127", "/wd4100", "/wd4324"; Config = "*-msvc-*" },
            { "-Wall", "-Werror"; Config = { "*-gcc-*", "*-clang-*" } },
            { "-g"; Config = { "*-gcc-debug", "*-clang-debug" } },
            { "-O2"; Config = { "*-gcc-production", "*-clang-production" } },
            { "-O3"; Config = { "*-gcc-release", "*-clang-release" } },
            { "/O2"; Config = "*-msvc-production" },
            { "/Ox"; Config = "*-msvc-release" },
        },
        CXXOPTS = {
            { "-std=c++11", "-stdlib=libc++"; Config = { "*-clang-*" } },
        },
        SHLIBOPTS = {
            { "-stdlib=libc++"; Config = { "*-clang-*" } },
        },
        PROGOPTS = {
            { "-stdlib=libc++"; Config = { "*-clang-*" } },
        },
        CPPDEFS = {
            { "_CRT_SECURE_NO_WARNINGS"; Config = "*-msvc-*" },
            { "NDEBUG"; Config = "*-*-release" },
        },
    },
    ReplaceEnv = {
        OBJECTROOT = "bin",
        LD = {
            { "clang++"; Config = { "*-clang-*" } },
        },
    },
}

Build {
    Units = "units.lua",
    Passes = {
        Compile = { Name = "Main compile pass", BuildOrder = 1 },
    },
    Configs = {
        Config { Name = "linux-clang", Inherit = common, Tools = { "clang" }, DefaultOnHost = "linux" },
        Config { Name = "macosx-clang", Inherit = common, Tools = { "clang-osx" }, DefaultOnHost = "macosx" },
        Config { Name = "win64-winsdk7", Inherit = common, Tools = { { "msvc-winsdk"; TargetArch = "x64", VcVersion = "11.0" } }, DefaultOnHost = "win32" },
    },
}
