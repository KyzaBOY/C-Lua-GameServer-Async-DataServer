#include "mysql.h"
#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <sqlext.h>
#include <iostream>

namespace SQLSystem {
    static SQLHENV hEnv = nullptr;
    static SQLHDBC hDbc = nullptr;
    static SQLHSTMT hStmt = nullptr;
    static bool connected = false;

    bool Connect(const std::string& odbc, const std::string& user, const std::string& pass) {
        if (connected) return true;

        SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
        SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
        SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

        std::string connStr = "DSN=" + odbc + ";UID=" + user + ";PWD=" + pass + ";";
        SQLCHAR outstr[1024];
        SQLSMALLINT outstrlen;

        SQLRETURN ret = SQLDriverConnect(hDbc, NULL,
            (SQLCHAR*)connStr.c_str(), SQL_NTS,
            outstr, sizeof(outstr), &outstrlen,
            SQL_DRIVER_COMPLETE);

        connected = (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);

        if (connected) {
            std::cout << "[SQL] ✅ Conectado com sucesso." << std::endl;
        }
        else {
            std::cout << "[SQL] ❌ Falha na conexão SQL." << std::endl;
        }

        return connected;
    }

    void Disconnect() {
        if (hStmt) { SQLFreeHandle(SQL_HANDLE_STMT, hStmt); hStmt = nullptr; }
        if (hDbc) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); hDbc = nullptr; }
        if (hEnv) { SQLFreeHandle(SQL_HANDLE_ENV, hEnv); hEnv = nullptr; }
        connected = false;
        std::cout << "[SQL] 🔌 Desconectado do banco de dados." << std::endl;
    }

    bool Query(const std::string& sql) {
        if (!connected) {
            std::cout << "[SQL] ❌ Query cancelada: conexão não estabelecida." << std::endl;
            return false;
        }

        if (hStmt) {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            std::cout << "[SQL] 🧹 Handle anterior liberado." << std::endl;
        }

        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        std::cout << "[SQL] 📦 Executando query: " << sql << std::endl;

        const auto result = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
        if (result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO) {
            std::cout << "[SQL] ✅ Query executada com sucesso." << std::endl;
            return true;
        }
        else {
            std::cout << "[SQL] ❌ Erro ao executar a query." << std::endl;
            return false;
        }
    }

    bool Fetch() {
        if (!hStmt) return false;

        SQLRETURN result = SQLFetch(hStmt);
        if (result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO) {
            std::cout << "[SQL] ✅ Linha disponível no resultado." << std::endl;
            return true;
        }
        else {
            std::cout << "[SQL] ⚠️ Nenhuma linha no resultado." << std::endl;
            return false;
        }
    }

    void Close() {
        if (hStmt) {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            hStmt = nullptr;
            std::cout << "[SQL] 🧹 Consulta encerrada." << std::endl;
        }
    }

    int GetNumber(int col) {
        SQLINTEGER val;
        SQLLEN indicator;
        SQLGetData(hStmt, col, SQL_C_SLONG, &val, 0, &indicator);
        return val;
    }

    std::string GetString(int col) {
        char buffer[1024] = { 0 };
        SQLLEN indicator;
        SQLGetData(hStmt, col, SQL_C_CHAR, buffer, sizeof(buffer), &indicator);
        return std::string(buffer);
    }

    float GetFloat(int col) {
        float val;
        SQLLEN indicator;
        SQLGetData(hStmt, col, SQL_C_FLOAT, &val, 0, &indicator);
        return val;
    }

    bool GetResult() {
        return hStmt != nullptr;
    }

    bool IsConnected() {
        return connected;
    }
}
