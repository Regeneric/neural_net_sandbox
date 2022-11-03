package.path = package.path..";./headers/lua/?.lua"    -- Optional
json = require "json" 
fennel = require("fennel").install()

function fileExists(file) 
    local file = io.open(file, "rb")
    if file then file:close() end
    return file ~= nil
end

function bytes(file, debug)
    if not fileExists(file) then 
        print("[LUA] File not found")
        return {} 
    end
    if debug then print("[LUA] Reading file...  "..file) end

    
    local data = io.open(file, "rb")
    local readjson = data:read("*a")
    local inputArray = json.decode(readjson)
    data:close()

    for i=1,#inputArray do
        inputArray[i] = inputArray[i] + .0
    end
    
    return inputArray
end

function dataset(file, debug)
    if not fileExists(file) then 
        print("[LUA] File not found")
        return {} 
    end
    if debug then print("[LUA] Reading file...  "..file) end

    local lines = {}
    for line in io.lines(file) do
        lines[#lines+1] = line
    end
    return lines
end

function matchVals(val, regex)
    local output = {}
    for match in val:gmatch(regex) do
        output[#output+1] = match
    end
    return output
end


function printTable(table)
    for key,val in pairs(table) do
        print("[LUA] table["..key.."] "..val)
    end
end


local unpack = unpack or table.unpack
table.slice = function(a, start ,_end)
    return {unpack(a, start, _end)}
end

function string.tohex(str)
    return (str:gsub('.',function(c)
        return string.format('%02X',string.byte(c))
    end))
end