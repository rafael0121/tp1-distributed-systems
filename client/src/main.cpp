// GRPC Library
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>

// System Library.
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <thread>
#include <random>

// Private library
#include <lamport.hpp>


// Proto library
#include <myproto/protocol.pb.h>
#include <myproto/protocol.grpc.pb.h>

// Functions to print info
#define REQUEST_ACCESS 1
#define RELEASE_ACCESS  2

// Client states
#define RELEASED 1
#define HOLDING  2
#define WAITING  3

// Port server.
std::string server_address;

// Port of clientes that also consume the printer.
std::list<std::string> neighbors_address;

// List of request awaiting.
int neighbors_waiting = 0;

// Count release replyed.
int neighbors_released = 0;

/* This client status */
int my_state = RELEASED;       // Current state.
Lamport my_timestamp;          // Current lamport timestamp.
int32_t my_request_number = 0; // Request number.
int32_t my_id;            // Client identifier.
std::string my_address;

/* Mutex free wanna_printer */
std::mutex mt_wanna;

void print_curstate(){

    std::string state_str = my_state == RELEASED ? "RELEASED" : my_state == HOLDING ? "HOLDING" : "WAITING";

    std::cout << std::endl
              << "[State: " << state_str << " ]" 
              << std::endl
              << "[Timestamp: " << my_timestamp.curTimestamp() << " ]" 
              << std::endl
              << "[RequestNumber: " << my_request_number << " ]"
              << std::endl
              << "[Neighbors Released: " << neighbors_released << " ]"
              << std::endl
              << "[Neighbors Waiting: " << neighbors_waiting << " ]"
              << std::endl
              << std::endl;
}

class MutualExclusionService final : public distributed_printing::MutualExclusionService::Service {
    public:
        grpc::Status RequestAccess(grpc::ServerContext *context, const distributed_printing::AccessRequest *request, distributed_printing::AccessResponse *response) override {
            bool access = false;

            int32_t client_id = request->client_id();
            int64_t client_timestamp = request->lamport_timestamp();
            int32_t client_request_number = request->request_number();

            switch(my_state){
                case RELEASED:
                    access = true;
                break;
                case WAITING:
                    if(my_timestamp.curTimestamp() < client_timestamp){
                        access = false;
                    } else
                    if(my_timestamp.curTimestamp() > client_timestamp){
                        access = true;
                    } else
                    if(my_timestamp.curTimestamp() == client_timestamp){
                        if(my_id < client_id){
                            access = true;
                        } else {
                            access = false;
                        }
                    }
                    break;
                case HOLDING:
                    access =  false;
                break;
            }

            response->set_access_granted(access);
            response->set_lamport_timestamp(my_timestamp.updateTimestamp(client_timestamp));

            return grpc::Status::OK;
        };

        grpc::Status ReleaseAccess (grpc::ServerContext *context, const distributed_printing::AccessRelease *request, distributed_printing::Empty *empty) override {

            int32_t client_id = request->client_id();
            int64_t client_timestamp = request->lamport_timestamp();
            int32_t client_request_number = request->request_number();
 
            switch(my_state) {
                case RELEASED:
                case HOLDING:
                    break;
                case WAITING:
                    neighbors_released++;
                    if(neighbors_released >= neighbors_address.size()){
                        // Free wanna_printer
                        mt_wanna.unlock();
                    }
            }

            my_timestamp.updateTimestamp(client_timestamp);

            return grpc::Status::OK;
        };
};

int request_access(){
    std::list<std::string> address = neighbors_address;

    while(!address.empty()){
        std::string _address = address.back();

        // Setup request
        distributed_printing::AccessRequest request;
        distributed_printing::AccessResponse response;

        request.set_client_id(my_id);
        request.set_lamport_timestamp(my_timestamp.curTimestamp());
        request.set_request_number(my_request_number);

        // Call
        auto channel = grpc::CreateChannel(_address, grpc::InsecureChannelCredentials());
        std::unique_ptr<distributed_printing::MutualExclusionService::Stub> stub = distributed_printing::MutualExclusionService::NewStub(channel);
        grpc::ClientContext context;

        grpc::Status status = stub->RequestAccess(&context, request, &response);

        if(!status.ok()) {
            // Try again.
            continue;
        }

        // Update my timestamp.
        my_timestamp.updateTimestamp(response.lamport_timestamp());

        if(response.access_granted()) {
            neighbors_released++;
        }

        address.pop_back();
    }

    if(neighbors_released >= neighbors_address.size()){
        // Free wanna_printer
        mt_wanna.unlock();
        neighbors_released = 0;
    }

    return true;
}

int release_access(){
    std::list<std::string> address = neighbors_address;

    while(!address.empty()){
        std::string _address = address.back();

        // Setup request
        distributed_printing::AccessRelease release;
        distributed_printing::Empty empty_response;

        release.set_client_id(my_id);
        release.set_lamport_timestamp(my_timestamp.curTimestamp());
        release.set_request_number(my_request_number);

        // Call
        auto channel = grpc::CreateChannel(_address, grpc::InsecureChannelCredentials());
        std::unique_ptr<distributed_printing::MutualExclusionService::Stub> stub = distributed_printing::MutualExclusionService::NewStub(channel);
        grpc::ClientContext context;
        grpc::Status status = stub->ReleaseAccess(&context, release, &empty_response);

        // Update my timestamp.
        my_timestamp.updateTimestamp();

        address.pop_back();
    }

    return 0;
}

void send_print(std::string message) {
    // Setup request
    distributed_printing::PrintRequest request;
    distributed_printing::PrintResponse response;

    request.set_client_id(0);
    request.set_message_content(message);
    request.set_lamport_timestamp(my_timestamp.curTimestamp());
    request.set_request_number(my_request_number);
    request.set_client_id(my_id);

    // Call
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<distributed_printing::PrintingService::Stub> stub = distributed_printing::PrintingService::NewStub(channel);
    grpc::ClientContext context;
    grpc::Status status = stub->SendToPrinter(&context, request, &response);

    std::cout <<"[TS: " << my_timestamp.curTimestamp() << "]"
              << "[Printer answer: " << response.confirmation_message()  << "]"
              << std::endl;

    my_timestamp.updateTimestamp(response.lamport_timestamp());
}

int wanna_print() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 30);

    while(true){
        std::cout << std::endl << "##########################" << std::endl << std::endl;

        // Sort a random number in seconds to sleep until the next asking to print.
        int seconds = dist(gen);
        sleep(seconds);

        my_state = WAITING;
        std::cout << ">>> Requesting access" << std::endl;
        my_request_number++; // 
        print_curstate();
        request_access(); // Request access to print.

        // Wait access to print.
        mt_wanna.lock();

        my_state = HOLDING;

        std::cout << ">>>>> Printing" << std::endl;
        print_curstate();
        send_print("Hello, World!");

        my_state = RELEASED;

        std::cout << ">>>>>>>>>>>>>> Releasing Access" << std::endl;
        print_curstate();
        release_access();

    }
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
        } else
        if (std::string(argv[i]) == "--port") {
            if(i + 1 < argc) {
                my_address = argv[++i];
            }
        }
    }

    if(server_address.empty() || my_address.empty() || my_id < 0){
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    /* Parse CLI Arguments*/
    int ret = -1;
    ret = parse_args(argc, argv);
    if(ret < 0){
        return -1;
    }

    /* Threads */
    // Answer access requests.
    grpc::ServerBuilder builder;
    builder.AddListeningPort(my_address, grpc::InsecureServerCredentials());

    MutualExclusionService client_service;
    builder.RegisterService(&client_service);

    std::unique_ptr<grpc::Server> client_server = builder.BuildAndStart();

    if(client_server.get() == nullptr){
        std::cout << "DEBUG: Failed to start client server" << std::endl;
        return false;
    }

    std::cout << "===# Client [ " << my_id << " ] Port: [ " << my_address << " ] Started #===" << std::endl;
    wanna_print(); // Printer user.

    return 0;
}