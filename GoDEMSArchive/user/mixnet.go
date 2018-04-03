package user

import (
	"fmt"
	"log"
	"net"
	"os"
	"strings"
	"time"

	mx "github.com/friedrich12/DEMS/mixnet"
	s "github.com/friedrich12/DEMS/secure"
	"github.com/hashicorp/consul/api"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
)

// Remote Procedure Call
func (n *user) RequestMix(ctx context.Context, in *mx.Request) (*mx.Response, error) {
	/*
		The request message containing the user's name.
				message Request {
					string command = 1;
					string fromAddr = 2;
				  	string fromUser = 3;
				}
				message Response {
					string response = 1;
					bytes  publicKey = 2;
					bytes  hash = 3;
					bytes  mixpublic = 4;
					bytes  mixhash = 5;
			    }
	*/
	r := PrepareResponse{response: "Accepted", myaddr: "", pubKey: &n.privateKey.PublicKey, hash: n.hash,
		pubMixKey: &n.mixprivateKey.PublicKey, mixhash: n.mixhash}
	ar := PrepareForRequest(r)
	switch in.Command {
	case "mix":
		fmt.Println("Accepted")
		return &mx.Response{Data: ar}, nil
	case "help":
		fmt.Println("Nothing concrete yet")
	case "other":
		fmt.Println("Nothing concrete yet")
	}
	return nil, s.NewError("Failed - Request Denied")
}

// Start listening/service.
func (n *user) StartListening() {

	lis, err := net.Listen("tcp", n.addr)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	_n := grpc.NewServer() // n is for serving purpose

	mx.RegisterMixnetServer(_n, n)
	// Register reflection service on gRPC server.
	reflection.Register(_n)

	// start listening
	if err := _n.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}

// Register self with the service discovery module.
// This implementation simply uses the key-value store.
// One major drawback is that when users crash. nothing is updated on the key-value store.
// Services are a better fit and should be used eventually.
func (n *user) registerService() {
	config := api.DefaultConfig()
	config.Address = n.SDAddress
	consul, err := api.NewClient(config)
	if err != nil {
		log.Panicln("Unable to contact Service Discovery.")
	}

	kv := consul.KV()
	p := &api.KVPair{Key: n.username, Value: []byte(n.addr)}
	_, err = kv.Put(p, nil)
	if err != nil {
		log.Panicln("Unable to register with Service Discovery.")
	}

	// store the kv for future use
	n.SDKV = *kv

	log.Println("Successfully registered with Consul.")
}

// Start the user.
// This starts listening at the configured address. It also sets up clients for it's peers.
func (n *user) Start() {
	// init required variables
	n.Mixers = make(map[string]mx.MixnetClient)

	// start service / listening
	go n.StartListening()

	// register with the service discovery unit
	n.registerService()

	// start the main loop here
	// in our case, simply time out for 1 minute and greet all

	// wait for other users to come up
	for {
		time.Sleep(20 * time.Second)
		n.GreetAll()
	}
}

// Setup a new grpc client for contacting the server at addr.
func (n *user) SetupClient(name string, addr string) {

	// setup connection with other user
	conn, err := grpc.Dial(addr, grpc.WithInsecure())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()
	n.Mixers[name] = mx.NewMixnetClient(conn)

	r, err := n.Mixers[name].RequestMix(context.Background(), &mx.Request{Name: n.Name})
	if err != nil {
		log.Fatalf("could not greet: %v", err)
	}
	log.Printf("Greeting from the other user: %s", r.Mixhash)

}

// Busy Work module, greet every new member you find
func (n *user) GreetAll() {
	// get all users -- inefficient, but this is just an example
	kvpairs, _, err := n.SDKV.List("Node", nil)
	if err != nil {
		log.Panicln(err)
		return
	}

	// fmt.Println("Found users: ")
	for _, kventry := range kvpairs {
		if strings.Compare(kventry.Key, n.username) == 0 {
			// ourself
			continue
		}
		if n.Mixers[kventry.Key] == nil {
			fmt.Println("New member: ", kventry.Key)
			// connection not established previously
			n.SetupClient(kventry.Key, string(kventry.Value))
		}
	}
}

func main() {
	// pass the port as an argument and also the port of the other user
	args := os.Args[1:]

	if len(args) < 3 {
		fmt.Println("Arguments required: <name> <listening address> <consul address>")
		os.Exit(1)
	}

	// args in order
	name := args[0]
	listenaddr := args[1]
	sdaddress := args[2]

	usern := NewUser("username", listenaddr)
	// usern is for opeartional purposes

	// start the user
	usern.Start()
}
