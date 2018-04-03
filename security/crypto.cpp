#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include "include/generate.h"

using namespace std;

const int kBits = 1024;
const int kExp = 3;
const int padding = RSA_PKCS1_PADDING;


// TODO: Fix public private key pairing
const char *
generate_private_public_rsa_key(int public){
    int keylen;
    char *pem_key;

    RSA *rsa = RSA_generate_key_ex(kBits, kExp, 0, 0);
    BIO *bio = BIO_new(BIO_s_mem());

    if(public){
         PEM_write_bio_RSAPublicKey(bio,rsa, NULL, NULL, 0, NULL, NULL);
    }else{
        PEM_write_bio_RSAPrivateKey(bio,rsa, NULL, NULL, 0, NULL, NULL);
    }
    keylen = BIO_pending(bio);
    pem_key = calloc(keylen+1, 1);
    BIO_read(bio, pem_key, keylen);

    BIO_free_all(bio);
    RSA_free(rsa);

    printf("%s", pem_key);

    return pem_key;
}
 
RSA * createRSA(unsigned char * key,int public)
{
    RSA *rsa= NULL;
    BIO *keybio;
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
 
int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = createRSA(key,1);
    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}
int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSA(key,0);
    int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}


byte[] prepare_message_for_sending(){
    
}