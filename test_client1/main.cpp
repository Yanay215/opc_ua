#include <stdio.h>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_subscriptions.h"
#include "open62541/client_highlevel.h"
#include <memory>
#include <vector>

void dataChangeHandler(UA_Client *client, UA_UInt32 subId, void *subContext,
                      UA_UInt32 monId, void *monContext,
                      UA_DataValue *value) {
    printf("Received data change:%i, ", value->hasValue);
    if (value->hasValue) {
        printf("value: %f", *(double*)value->value.data);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    /* Create a client and connect */
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://127.0.0.1:4840");
    std::cout << "Connection status: 0x" << std::hex << status << std::endl;
    
    if(status != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

    /* Create a subscription */
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request, NULL, NULL, NULL);
    
    if(response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        std::cerr << "Subscription failed: 0x" << std::hex << response.responseHeader.serviceResult << std::endl;
        UA_CreateSubscriptionResponse_delete(&response);
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }
    
    UA_UInt32 subId = response.subscriptionId;
    std::cout << "Created subscription with ID: " << subId << std::endl;
    std::vector<UA_MonitoredItemCreateRequest> items;
    UA_MonitoredItemCreateRequest item = UA_MonitoredItemCreateRequest_default(UA_NODEID_NUMERIC(2, 2));
    item.itemToMonitor.nodeId = UA_NODEID_NUMERIC(2, 2);
    item.requestedParameters.samplingInterval = 1000;
    item.requestedParameters.queueSize = 10;
    items.push_back(item);
    // Note: You would typically add monitored items here using UA_Client_MonitoredItems_createDataChange()
    std::vector<UA_MonitoredItemCreateResult> results;
    UA_TimestampsToReturn t = UA_TIMESTAMPSTORETURN_BOTH;
    for (int i = 0; i < items.size(); ++i) {
        UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(client, subId, t, items[i], NULL, &dataChangeHandler, NULL);
        if (result.statusCode != UA_STATUSCODE_GOOD) {
            std::cerr << "Monitored item creation failed: 0x" << std::hex << result.statusCode << std::endl;
            UA_Client_delete(client);
            return EXIT_FAILURE;
        }
        results.push_back(result);
    }
    /* Run the client for 5 seconds to process subscription messages */
    for(int i = 0; i < 30; i++) {
        status = UA_Client_run_iterate(client, 1000);
        if(status != UA_STATUSCODE_GOOD) {
            std::cerr << "Client run iteration failed: 0x" << std::hex << status << std::endl;
            break;
        }
    }
    std::cout << "Client run iteration status: 0x" << std::hex << status << std::endl;
    /* Clean up */
    //UA_CreateSubscriptionResponse_clear(&response);
    std::cout << "Deleting subscription with ID: " << subId << std::endl;
    UA_Client_delete(client);
    
    std::cout << "Goodbye" << std::endl;
    return (status == UA_STATUSCODE_GOOD) ? EXIT_SUCCESS : EXIT_FAILURE;
}