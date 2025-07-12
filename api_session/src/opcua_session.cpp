#include "opcua_session.h"

OPCUA_Session::OPCUA_Session() = default;

OPCUA_Session::~OPCUA_Session() { disconnect(); }

bool OPCUA_Session::connect(const std::string& endpoint_url, bool async) {
    bool connected = async ? client.connect(endpoint_url) : client.connectAsync(endpoint_url);
    if (connected) {
        subscriptions = std::make_unique<Subscriptions>(client.getClient());
        methods = std::make_unique<Methods>(client.getClient());
    }
    return connected;
}

UA_StatusCode OPCUA_Session::runIterate(int timeout) { return client.runIterate(timeout); }

void OPCUA_Session::disconnect() { client.disconnect(); }

bool OPCUA_Session::createSubscription(int publishingInterval) {
    if (!subscriptions) {
        return false;
    }
    auto subscriptionId = subscriptions->createSubscription(publishingInterval);
    subscriptionsCreated = subscriptionId != 0;
    return subscriptionsCreated;
}

bool OPCUA_Session::createMonitoredItems(UA_NodeId nodeId, UA_Double samplingInterval, UA_UInt32 queueSize) {
    if (!subscriptionsCreated) {
        return false;
    }
    subscriptions->addRequest(nodeId, samplingInterval, queueSize);
    return subscriptions->createMonitoredItems<double>(UA_TIMESTAMPSTORETURN_BOTH);
}

UA_StatusCode OPCUA_Session::callMethod(const UA_NodeId& objectId, const UA_NodeId& methodId, size_t inputSize, int arg, bool async) {
    if (!methods) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    return async ? methods->callMethodAsync(objectId, methodId, inputSize, arg) : methods->callMethod(objectId, methodId, inputSize, arg);
}