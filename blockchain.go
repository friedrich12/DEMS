package main

import (
	"encoding/hex"
	"github.com/boltdb/bolt"
	"crypto"
	"log"
)

// Interactions with DB
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
const genesisCoinbaseData = "DEMS is awesome"

func (bc *Blockchain)MineBlock(trans []*Transaction) {
	var lasthash []byte

	err := bc.db.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))
		lasthash = b.Get([]byte("1"))

		return nil
	})
	if err != nil {log.Panic(err)}

	newBlock := NewBlock(trans, lasthash)

	err = bc.db.Update(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))
		err := b.Put(newBlock.Hash, newBlock.Serialize())
		if err != nil {log.Panic(err)}
		err = b.Put([]byte("1"),newBlock.Hash)
		bc.tip = newBlock.Hash
		return nil
	})
}

// Just an iterator for the blockchain
func (bc *Blockchain) Iterator() *BlockchainIterator {
	bci := &BlockchainIterator{bc.tip, bc.db}

	return bci
}

// The next block in the chain
func (i *BlockchainIterator) Next() *Block {
	var block *Block
	
	err := i.db.View(func(tx *bolt.Tx) error {
		b := tx.Bucket([]byte(blocksBucket))
		encBlock := b.Get(i.currenthash)
		block = DeserializeBlock(encBlock)

		return nil
	})
	if err != nil {log.Panic(err)}
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
			if err != nil {log.Panic(err)}
			err = b.Put(genesis.Hash, genesis.Serialize())
			err = b.Put([]byte("1"), genesis.Hash)
			tip = genesis.Hash
		} else {
			tip = b.Get([]byte("1"))
		}
		return nil
	})

	if err != nil {
		log.Panic(err)
	}

	bc := Blockchain{tip, db}
	return &bc
}

func (bc *Blockchain) FindUnspentTransactions(address string) [] Transaction{
	var unspentTXs []Transaction
	spentTXOs := make(map[string][]int)
	bci := bc.Iterator()

	for {
		block := bci.Next()

		for _, tx := range block.Transactions {
			txID := hex.EncodeToString(tx.ID)

		Outputs:
				for outIdx, out := range tx.Vout {
					// Let's check if the output was spent
					if spentTXOs[txID] != nil {
						for _, spentOut := range spentTXOs[txID] {
							if spentOut == outIdx{
								continue Outputs
							}
						}
					}
					
					// If it can be unlocked with this address
					// it means that this address is the one 
					// that locked it
					if out.CanBeUnlockedWith(address) {
						unspentTXs = append(unspentTXs, *tx)
					}
				}

				
			}
		
	}
}