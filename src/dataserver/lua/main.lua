-- Conecta com o banco de dados (ajuste para seu ODBC)
SQLConnect("Eidon", "user", "pass")

if SQLIsConnected() then
    LogText("SQL Connection Established") 
else
    LogText("SQL Connection Failed") 
end
-- your requires!

require("ScriptCore")
