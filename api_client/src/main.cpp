#include "opcua_client.h"
#include "subscriptions.h"
#include "methods.h"
#include <iostream>
#include <signal.h>

static UA_Boolean running = true;

static void sigintHandler(int sig) {
    std::cout << "SIGINT received, exiting..." << std::endl;
    running = false;
}

int main(int argc, char** argv) {
    signal(SIGINT, sigintHandler);
    signal(SIGTERM, sigintHandler);

    OPCUA_Client client;
    if (!client.connectAsync("opc.tcp://127.0.0.1:4840")) {
        return EXIT_FAILURE;
    }
    Subscriptions subscriptions(client.getClient());
    UA_UInt32 subscriptionId = subscriptions.createSubscription(7000); 
    if (subscriptionId == 0) {
        std::cout << "Failed to create subscription" << std::endl;
        return EXIT_FAILURE;
    }
    subscriptions.addRequest(UA_NODEID_NUMERIC(2, 2), 5000, 10);
    if (!subscriptions.createMonitoredItems<double>(UA_TIMESTAMPSTORETURN_BOTH)) {
        std::cout << "Failed to create monitored items" << std::endl;
        return EXIT_FAILURE;
    }
    Methods methods(client.getClient());    
    UA_NodeId objectId = UA_NODEID_NUMERIC(2, 1);
    UA_NodeId randomMethodId = UA_NODEID_NUMERIC(2, 6);
    while (running) {
        UA_StatusCode retval = client.runIterate(1000);
        if (retval != UA_STATUSCODE_GOOD) {
            std::cout << "Failed to run iterate: " << UA_StatusCode_name(retval) << std::endl;
        }
        if (!methods.isStarted()) {
            retval = methods.callMethodAsync(objectId, randomMethodId);
            if (retval != UA_STATUSCODE_GOOD) {
                std::cout << "Failed to call method: " << UA_StatusCode_name(retval) << std::endl;
            }
        }
    }
    return EXIT_SUCCESS;
}
