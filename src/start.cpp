#include <mixnet.h>
#include <mixsig.h>


int peeler_fd;
int mixer_fd;

void handle_mixer_sigkill(int sig)
{
  close(mixer_fd);
  exit(0);
}

void handle_peeler_sigkill(int sig)
{
  char *up = "peeler_down";
  sendto(peeler_fd,up,strlen(up),0,&tracker_sa,sizeof(tracker_sa));
  close(peeler_fd);
  exit(0);
}

void handle_sigchild(int sig)
{
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}


void Mixnet::start()
{
  RSAKEY = RSA_new();
  FILE *pubkey = fopen("/usr/etc/mixnet/pubkey.pem","r+");
  FILE *privkey = fopen("/usr/etc/mixnet/privkey.pem","r+");
  PEM_read_RSAPublicKey(pubkey, &RSAKEY, NULL,NULL);
  PEM_read_RSAPrivateKey(privkey, &RSAKEY, NULL,NULL);

  char *public_key = (char*)malloc(4096);
  fseek(pubkey,0,SEEK_END);
  int size = ftell(pubkey);
  rewind(pubkey);
  fread(public_key,1,size,pubkey);

  printf("RSA key check: %i\n",RSA_check_key(RSAKEY));
  fclose(pubkey);
  fclose(privkey);
  fflush(stdout);

  struct sockaddr mixer_addr,peeler_addr;

  mixer_fd = create_socket(AF_INET,SOCK_DGRAM,localhost,mixer_port, &mixer_addr);
  peeler_fd = create_socket(AF_INET,SOCK_DGRAM,localhost,peeler_port, &peeler_addr);

  //mixer(mixer_fd,mixer_addr);
  pid_t mixer_pid;
  mixer_pid=fork();
  if (mixer_pid==0)
  {
    int pid = (int)getpid();
    char path[128];
    sprintf(path,"/usr/etc/mixnet/mixer_%i_out",pid);
    if (!verbose) freopen(path,"w+",stdout);
    sprintf(path,"/usr/etc/mixnet/mixer_%i_err",pid);
    if (!verbose) freopen(path,"w+",stderr);
    if (signal(3,handle_mixer_sigkill) == SIG_ERR) mn_error("failed to regester handler for SIGKILL");
    time_t t = time(NULL);
    RAND_seed(&t,sizeof(time_t));
    RAND_poll();
    mixer(mixer_fd,mixer_addr);

  } else
  {
    FILE *pid_file = fopen("/usr/etc/mixnet/mixer_pid","w+");
    fprintf(pid_file,"%d",mixer_pid);
    fclose(pid_file);
    pid_t peeler_pid;
    peeler_pid = fork();
    if (peeler_pid == 0)
    {
      char path[128];
      int pid = (int)getpid();
      sprintf(path,"/usr/etc/mixnet/peeler_%i_out",pid);
      if (!verbose) freopen(path,"w+",stdout);
      sprintf(path,"/usr/etc/mixnet/peeler_%i_err",pid);
      if (!verbose) freopen(path,"w+",stderr);
      if (signal(3,handle_peeler_sigkill) == SIG_ERR) mn_error("failed to regester handler for SIGKILL");
      time_t t = time(NULL);
      RAND_seed(&t,sizeof(time_t));
      RAND_poll();
      peeler(peeler_fd,peeler_addr);
    } else
    {
      FILE *pid_file = fopen("/usr/etc/mixnet/peeler_pid","w+");
      fprintf(pid_file,"%d",peeler_pid);
      fclose(pid_file);

      char *up = (char*)malloc(size+strlen("peer_up"));
      memcpy(up,"peer_up",strlen("peer_up"));
      memcpy(up+strlen("peer_up"),public_key,size);
      sendto(peeler_fd,up,size+strlen("peer_up"),0,&tracker_sa,sizeof(tracker_sa));
      free(up);
      free(public_key);
    }
  }
}
