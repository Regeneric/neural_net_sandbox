require "headers/lua/commons"   -- Option

labels     = {}     -- Optional
topology   = {}     -- Mandatory
inputData  = {}     -- Mandatory
targetData = {}     -- Mandatory
resultData = {}     -- Mandatory

iterations = 1000   -- Mandatory
threshold  = 0.91   -- Optional

failCounter = 0     -- Optional
passCounter = 0     -- Optional


function setup()
    local file = "./datasets/not.data"   -- Loading test data
    local data = dataset(file)

    for key,val in pairs(data) do
        -- Looking for `2 4 1` etc.
        if string.find(val, "topology:") then
            -- topology = matchVals(val, "%d+")
            for match in val:gmatch("%d+") do
                topology[#topology+1] = match
            end
        end

        -- Looking for `1.0 0.0` etc.
        if string.find(val, "in:") then
            -- inputData = matchVals(val, "%d+%.?%d+")
            for match in val:gmatch("%d+%.?%d+") do
                inputData[#inputData+1] = match
            end
        end

        -- Looking for `0.0` etc.
        if string.find(val, "out:") then
            -- targetData = matchVals(val, "%d+%.?%d+")
            for match in val:gmatch("%d+%.?%d+") do
                targetData[#targetData+1] = match
            end
        end

        -- Description is optional, can be anything
        if string.match(val, "desc:") then
            labels[#labels+1] = val:sub(7)
        end
    end
end


-- `index` is passed from C++ - iterator over elemnts in single row of inputData
-- `resultData` is passed from C++ - calculated data based on inputs and weights
function display(index) 
    -- It goes in a loop
    -- index == 1;  index == 2;  index == 3; etc.
    
    if next(labels) ~= nil then
        print("\nStates: "..labels[index])
    end

    print("Input: "..inputData[index])
    
    for key,val in pairs(resultData) do 
        if val >= threshold then
            print("Output: "..string.format("%6f", val).." PASS")
            passCounter = passCounter+1
        else
            print("Output: "..string.format("%6f", val).." FAIL")
            failCounter = failCounter+1
        end
    end

    print("Target: "..targetData[index])
end