#pragma once
#include <mixnet.h>

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

void handle_tracker_sigkill(int sig)
{
  close(tracker_fd);
  exit(0);
}

