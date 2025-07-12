#ifndef METHODS_H
#define METHODS_H

#include <memory>
#include <vector>
#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_highlevel.h"
#include "open62541/client_highlevel_async.h"

class Methods {
    public:
        Methods(std::shared_ptr<UA_Client> client);
        ~Methods();
        UA_StatusCode callMethod(const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize = 0, int arg = NULL);
        UA_StatusCode callMethodAsync(const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize = 0, int arg = NULL);
        bool isStarted() const;
    private:
        static void callback(UA_Client* client, void* userdata, UA_UInt32 requestId, UA_CallResponse* response);
        std::shared_ptr<UA_Client> client;
        static bool started;
};

#endif // METHODS_H