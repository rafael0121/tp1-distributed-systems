//===================================================================
// Libraries
//===================================================================

// System and Language
#include <iostream>
#include <string>

// GRPC
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

// Protocol
#include <myproto/protocol.grpc.pb.h>
#include <myproto/protocol.pb.h>

// Private
#include <lamport.hpp>

//===================================================================
// Macros
//===================================================================

#define PRINT_TIME 3

//===================================================================
// Global Variables
//===================================================================

// Lamport CLock
Lamport server_lamportClock;

// Server address
std::string server_address;

//===================================================================
// GRPC Service
//===================================================================

/**
 * GRPC service to print messagens in screen.
 */
class PrintingService final : public distributed_printing::PrintingService::Service{
    public:
        grpc::Status SendToPrinter(grpc::ServerContext *context, const distributed_printing::PrintRequest *request, distributed_printing::PrintResponse *response){

            int32_t client_id = request->client_id();
            int64_t lamport_timestamp = request->lamport_timestamp();
            std::string message = request->message_content();

            // Simulate a printing time.
            sleep(PRINT_TIME);

            // Write message to stdout.
            std::cout << "[TS: " << lamport_timestamp << "] " << "CLIENT " << client_id << ": " << message << std::endl;

            response->set_success(true);
            response->set_confirmation_message("Ok, this was printed!");
            response->set_lamport_timestamp(server_lamportClock.updateTimestamp(lamport_timestamp));

            return grpc::Status::OK;
        }

};

//===================================================================
// Private Functions
//===================================================================

/**
 * Parse server args.
 */
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

//===================================================================
// Server main.
//===================================================================

int main(int argc, char *argv[]) {
    std::cout << "===# Server Started #===" << std::endl;

    /* Parse CLI Arguments*/
    int ret = -1;
    ret = parse_args(argc, argv);
    if(ret < 0){
        return -1;
    }

    /* GRPC server setup */

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    PrintingService server_service;
    builder.RegisterService(&server_service);

    // Start server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    // Wait finish.
    server->Wait();

    return 0;
}