#ifndef CRYPTO_H
#define CRYPTO_H

#include <functional>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <thread>
#include <string>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include <ctype.h>
#include <stdlib.h>
#include <agent.h>
#include <gio/gnetworking.h>

const char *candidate_type_name[] = {"host", "srflx", "prflx", "relay"};

const char *state_name[] = {"disconnected", "gathering", "connecting","connected", "ready", "failed"};

// Generate Public Key
class MixnetServer{
    public:
        MixnetServer();
        ~MixnetServer();
        std::string get_print_local_data(NiceAgent *agent, guint stream_id,guint component_id) noexcept;
    private:
        void Run();
        int parse_remote_data(NiceAgent *agent, guint stream_id, guint component_id, char *line);
        static NiceCandidate *MixnetServer::parse_candidate(char *scand, guint stream_id);
        void cb_new_selected_pair(NiceAgent *agent, guint stream_id,guint component_id,
         gchar *lfoundation, gchar *rfoundation, gpointer data);
        void cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id,guint len, gchar *buf, gpointer data);
        void cb_component_state_changed(NiceAgent *agent, guint stream_id, guint component_id, guint state, gpointer data);
        void cb_candidate_gathering_done(NiceAgent *agent, guint stream_id, gpointer data);
        std::mutex gather_mutex, negotiate_mutex;
        std::condition_variable gather_cond, negotiate_cond;
        bool exit_thread;
        bool candidate_gathering_done;
        bool negotiation_done;
        GMainLoop *gloop;
        // STUN SERVER
        char *stun_addr = NULL;
        uint stun_port;
        char* local_data;
        bool controlling;
};

#endif