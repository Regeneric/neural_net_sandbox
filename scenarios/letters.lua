require "./headers/lua/commons"   -- Optional


labels     = {}     -- Optional
topology   = {}     -- Mandatory
inputData  = {}     -- Mandatory
targetData = {}     -- Mandatory
resultData = {}     -- Mandatory

iterations = 50000  -- Mandatory  -- minimal number of iterations when using known weights == 2 ;  recommended == input neurons * 2
threshold  = 0.91   -- Optional

failCounter = 0     -- Optional
passCounter = 0     -- Optional

useTrainedWeights  = false   -- Optional
trainedWeightsFile = "./datasets/weights/letters-owen.weights.data"  -- Optional

useRetina = false   -- Optional  -  use SFML canvas to provide input data


function setup()
    local inputFile = "./datasets/letters-owen.input.data"
    local dataFile  = "./datasets/letters.data"
    
    inputData = bytes(inputFile, true)
    local data  = dataset(dataFile, true)
    

    for key,val in pairs(data) do
        -- Looking for `2 4 1` etc.
        if string.find(val, "topology:") then
            -- topology = matchVals(val, "%d+")
            for match in val:gmatch("%d+") do
                topology[#topology+1] = match
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

    -- minimal number of iterations when using known weights == 2 ;  recommended == input neurons * 2
    if useTrainedWeights == true then iterations = topology[1]*2 end  
end


function display(index, iterTo)
    if index > tonumber(topology[#topology]) then return {} end

    if next(labels) ~= nil then
        print("\nLetter: "..labels[index])
    end

    
    io.write('\n')
    for key,val in pairs(resultData) do
        if val >= threshold then
            print("Output["..key.."]:", string.format("%4f", val), "PASS")
            passCounter = passCounter+1
        else
            print("Output["..key.."]:", string.format("%4f", val), "FAIL")
            failCounter = failCounter+1
        end
    end

    local outIter = tonumber(topology[#topology])
    offsetBgn = ((outIter*index)+1)-outIter
    offsetEnd = (outIter*(index+1))-outIter

    io.write("Target: ")
    local tgtBuf = {}
    for key,val in pairs(table.slice(targetData, offsetBgn, offsetEnd)) do
        tgtBuf[#tgtBuf+1] = val
        io.write(val, ' ')
    end 

    io.write('\n')
end