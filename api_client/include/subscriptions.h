#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

#include <memory>
#include <vector>
#include <iostream>
#include "open62541/client.h"
#include "open62541/client_subscriptions.h"

class Subscriptions {
    public:
        Subscriptions(std::shared_ptr<UA_Client> client);
        ~Subscriptions();
        UA_UInt32 createSubscription(int publishingInterval);
        void addRequest(UA_NodeId nodeId, UA_Double samplingInterval, UA_UInt32 queueSize);
        template<typename T>
        bool createMonitoredItems(UA_TimestampsToReturn timestamp) {
            bool allSuccess = true;
            for (auto& request : requests) {
                auto response = MonitoredItemResponsePtr(new UA_MonitoredItemCreateResult, &Subscriptions::deleteResponses);
                *response = UA_Client_MonitoredItems_createDataChange(client.get(), subscriptionId, timestamp, *request.get(), nullptr, &dataChangeHandler<T>, nullptr);
                if (response.get()->statusCode != UA_STATUSCODE_GOOD) {
                    std::cout << "Failed to create monitored item: " << UA_StatusCode_name(response.get()->statusCode) << std::endl;
                    allSuccess = false;
                }
                responses.push_back(std::move(response));
            }
            return allSuccess;
        }
    private:
        template<typename T>
        static void dataChangeHandler(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value){
            if (value->hasValue) {
                std::cout << "value: ";
                try {
                    T* data = static_cast<T*>(value->value.data);
                    std::cout << *data << std::endl;
                } catch (std::bad_cast& e) {
                    std::cout << "Bad cast: " << e.what() << std::endl;
                }
            }
        }
        static void deleteRequests(UA_MonitoredItemCreateRequest *request);
        using MonitoredItemRequestPtr = std::unique_ptr<UA_MonitoredItemCreateRequest, decltype(&deleteRequests)>;
        static void deleteResponses(UA_MonitoredItemCreateResult *response);
        using MonitoredItemResponsePtr = std::unique_ptr<UA_MonitoredItemCreateResult, decltype(&deleteResponses)>;
        std::shared_ptr<UA_Client> client;
        UA_UInt32 subscriptionId = 0;
        std::vector<MonitoredItemRequestPtr> requests;
        std::vector<MonitoredItemResponsePtr> responses;
};

#endif // SUBSCRIPTIONS_H