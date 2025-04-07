BridgeFunctionTable = {}

function BridgeFunctionAttach(BridgeName, FunctionName)
    local bridgeIndex = nil

    if BridgeName == "OnDsQuery" then
        bridgeIndex = 1
    end

    if bridgeIndex ~= nil then
        if BridgeFunctionTable[bridgeIndex] == nil then
            BridgeFunctionTable[bridgeIndex] = {}
        end
        table.insert(BridgeFunctionTable[bridgeIndex], { Function = FunctionName })
    end
end

function OnDsQuery(size, query, label, param)
    if BridgeFunctionTable[1] ~= nil then
        for i = 1, #BridgeFunctionTable[1] do
            _G[BridgeFunctionTable[1][i].Function](size, query, label, param)
        end
    end
end
