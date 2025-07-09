#include "open62541/client.h"
#include "open62541/client_config_default.h"
#include "open62541/client_subscriptions.h"
#include <iostream>
#include <vector>
#include <memory>
#include <signal.h>

class OPCUA_Client {
    public:
        OPCUA_Client() : client(UA_Client_new(), UA_Client_delete) {
            UA_ClientConfig_setDefault(UA_Client_getConfig(client.get()));
        }

        bool connect(const std::string& endpointUrl) {
            UA_StatusCode status = UA_Client_connect(client.get(), endpointUrl.c_str());
            if (status != UA_STATUSCODE_GOOD) {
                std::cerr << "Failed to connect to endpoint " << endpointUrl << ": " << std::hex << status << std::endl;
                return false;
            }
            return true;
        }

        void disconnect() {
            UA_Client_disconnect(client.get());
        }

        UA_StatusCode runIterate(int timeout) {
            return UA_Client_run_iterate(client.get(), timeout);
        }

        std::shared_ptr<UA_Client> getClient() const { return client; }
    private:
        std::shared_ptr<UA_Client> client;
};

class Subscriptions {
    public:
        Subscriptions(std::shared_ptr<UA_Client> client) : client(client) {}
        ~Subscriptions() { clear(); }

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

        void addRequest(UA_NodeId nodeId, UA_Double samplingInterval, UA_UInt32 queueSize){
            UA_MonitoredItemCreateRequest request = UA_MonitoredItemCreateRequest_default(nodeId);
            request.requestedParameters.samplingInterval = samplingInterval;
            request.requestedParameters.queueSize = queueSize;
            requests.push_back(request);
        }

        template<typename T>
        bool createMonitoredItems(UA_TimestampsToReturn timestamp){
            bool allSuccess = true;
            for (auto& request : requests) {
                UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(client.get(), subscriptionId, timestamp, request, nullptr, &dataChangeHandler<T>, nullptr);
                if (result.statusCode != UA_STATUSCODE_GOOD) {
                    allSuccess = false;
                    std::cerr << "Failed to create monitored item: " << std::hex << result.statusCode << std::endl;
                }
                responses.push_back(result);
            }
            return allSuccess;
        }
    private:
        template<typename T>
        static void dataChangeHandler(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value) {
            printf("Received data change:%i, ", value->hasValue);
            if (value->hasValue) {
                printf("value: ");
                if (value->value.type == &UA_TYPES[UA_TYPES_DOUBLE] && typeid(T) == typeid(UA_Double)) {
                    printf("%f", *(double*)value->value.data);
                } else if (value->value.type == &UA_TYPES[UA_TYPES_INT32] && typeid(T) == typeid(UA_Int32)) {
                    printf("%i", *(int*)value->value.data);
                } else if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN] && typeid(T) == typeid(UA_Boolean)) {
                    printf("%i", *(bool*)value->value.data);
                } else if (value->value.type == &UA_TYPES[UA_TYPES_STRING] && typeid(T) == typeid(UA_String)) {
                    printf("%s", (char*)value->value.data);
                } else {
                    printf("Unknown type");
                }
            }
            printf("\n");
        }

        void clear() {
            for (auto& response : responses) {
                if (response.monitoredItemId != 0 && subscriptionId != 0) {
                    UA_Client_MonitoredItems_deleteSingle(client.get(), subscriptionId, response.monitoredItemId);
                }
                UA_MonitoredItemCreateResult_clear(&response);
            }
            responses.clear();
            if (subscriptionId != 0) {
                UA_Client_Subscriptions_deleteSingle(client.get(), subscriptionId);
                subscriptionId = 0;
            }
            for (auto& request : requests) {
                UA_MonitoredItemCreateRequest_clear(&request);
            }
            requests.clear();
        }

        std::shared_ptr<UA_Client> client;
        UA_UInt32 subscriptionId = 0;
        std::vector<UA_MonitoredItemCreateRequest> requests;
        std::vector<UA_MonitoredItemCreateResult> responses;
};

static UA_Boolean running = true;

static void sigintHandler(int sig) {
    std::cout << "SIGINT received, exiting..." << std::endl;
    running = false;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigintHandler);
    signal(SIGTERM, sigintHandler);

    OPCUA_Client client;
    if (!client.connect("opc.tcp://127.0.0.1:4840")) {
        std::cerr << "Failed to connect to OPC UA server" << std::endl;
        return EXIT_FAILURE;
    }

    Subscriptions subscriptions(client.getClient());
    if (!subscriptions.createSubscription(7000)) {
        std::cerr << "Failed to create subscription" << std::endl;
        return EXIT_FAILURE;
    }

    subscriptions.addRequest(UA_NODEID_NUMERIC(2, 2), 5000, 10);

    if (!subscriptions.createMonitoredItems<UA_Double>(UA_TIMESTAMPSTORETURN_BOTH)) {
        std::cerr << "Failed to create monitored items" << std::endl;
        return EXIT_FAILURE;
    }

    while (running) {
        UA_StatusCode status = client.runIterate(1000);
        if (status != UA_STATUSCODE_GOOD) {
            std::cerr << "Failed to run iterate: " << std::hex << status << std::endl;
            return EXIT_FAILURE;
        }
    }
    client.disconnect();
    return EXIT_SUCCESS;
}
