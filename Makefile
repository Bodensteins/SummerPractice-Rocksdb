S = src
B = build
T = target

CC = g++
FLAGS = -std=c++17 -fno-rtti
DYN_LIB = -lrocksdb -pthread


CLNT_SRC := \
	$S/client.cpp \
	$S/compaction_service_impl.cpp \
	$S/files.cpp \
	$S/socket_lib.cpp

$T/client: ${CLNT_SRC}
	${CC} ${FLAGS} -o $@ $^ ${DYN_LIB}


SRV_SRC := \
	$S/server.cpp \
	$S/remote_compaction_server.cpp \
	$S/socket_lib.cpp

$T/server: ${SRV_SRC}
	${CC} ${FLAGS} -o $@ $^ ${DYN_LIB}


PROXY_SRC := \
	$S/proxy_server.cpp \
	$S/socket_lib.cpp

$T/proxy: ${PROXY_SRC}
	${CC} ${FLAGS} -o $@ $^ -pthread


$T/generate: $S/generate.cpp
	${CC} ${FLAGS} -o $@ $^ ${DYN_LIB}


test: $S/test/test_client.cpp $S/test/test_proxy.cpp $S/test/test_server.cpp $S/socket_lib.cpp
	${CC} $S/test/test_server.cpp $S/socket_lib.cpp -o $T/test_server
	${CC} $S/test/test_proxy.cpp $S/socket_lib.cpp -o $T/test_proxy
	${CC} $S/test/test_client.cpp $S/socket_lib.cpp -o $T/test_client
	${CC} ${FLAGS} $S/test/test_db.cpp $S/socket_lib.cpp -o $T/test_db ${DYN_LIB}

generate: $T/generate

client: $T/client

server: $T/server

proxy: $T/proxy

all: client server proxy

clean:
	rm -rf $T/* $B/*

clean-db:
	rm -rf db/data1 db/data2

.PHONY: client server proxy generate test all clean clean-db