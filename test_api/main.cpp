#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_subscriptions.h"
#include "open62541/client_highlevel.h"
#include <stdio.h>
#include <memory>
#include <vector>
#include <iostream>


void dataChangeHandler(UA_Client *client, UA_UInt32 subId, void *subContext,
                      UA_UInt32 monId, void *monContext,
                      UA_DataValue *value) {
    printf("Received data change:%i, ", value->hasValue);
    if (value->hasValue) {
        printf("value: %f", *(double*)value->value.data);
    }
    printf("\n");
}


class Subscriptions {
    public:
        Subscriptions(UA_Client *client) : client(client, UA_Client_delete) {}
        ~Subscriptions() {
            requests.clear();
            responses.clear();
        }

        UA_UInt32 createSubscription(double publishingInterval){
            UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
            request.requestedPublishingInterval = publishingInterval;
            UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client.get(), request, nullptr, nullptr, nullptr);
            if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
                UA_CreateSubscriptionResponse_clear(&response);
                return 1;
            }
            subscriptionId = response.subscriptionId;
            return subscriptionId;
        }

        void fillRequests(UA_NodeId nodeId, UA_Double samplingInterval, UA_UInt32 queueSize){
            UA_MonitoredItemCreateRequest request = UA_MonitoredItemCreateRequest_default(nodeId);
            request.requestedParameters.samplingInterval = samplingInterval;
            request.requestedParameters.queueSize = queueSize;
            requests.push_back(request);
        }

        bool addMonitoredItems(UA_TimestampsToReturn timestamp){
            bool allSuccess = true;
            for (auto request : requests) {
                UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(client.get(), subscriptionId, timestamp, request, nullptr, &dataChangeHandler, nullptr);
                if (result.statusCode != UA_STATUSCODE_GOOD) {
                    allSuccess = false;
                }
                responses.push_back(result);
            }
            return allSuccess;
        }

    private:
        std::shared_ptr<UA_Client> client;
        UA_UInt32 subscriptionId;
        std::vector<UA_MonitoredItemCreateRequest> requests;
        std::vector<UA_MonitoredItemCreateResult> responses;
};

int main() {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://127.0.0.1:4840");
    if (retval != UA_STATUSCODE_GOOD) {
        printf("Connection failed: %s\n", UA_StatusCode_name(retval));
        return 1;
    }
    Subscriptions subscriptions(client);
    subscriptions.createSubscription(7000);
    subscriptions.fillRequests(UA_NODEID_NUMERIC(2, 2), 5000, 10);
    if (!subscriptions.addMonitoredItems(UA_TIMESTAMPSTORETURN_BOTH)) {
        printf("Failed to add monitored items\n");
        return 1;
    }
    for(int i = 0; i < 30; i++) {
        retval = UA_Client_run_iterate(client, 1000);
        if(retval != UA_STATUSCODE_GOOD) {
            std::cerr << "Client run iteration failed: 0x" << std::hex << retval << std::endl;
            break;
        }
    }
    return 0;
}