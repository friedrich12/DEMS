#include "include/crypto.h"

using namespace std;

const int kBits = 2048;
const int kExp = 3;
const int padding = RSA_PKCS1_PADDING;
//unsigned char encrypted[4098]={};
//unsigned char decrypted[4098]={};

RSA * getKeyFromText(unsigned char * key,int public)
{
    RSA *rsa= NULL;
    BIO *keybio ;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL)
    {
        printf( "Failed to create key BIO");
        return 0;
    }
    if(public)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }
    if(rsa == NULL)
    {
        printf( "Failed to create RSA");
    }
 
    return rsa;
}

EVP_PKEY* create_rsa_key(void)
{
    RSA *pRSA      = NULL;
    EVP_PKEY* pKey = NULL;
    pRSA = RSA_generate_key(kBits,RSA_3,gen_callback,NULL);
    pKey = EVP_PKEY_new();
    if(pRSA && pKey && EVP_PKEY_assign_RSA(pKey,pRSA))
    {
        /* pKey owns pRSA from now */
        if(RSA_check_key(pRSA) <= 0)
        {
            fprintf(stderr,"RSA_check_key failed.\n");
            handle_openssl_error();
            EVP_PKEY_free(pKey);
            pKey = NULL;
        }
    }
    else
    {
        handle_openssl_error();
        if(pRSA)
        {
            RSA_free(pRSA);
            pRSA = NULL;
        }
        if(pKey)
        {
            EVP_PKEY_free(pKey);
            pKey = NULL;
        }
    }
    return pKey;
}

void gen_callback(int iWhat, int inPrime, void* pParam)
{
    char c='*';
    switch(iWhat)
    {
        case 0: c = '.';  break;
        case 1: c = '+';  break;
        case 2: c = '*';  break;
        case 3: c = '\n'; break;
    }
    fprintf(stderr,"%c",c);
}

int passwd_callback(char *pcszBuff,int size,int rwflag, void *pPass)
{
    size_t unPass = strlen((char*)pPass);
    if(unPass > (size_t)size)
        unPass = (size_t)size;
    memcpy(pcszBuff, pPass, unPass);
    return (int)unPass;
}

void init_openssl(void)
{
    #if OPENSSL_VERSION_NUMBER < 0x10100000L
        int r = SSL_library_init();
    #else
        int r = OPENSSL_init_ssl(0, NULL);
    #endif

    if(r)
    {
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        RAND_load_file("/dev/urandom", 1024);
    }
    else
        exit(EXIT_FAILURE);
}

void cleanup_openssl(void)
{
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    ERR_remove_thread_state(0);
    EVP_cleanup();
}

void handle_openssl_error(void)
{
    ERR_print_errors_fp(stderr);
}

int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = getKeyFromText(key,1);
    int result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}
int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = getKeyFromText(key,0);
    int  result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}