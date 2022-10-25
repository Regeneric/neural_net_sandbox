require "headers/lua/commons"   -- Optional

labels     = {}     -- Optional
topology   = {}     -- Mandatory
inputData  = {}     -- Mandatory
targetData = {}     -- Mandatory
resultData = {}     -- Mandatory

iterations = 1000   -- Mandatory  -- minimal number of iterations when using known weights == 2;  recommended == input neurons * 2
threshold  = 0.91   -- Optional

failCounter = 0     -- Optional
passCounter = 0     -- Optional

useTrainedWeights  = true   -- Optional
trainedWeightsFile = "./datasets/weights/plusminus.weights.data"  -- Optional


function setup()
    local file = "./datasets/plusminus.data"   -- Loading test data
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

    -- minimal number of iterations when using known weights == 2;  recommended == input neurons * 2
    if useTrainedWeights == true then iterations = topology[1]*2 end
end


-- `index` is passed from C++ - iterator over elemnts in single row of inputData
-- `iterTo` is passed from C++ - maximum `index` value
function display(index, iterTo) 
    -- It goes in a loop
    -- index == 1;  index == 2;  index == 3; etc.

    local offsetBgn = 1
    local offsetEnd = 1 


    if next(labels) ~= nil then
        print("\n"..labels[index])
    end


    -- if index == 1 then 
    --     offsetBgn = 1
    --     offsetEnd = (iterTo*index)+1
    -- else
    --     offsetBgn = iterTo*index
    --     offsetEnd = iterTo*(index+1)
    -- end
    
    if index == 1 then 
        offsetBgn = (iterTo*index)-iterTo+1
        offsetEnd = iterTo*(index+1)-iterTo+1
    else 
        offsetBgn = (iterTo*index)-iterTo+2
        offsetEnd = iterTo*(index+1)-iterTo+2 
    end

    io.write("Input: ")
    local inpBuf = {}
    for key,val in pairs(table.slice(inputData, offsetBgn, offsetEnd)) do
        inpBuf[#inpBuf+1] = val
        io.write(val, ' ')
    end 


    io.write('\nOutput: ')
    for key,val in pairs(resultData) do
        -- if val >= threshold then
        --     print("Output["..key.."]:", string.format("%4f", val), "PASS")
        --     passCounter = passCounter+1
        -- else
        --     print("Output["..key.."]:", string.format("%4f", val), "FAIL")
        --     failCounter = failCounter+1
        -- end
        io.write(string.format("%4f ", val))
    end


    local outIter = tonumber(topology[#topology])
    offsetBgn = ((outIter*index)+1)-outIter
    offsetEnd = (outIter*(index+1))-outIter

    io.write("\nTarget: ")
    local tgtBuf = {}
    for key,val in pairs(table.slice(targetData, offsetBgn, offsetEnd)) do
        tgtBuf[#tgtBuf+1] = val
        io.write(string.format("%4f ", val))
    end 

    io.write('\n')
end