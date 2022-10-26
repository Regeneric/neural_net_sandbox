package.path = package.path..";../headers/lua/?.lua"    -- Optional
json = require "json"                                   -- Optional


width  = 50  -- Mandatory
height = 50  -- Mandatory 
inputs = 26  -- Mandatory
offset = 2   -- Optional


inputFile  = "./datasets/letters-owen.png"           -- Mandatory
outputFile = "./output/letters-owen.input.data"      -- Optional

showOutput = true  -- Optional
framerate = 7      -- Optional

inputData = {}


function convert()
    print("[LUA] Input data size: "..#inputData)
    print("[LUA] Writing to file: "..outputFile)
    inputData = json.encode(inputData)

    local of = assert(io.open(outputFile, "wb"))
        of:write(inputData)
        of:close()
    
    print("[LUA] Writing has ended")
end