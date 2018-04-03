package user

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1"
	"fmt"
	"hash"
	"os"

	mx "github.com/friedrich12/DEMS/mixnet"

	"github.com/hashicorp/consul/api"
	"gopkg.in/mgo.v2/bson"
)

var (
	size = 1024
)

type user struct {
	username      string
	uid           string
	addr          string
	privateKey    *rsa.PrivateKey
	hash          hash.Hash
	mixprivateKey *rsa.PrivateKey
	mixhash       hash.Hash

	SDAddress string
	SDKV      api.KV

	Mixers map[string]mx.MixnetClient
}

type message struct {
	text     string
	fromAddr string
	toAddr   string
}

//NewUser ... Creates a new user
func NewUser(username string, addr string) user {
	h := sha1.New()
	m1 := sha1.New()
	mixpriv, err := rsa.GenerateKey(rand.Reader, size)
	priv, err := rsa.GenerateKey(rand.Reader, size)
	if err != nil {
		fmt.Println("Failed to gnerate keys")
	}

	return user{username: username, addr: addr, privateKey: priv, hash: h,
		mixprivateKey: mixpriv, mixhash: h, SDAddress: "159.89.114.9", Mixers: nil}
}

func (u user) EncryptM(mess message) []byte {
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

func (u user) DecryptM(mess []byte) message {
	dec, err := rsa.DecryptOAEP(u.hash, rand.Reader, u.privateKey, mess, nil)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	var m message
	bson.Unmarshal(mess, m)
	return m
}

func (u user) GetPublicKey() *rsa.PublicKey {
	return &u.privateKey.PublicKey
}

func (u user) GetMixPublicKey() *rsa.PublicKey {
	return &u.mixprivateKey.PublicKey
}

func (u user) GetAddr() string {
	return u.addr
}
