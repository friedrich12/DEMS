export LD_LIBRARY_PATH=/usr/local/lib
g++ -o threaded-example mixnet.cpp `pkg-config --cflags --libs nice`