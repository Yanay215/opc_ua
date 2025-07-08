#include <stdio.h>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_highlevel.h"

UA_StatusCode infiniteLoop(UA_Client *client, UA_Variant value, UA_StatusCode status, int& count, double& begin, timeval& tv) {
    while (true) {
        UA_Variant_init(&value);
        status = UA_Client_readValueAttribute(client, UA_NODEID_NUMERIC(2, 2), &value);
        if(status == UA_STATUSCODE_GOOD) {
            printf("the value is: %i\n", *(double*)value.data);
            if (count > 0) {
                return status;
            }
        }
        else {
            if (count == 0) {
                gettimeofday(&tv, NULL);
                begin = ((double)tv.tv_sec) * 1000 + ((double)tv.tv_usec) / 1000;
            }
            ++count;
        }
        UA_Variant_clear(&value);
        //std::cout << std::hex << status << std::endl;
    }
}

int main(int argc, char *argv[])
{
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

    /* Read the value attribute of the node. UA_Client_readValueAttribute is a
     * wrapper for the raw read service available as UA_Client_Service_read. */
    UA_Variant value; 
    struct timeval tv;
    double begin;
    int count = 0;
    status = infiniteLoop(client, value, status, count, begin, tv);
    gettimeofday(&tv, NULL);
    double end = ((double)tv.tv_sec) * 1000 + ((double)tv.tv_usec) / 1000;
    double total = end - begin;
    std:: cout << "===== " << count << " =====" << std::endl;
    std::cout << "Total time: " << total << std::endl;
    std::cout << "Average count: " << total / (count * 1000) << std::endl;
    printf("1\n");
    UA_Client_delete(client); /* Disconnects the client internally */
    printf("Goodbye\n");
    return status == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}