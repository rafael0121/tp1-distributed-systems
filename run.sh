#!/bin/bash

cd ./build
cmake --build . --config Debug

CLIENT_1_ADDRESS=localhost:55001
CLIENT_2_ADDRESS=localhost:55002
CLIENT_3_ADDRESS=localhost:55003
SERVER_ADDRESS=localhost:55000

footclient -- bash -c "./server/server --port $SERVER_ADDRESS ; bash" &

# Wait server start
sleep 1 

footclient -- bash -c "./client/client --id 1 --server $SERVER_ADDRESS --port $CLIENT_1_ADDRESS --client $CLIENT_2_ADDRESS --client $CLIENT_3_ADDRESS ; bash" &
footclient -- bash -c "./client/client --id 2 --server $SERVER_ADDRESS --port $CLIENT_2_ADDRESS --client $CLIENT_1_ADDRESS --client $CLIENT_2_ADDRESS ; bash" &
footclient -- bash -c "gdb --args ./client/client --id 3 --server $SERVER_ADDRESS --port $CLIENT_3_ADDRESS --client $CLIENT_2_ADDRESS --client $CLIENT_3_ADDRESS ; bash" &
