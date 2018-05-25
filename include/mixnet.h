#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <pthread.h>
#include <openssl/blowfish.h>
#include <time.h>
#include <fcntl.h>

struct host
{
  char *ip_addr;
  unsigned short port;
  RSA *key;
};

struct mix_t
{
  int fd;
  struct sockaddr sa;
  char *msg;
  int len;
};

struct mixchain
{
  unsigned char flag;
  char signed_key[16], dest[32];
  int len;
  char body;
};

struct peer
{
  char flag;
  struct sockaddr_in addr;
  char *public_key_str;
  RSA *key;
};


typedef struct mixchain mixchain;
typedef struct mix_t mix_t;


class Mixnet {
    public:
        unsigned char verbose;
        char * MIXNET_PATH, * localhost, * tracker_addr;
        unsigned short mixer_port, peeler_port, tracker_port;

        int NHOSTS;

        //struct peer;

        Mixnet();
        ~Mixnet();

        void peer_add(struct sockaddr_in from, char * pubkey, int len);
        void peer_remove(struct sockaddr_in from);
        struct peer * peer_get(int idx);
        struct peer * peer_find(struct sockaddr_in addr);
        int peer_count();

        //bool memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);

        void mn_error(char * err);

        int create_socket(int domain, int type, char * ip_addr, int port, struct sockaddr * saddr);

        void setup(char * name);
        void start();
        void stop();
        void mix(int fd, struct sockaddr_in sa, char *msg, int len);

        void peel(char *msg, int len, int fd, struct sockaddr_in from);
        void peeler(int fd, struct sockaddr sa);

        void track(char *msg, int len, struct sockaddr_in from);
        void tracker(int fd, struct sockaddr sa);

        void startTracker();
        void stopTracker();

        FILE * mixer_out, * mixer_err;
        void mixer(int fd, struct sockaddr sa);

        void pack(void * * bytes, int * bytes_len, struct peer last_hop, struct peer next_hop, char * msg, int msg_len);
        void unpack(void * * bytes, int * bytes_len, struct peer * * next_hop, char * msg, int msg_len);

        FILE * peeler_out, * peeler_err;

        RSA * generate_rsa_key();
        RSA * RSAKEY;

        void public_encrypt(char * plaintext, char * * ciphertext, RSA * key, int plen, int * clen);
        void private_decrypt(char * * plaintext, char * ciphertext, RSA * key, int * plen, int clen);

        void private_encrypt(char * plaintext, char * * ciphertext, RSA * key, int plen, int * clen);
        void public_decrypt(char * * plaintext, char * ciphertext, RSA * key, int * plen, int clen);

        void symmetric_encrypt(char * plaintext, char * * ciphertext, BF_KEY * key, char * * k, int plen, int * clen);
        void symmetric_decrypt(char * * plaintext, char * ciphertext, BF_KEY * key, int * plen, int clen);

        int mixer_fd, peeler_fd, tracker_fd;
        struct sockaddr tracker_sa;
};

