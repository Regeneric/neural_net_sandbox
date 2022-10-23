require "headers/lua/commons"

labels = {}         -- Optional
topology = {}       -- Mandatory
inputData = {}      -- Mandatory
targetData = {}     -- Mandatory
resultData = {}     -- Mandatory

iterations = 1000   -- Mandatory
threshold  = 0.91   -- Optional

failCounter = 0     -- Optional
passCounter = 0     -- Optional


function setup()
    local file = "./datasets/and.data"   -- Loading test data
    local data = dataset(file)

    -- All kinds of data can be structured differently, so it's up to user 
    -- to get out only valid inp and out values
    
    -- Example
    -- topology: 2 4 1
    -- desc: ON OFF
    -- in: 1.0 0.0
    -- out: 1.0
    -- desc: OFF ON
    -- in: 0.0 1.0
    -- out: 1.0
    -- ...

   
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

    print("Input: "..inputData[index+(index-1)]..' '..inputData[index*2])
    
    local ind = tonumber(topology[#topology])   -- Number of outputs
    for i = 1,ind do 
        if resultData[i] >= threshold then
            print("Output: "..string.format("%6f", resultData[i]).." PASS")
            passCounter = passCounter+1
        else
            print("Output: "..string.format("%6f", resultData[i]).." FAIL")
            failCounter = failCounter+1
        end
    end

    print("Target: "..targetData[index])
end