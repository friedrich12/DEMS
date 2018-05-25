#include <mixnet.h>

void Mixnet::mn_error(char *err)
{
  printf("#########################\n####  DEMS error:  ####\n#########################\n%s\n#########################\n",err);
  fflush(stdout);
  fflush(stderr);
  exit(1);
}
