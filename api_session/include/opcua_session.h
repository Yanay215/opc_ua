#ifndef OPCUA_SESSION_H
#define OPCUA_SESSION_H

#include <../../api_client_v2/include/opcua_client.h>
#include <../../api_client_v2/include/subscriptions.h>
#include <../../api_client_v2/include/methods.h>

class OPCUA_Session {
    public:
        OPCUA_Session();
        ~OPCUA_Session();
        bool connect(const std::string& endpoint_url, bool async = true);
        UA_StatusCode runIterate(int timeout);
        void disconnect();
        bool createSubscription(int publishingInterval);
        bool createMonitoredItems(UA_NodeId nodeId, UA_Double samplingInterval, UA_UInt32 queueSize);
        UA_StatusCode callMethod(const UA_NodeId& objectId, const UA_NodeId& methodId, size_t inputSize = 0, int arg = NULL, bool async = true);
    private:
        OPCUA_Client client;
        std::unique_ptr<Subscriptions> subscriptions;
        std::unique_ptr<Methods> methods;
        bool subscriptionsCreated = false;
};

#endif // OPCUA_SESSION_H