#!/bin/bash

# Compile Program.
cd ./build
cmake --build . --config Debug

# 1. Setup clients network address
CLIENT_1_ADDRESS=0.0.0.0:55001
CLIENT_2_ADDRESS=0.0.0.0:55002
CLIENT_3_ADDRESS=0.0.0.0:55003

# 2. Setup server network
SERVER_ADDRESS=0.0.0.0:55000

# 3. Setup 
EXEC="gnome-terminal -- bash -c"

# 4. Execute server
$EXEC "./server/server --port $SERVER_ADDRESS ; bash" &

# 5. Execute cliets
$EXEC "./client/client --id 1 --server $SERVER_ADDRESS --port $CLIENT_1_ADDRESS --client $CLIENT_2_ADDRESS --client $CLIENT_3_ADDRESS ; bash" &
$EXEC "./client/client --id 2 --server $SERVER_ADDRESS --port $CLIENT_2_ADDRESS --client $CLIENT_1_ADDRESS --client $CLIENT_3_ADDRESS ; bash" &
$EXEC "./client/client --id 3 --server $SERVER_ADDRESS --port $CLIENT_3_ADDRESS --client $CLIENT_2_ADDRESS --client $CLIENT_1_ADDRESS ; bash" &
