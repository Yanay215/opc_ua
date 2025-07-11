#include "subscriptions.h"
#include <iostream>
#include <typeinfo>

Subscriptions::Subscriptions(std::shared_ptr<UA_Client> client) : client(client) {}

UA_UInt32 Subscriptions::createSubscription(int publishingInterval) {
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = publishingInterval;
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client.get(), request, nullptr, nullptr, nullptr);
    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        std::cout << "Failed to create subscription: " << UA_StatusCode_name(response.responseHeader.serviceResult) << std::endl;
        UA_CreateSubscriptionResponse_clear(&response);
        return 1;
    }
    subscriptionId = response.subscriptionId;
    return subscriptionId;
}

void Subscriptions::addRequest(UA_NodeId nodeId, UA_Double samplingInterval, UA_UInt32 queueSize) {
    auto request = MonitoredItemRequestPtr(new UA_MonitoredItemCreateRequest, &Subscriptions::deleteRequests);
    *request = UA_MonitoredItemCreateRequest_default(nodeId);
    request->requestedParameters.samplingInterval = samplingInterval;
    request->requestedParameters.queueSize = queueSize;
    requests.push_back(std::move(request));
}

void Subscriptions::deleteRequests(UA_MonitoredItemCreateRequest *request) {
    UA_MonitoredItemCreateRequest_clear(request);
    delete request;
}

void Subscriptions::deleteResponses(UA_MonitoredItemCreateResult *response) {
    UA_MonitoredItemCreateResult_clear(response);
    delete response;
}