#include "methods.h"
#include <iostream>

bool Methods::started = false;

Methods::Methods(std::shared_ptr<UA_Client> client) : client(client) {}
Methods::~Methods() {}

UA_StatusCode Methods::callMethod(const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize, int arg) {
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
    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    return retval;
}

UA_StatusCode Methods::callMethodAsync(const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize, int arg) {
    if (started) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    started = true;
    UA_Variant input;
    UA_Variant_init(&input);
    if (arg != NULL) {
        UA_Variant_setScalar(&input, &arg, &UA_TYPES[UA_TYPES_INT64]);
    }

    UA_StatusCode status = UA_Client_call_async(client.get(), objectId, methodId, inputSize, &input, callback, NULL, 0);
    UA_Variant_clear(&input);
    if (status != UA_STATUSCODE_GOOD) {
        std::cout << "Error calling method: " << UA_StatusCode_name(status) << std::endl;
        started = false;
    }
    return status;
}

void Methods::callback(UA_Client* client, void* userdata, UA_UInt32 requestId, UA_CallResponse* response) {
    size_t outputSize;
    UA_Variant* output;
    UA_StatusCode retval = response->responseHeader.serviceResult;
    if (retval == UA_STATUSCODE_GOOD) {
        if (response->resultsSize == 1) {
            retval = response->results[0].statusCode;
        } else {
            retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
        }
    }
    if (retval != UA_STATUSCODE_GOOD) {
        std::cout << "Failed to call method: " << UA_StatusCode_name(retval) << std::endl;
        UA_CallResponse_clear(response);
    } else {
        output = response->results[0].outputArguments;
        outputSize = response->results[0].outputArgumentsSize;
        response->results[0].outputArguments = NULL;
        response->results[0].outputArgumentsSize = 0;
        std::cout << "Result: " << output->data << std::endl;
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    UA_CallResponse_clear(response);
    started = false;
}

bool Methods::isStarted() const { return started; }