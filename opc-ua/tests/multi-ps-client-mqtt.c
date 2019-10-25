/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

/**
 * .. _pubsub-tutorial:
 *
 * Working with Publish/Subscribe
 * ------------------------------
 *
 * Work in progress:
 * This Tutorial will be continuously extended during the next PubSub batches. More details about
 * the PubSub extension and corresponding open62541 API are located here: :ref:`pubsub`.
 *
 * Publishing Fields
 * ^^^^^^^^^^^^^^^^^
 * The PubSub mqtt publish subscribe example demonstrate the simplest way to publish
 * informations from the information model over MQTT using
 * the UADP (or later JSON) encoding.
 * To receive information the subscribe functionality of mqtt is used.
 * A periodical call to yield is necessary to update the mqtt stack.
 *
 * **Connection handling**
 * PubSubConnections can be created and deleted on runtime. More details about the system preconfiguration and
 * connection can be found in ``tutorial_pubsub_connection.c``.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "math.h"

#define UA_NO_AMALGAMATION
#include "ua_pubsub_networkmessage.h"
#include "ua_log_stdout.h"
#include "ua_server.h"
#include "ua_config_default.h"
#include "ua_pubsub.h"
#include "ua_network_pubsub_udp.h"
#ifdef UA_ENABLE_PUBSUB_ETH_UADP
#include "ua_network_pubsub_ethernet.h"
#endif

#include "mqtt-c/mqtt.h"

#include "ua_network_pubsub_mqtt.h"
//#include "src_generated/ua_types_generated.h"

#include "ua_client.h"
#include "ua_client_subscriptions.h"

#include <signal.h>
#include <ua_types.h>
#include "../../plugins/mqtt/ua_mqtt_adapter.h"

//UA_NodeId connectionIdent, publishedDataSetIdent, writerGroupIdent;
//UA_NodeId subConnectionIdent;

UA_Server *server;
UA_Client *client;

UA_NodeId payloadVariableNodeId = {
        .namespaceIndex = 1,
        .identifierType = UA_NODEIDTYPE_NUMERIC,
        .identifier.numeric = 50123
};

uint64_t get_microseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void manuallyPublish(UA_Server *server, UA_NodeId writerGroupIdent) {
    UA_WriterGroup *wg = UA_WriterGroup_findWGbyId(server, writerGroupIdent);

    UA_WriterGroup_publishCallback(server, wg);
}

UA_UInt32 dataReceived = 0;

//#define PAYLOAD_SIZE 279// For binary
//#define PAYLOAD_SIZE 31 // 31 bytes for binary encoding and 281 for JSON
//#define PAYLOAD_SIZE 10240
//#define PAYLOAD_SIZE 1024
int PAYLOAD_SIZE = 1024;
//#define PARALLEL_FORWARD 30//10
int PARALLEL_FORWARD = 10;
#define RUNS 100

static void
addPubSubConnection(UA_Server *server, UA_String *transportProfile,
                    UA_NetworkAddressUrlDataType *networkAddressUrl, UA_NodeId *connectionIdent){
//static void
//addPubSubConnection(UA_Server *server){
    /* Details about the connection configuration and handling are located
     * in the pubsub connection tutorial */
    UA_PubSubConnectionConfig connectionConfig;
    memset(&connectionConfig, 0, sizeof(connectionConfig));
    connectionConfig.name = UA_STRING("MQTT-PubSubConnection");
    connectionConfig.transportProfileUri = *transportProfile;
    connectionConfig.enabled = UA_TRUE;    
    UA_Variant_setScalar(&connectionConfig.address, networkAddressUrl,
                         &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    connectionConfig.publisherId.numeric = UA_UInt32_random();

    UA_KeyValuePair connectionOptions[1];
    connectionOptions[0].key = UA_QUALIFIEDNAME(0, "mqttClientId");

    char clientId[255];
    sprintf(clientId, "TestClientPubSubMQTT_%d", UA_UInt32_random());

    UA_String mqttClientId = UA_STRING(clientId);
    UA_Variant_setScalar(&connectionOptions[0].value, &mqttClientId, &UA_TYPES[UA_TYPES_STRING]);
    
    connectionConfig.connectionProperties = connectionOptions;
    connectionConfig.connectionPropertiesSize = 1;
    
    UA_Server_addPubSubConnection(server, &connectionConfig, connectionIdent);
}

/**
 * **PublishedDataSet handling**
 * The PublishedDataSet (PDS) and PubSubConnection are the toplevel entities and can exist alone. The PDS contains
 * the collection of the published fields.
 * All other PubSub elements are directly or indirectly linked with the PDS or connection.
 */
static void
addPublishedDataSet(UA_Server *server, UA_NodeId *publishedDataSetIdent) {
    /* The PublishedDataSetConfig contains all necessary public
    * informations for the creation of a new PublishedDataSet */
    UA_PublishedDataSetConfig publishedDataSetConfig;
    memset(&publishedDataSetConfig, 0, sizeof(UA_PublishedDataSetConfig));
    publishedDataSetConfig.publishedDataSetType = UA_PUBSUB_DATASET_PUBLISHEDITEMS;
    publishedDataSetConfig.name = UA_STRING("PDS");
    /* Create new PublishedDataSet based on the PublishedDataSetConfig. */
    UA_Server_addPublishedDataSet(server, &publishedDataSetConfig, publishedDataSetIdent);
}

static void
addVariable(UA_Server *server) {
    // Define the attribute of the myInteger variable node 
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = 4233;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US","the answer");
    attr.displayName = UA_LOCALIZEDTEXT("en-US","the answer");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    // Add the variable node to the information model 
    UA_NodeId myIntegerNodeId = UA_NODEID_NUMERIC(1, 42);
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
}


static void
addDataVariable(UA_Server *server, UA_ByteString *data) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("", "Binary Data");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "Binary Data");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_Variant_setScalar(&attr.value, data, &UA_TYPES[UA_TYPES_BYTESTRING]);
    attr.dataType = UA_TYPES[UA_TYPES_BYTESTRING].typeId;

    UA_StatusCode retVal = UA_Server_addVariableNode(server, payloadVariableNodeId, parentNodeId,
                                                     parentReferenceNodeId, currentName,
                                                     variableTypeNodeId, attr, NULL, NULL);
    if (retVal != UA_STATUSCODE_GOOD) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Adding node failed: %s!",
                       UA_StatusCode_name(retVal));
    }
}

/**
 * **DataSetField handling**
 * The DataSetField (DSF) is part of the PDS and describes exactly one published field.
 */
static void
addDataSetField(UA_Server *server, UA_NodeId publishedDataSetIdent) {
    /* Add a field to the previous created PublishedDataSet */
    // Here we add variable we are going to publish
    // OK: Value which we publishing is added here, so it will be x-axis=42
    UA_NodeId dataSetFieldIdent2;
    UA_DataSetFieldConfig dataSetFieldConfig;
    memset(&dataSetFieldConfig, 0, sizeof(UA_DataSetFieldConfig));
    //dataSetFieldConfig.dataSetFieldType = UA_PUBSUB_DATASETFIELD_VARIABLE;
    //dataSetFieldConfig.field.variable.fieldNameAlias = UA_STRING("x-axis");
    dataSetFieldConfig.field.variable.fieldNameAlias = UA_STRING("");
    dataSetFieldConfig.field.variable.promotedField = UA_FALSE;
    //dataSetFieldConfig.field.variable.publishParameters.publishedVariable = UA_NODEID_NUMERIC(1, 42);
    dataSetFieldConfig.field.variable.publishParameters.publishedVariable = payloadVariableNodeId;
    dataSetFieldConfig.field.variable.publishParameters.attributeId = UA_ATTRIBUTEID_VALUE;
    UA_Server_addDataSetField(server, publishedDataSetIdent, &dataSetFieldConfig, &dataSetFieldIdent2);
}

/**
 * **WriterGroup handling**
 * The WriterGroup (WG) is part of the connection and contains the primary configuration
 * parameters for the message creation.
 */
static void
addWriterGroup(UA_Server *server, UA_NodeId connectionIdent, UA_NodeId *writerGroupIdent) {
    /* Now we create a new WriterGroupConfig and add the group to the existing PubSubConnection. */
    UA_WriterGroupConfig writerGroupConfig;
    memset(&writerGroupConfig, 0, sizeof(UA_WriterGroupConfig));
    writerGroupConfig.name = UA_STRING("WG");
    writerGroupConfig.publishingInterval = 0;
    writerGroupConfig.enabled = UA_FALSE;
    writerGroupConfig.writerGroupId = 100;

    
    /* Choose the encoding, UA_PUBSUB_ENCODING_JSON is available soon */
    //writerGroupConfig.encodingMimeType = UA_PUBSUB_ENCODING_JSON;
    writerGroupConfig.encodingMimeType = UA_PUBSUB_ENCODING_UADP;
    
    UA_BrokerWriterGroupTransportDataType brokerTransportSettings;
    memset(&brokerTransportSettings, 0, sizeof(UA_BrokerWriterGroupTransportDataType));
    brokerTransportSettings.queueName = UA_STRING("customTopic");
    brokerTransportSettings.resourceUri = UA_STRING_NULL;
    brokerTransportSettings.authenticationProfileUri = UA_STRING_NULL;

    /* Choose the QOS Level for MQTT */
    brokerTransportSettings.requestedDeliveryGuarantee = UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT;
    
    UA_ExtensionObject transportSettings;
    memset(&transportSettings, 0, sizeof(UA_ExtensionObject));
    transportSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    transportSettings.content.decoded.type = &UA_TYPES[UA_TYPES_BROKERWRITERGROUPTRANSPORTDATATYPE];
    transportSettings.content.decoded.data = &brokerTransportSettings;
    
    writerGroupConfig.transportSettings = transportSettings;
    /* The configuration flags for the messages are encapsulated inside the
     * message- and transport settings extension objects. These extension objects
     * are defined by the standard. e.g. UadpWriterGroupMessageDataType */
    UA_Server_addWriterGroup(server, connectionIdent, &writerGroupConfig, writerGroupIdent);
}

/**
 * **DataSetWriter handling**
 * A DataSetWriter (DSW) is the glue between the WG and the PDS. The DSW is linked to exactly one
 * PDS and contains additional informations for the message generation.
 */
static void
addDataSetWriter(UA_Server *server, UA_NodeId writerGroupIdent, UA_NodeId publishedDataSetIdent) {
    /* We need now a DataSetWriter within the WriterGroup. This means we must
     * create a new DataSetWriterConfig and add call the addWriterGroup function. */
    UA_NodeId dataSetWriterIdent;
    UA_DataSetWriterConfig dataSetWriterConfig;
    memset(&dataSetWriterConfig, 0, sizeof(UA_DataSetWriterConfig));
    dataSetWriterConfig.name = UA_STRING("DSW");
    dataSetWriterConfig.dataSetWriterId = 62541;
    dataSetWriterConfig.keyFrameCount = 10;
    
    /* JSON config for the dataSetWriter */
    UA_JsonDataSetWriterMessageDataType jsonDswMd;
    jsonDswMd.dataSetMessageContentMask = (UA_JsonDataSetMessageContentMask)
            (UA_JSONDATASETMESSAGECONTENTMASK_DATASETWRITERID 
            | UA_JSONDATASETMESSAGECONTENTMASK_SEQUENCENUMBER
            | UA_JSONDATASETMESSAGECONTENTMASK_STATUS
            | UA_JSONDATASETMESSAGECONTENTMASK_METADATAVERSION
            | UA_JSONDATASETMESSAGECONTENTMASK_TIMESTAMP);

    
    UA_ExtensionObject messageSettings;
    messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_JSONDATASETWRITERMESSAGEDATATYPE];
    messageSettings.content.decoded.data = &jsonDswMd;
    
    dataSetWriterConfig.messageSettings = messageSettings;
    
    
    UA_Server_addDataSetWriter(server, writerGroupIdent, publishedDataSetIdent,
                               &dataSetWriterConfig, &dataSetWriterIdent);
}

/**
 * That's it! You're now publishing the selected fields.
 * Open a packet inspection tool of trust e.g. wireshark and take a look on the outgoing packages.
 * The following graphic figures out the packages created by this tutorial.
 *
 * .. figure:: ua-wireshark-pubsub.png
 *     :figwidth: 100 %
 *     :alt: OPC UA PubSub communication in wireshark
 *
 * The open62541 subscriber API will be released later. If you want to process the the datagrams,
 * take a look on the ua_network_pubsub_networkmessage.c which already contains the decoding code for UADP messages.
 *
 * It follows the main server code, making use of the above definitions. */
UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}

/* Periodically refreshes the MQTT stack (sending/receiving) */
// OK: We are not using standard yield callback, we call yield manually from main, just keep it here to be in sync with MQTT tutorial
static void
mqttYieldPollingCallback(UA_Server *server, UA_PubSubConnection *connection) {
    //connection->channel->yield(connection->channel);
}

static void publisherCallback(UA_ByteString *encodedBuffer, UA_ByteString *topic){
    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Publisher callback received");
    return;

    UA_NetworkMessage networkMessage;
    size_t currentPosition = 0;
    
    UA_StatusCode ret = UA_NetworkMessage_decodeBinary(encodedBuffer, &currentPosition, &networkMessage);
    
    if ( !((networkMessage.networkMessageType != UA_NETWORKMESSAGE_DATASET) ||
        (networkMessage.payloadHeaderEnabled && networkMessage.payloadHeader.dataSetPayloadHeader.count < 1))
            )
    {
        UA_DataSetMessage *dsm = &networkMessage.payload.dataSetPayload.dataSetMessages[0];
        if(dsm->header.dataSetMessageType == UA_DATASETMESSAGE_DATAKEYFRAME) {
            // Loop over the fields and print well-known content types
            for(int i = 0; i < dsm->data.keyFrameData.fieldCount; i++) {
                const UA_DataType *currentType = dsm->data.keyFrameData.dataSetFields[i].value.type;

                if(currentType == &UA_TYPES[UA_TYPES_BYTESTRING]) {
                    UA_ByteString value = *(UA_ByteString *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "ByteString received %s", value.data);
                } else {
                    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Publisher received data type %s", currentType->typeName);
                }
            }

            //dataReceived += encodedBuffer->length;
            
            UA_ByteString_delete(encodedBuffer);
            UA_ByteString_delete(topic);
            
            UA_NetworkMessage_deleteMembers(&networkMessage);
            
        } else {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Expected key framed message");
            UA_NetworkMessage_deleteMembers(&networkMessage);
        }
    }
}

// OK: Standard callback, but is not called with hacked MQTT client
static void subscriberCallback(UA_ByteString *encodedBuffer, UA_ByteString *topic){
     //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Subscriber message Received");

    UA_NetworkMessage networkMessage;
    size_t currentPosition = 0;

    UA_StatusCode ret = UA_NetworkMessage_decodeBinary(encodedBuffer, &currentPosition, &networkMessage);

    if ( !((networkMessage.networkMessageType != UA_NETWORKMESSAGE_DATASET) ||
           (networkMessage.payloadHeaderEnabled && networkMessage.payloadHeader.dataSetPayloadHeader.count < 1))
            )
    {
        UA_DataSetMessage *dsm = &networkMessage.payload.dataSetPayload.dataSetMessages[0];
        if(dsm->header.dataSetMessageType == UA_DATASETMESSAGE_DATAKEYFRAME) {
            // Loop over the fields and print well-known content types
            for(int i = 0; i < dsm->data.keyFrameData.fieldCount; i++) {
                const UA_DataType *currentType = dsm->data.keyFrameData.dataSetFields[i].value.type;

                if(currentType == &UA_TYPES[UA_TYPES_BYTESTRING]) {
                    UA_ByteString value = *(UA_ByteString *)dsm->data.keyFrameData.dataSetFields[i].value.data;
                    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "ByteString received %s", value.data);
                    dataReceived += value.length;
                } else {
                    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Publisher received data type %s", currentType->typeName);
                }
            }



            UA_ByteString_delete(encodedBuffer);
            UA_ByteString_delete(topic);

            UA_NetworkMessage_deleteMembers(&networkMessage);

        } else {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Expected key framed message");
            UA_NetworkMessage_deleteMembers(&networkMessage);
        }
    }
}


/* Adds a subscription */
static void
addSubscription(UA_Server *server, UA_PubSubConnection *connection, int qos,
                void (*callback)(UA_ByteString *, UA_ByteString *)) {

    //Register Transport settings
    UA_BrokerWriterGroupTransportDataType brokerTransportSettings;
    memset(&brokerTransportSettings, 0, sizeof(UA_BrokerWriterGroupTransportDataType));
    brokerTransportSettings.queueName = UA_STRING("customTopic");
    brokerTransportSettings.resourceUri = UA_STRING_NULL;
    brokerTransportSettings.authenticationProfileUri = UA_STRING_NULL;

    /* QOS */

    brokerTransportSettings.requestedDeliveryGuarantee = qos;
            ////UA_BROKERTRANSPORTQUALITYOFSERVICE_ATMOSTONCE;
            //UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT;

    UA_ExtensionObject transportSettings;
    memset(&transportSettings, 0, sizeof(UA_ExtensionObject));
    transportSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    transportSettings.content.decoded.type = &UA_TYPES[UA_TYPES_BROKERWRITERGROUPTRANSPORTDATATYPE];
    transportSettings.content.decoded.data = &brokerTransportSettings;

    UA_StatusCode rv = connection->channel->regist(connection->channel, &transportSettings, callback);
    if (rv == UA_STATUSCODE_GOOD) {
            UA_UInt64 subscriptionCallbackId;
            UA_Server_addRepeatedCallback(server, (UA_ServerCallback)mqttYieldPollingCallback,
                                          connection, 200, &subscriptionCallbackId);
        } else {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "register channel failed: %s!",
                           UA_StatusCode_name(rv));
    }

    return;
}

int main(int argc, char **argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    /*if (argc != 3) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                     "Usage: program MQTT_ENDPOINT MQTT_START_PORT PARALLEL_FORWARD PAYLOAD_SIZE QOS");
        return 1;
    }*/

    UA_UInt16 mqtt_start_port = 1883;
    if (argc >= 2 && argv[2] != "") {
        mqtt_start_port =   atoi(argv[2]);
    }

    if (mqtt_start_port == 0) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Invalid MQTT start port");
        return 1;
    }

    UA_NetworkAddressUrlDataType mqttNetworkAddressUrl;
    char mqttAddressPort[255];
    char* mqttAddress = argv[1];

    if(argc > 1 && strncmp(argv[1], "opc.mqtt://", 11) == 0) {
        mqttNetworkAddressUrl.networkInterface = UA_STRING_NULL;

        snprintf(mqttAddressPort, 255, "%s:%d", argv[1], mqtt_start_port);
        mqttNetworkAddressUrl.url = UA_STRING(mqttAddressPort);

    } else {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                     "Usage: program MQTT_ENDPOINT MQTT_START_PORT, MQTT endpoint as opc.mqtt://127.0.0.1:1883");

        mqttNetworkAddressUrl.networkInterface = UA_STRING_NULL;
        mqttNetworkAddressUrl.url = UA_STRING("opc.mqtt://127.0.0.1:1883");
        snprintf(mqttAddressPort, 255, "%s:%d", "opc.mqtt://127.0.0.1:1883", mqtt_start_port);
        snprintf(mqttAddress, 255, "%s", "opc.mqtt://127.0.0.1:1883");

    }

    PARALLEL_FORWARD = 10;
    if (argc >= 3 && argv[3] != "") {
        PARALLEL_FORWARD =   atoi(argv[3]);
    }

    PAYLOAD_SIZE = 1024;
    if (argc >= 4 && argv[4] != "") {
        PAYLOAD_SIZE =   atoi(argv[4]);
    }

    // Valid values
    /* typedef enum {
        UA_BROKERTRANSPORTQUALITYOFSERVICE_NOTSPECIFIED = 0,
        UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT = 1,
        UA_BROKERTRANSPORTQUALITYOFSERVICE_ATLEASTONCE = 2,
        UA_BROKERTRANSPORTQUALITYOFSERVICE_ATMOSTONCE = 3,
        UA_BROKERTRANSPORTQUALITYOFSERVICE_EXACTLYONCE = 4,
    } UA_BrokerTransportQualityOfService; */
    int qos = UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT;
    if (argc >= 5 && argv[5] != "") {
        qos =   atoi(argv[5]);
    }


    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                 "Running with: URL=%s, start port=%d, parallel forward=%d, payload=%d, qos=%d",
                 mqttNetworkAddressUrl.url.data, mqtt_start_port, PARALLEL_FORWARD, PAYLOAD_SIZE, qos);

    // --> init publish payload
    UA_Byte dataArr[PAYLOAD_SIZE];
    memset(dataArr, '*', PAYLOAD_SIZE);

    dataArr[0] = 1;
    UA_ByteString publishPayloadData;
    // init. the struct that will be sent to the server, and the variable that will hold the elapsed time for each call
    publishPayloadData.data = &dataArr[0];
    publishPayloadData.length = PAYLOAD_SIZE;
    // <-- init publish payload

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_ServerConfig *config = UA_ServerConfig_new_minimal(16660, NULL);
    /* Details about the connection configuration and handling are located in
    * the pubsub connection tutorial */
    config->pubsubTransportLayers =
            (UA_PubSubTransportLayer *) UA_calloc(2, sizeof(UA_PubSubTransportLayer));
    if(!config->pubsubTransportLayers) {
        UA_ServerConfig_delete(config);
        return -1;
    }
    
    config->pubsubTransportLayers[0] = UA_PubSubTransportLayerMQTT();
    config->pubsubTransportLayersSize++;

    UA_String transportProfile = UA_STRING("http://opcfoundation.org/UA-Profile/Transport/pubsub-mqtt");


    server = UA_Server_new(config);

    addDataVariable(server, &publishPayloadData);

    char subscribeUrls[PARALLEL_FORWARD][255];
    UA_NetworkAddressUrlDataType subNetworkAddressUrl[PARALLEL_FORWARD];
    UA_NodeId subConnectionIdent[PARALLEL_FORWARD];

    for (unsigned int i = 0; i < PARALLEL_FORWARD; i++) {


        //snprintf(publishUrls[i], 255, "%s:%d", mqtt_endpoint, mqtt_start_port + i);
        snprintf(subscribeUrls[i], 255, "%s:%d", "opc.mqtt://127.0.0.1", mqtt_start_port + i);


        subNetworkAddressUrl[i].networkInterface = UA_STRING_NULL;
        subNetworkAddressUrl[i].url = UA_STRING(subscribeUrls[i]);

        addPubSubConnection(server, &transportProfile, &subNetworkAddressUrl[i], &subConnectionIdent[i]);

        UA_PubSubConnection *connection =
                UA_PubSubConnection_findConnectionbyId(server, subConnectionIdent[i]);
        if (connection != NULL) {
            addSubscription(server, connection, qos,
                            &subscriberCallback);
        }
    }

    UA_NodeId writerGroupIdent[PARALLEL_FORWARD];
    UA_NodeId publishedDataSetIdent[PARALLEL_FORWARD];
    UA_NodeId pubConnectionIdent[PARALLEL_FORWARD];
    UA_NetworkAddressUrlDataType pubNetworkAddressUrl[PARALLEL_FORWARD];
    char publishUrls[PARALLEL_FORWARD][255];

    for (unsigned int i = 0; i < PARALLEL_FORWARD; i++) {


        //snprintf(publishUrls[i], 255, "%s:%d", mqtt_endpoint, mqtt_start_port + i);
        snprintf(publishUrls[i], 255, "%s:%d", "opc.mqtt://127.0.0.1", mqtt_start_port + i);


        pubNetworkAddressUrl[i].networkInterface = UA_STRING_NULL;
        pubNetworkAddressUrl[i].url = UA_STRING(publishUrls[i]);

        addPubSubConnection(server, &transportProfile, &(pubNetworkAddressUrl[i]), &(pubConnectionIdent[i]));
        addPublishedDataSet(server, &(publishedDataSetIdent[i]));
        addDataSetField(server, publishedDataSetIdent[i]);
        addWriterGroup(server, pubConnectionIdent[i], &(writerGroupIdent[i]));
        addDataSetWriter(server, writerGroupIdent[i], publishedDataSetIdent[i]);

        UA_PubSubConnection *pubConnection =
                UA_PubSubConnection_findConnectionbyId(server, pubConnectionIdent[i]);
        if(pubConnection != NULL) {
            addSubscription(server, pubConnection, qos, &publisherCallback);
        }

    }
////

    //addClientSubscription(server);

    // OK: Normal way to start server with built-in processing loop and callbacks
    //retval |= UA_Server_run(server, &running);

    // OK: Running one-thread server in "iterative" or step-by-step mode with custom processing loop
    UA_Server_run_startup(server);
    UA_Server_run_iterate(server, false);
    uint64_t elapsedTime[RUNS];

    for (unsigned int k = 0; k < RUNS && running; k++) {
        bool success = false;
        unsigned int timeoutMicros = 500 * 10000;
        uint64_t start_time;
        do {
            dataReceived = 0;
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "[%d/%d] Publishing data", k, RUNS);
            start_time = get_microseconds();
            for (unsigned int i = 0; i < PARALLEL_FORWARD; i++) {
                //manuallyPublish(server, writerGroupIdent[i]);

                // OK: We do yet ONLY ONE parallel forward (multiple server commands)
                manuallyPublish(server, writerGroupIdent[i]);
                UA_PubSubConnection *pubConnection =
                        UA_PubSubConnection_findConnectionbyId(server, pubConnectionIdent[i]);
                if(pubConnection != NULL) {
                    pubConnection->channel->yield(pubConnection->channel);
                }

            }

            while (running && !success && (get_microseconds() - start_time) < timeoutMicros) {
                // OK: trigger processing of server events for MQTT
                //connection->channel->yield(connection->channel);

                for (unsigned int j = 0; j < PARALLEL_FORWARD; j++) {
                    UA_PubSubConnection *subConnection =
                            UA_PubSubConnection_findConnectionbyId(server, subConnectionIdent[j]);
                    if (subConnection != NULL) {
                        subConnection->channel->yield(subConnection->channel);
                    }
                }

                success = dataReceived == PARALLEL_FORWARD * PAYLOAD_SIZE;
                //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Check data received %d vs %d", dataReceived, PARALLEL_FORWARD * PAYLOAD_SIZE);

                if (!success)
                    usleep(10);
            }
            if (!success) {
                printf("TIMEOUT! Only got %d of %d bytes\n", dataReceived, PARALLEL_FORWARD * PAYLOAD_SIZE);
            }
        } while (running && !success);
        elapsedTime[k] = get_microseconds() - start_time;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "[%d/%d] All data received.", k, RUNS);
    }

    if (running) {
        uint64_t totalTime = 0;
        printf("-------------\nNode;Microseconds\n");
        for (unsigned int k = 0; k < RUNS; k++) {
            printf("%d;%ld\n", k, elapsedTime[k]);
            if (k > 0) // skip first, since it also includes setting up connection
                totalTime += elapsedTime[k];
        }
        printf("---------------\n");
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Total Elapsed = %fms, Average for RUN = %fms, Average RTT = %fms",
                       totalTime / 1000.0, (totalTime / (double) (RUNS - 1)) / 1000.0, (totalTime / (double) (RUNS - 1)) / 1000.0 / PARALLEL_FORWARD);
    }
/**/
    running = false;
    UA_Server_run_shutdown(server);


    UA_Server_delete(server);
    UA_ServerConfig_delete(config);
    return (int)retval;
}

