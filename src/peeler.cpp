#include <mixnet.h>

void Mixnet::peeler(int fd, struct sockaddr sa)
{
  printf("Peeler running on port %i\n",peeler_port); fflush(stdout);
  while(1)
  {
    struct sockaddr_in from;
    socklen_t from_len = sizeof(struct sockaddr_in);
    int sz = 4096;
    char *buf = (char*)malloc(sz);
    int bytes = recvfrom(peeler_fd,buf,sz,MSG_WAITALL,(struct sockaddr*)&from,&from_len);
    printf("got %i bytes\n",bytes);
    if (bytes < 0)
    {
      close(fd);
      mn_error("peeler: error receiving bytes");
    }
    peel(buf,bytes,fd,from);
  }
}
