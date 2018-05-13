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


void 
MixnetServer::Run(){
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
        rval = this->parse_remote_data(agent, stream_id, 1, line);
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

std::string 
MixnetServer::get_print_local_data(NiceAgent *agent, guint stream_id,
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

static NiceCandidate *
MixnetServer::parse_candidate(char *scand, guint stream_id)
{
  
  NiceCandidate *cand = NULL;
  NiceCandidateType ntype;
  gchar **tokens = NULL;
  guint i;

  tokens = g_strsplit (scand, ",", 5);
  for (i = 0; tokens[i]; i++);
  if (i != 5)
    goto end;

  for (i = 0; i < G_N_ELEMENTS (candidate_type_name); i++) {
    if (strcmp(tokens[4], candidate_type_name[i]) == 0) {
      ntype = (NiceCandidateType) i;
      break;
    }
  }
  if (i == G_N_ELEMENTS (candidate_type_name))
    goto end;

  cand = nice_candidate_new(ntype);
  cand->component_id = 1;
  cand->stream_id = stream_id;
  cand->transport = NICE_CANDIDATE_TRANSPORT_UDP;
  strncpy(cand->foundation, tokens[0], NICE_CANDIDATE_MAX_FOUNDATION);
  cand->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
  cand->priority = atoi (tokens[1]);

  if (!nice_address_set_from_string(&cand->addr, tokens[2])) {
    g_message("failed to parse addr: %s", tokens[2]);
    nice_candidate_free(cand);
    cand = NULL;
    goto end;
  }

  nice_address_set_port(&cand->addr, atoi (tokens[3]));

 end:
  g_strfreev(tokens);
  return cand;
}

int
MixnetServer::parse_remote_data(NiceAgent *agent, guint stream_id, guint component_id, char *line)
{
  GSList *remote_candidates = NULL;
  gchar **line_argv = NULL;
  const gchar *ufrag = NULL;
  const gchar *passwd = NULL;
  int result = EXIT_FAILURE;
  int i;

  line_argv = g_strsplit_set (line, " \t\n", 0);
  for (i = 0; line_argv && line_argv[i]; i++) {
    if (strlen (line_argv[i]) == 0)
      continue;

    // first two args are remote ufrag and password
    if (!ufrag) {
      ufrag = line_argv[i];
    } else if (!passwd) {
      passwd = line_argv[i];
    } else {
      // Remaining args are serialized canidates (at least one is required)
      NiceCandidate *c = this->parse_candidate(line_argv[i], stream_id);

      if (c == NULL) {
        g_message("failed to parse candidate: %s", line_argv[i]);
        goto end;
      }
      remote_candidates = g_slist_prepend(remote_candidates, c);
    }
  }
  if (ufrag == NULL || passwd == NULL || remote_candidates == NULL) {
    g_message("line must have at least ufrag, password, and one candidate");
    goto end;
  }

  if (!nice_agent_set_remote_credentials(agent, stream_id, ufrag, passwd)) {
    g_message("failed to set remote credentials");
    goto end;
  }

  // Note: this will trigger the start of negotiation.
  if (nice_agent_set_remote_candidates(agent, stream_id, component_id,
      remote_candidates) < 1) {
    g_message("failed to set remote candidates");
    goto end;
  }

  result = EXIT_SUCCESS;

 end:
  if (line_argv != NULL)
    g_strfreev(line_argv);
  if (remote_candidates != NULL)
    g_slist_free_full(remote_candidates, (GDestroyNotify)&nice_candidate_free);

  return result;
}
void
MixnetServer::cb_candidate_gathering_done(NiceAgent *agent, guint stream_id,
    gpointer data)
{
  g_debug("SIGNAL candidate gathering done\n");

  g_mutex_lock(&gather_mutex);
  candidate_gathering_done = TRUE;
  g_cond_signal(&gather_cond);
  g_mutex_unlock(&gather_mutex);
}

void
MixnetServer::cb_component_state_changed(NiceAgent *agent, guint stream_id,
    guint component_id, guint state,
    gpointer data)
{
  g_debug("SIGNAL: state changed %d %d %s[%d]\n",
      stream_id, component_id, state_name[state], state);

  if (state == NICE_COMPONENT_STATE_READY) {
    g_mutex_lock(&negotiate_mutex);
    negotiation_done = TRUE;
    g_cond_signal(&negotiate_cond);
    g_mutex_unlock(&negotiate_mutex);
  } else if (state == NICE_COMPONENT_STATE_FAILED) {
    g_main_loop_quit (gloop);
  }
}


void
MixnetServer::cb_new_selected_pair(NiceAgent *agent, guint stream_id,
    guint component_id, gchar *lfoundation,
    gchar *rfoundation, gpointer data)
{
  g_debug("SIGNAL: selected pair %s %s", lfoundation, rfoundation);
}

void
MixnetServer::cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id,
    guint len, gchar *buf, gpointer data)
{
  if (len == 1 && buf[0] == '\0')
    g_main_loop_quit (gloop);

  printf("%.*s", len, buf);
  fflush(stdout);
}