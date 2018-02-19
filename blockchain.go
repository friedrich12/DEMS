package main

import (
	"github.com/boltdb/bolt"
	"fmt"
)


type Blockchain struct {
	tip []byte
	db *bolt.DB
}


const dbFile = "demschain.db"
const blocksBucket = "blocks"

func (bc *Blockchain) AddBlock(data string) {
	prevBlock := bc.blocks[len(bc.blocks)-1]
	newBlock := NewBlock(data, prevBlock.Hash)
	bc.blocks = append(bc.blocks, newBlock)
}

func NewGenesisBlock() *Block {
	return NewBlock("Genesis", []byte{})
}

func NewBlockchain() *Blockchain {
	var tip []byte
	db, err := bolt.Open(dbFile, 0600, nil)

	err = db.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))

		if b == nil {
			genesis := NewGenesisBlock()
			b, err := tx.CreateBucket([]byte(blocksBucket))
			err = b.Put(genesis.Hash, genesis.Serialize())
			err = b.Put([]byte("1"), genesis.Hash)
			tip = genesis.Hash
		} else {
			tip = b.Get([]byte("1"))
		}
		return nil
	})
	bc := Blockchain{tip, db}
	return &bc
}
