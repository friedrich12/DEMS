package main

import (
	"github.com/boltdb/bolt"
	"fmt"
)


type Blockchain struct {
	tip []byte
	db *bolt.DB
}

type BlockchainIterator struct {
	currenthash []byte
	db		*bolt.DB
}


const dbFile = "demschain.db"
const blocksBucket = "blocks"

func (bc *Blockchain) AddBlock(data string) {
	var lasthash []byte

	err := bc.db.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))
		lasthash = b.Get([]byte("1"))

		return nil
	})

	newBlock := NewBlock(data, lasthash)

	err = bc.db.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))
		err := b.Put(newBlock.Hash, newBlock.Serialize())
		err = b.Put([]byte("1"),newBlock.Hash)
		bc.tip = newBlock.Hash
		return nil
	})
}

func NewGenesisBlock() *Block {
	return NewBlock("Genesis", []byte{})
}

func (bc *Blockchain) Iterator() *BlockchainIterator {
	bci := &BlockchainIterator{bc.tip, bc.db}

	return bci
}

func (i *BlockchainIterator) Next() *Block {
	var block *Block
	
	err := i.db.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))
		encBlock := b.Get(i.currenthash)
		block = DeserializeBlock(encBlock)

		return nil
	})

	i.currenthash = block.PrevBlockHash

	return block
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
