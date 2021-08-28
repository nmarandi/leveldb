#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "protos/leveldb.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace leveldb::grpc;

class DBClient
{
public:
    DBClient(std::shared_ptr<Channel> channel)
        : stub_(DBRPC::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string Open()
    {
        // Data we are sending to the server.
        DBReq request;
        DB* db = new DB();
        db->set_name("example_db");
        request.set_allocated_db(db);
        Options* options = new Options();
        options->set_create_if_missing(true);
        request.set_allocated_options(options);

        // Container for the data we expect from the server.
        DBRes reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        ::grpc::Status status = stub_->Open(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            return reply.status().message();
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<DBRPC::Stub> stub_;
};

int main(int argc, char** argv)
{
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint specified by
    // the argument "--target=" which is the only expected argument.
    // We indicate that the channel isn't authenticated (use of
    // InsecureChannelCredentials()).
    std::string target_str;
    std::string arg_str("--target");
    if (argc > 1) {
        std::string arg_val = argv[1];
        size_t start_pos = arg_val.find(arg_str);
        if (start_pos != std::string::npos) {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=') {
                target_str = arg_val.substr(start_pos + 1);
            } else {
                std::cout << "The only correct argument syntax is --target="
                          << std::endl;
                return 0;
            }
        } else {
            std::cout << "The only acceptable argument is --target=" << std::endl;
            return 0;
        }
    } else {
        target_str = "localhost:50051";
    }
    DBClient greeter(
        grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::string reply = greeter.Open();
    std::cout << "Greeter received: " << reply << std::endl;

    return 0;
}