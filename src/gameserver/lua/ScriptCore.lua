-- ScriptCore.lua
BridgeFunctionTable = {}

function BridgeFunctionAttach(BridgeName, FunctionName)
    local bridgeIndex = nil

    if BridgeName == "OnClientConnect" then
        bridgeIndex = 1
    elseif BridgeName == "OnPacketRecv" then
        bridgeIndex = 2
    elseif BridgeName == "OnClientDisconnect" then
        bridgeIndex = 3
    elseif BridgeName == "OnSQLAsyncResult" then
        bridgeIndex = 4
    elseif BridgeName == "OnGlobalTick" then
        bridgeIndex = 5
    end

    if bridgeIndex ~= nil then
        if BridgeFunctionTable[bridgeIndex] == nil then
            BridgeFunctionTable[bridgeIndex] = {}
        end
        table.insert(BridgeFunctionTable[bridgeIndex], { Function = FunctionName })
    end
end

function OnClientConnect(aIndex)
    if BridgeFunctionTable[1] ~= nil then
        for i = 1, #BridgeFunctionTable[1] do
            _G[BridgeFunctionTable[1][i].Function](aIndex)
        end
    end
end

function OnPacketRecv(aIndex, hCode, sCode, data, dataLen)
    if BridgeFunctionTable[2] ~= nil then
        for i = 1, #BridgeFunctionTable[2] do
            _G[BridgeFunctionTable[2][i].Function](aIndex, hCode, sCode, data, dataLen)
        end
    end
end

function OnClientDisconnect(cIndex)
    if BridgeFunctionTable[3] ~= nil then
        for i = 1, #BridgeFunctionTable[3] do
            _G[BridgeFunctionTable[3][i].Function](cIndex)
        end
    end
end

function OnSQLAsyncResult(label, param, result)
    if BridgeFunctionTable[4] ~= nil then
        for i = 1, #BridgeFunctionTable[4] do
            _G[BridgeFunctionTable[4][i].Function](label, param, result)
        end
    end
end

function OnGlobalTick()
    if BridgeFunctionTable[5] ~= nil then
        for i = 1, #BridgeFunctionTable[5] do
            _G[BridgeFunctionTable[5][i].Function]()
        end
    end
end