#include "opcua_client.h"
#include <iostream>

OPCUA_Client::OPCUA_Client() : client(UA_Client_new(), UA_Client_delete) {
    UA_ClientConfig_setDefault(UA_Client_getConfig(client.get()));
}

OPCUA_Client::~OPCUA_Client() {
    disconnect();
}

bool OPCUA_Client::connect(const std::string& endpointUrl) {
    if (state == State::Connected) {
        disconnect();
    }
    state = State::Connecting;
    UA_StatusCode retval = UA_Client_connect(client.get(), endpointUrl.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        std::cout << "Failed to connect to OPC UA server: " << UA_StatusCode_name(retval) << std::endl;
        return false;
    }
    state = State::Connected;
    return true;
}

bool OPCUA_Client::connectAsync(const std::string& endpointUrl) {
    if (state == State::Connected) {
        disconnect();
    }
    state = State::Connecting;
    UA_StatusCode retval = UA_Client_connectAsync(client.get(), endpointUrl.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        std::cout << "Failed to connect to OPC UA server: " << UA_StatusCode_name(retval) << std::endl;
        return false;
    }
    state = State::Connected;
    return true;
}

void OPCUA_Client::disconnect() {
    UA_Client_disconnect(client.get());
}

UA_StatusCode OPCUA_Client::runIterate(int timeout) {
    return UA_Client_run_iterate(client.get(), timeout);
}

std::shared_ptr<UA_Client> OPCUA_Client::getClient() const {
    return client;
}