package main

import (
	"time"
	"encoding/gob"
)

type Block struct {
		Timestamp	 	int64   // Current Timestamp
		Data	  		[]byte	// Information
		PrevBlockHash 	[]byte	// Hash of the previous block
		Hash 		 	[]byte	
		Nonce			int		// Required to verfity the proof
}

/*func (b *Block) SetHash(){
	timestamp := []byte(strconv.FormatInt(b.Timestamp,10))
	headers := bytes.Join([][]byte{b.PrevBlockHash, b.Data, timestamp}, []byte{})
	hash := sha256.Sum256(headers)

	b.Hash = hash[:]
}*/

func (b *Block) Serialize() []byte {
	var result bytes.Buffer
}

func NewBlock(data string, prevBlockHash []byte) *Block {
	block := &Block{time.Now().Unix(), []byte(data),
	prevBlockHash, []byte{}, 0}

	pow := NewProofOfWork(block)
	nonce, hash := pow.Run()

	block.Hash = hash[:]
	block.Nonce = nonce

	return block
}
