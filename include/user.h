#ifndef USER_H
#define USER_H

class User{
    public:
        User();
        ~User();
        char* GetPublicKey();
        char* GetPrivateKey();
    private:
        char* public_key;
        char* private_key;
        char* local_data;
};

#endif