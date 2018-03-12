package user

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1"
	"fmt"
	"hash"
	"os"

	"gopkg.in/mgo.v2/bson"
)

var (
	size = 1024
)

type user struct {
	username   string
	addr       string
	privateKey *rsa.PrivateKey
	hash       hash.Hash
}

type message struct {
	text     string
	fromAddr string
	toAddr   string
}

//NewUser ... Creates a new user
func NewUser(username string, addr string) user {
	h := sha1.New()
	priv, err := rsa.GenerateKey(rand.Reader, size)
	if err != nil {
		fmt.Println("Failed to gnerate keys")
	}
	return user{username, addr, priv, h}
}

func (u user) Encrypt(mess message) []byte {
	var pub *rsa.PublicKey
	pub = &u.privateKey.PublicKey

	data, err := bson.Marshal(mess)
	// I might user labels later
	enc, err := rsa.EncryptOAEP(u.hash, rand.Reader, pub, data, nil)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	return enc
}

func (u user) Decrypt(mess []byte) []byte {
	dec, err := rsa.DecryptOAEP(u.hash, rand.Reader, u.privateKey, mess, nil)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	return dec
}

func (u user) GetPublicKey() *rsa.PublicKey {
	return &u.privateKey.PublicKey
}

func (u user) GetAddr() string {
	return u.addr
}

func (u user) NewMessage() {

}
