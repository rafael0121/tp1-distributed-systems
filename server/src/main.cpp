#include <iostream>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include <myproto/protocol.grpc.pb.h>
#include <myproto/protocol.pb.h>

#include <lamport.hpp>

// Lamport CLock
Lamport server_lamportClock;

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

int main() {
    std::cout << "===# Server Started #===" << std::endl;

    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

    PrintingService server_service;
    builder.RegisterService(&server_service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    server->Wait();
}
