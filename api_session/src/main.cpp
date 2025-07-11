#include "opcua_session.h"
#include <iostream>
#include <signal.h>

static UA_Boolean running = true;
void signalHandler(int signal) {
    std::cout << "Signal " << signal << " received." << std::endl;
    running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    OPCUA_Session session;
    if (!session.connect("opc.tcp://127.0.0.1:4840")) {
        return EXIT_FAILURE;
    }
    std::cout << "Connected to OPC UA server." << std::endl;
    if (!session.createSubscription(7000)) {
        return EXIT_FAILURE;
    }
    std::cout << "Subscription created." << std::endl;
    if (!session.createMonitoredItems(UA_NODEID_NUMERIC(2, 2), 5000, 10)) {
        return EXIT_FAILURE;
    }
    std::cout << "Monitored items created." << std::endl;

    UA_NodeId objectId = UA_NODEID_NUMERIC(2, 1);
    UA_NodeId randomMethodId = UA_NODEID_NUMERIC(2, 6);
    while (running) {
        session.runIterate(1000);
        session.callMethod(objectId, randomMethodId);
    }
    std::cout << "Disconnecting from OPC UA server." << std::endl;
    
    return EXIT_SUCCESS;
}