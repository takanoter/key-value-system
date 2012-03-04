#!/bin/sh
./client_get 10000 127.0.0.1 5555 ~/index_100G.1 &
./client_get 10000 127.0.0.1 5555 ~/index_100G.2 &
./client_get 10000 127.0.0.1 5555 ~/index_100G.3 &
./client_get 10000 127.0.0.1 5555 ~/index_100G.4 &
./client_get 10000 127.0.0.1 5555 ~/index_100G.5 &


