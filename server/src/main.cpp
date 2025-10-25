#include <iostream>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include <myproto/protocol.grpc.pb.h>
#include <myproto/protocol.pb.h>

#include <lamport.hpp>
#include <string>

// Lamport CLock
Lamport server_lamportClock;

// Server address
std::string server_address;

/* Printing Service: SendToPrinter*/
class PrintingService final : public distributed_printing::PrintingService::Service{
    public:
        grpc::Status SendToPrinter(grpc::ServerContext *context, const distributed_printing::PrintRequest *request, distributed_printing::PrintResponse *response){

            int32_t client_id = request->client_id();
            int64_t lamport_timestamp = request->lamport_timestamp();
            std::string message = request->message_content();

            // Write message to stdout.
            std::cout << "[TS: " << lamport_timestamp << "] " << "CLIENT " << client_id << ": " << message << std::endl;

            // Simulate a printing time.
            sleep(3);

            response->set_success(true);
            response->set_confirmation_message("Ok, this was printed!");
            response->set_lamport_timestamp(server_lamportClock.updateTimeStamp(lamport_timestamp));

            return grpc::Status::OK;
        }

};
int parse_args(int argc, char *argv[]){
    if (argc != 3) {
        return -1;
    }

    if (std::string(argv[1]) == "--port") {
        server_address = argv[2];
    } 

    if(server_address.empty()){
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    std::cout << "===# Server Started #===" << std::endl;

    /* Parse CLI Arguments*/
    int ret = -1;
    ret = parse_args(argc, argv);
    if(ret < 0){
        return -1;
    }

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    PrintingService server_service;
    builder.RegisterService(&server_service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    server->Wait();
}