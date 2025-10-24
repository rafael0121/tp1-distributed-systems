#include <myproto/protocol.pb.h>
#include <myproto/protocol.grpc.pb.h>

#include <lamport.hpp>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>

#include <iostream>
#include <string>
#include <vector>
#include <list>

#define TOTAL_CLIENTS 2

// Client states
#define WHATEVER 1
#define HOLDING  2
#define WAITING  3

// Port server.
std::string server_address;

// Port of clientes that also consume the printer.
std::list<std::string> neighbors_address;

// List of request awaiting.
int neighbors_waiting = 0;
std::vector<int> neighbors_requests = {0, 0, 0};

// Count release replyed.
int neighbors_released = 0;

/* This client status */
int my_state = WHATEVER;       // Current state.
Lamport my_timestamp;          // Current lamport timestamp.
int32_t my_request_number = 0; // Request number.
int32_t my_id = -1;            // Client identifier.


class MutualExclusionService final : public distributed_printing::MutualExclusionService::Service {
    public:
        grpc::Status RequestAccess(grpc::ServerContext *context, const distributed_printing::AccessRequest *request, distributed_printing::AccessResponse *response) {
            bool access = false;

            int32_t client_id = request->client_id();
            int64_t client_timestamp = request->lamport_timestamp();
            int32_t client_request_number = request->request_number();

            switch(my_state){
                case WHATEVER:
                    access = true;
                break;
                case WAITING:
                    access = my_timestamp.curTimeStamp() < client_timestamp ? true : false;
                    if(access == false ){
                        neighbors_waiting++;
                        neighbors_requests[client_id] = neighbors_waiting;
                    }
                    break;
                case HOLDING:
                    access =  false;
                break;
            }

            response->set_access_granted(access);
            response->set_lamport_timestamp(my_timestamp.updateTimeStamp(client_timestamp));

            return grpc::Status::OK;
        }

        grpc::Status ReleaseAccess (grpc::ServerContext *context, const distributed_printing::AccessRelease *request) {
            int32_t client_id = request->client_id();
            int64_t client_timestamp = request->lamport_timestamp();
            int32_t client_request_number = request->request_number();
 
            switch(my_state) {
                case WHATEVER:
                case HOLDING:
                    break;
                case WAITING:
                    neighbors_released++;
            }

            my_timestamp.updateTimeStamp(client_timestamp);

            return grpc::Status::OK;
        }
};

int start_client_server (){
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50052", grpc::InsecureServerCredentials());

    MutualExclusionService client_service;
    builder.RegisterService(&client_service);

    std::unique_ptr<grpc::Server> client_server(builder.BuildAndStart());

    return true;
}

int request_access(){
    std::list<std::string> address = neighbors_address;

    while(!address.empty()){
        std::string _address = address.back();

        // Setup request
        distributed_printing::AccessRequest request;
        distributed_printing::AccessResponse response;

        request.set_client_id(my_id);
        request.set_lamport_timestamp(my_timestamp.curTimeStamp());
        request.set_request_number(my_request_number++);

        // Call
        auto channel = grpc::CreateChannel(_address, grpc::InsecureChannelCredentials());
        std::unique_ptr<distributed_printing::MutualExclusionService::Stub> stub = distributed_printing::MutualExclusionService::NewStub(channel);
        grpc::ClientContext context;
        grpc::Status status = stub->RequestAccess(&context, request, &response);

        // Update my timestamp.
        my_timestamp.updateTimeStamp(response.lamport_timestamp());

        if(response.access_granted()) {
            neighbors_released++;
        }

        address.pop_back();
    }

    return true;
}

int wannna_print() {
    request_access();

    // Setup request
    distributed_printing::PrintRequest request;
    distributed_printing::PrintResponse response;

    request.set_client_id(0);
    request.set_message_content("Hello World!");
    request.set_lamport_timestamp(my_timestamp.curTimeStamp());
    request.set_request_number(my_request_number);

    // Call
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    std::unique_ptr<distributed_printing::PrintingService::Stub> stub = distributed_printing::PrintingService::NewStub(channel);
    grpc::ClientContext context;
    grpc::Status status = stub->SendToPrinter(&context, request, &response);

    std::cout << "Printer answer:" << response.confirmation_message() << std::endl;

    my_timestamp.updateTimeStamp(response.lamport_timestamp());

    return true;
}

int parse_args(int argc, char *argv[]){
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--id") {
            if(i + 1 < argc){
                my_id = std::atoi(argv[++i]);
            }
        } else 
        if (std::string(argv[i]) == "--client") {
            if(i + 1 < argc){
                neighbors_address.push_back(argv[++i]);
            }
        } else
        if (std::string(argv[i]) == "--server") {
            if(i + 1 < argc){
                server_address = argv[++i];
            }
        }
    }

    if(server_address.empty() || my_id < 0){
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    std::cout << "===# Client Started #===" << std::endl;

    int ret = -1;
    ret = parse_args(argc, argv);
    if(ret < 0){
        return -1;
    }
}