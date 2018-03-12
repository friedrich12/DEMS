package user

import (
	"bytes"
	"encoding/gob"
	"log"
)

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
