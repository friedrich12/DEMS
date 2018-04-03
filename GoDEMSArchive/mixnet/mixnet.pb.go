// Code generated by protoc-gen-go. DO NOT EDIT.
// source: mixnet.proto

/*
Package mixnet is a generated protocol buffer package.

It is generated from these files:
	mixnet.proto

It has these top-level messages:
	Request
	Response
*/
package mixnet

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

// The request message containing the user's name.
type Request struct {
	Command  string `protobuf:"bytes,1,opt,name=command" json:"command,omitempty"`
	FromAddr string `protobuf:"bytes,2,opt,name=fromAddr" json:"fromAddr,omitempty"`
	FromUser string `protobuf:"bytes,3,opt,name=fromUser" json:"fromUser,omitempty"`
}

func (m *Request) Reset()                    { *m = Request{} }
func (m *Request) String() string            { return proto.CompactTextString(m) }
func (*Request) ProtoMessage()               {}
func (*Request) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

func (m *Request) GetCommand() string {
	if m != nil {
		return m.Command
	}
	return ""
}

func (m *Request) GetFromAddr() string {
	if m != nil {
		return m.FromAddr
	}
	return ""
}

func (m *Request) GetFromUser() string {
	if m != nil {
		return m.FromUser
	}
	return ""
}

type Response struct {
	Data []byte `protobuf:"bytes,1,opt,name=data,proto3" json:"data,omitempty"`
}

func (m *Response) Reset()                    { *m = Response{} }
func (m *Response) String() string            { return proto.CompactTextString(m) }
func (*Response) ProtoMessage()               {}
func (*Response) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

func (m *Response) GetData() []byte {
	if m != nil {
		return m.Data
	}
	return nil
}

func init() {
	proto.RegisterType((*Request)(nil), "mixnet.Request")
	proto.RegisterType((*Response)(nil), "mixnet.Response")
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for Mixnet service

type MixnetClient interface {
	// Sends a greeting
	RequestMix(ctx context.Context, in *Request, opts ...grpc.CallOption) (*Response, error)
}

type mixnetClient struct {
	cc *grpc.ClientConn
}

func NewMixnetClient(cc *grpc.ClientConn) MixnetClient {
	return &mixnetClient{cc}
}

func (c *mixnetClient) RequestMix(ctx context.Context, in *Request, opts ...grpc.CallOption) (*Response, error) {
	out := new(Response)
	err := grpc.Invoke(ctx, "/mixnet.Mixnet/RequestMix", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for Mixnet service

type MixnetServer interface {
	// Sends a greeting
	RequestMix(context.Context, *Request) (*Response, error)
}

func RegisterMixnetServer(s *grpc.Server, srv MixnetServer) {
	s.RegisterService(&_Mixnet_serviceDesc, srv)
}

func _Mixnet_RequestMix_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(Request)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(MixnetServer).RequestMix(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/mixnet.Mixnet/RequestMix",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(MixnetServer).RequestMix(ctx, req.(*Request))
	}
	return interceptor(ctx, in, info, handler)
}

var _Mixnet_serviceDesc = grpc.ServiceDesc{
	ServiceName: "mixnet.Mixnet",
	HandlerType: (*MixnetServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "RequestMix",
			Handler:    _Mixnet_RequestMix_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "mixnet.proto",
}

func init() { proto.RegisterFile("mixnet.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 179 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0xe2, 0xc9, 0xcd, 0xac, 0xc8,
	0x4b, 0x2d, 0xd1, 0x2b, 0x28, 0xca, 0x2f, 0xc9, 0x17, 0x62, 0x83, 0xf0, 0x94, 0xa2, 0xb9, 0xd8,
	0x83, 0x52, 0x0b, 0x4b, 0x53, 0x8b, 0x4b, 0x84, 0x24, 0xb8, 0xd8, 0x93, 0xf3, 0x73, 0x73, 0x13,
	0xf3, 0x52, 0x24, 0x18, 0x15, 0x18, 0x35, 0x38, 0x83, 0x60, 0x5c, 0x21, 0x29, 0x2e, 0x8e, 0xb4,
	0xa2, 0xfc, 0x5c, 0xc7, 0x94, 0x94, 0x22, 0x09, 0x26, 0xb0, 0x14, 0x9c, 0x0f, 0x93, 0x0b, 0x2d,
	0x4e, 0x2d, 0x92, 0x60, 0x46, 0xc8, 0x81, 0xf8, 0x4a, 0x72, 0x5c, 0x1c, 0x41, 0xa9, 0xc5, 0x05,
	0xf9, 0x79, 0xc5, 0xa9, 0x42, 0x42, 0x5c, 0x2c, 0x29, 0x89, 0x25, 0x89, 0x60, 0xa3, 0x79, 0x82,
	0xc0, 0x6c, 0x23, 0x6b, 0x2e, 0x36, 0x5f, 0xb0, 0x33, 0x84, 0x0c, 0xb9, 0xb8, 0xa0, 0xce, 0xf0,
	0xcd, 0xac, 0x10, 0xe2, 0xd7, 0x83, 0xba, 0x15, 0x2a, 0x26, 0x25, 0x80, 0x10, 0x80, 0x18, 0xa7,
	0xc4, 0xe0, 0x24, 0xce, 0x05, 0xf5, 0x83, 0x13, 0x37, 0xc4, 0x90, 0x00, 0x90, 0xc7, 0x02, 0x18,
	0x93, 0xd8, 0xc0, 0x3e, 0x34, 0x06, 0x04, 0x00, 0x00, 0xff, 0xff, 0x82, 0x31, 0xfd, 0x9f, 0xf1,
	0x00, 0x00, 0x00,
}