#include <iostream>
#include "../api_client_v2/include/opcua_client.h"

UA_Boolean runnning = true;

void signalHandler(int signum) {
    std::cout << "Caught signal " << signum << std::endl;
    runnning = false;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    OPCUA_Client client;
    if(!client.connect("opc.tcp://127.0.0.1:4840")) {
        std::cout << "Connection failed" << std::endl;
        return EXIT_FAILURE;
    }
    while (runnning) {
        UA_NodeId objectId = UA_NODEID_NUMERIC(2, 1);
        UA_NodeId randomMethodId = UA_NODEID_NUMERIC(2, 6);
        UA_NodeId boolMethodId = UA_NODEID_NUMERIC(2, 3);
        UA_StatusCode status = client.callMethod(objectId, randomMethodId, 0);
        if (status != UA_STATUSCODE_GOOD) {
            std::cout << "Call method failed" << std::endl;
        }
        status = client.callMethod(objectId, boolMethodId, 1, rand());
        if (status != UA_STATUSCODE_GOOD) {
            std::cout << "Call second method failed" << std::endl;
        }
    }
    return EXIT_SUCCESS;
}