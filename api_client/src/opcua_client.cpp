#include "opcua_client.h"
#include <iostream>

OPCUA_Client::OPCUA_Client() : client(UA_Client_new(), UA_Client_delete) {
    UA_ClientConfig_setDefault(UA_Client_getConfig(client.get()));
}

OPCUA_Client::~OPCUA_Client() {
    disconnect();
}

bool OPCUA_Client::connect(const std::string& endpointUrl) {
    UA_StatusCode retval = UA_Client_connect(client.get(), endpointUrl.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        std::cout << "Failed to connect to OPC UA server: " << UA_StatusCode_name(retval) << std::endl;
        return false;
    }
    return true;
}

void OPCUA_Client::disconnect() {
    UA_Client_disconnect(client.get());
}

UA_StatusCode OPCUA_Client::runIterate(int timeout) {
    return UA_Client_run_iterate(client.get(), timeout);
}

UA_StatusCode OPCUA_Client::callMethod(const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize, int arg) {
    UA_Variant input;
    UA_Variant_init(&input);
    if (arg != NULL) {
        UA_Variant_setScalar(&input, &arg, &UA_TYPES[UA_TYPES_INT64]);
    }
    UA_Variant *output;
    size_t outputSize;
    UA_StatusCode retval = UA_Client_call(client.get(), objectId, methodId, inputSize, &input, &outputSize, &output);
    if (retval != UA_STATUSCODE_GOOD) {
        std::cout << "Failed to call method: " << UA_StatusCode_name(retval) << std::endl;
        return retval;
    }
    if (arg != NULL) {
        auto result = static_cast<bool>(output->data);
        std::cout << "Result: " << result << std::endl;
    } else {
        auto result = static_cast<double*>(output->data);
        std::cout << "Result: " << *result << std::endl;
    }
    return retval;
}

std::shared_ptr<UA_Client> OPCUA_Client::getClient() const {
    return client;
}