#include <mixnet.h>

void Mixnet::symmetric_encrypt(char *plaintext, char **ciphertext, BF_KEY *key, char **k, int plen, int *clen)
{
  //  printf("symmetric_encrypting %i bytes\n",plen);

    int len = 128;
    *k = (char*)malloc(len);
    RAND_bytes(reinterpret_cast<unsigned char *>(k),len);
    BF_set_key(key,len,reinterpret_cast<const unsigned char *>(k));

    int r = plen % 8;
    *clen = (r==0) ? plen : plen + (8-r);
    plaintext = (char*)realloc(plaintext, *clen);
    *ciphertext = (char*)malloc(*clen);
    memset(*ciphertext,0,*clen);
    for (int i = 0; i < *clen; i+=8)
    {
      BF_ecb_encrypt(reinterpret_cast<const unsigned char *>(plaintext+i), reinterpret_cast<unsigned char *>(ciphertext+i), key, BF_ENCRYPT);
    }
    printf("symmetrically encrypted %i bytes plaintext: \" ",plen);
    fwrite(plaintext,1,plen,stdout);
    printf("\"\n to %i bytes ciphertext \"",*clen);
    fwrite(*ciphertext,1,*clen,stdout);
    printf("\"\n");
    fflush(stdout);
}
