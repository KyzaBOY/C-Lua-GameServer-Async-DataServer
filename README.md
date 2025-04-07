# 🧠 C++ Lua GameServer + Async DataServer (GS/DS System)

Welcome! This is a custom **GameServer + DataServer** architecture entirely written in **C++**, with all the logic controlled via **Lua** and full asynchronous SQL communication using **UDP**.

I built this alone — it might not be perfect, but I gave it my best. I hope it helps others, and feel free to improve it or use it in your own projects!

---

## ⚙️ Architecture Overview

This system is split into two main components:

### 🎮 GameServer (GS)

Responsible for:
- Handling player connections
- Processing packets
- Movement, combat, logic, validations
- Calling Lua events and controlling all gameplay logic

### 🧠 DataServer (DS)

Responsible for:
- Receiving SQL queries via UDP
- Processing them **asynchronously**
- Returning results to GameServer
- Controlled entirely via Lua

> **Why not use threads inside GS?**  
> Because I believe in **modularity**. For me, running SQL logic in a separate executable (`DataServer`) and communicating via UDP is cleaner, safer, and more scalable.

---

## 🚀 How to use

1. Compile both **GameServer** and **DataServer** using Visual Studio.
2. Place your `main.lua` and `SystemCore.lua` (or your main Lua logic files) next to each executable. -> They have their own SystemCore/main inside lua folder DS and GS!
3. Launch the GameServer and DataServer — they will initialize their own threads and Lua states.

The servers communicate through UDP, and Lua handles the entire logic.

## 🧩 GameServer Lua Bridge Functions

These functions are automatically called by the engine:

OnClientConnect(socketID)
OnPacketRecv(socketID, headCode, subCode, data, dataLen)
OnClientDisconnect(socketID)
OnGlobalTick() -- Called every second
OnSQLAsyncResult(label, param, result)

## 🧩 GameServer Interface Functions (usable from Lua)

DisconnectClient(socketID)        -- Forces disconnection
BufferCreate()                    -- Creates a new buffer
BufferGetSize(bufferID)          -- Returns buffer size
BufferFromData(dataString)       -- Creates buffer from packet data (use in OnPacketRecv)
BufferDelete(bufferID)           -- Deletes the buffer

-- Reading from buffer
BufferReadByte(bufferID)
BufferReadWord(bufferID)
BufferReadDword(bufferID)
BufferReadString(bufferID)

-- Writing to buffer
BufferWriteByte(bufferID, value)
BufferWriteWord(bufferID, value)
BufferWriteDword(bufferID, value)
BufferWriteString(bufferID, "text")

-- Sending
SendBuffer(socketID, bufferID)    -- Sends buffer to the client

-- Utilities
LogText("Debug message")          -- Print to server console
DSQuery(query, label, param)     -- Sends SQL query to DataServer

## 💡 Use /reload in the console to reload Lua scripts without restarting the server.

## 🧩 DataServer Lua Bridge

OnDsQuery(size, query, label, param)

## 🧩 DataServer Interface Functions (usable from Lua)

-- SQL Management
SQLConnect("odbc", "user", "pass")
SQLDisconnect()

-- Query Execution
SQLQuery("SELECT * FROM ...")       -- Returns true if success
SQLFetch()                          -- Call this to fetch each row
SQLClose()                          -- Ends result set
SQLFree()                           -- Frees current query and processes next in queue

-- Data Retrieval (inside Fetch)
SQLGetNumber(columnIndex)
SQLGetString(columnIndex)
SQLGetFloat(columnIndex)

-- Result control
SQLGetResult()        -- true if last query succeeded
SQLIsConnected()      -- true if SQL is connected

-- Communication
SendPacket(label, param, resultString)   -- Send async result back to GameServer

-- Debug
LogText("Hello world")

## ✍️ Notes

label and param are always max 10 characters and padded with spaces ("label ").

You must trim them in Lua using string.gsub(label, "%s+$", "") or similar before using.

This system is designed to be modular, scalable and readable.

## 📦 Why use this system?

## 🧠 Clean separation of game logic and database

## 📜 Full Lua control over gameplay

## 🚀 Real asynchronous SQL communication via UDP

## 🛠️ Easily extendable, open-source, and readable

## 🔁 Hot-reload of Lua scripts in real time

## 📌 License:

This project is licensed under the MIT License.
Use it, modify it, and share it freely. Attribution is appreciated!

## ❤️ Final thoughts
This is a personal project that evolved into something modular and solid.
If it helps you, feel free to star, fork, or contribute.
Feedback is welcome — and if you use it in your game, let me know!
