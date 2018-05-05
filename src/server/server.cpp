#include "include/server.h"

MixnetServer::MixnetServer(int argc, char * argv[]) {
    GThread * thread;
    // Parse arguments
    if (argc > 4 || argc < 2 || argv[1][1] != NULL) {
        fprintf(stderr, "Usage: %s 0|1 stun_addr [stun_port]\n", argv[0]);
        printf("To use defualt stun_addr replace with default");
        return EXIT_FAILURE;
    }

    // Parse 0|1
    controlling = argv[1][0] - '0';
    if (controlling != 0 && controlling != 1) {
        fprintf(stderr, "Usage: %s 0|1 stun_addr [stun_port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        stun_addr = argv[2];
        if (stun_addr == "default") {
            stun_addr = "stun.l.google.com";
        }
        if (argc > 3)
            stun_port = atoi(argv[3]);
        else
            stun_port = 19302;

        g_debug("Using stun server '[%s]:%u'\n", stun_addr, stun_port);
    }

    // Initilize network platforming libraries
    g_networking_init();

    // Main Event Loop of Glib Application
    gloop = g_main_loop_new(NULL, FALSE);

    // Run the mainloop and the example thread
    exit_thread = FALSE;
    std::thread t1(run);

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
    gchar * line = NULL;
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

        // Condition Variable
        std::unique_lock<std::mutex> lk(this->gather_mutex)
        while (!this->exit_thread && !this->candidate_gathering_done)
            this->gather_cond.wait(lk, []{return ready;});
        lk.unlock();
        if (exit_thread)
            goto end;

        // Candidate gathering is done.
        
        this->local_data = this->print_local_data(agent, stream_id, 1);

        // Listen on stdin for the remote candidate list
        printf("Enter remote data (single line, no wrapping):\n");
        printf("> ");
        fflush(stdout);
        while (!exit_thread) {
            GIOStatus s = g_io_channel_read_line(io_stdin, & line, NULL, NULL, NULL);
            if (s == G_IO_STATUS_NORMAL) {
                // Parse remote candidate list and set it on the agent
                rval = parse_remote_data(agent, stream_id, 1, line);
                if (rval == EXIT_SUCCESS) {
                    g_free(line);
                    break;
                } else {
                    fprintf(stderr, "ERROR: failed to parse remote data\n");
                    printf("Enter remote data (single line, no wrapping):\n");
                    printf("> ");
                    fflush(stdout);
                }
                g_free(line);
            } else if (s == G_IO_STATUS_AGAIN) {
                g_usleep(100000);
            }
        }

        g_debug("waiting for state READY or FAILED signal...");
        g_mutex_lock( & negotiate_mutex);
        while (!exit_thread && !negotiation_done)
            g_cond_wait( & negotiate_cond, & negotiate_mutex);
        g_mutex_unlock( & negotiate_mutex);
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
        printf("\nSend lines to remote (Ctrl-D to quit):\n");
        printf("> ");
        fflush(stdout);
        while (!exit_thread) {
            GIOStatus s = g_io_channel_read_line(io_stdin, & line, NULL, NULL, NULL);
            if (s == G_IO_STATUS_NORMAL) {
                nice_agent_send(agent, stream_id, 1, strlen(line), line);
                g_free(line);
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

        return NULL;
}

std::string MixnetServer::print_local_data(NiceAgent *agent, guint stream_id,
    guint component_id) noexcept{
    std::string result = NULL;
    std::stringstream buffer;
    gchar * local_ufrag = NULL;
    gchar * local_password = NULL;
    gchar ipaddr[INET6_ADDRSTRLEN];
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
            g_free(local_ufrag);
    if (local_password)
        g_free(local_password);
    if (cands)
        g_slist_free_full(cands, (GDestroyNotify) & nice_candidate_free);

    return buffer.str();
}