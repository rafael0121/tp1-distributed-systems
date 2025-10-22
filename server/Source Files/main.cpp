#include <iostream>

#include <myproto/protocol.grpc.pb.h>
#include <myproto/protocol.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

int main() {
    std::cout << "Hello World, Server!";
}
