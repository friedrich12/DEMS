package main

import (
	"time"
	"encoding/gob"
	"crypto/sha256"
	"bytes"
	"log"
)

type Block struct {
		Timestamp	 	int64   		// Current Timestamp
		Transactions	[]*Transaction	// Information
		PrevBlockHash 	[]byte			// Hash of the previous block
		Hash 		 	[]byte	
		Nonce			int				// Required to verfity the proof
}

/*func (b *Block) SetHash(){
	timestamp := []byte(strconv.FormatInt(b.Timestamp,10))
	headers := bytes.Join([][]byte{b.PrevBlockHash, b.Data, timestamp}, []byte{})
	hash := sha256.Sum256(headers)

	b.Hash = hash[:]
}*/


// Serialization Encode The Block
// Needed because BlotDB only used byte
// arrays
func (b *Block) Serialize() []byte {
	var result bytes.Buffer
	encoder := gob.NewEncoder(&result)

	err := encoder.Encode(b)
	if err != nil {log.Panic(err)}

	return result.Bytes()
}

// Decode the Block
func DeserializeBlock(d []byte) *Block {
	var block Block

	decoder := gob.NewDecoder(bytes.NewReader(d))
	
	err := decoder.Decode(&block)
	if err != nil {log.Panic(err)}

	return &block
}
// Create and mine a new block
func NewBlock(trans []*Transaction, prevBlockHash []byte) *Block {
	block := &Block{time.Now().Unix(), trans,
	prevBlockHash, []byte{}, 0}

	pow := NewProofOfWork(block)
	nonce, hash := pow.Run()

	block.Hash = hash[:]
	block.Nonce = nonce

	return block
}

// Return a hash of the transaction in the block
// Transactions in a block will be uniquely identified by 
// a single hash
func (b *Block) HashTransactions() []byte {
	var txHashes [][]byte
	var txHash [32]byte

	for _, tx := range b.Transactions {
		txHashes = append(txHashes, tx.ID)
	}
	txHash = sha256.Sum256(bytes.Join(txHashes,[]byte{}))

	return txHash[:]
}

func NewGenesisBlock(coinbase *Transaction) *Block {
	return NewBlock([]*Transaction{coinbase}, []byte{})
}