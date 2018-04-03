package network

import (
	"fmt"
	"net"
	"strconv"
)

type Client struct {
	serverAddr string
	conn       net.PacketConn
	info       string
}

func NewClient() *Client {
	var c Client
	return &c
}

func NewConnectionWithClient(conn net.PacketConn) *Client {
	var c Client
	c.conn = conn
	return &c
}

func (c *Client) SetHostServer(host string, port int) {
	c.serverAddr = new.JoinHostPort(host, strconv.Itoa(port))
}

// TODO: Check this function
func (c *Client) SetServerAddr(address string) {
	c.serverAddr = address
}

// Contact server get the NAT type and UDP hole punching
func (c *Client) Discover() (NATType, *Host, error) {
	serverUDPAddr, err = net.ResolveUDPAddr("udp", c.serverAddr)
	if err != nil {
		return NATError, nil, err
	}
	conn := c.conn
	if conn == nil {
		conn, err = net.ListenUDP("udp", nil)
		if err != nil {
			return NATError, nil, err
		}
		defer conn.Close()
	}
	return c.discover(conn, serverAddr)
}

func start() {
	fmt.Println("start")
}
