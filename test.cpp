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

const char* pcszPassphrase = "open sezamee";



int main(int argc, char **argv)
{
    int iRet = EXIT_SUCCESS;
    EVP_PKEY* pPrivKey = NULL;
    EVP_PKEY* pPubKey  = NULL;
    FILE*     pFile    = NULL;
    const EVP_CIPHER* pCipher = NULL;
    init_openssl();

    pPrivKey = create_rsa_key();
    pPubKey  = create_rsa_key();

    // Both Passed Creation
    if(pPrivKey && pPubKey)
    {
        // Save the Keys
        // Advanced Encryption Standard 256 bit key size
        if((pFile = fopen("privkey.pem","wt")) && (pCipher = EVP_aes_256_cbc()))
        {

            if(!PEM_write_PrivateKey(pFile,pPrivKey,pCipher,
                                    (unsigned char*)pcszPassphrase,
                                    (int)strlen(pcszPassphrase),NULL,NULL))
            {
                fprintf(stderr,"PEM_write_PrivateKey failed.\n");
                handle_openssl_error();
                iRet = EXIT_FAILURE;
            }
            fclose(pFile);
            pFile = NULL;
            if(iRet == EXIT_SUCCESS)
            {
                if((pFile = fopen("pubkey.pem","wt")) && PEM_write_PUBKEY(pFile,pPubKey))
                    fprintf(stderr,"Both keys saved.\n");
                else
                {
                    handle_openssl_error();
                    iRet = EXIT_FAILURE;
                }
                if(pFile)
                {
                    fclose(pFile);
                    pFile = NULL;
                }
            }
        }
        else
        {
            fprintf(stderr,"Cannot create \"privkey.pem\".\n");
            handle_openssl_error();
            iRet = EXIT_FAILURE;
            if(pFile)
            {
                fclose(pFile);
                pFile = NULL;
            }
        }
        if(iRet == EXIT_SUCCESS)
        {
            /* Read the keys */
            // Decrease the reference counter of the keys
            EVP_PKEY_free(pPrivKey);
            pPrivKey = NULL;
            EVP_PKEY_free(pPubKey);
            pPubKey = NULL;

            if((pFile = fopen("privkey.pem","rt")) && 
               (pPrivKey = PEM_read_PrivateKey(pFile,NULL,passwd_callback,(void*)pcszPassphrase)))
            {
                fprintf(stderr,"Private key read.\n");
            }
            else
            {
                fprintf(stderr,"Cannot read \"privkey.pem\".\n");
                handle_openssl_error();
                iRet = EXIT_FAILURE;
            }
            if(pFile)
            {
                fclose(pFile);
                pFile = NULL;
            }

            if((pFile = fopen("pubkey.pem","rt")) && 
               (pPubKey = PEM_read_PUBKEY(pFile,NULL,NULL,NULL)))
            {
                fprintf(stderr,"Public key read.\n");
            }
            else
            {
                fprintf(stderr,"Cannot read \"pubkey.pem\".\n");
                handle_openssl_error();
                iRet = EXIT_FAILURE;
            }
        }
    }

    if(pPrivKey)
    {
        // Decrease Reference Counter
        EVP_PKEY_free(pPrivKey);
        pPrivKey = NULL;
    }
    if(pPubKey)
    {
        EVP_PKEY_free(pPubKey);
        pPubKey = NULL;
    }
    cleanup_openssl();
    return iRet;
}

