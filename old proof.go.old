package main

import (
	"bytes"
	"crypto/sha256"
	"fmt"
	"math"
	"encoding/binary"
	"log"
	"math/big"
)

// 256 - 25 = OKAY
// The bigger I make the difference
// The harder it is to compute the hash

const targetBits = 24
const maxNonce = math.MaxInt64

func IntToHex(num int64) []byte {
	buff := new(bytes.Buffer)
	err := binary.Write(buff, binary.BigEndian, num)
	if err != nil {
		log.Panic(err)
	}

	return buff.Bytes()
}

type ProofOfWork struct {
	block *Block
	target *big.Int
}

// Must be valid with the target
func NewProofOfWork(b *Block) *ProofOfWork {
	target := big.NewInt(1)
	// Make the shift << 
	target.Lsh(target, uint(256 - targetBits))

	pow := &ProofOfWork{b, target}

	return pow
}

func (pow *ProofOfWork) prepareData(nonce int) []byte{
	// nonce is the counter
	data := bytes.Join(
		[][]byte{
			pow.block.PrevBlockHash,
			pow.block.HashTransactions(),
			IntToHex(pow.block.Timestamp),
			IntToHex(int64(targetBits)),
			IntToHex(int64(nonce)),
		},
		[]byte{},
	)

	return data
}

// PoW Algorithm

func (pow *ProofOfWork) Run() (int, []byte){
	var hashInt big.Int
	var hash [32]byte
	nonce := 0

	fmt.Printf("Mining a new block")
	// Brute-force until we find a valid hash
	for nonce < maxNonce {
		data := pow.prepareData(nonce)
		hash = sha256.Sum256(data)
		fmt.Printf("\r%x", hash)
		hashInt.SetBytes(hash[:])		//bytes of a big-endian unsigned integer(Bit Int)

		// Returns -1 if x < y
		if hashInt.Cmp(pow.target) == -1 {
			break
		} else {
			nonce++
		}
	}
	fmt.Print("\n\n")
	return nonce, hash[:]
}

func (pow *ProofOfWork) Validate() bool {
	var hashInt big.Int

	data := pow.prepareData(pow.block.Nonce)
	hash := sha256.Sum256(data)
	hashInt.SetBytes(hash[:])

	isValid := hashInt.Cmp(pow.target) == -1

	return isValid
}