#include <bits/stdc++.h>

#include "include/crypto.h"
#include "include/user.h"

using namespace std;

// Generates keys
User::User(){
    public_key = generate_rsa_key();
    private_key = generate_rsa_key();
}

User::~User(){
    free(public_key);
    free(private_key);
}

char* User::GetPublicKey(){
    return public_key;
}

char* User::GetPrivateKey(){
    return private_key;
}