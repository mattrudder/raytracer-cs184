
require 'tundra.syntax.osx-bundle'
require 'tundra.syntax.glob'

local native = require 'tundra.native'

Program {
    Name = "raytracer",
    Sources = {
        Glob {
            Dir = "src",
            Extensions = { ".cpp" },
        },
    },
    Libs = {
        { "freeimage"; Config = "macosx-*-*" }
    },
    Env = {
        CPPPATH = {
            "src",
        },
    },
}

Default('raytracer')
