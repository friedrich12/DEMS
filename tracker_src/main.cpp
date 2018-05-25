#include <mixnet.h>

int main(int argc, char const *argv[]) {
  argv[0] = "mixnet_tracker";
  Mixnet mix;
  if (argc < 2) mix.mn_error("too few arguments");

  unsigned char verbose = 0b1;
  mix.tracker_port = 5002;
  mix.localhost = "127.0.0.1";
  mix.tracker_addr = mix.localhost;


  for (int i = 2; i < argc; i++)
  {
    if (strncmp(argv[i],"-q",2) == 0)
    {
      mix.verbose = 0b0;
    }
  }

  if (strcmp(argv[1],"start")==0) mix.startTracker();
  else if (strcmp(argv[1],"stop")==0) mix.stopTracker();
  else mix.mn_error("bad input arguments");

  return 0;
}
