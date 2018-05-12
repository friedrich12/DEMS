#include "include/server.h"

MixnetServer::MixnetServer(std::string StunAddress, std::string StunPort) {
    GThread * thread;

    
    if((StunAddress == NULL) || (StunPort == NULL)){
        this->stun_addr = "stun.l.google.com";
        this->stun_port = 19302;
        std::cout << "Using defualt stun server stun.l.google.com:19302" <<  std::endl;
    }

    // Initilize network platforming libraries
    g_networking_init();

    // Main Event Loop of Glib Application
    gloop = g_main_loop_new(NULL, FALSE);

    // Run the mainloop and the example thread
    exit_thread = FALSE;
    std::thread t1(this->run());

    //thread = g_thread_new("thread", &run, NULL);
    g_main_loop_run (gloop);
    exit_thread = TRUE;

    t1.join();

    // Decrease reference count of the loop
    // If this is zero free the loop and associated memeory
    g_main_loop_unref(gloop);
}

MixnetServer::~MixnetServer() {
    // Nothing Here Yet...
}


void MixnetServer::Run(){
    NiceAgent * agent;
    NiceCandidate * local, * remote;
    GIOChannel * io_stdin;
    guint stream_id;

    // Data
    char * line = NULL;
    int rval;

    // Create IO channel
    #ifdef G_OS_WIN32
        io_stdin = g_io_channel_win32_new_fd(_fileno(stdin));
    #else
        io_stdin = g_io_channel_unix_new(fileno(stdin));
    #endif
        g_io_channel_set_flags(io_stdin, G_IO_FLAG_NONBLOCK, NULL);

    // Create the nice agent
    agent = nice_agent_new(g_main_loop_get_context(gloop),
        NICE_COMPATIBILITY_RFC5245);
    if (agent == NULL) {
        g_error("Failed to create agent");
    }

    // Set the STUN settings and controlling mode
    if (stun_addr) {
        g_object_set(agent, "stun-server", stun_addr, NULL);
        g_object_set(agent, "stun-server-port", stun_port, NULL);
    }
    g_object_set(agent, "controlling-mode", controlling, NULL);

    // Connect to the signals
    g_signal_connect(agent, "candidate-gathering-done",
        G_CALLBACK(cb_candidate_gathering_done), NULL);
    g_signal_connect(agent, "new-selected-pair",
        G_CALLBACK(cb_new_selected_pair), NULL);
    g_signal_connect(agent, "component-state-changed",
        G_CALLBACK(cb_component_state_changed), NULL);

    // Create a new stream with one component
    stream_id = nice_agent_add_stream(agent, 1);
    if (stream_id == 0)
        g_error("Failed to add stream");

    // Attach to the component to receive the data
    // Without this call, candidates cannot be gathered
    nice_agent_attach_recv(agent, stream_id, 1,
        g_main_loop_get_context(gloop), cb_nice_recv, NULL);

    // Start gathering local candidates
    if (!nice_agent_gather_candidates(agent, stream_id))
        g_error("Failed to start candidate gathering");

    g_debug("waiting for candidate-gathering-done signal...");
    //INDET STARTS HERE
        // Condition Variable
    std::unique_lock<std::mutex> lk(this->gather_mutex);
    this->gather_cond.wait(lk, []{!this->exit_thread && !this->candidate_gathering_done;});
    lk.unlock();
    cv.notify_all();

    if (exit_thread)
        goto end;

       
        
    this->local_data = this->print_local_data(agent, stream_id, 1);

    // Fix This!
    while (!this->exit_thread) {
        rval = parse_remote_data(agent, stream_id, 1, line);
        if (rval == EXIT_SUCCESS) {
            std::free(line);
            break;
        } else {
            fprintf(stderr, "ERROR: failed to parse remote data\n");
        }
        std::free(line);
    }

    g_debug("waiting for state READY or FAILED signal...");

    std::unique_lock<std::mutex> lk(this->negotiate_mutex);
    this->negotiate_cond.wait(lk, []{!this->exit_thread && !this->negotiation_done;});
    lk.unlock();
    cv.notify_all();

    if (exit_thread)
        goto end;

    // Get current selected candidate pair and print IP address used
    if (nice_agent_get_selected_pair(agent, stream_id, 1, & local, & remote)) {
        gchar ipaddr[INET6_ADDRSTRLEN];

        nice_address_to_string( & local - > addr, ipaddr);
        printf("\nNegotiation complete: ([%s]:%d,",
             ipaddr, nice_address_get_port( & local - > addr));
        nice_address_to_string( & remote - > addr, ipaddr);
        printf(" [%s]:%d)\n", ipaddr, nice_address_get_port( & remote - > addr));
    }

        // Listen to stdin and send data written to it
    printf("\nSend data:\n");
    printf("> ");
    fflush(stdout);
    while (!exit_thread) {
        GIOStatus s = g_io_channel_read_line(io_stdin, & line, NULL, NULL, NULL);
        if (s == G_IO_STATUS_NORMAL) {
            nice_agent_send(agent, stream_id, 1, strlen(line), line);
            std::free(line);
            printf("> ");
            fflush(stdout);
        } else if (s == G_IO_STATUS_AGAIN) {
            g_usleep(100000);
        } else {
            // Ctrl-D was pressed.
            nice_agent_send(agent, stream_id, 1, 1, "\0");
            break;
        }
    }

    end:
        g_io_channel_unref(io_stdin);
        g_object_unref(agent);
        g_main_loop_quit(gloop);
}

std::string MixnetServer::print_local_data(NiceAgent *agent, guint stream_id,
    guint component_id) noexcept{
    std::string result = NULL;
    std::stringstream buffer;
    char * local_ufrag = NULL;
    char * local_password = NULL;
    char ipaddr[INET6_ADDRSTRLEN];
    GSList * cands = NULL, * item;

    if (!nice_agent_get_local_credentials(agent, stream_id, & local_ufrag, & local_password))
        goto end;

    cands = nice_agent_get_local_candidates(agent, stream_id, component_id);
    if (cands == NULL)
        goto end;

    buffer << local_ufrag << local_password;

    for (item = cands; item; item = item - > next) {
        NiceCandidate * c = (NiceCandidate * ) item - > data;

        nice_address_to_string &c-> addr, ipaddr);

        // (foundation),(prio),(addr),(port),(type)
        
        buffer << c->foundation;
        buffer << c->priority;
        buffer << ipaddr;
        buffer << nice_address_get_port( & c - > addr);
        buffer << candidate_type_name[c - > type];
    }
    buffer << endl;
    result = EXIT_SUCCESS;

    end:
        if (local_ufrag)
            std::free(local_ufrag);
    if (local_password)
        std::free(local_password);
    if (cands)
        g_slist_free_full(cands, (GDestroyNotify) & nice_candidate_free);

    return buffer.str();
}