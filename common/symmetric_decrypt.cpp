#include <mixnet.h>

void Mixnet::symmetric_decrypt(char **plaintext, char *ciphertext, BF_KEY *key, int *plen, int clen)
{
    printf("symmetric_decrypt %i bytes\n",clen);
    fflush(stdout);
    *plaintext = (char*)malloc(clen);
    for (int i=0; i < clen; i+=8)
    {
      BF_ecb_encrypt(reinterpret_cast<const unsigned char *>(ciphertext+i),reinterpret_cast<unsigned char *>(plaintext+i), key, BF_DECRYPT);
    }
    *plen = clen;
    printf("symmetrically decrypted %i bytes ciphertext: \" ",clen);
    fwrite(ciphertext,1,clen,stdout);
    printf("\"\n to %i bytes plaintext \"",*plen);
    fwrite(*plaintext,1,*plen,stdout);
    printf("\"\n");
    fflush(stdout);
}
