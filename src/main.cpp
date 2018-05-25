#include <mixnet.h>
#include <mixsig.h>

int main(int argc, char **argv)
{
  argv[0] = "mixnet";
  Mixnet mix;
  if (argc < 2) mix.mn_error("too few arguments");

  if (signal(SIGCHLD,handle_sigchild) == SIG_ERR) mix.mn_error("failed to regester handler for SIGCHLD");

  //hanlde options
  mix.verbose = 0b1;
  mix.mixer_port = 5000;
  mix.peeler_port = 5001;
  mix.tracker_port = 5002;
  mix.localhost = "127.0.0.1";
  mix.tracker_addr = mix.localhost;
  mix.NHOSTS=0;
  for (int i = 2; i < argc; i++)
  {
    if (strncmp(argv[i],"-m",2) == 0)
    {
      if (strlen(argv[i]) > 2) mix.mixer_port = atoi(argv[i]+2);
      else mix.mixer_port = atoi(argv[i+1]);
    } else if (strncmp(argv[i],"-p",2) == 0)
    {
      if (strlen(argv[i]) > 2) mix.peeler_port = atoi(argv[i]+2);
      else mix.peeler_port = atoi(argv[i+1]);
    } else if (strncmp(argv[i],"-q",2) == 0)
    {
      mix.verbose = 0b0;
    }
  }
  struct sockaddr_in *taddrin;
  taddrin = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
  taddrin->sin_family = AF_INET;
  taddrin->sin_port = htons(mix.tracker_port);
  inet_pton(AF_INET,mix.tracker_addr,&(taddrin->sin_addr));
  memcpy(&tracker_sa,(struct sockaddr*)taddrin,sizeof(struct sockaddr));

  if (strcmp(argv[1],"start")==0) mix.start();
  else if (strcmp(argv[1],"stop")==0) mix.stop();
  else if (strcmp(argv[1],"setup")==0)
  {
    if (argc < 3) mix.mn_error("too few arguments");
    mix.setup(argv[2]);
  } else mix.mn_error("bad input arguments");

  return 0;
}
