require "./headers/lua/commons"   -- Optional


labels     = {}     -- Optional
topology   = {}     -- Mandatory
inputData  = {}     -- Mandatory
targetData = {}     -- Mandatory
resultData = {}     -- Mandatory

iterations = 1000  -- Mandatory  -- minimal number of iterations when using known weights == 2 ;  recommended == input neurons * 2
threshold  = 0.91   -- Optional

failCounter = 0     -- Optional
passCounter = 0     -- Optional

useTrainedWeights  = true   -- Optional
trainedWeightsFile = "./datasets/weights/test-pattern.weights.data"  -- Optional

useRetina = false   -- Optional  -  use SFML canvas to provide input data


function setup()
    local inputFile = "./datasets/test-pattern.input.data"
    local dataFile  = "./datasets/test-pattern.data"
    
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
    local width  = 50
    local height = 50  

    if next(labels) ~= nil then
        print("\n"..labels[index])
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

    io.write('\nTarget: ')
    for key,val in pairs(targetData) do
        io.write(string.format("%2f ", val))
    end

    io.write('\n')
end