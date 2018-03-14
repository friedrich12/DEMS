package user

import (
	"bytes"
	"crypto/rsa"
	"encoding/gob"
	"fmt"
	"hash"
	"log"

	"gopkg.in/mgo.v2/bson"
)

// PrepareResponse ... for sending
type PrepareResponse struct {
	response  string
	myaddr    string
	pubKey    *rsa.PublicKey
	hash      hash.Hash
	pubMixKey *rsa.PublicKey
	mixhash   hash.Hash
}

// All of this is not needed for now
func encodeUser(u user) bytes.Buffer {
	var junk bytes.Buffer        // Stand-in for a network connection
	enc := gob.NewEncoder(&junk) // Will write to junk.
	//dec := gob.NewDecoder(&junk) // Will read from junk.
	// Encode (send) the value.
	err := enc.Encode(u)
	if err != nil {
		log.Fatal("encode error:", err)
	}

	return junk
}

func decodeUser(b bytes.Buffer) user {
	var q user
	dec := gob.NewDecoder(&b)
	err := dec.Decode(&q)
	if err != nil {
		log.Fatal("decode error:", err)
	}
	return q
}

// PrepareForRequest ... Used to marshall data
func PrepareForRequest(r PrepareResponse) []byte {
	data, err := bson.Marshal(r)
	if err != nil {
		fmt.Println("Failed to send request: ", err)
	}
	return data
}

// ProcessResponse .. Used to process data
func ProcessResponse(data []byte) PrepareResponse {
	var q PrepareResponse
	err := bson.Unmarshal(data, q)
	if err != nil {
		fmt.Println("Failed to process response: ", err)
	}
	return q
}
