#include <functional>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <string>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include <ctype.h>
#include <stdlib.h>

#ifndef CRYPTO_H
#define CRYPTO_H

const char *candidate_type_name[] = {"host", "srflx", "prflx", "relay"};

const char *state_name[] = {"disconnected", "gathering", "connecting","connected", "ready", "failed"};

// Generate Public Key
class MixnetServer{
    public:
        MixnetServer();
        ~MixnetServer();
        std::string get_print_local_data(NiceAgent *agent, guint stream_id,
    guint component_id) noexcept;
    private:
        void Run();
        std::mutex gather_mutex, negotiate_mutex;
        std::condition_variable gather_cond, negotiate_cond;
        bool exit_thread;
        bool candidate_gathering_done;
        GMainLoop *gloop;
        char *stun_addr = NULL;
        uint stun_port;
        bool controlling;
};

#endif