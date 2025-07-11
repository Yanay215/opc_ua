#ifndef OPCUA_CLIENT_H
#define OPCUA_CLIENT_H

#include <memory>
#include <string>
#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_highlevel.h"
#include "open62541/client_highlevel_async.h"

class OPCUA_Client {
    public:
        OPCUA_Client();
        ~OPCUA_Client();
        bool connect(const std::string& endpointUrl);
        bool connectAsync(const std::string& endpointUrl);
        void disconnect();
        UA_StatusCode runIterate(int timeout); 
        std::shared_ptr<UA_Client> getClient() const;
    private:
        std::shared_ptr<UA_Client> client;
        enum class State { Disconnected, Connecting, Connected };
        State state = State::Disconnected;
};

#endif // OPCUA_CLIENT_H