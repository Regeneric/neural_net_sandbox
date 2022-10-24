function fileExists(file) 
    local file = io.open(file, "rb")
    if file then file:close() end
    return file ~= nil
end

function dataset(file)
    if not fileExists(file) then return {} end
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
        print("table["..key.."] "..val)
    end
end

function printTableElem(table, index) 
    for key,val in pairs(table) do
        if key == index then print(val) end
    end
end


local unpack = unpack or table.unpack
table.slice = function(a, start ,_end)
    return {unpack(a, start, _end)}
end