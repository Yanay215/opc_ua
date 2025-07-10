#include <stdio.h>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_highlevel.h"

UA_Boolean runnning = true;

void signalHandler(int signum) {
    std::cout << "Caught signal " << signum << std::endl;
    runnning = false;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    /* Create a client and connect */
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(config);
    config->noReconnect = true;
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://127.0.0.1:4840");
    std::cout << std::hex << status << std::endl;
    if(status != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        std::cout << std::hex << status << std::endl;
        return status;
    }
    while (runnning) {
        int i = rand();
        UA_Variant input;
        UA_Variant_init(&input);
        UA_Variant_setScalar(&input, &i, &UA_TYPES[UA_TYPES_INT64]);
        UA_Variant *output;
        size_t outputSize;
        UA_Variant input1;
        UA_Variant_init(&input1);
        status = UA_Client_call(client, UA_NODEID_NUMERIC(2, 1), UA_NODEID_NUMERIC(2, 6), 0, &input1, &outputSize, &output);
        if(status == UA_STATUSCODE_GOOD) {
            double value = *(double*)(output->data);
            std::cout << value << std::endl;
        } else {
            std::cout << "Error" << std::endl;
            std::cerr << std::hex << status << std::endl;
        }
        status = UA_Client_call(client, UA_NODEID_NUMERIC(2, 1), UA_NODEID_NUMERIC(2, 3), 1, &input, &outputSize, &output);
        if(status == UA_STATUSCODE_GOOD) {
            auto value = *(bool*)output->data;
            std::cout << value << std::endl;
            std::cout << "====================" << std::endl;
        } else {
            std::cout << "Error123" << std::endl;
            std::cerr << std::hex << status << std::endl;
        }
    }

    /* Read the value attribute of the node. UA_Client_readValueAttribute is a
     * wrapper for the raw read service available as UA_Client_Service_read. */
    UA_Client_delete(client); /* Disconnects the client internally */
    printf("Goodbye\n");
    return status == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}