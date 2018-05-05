#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>

#ifndef CRYPTO_H
#define CRYPTO_H

// Generate Public Key
EVP_PKEY* create_rsa_key(void);
static void gen_callback(int iWhat, int inPrime, void* pParam);
static void init_openssl(void);
static void cleanup_openssl(void);
static int passwd_callback(char *pcszBuff,int size,int rwflag, void *pPass);
static EVP_PKEY* create_rsa_key(void);
static void handle_openssl_error(void);



#endif