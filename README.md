# OPC UA performance evauluation with MQTT broker

Measuring rount trip time of a published message.

Based on:
https://github.com/open62541/open62541
https://mediatum.ub.tum.de/node?id=1470362



## Building

To build with Eprosima FastRTPS, make sure that you are calling CMake with `-DTHIRDPARTY=ON`:

```sh
// To build base middleware and OPC UA 
cd opcua-pubsub-mqtt
git submodule update --init --recursive
mkdir build && cd build
cmake -DTHIRDPARTY=ON ..
make
```

If you want to run MQTT tests using `opcuamqtt-multi-ps-client` or `opcuamqtt-perf-ps-client` client for MQTT, manual change of the buffers is needed, 
also redundant console log makes test slower. Edit `libs/open62541/plugins/ua_network_pubsub_mqtt.c`:

- set bigger buffers from 2kb to e.g. 300kb
```
line 121:   memcpy(channelDataMQTT, &(UA_PubSubChannelDataMQTT){address, 300720,300720, NULL, NULL,&mqttClientId, NULL, NULL, NULL}, sizeof(UA_PubSubChannelDataMQTT)); 
```
- comment redundant logging
```
line 285:    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Publish");
```

Rebuild again.

If dependant submodule open62541 is not in branch mqtt_demo, switch branch for submodule as following, after repear build:

```sh
git submodule deinit libs/open62541   
git rm libs/open62541
git commit-m "Removed submodule "
rm -rf .git/modules/libs/open62541

git submodule add -b mqtt_demo https://github.com/open62541/open62541.git libs/open62541
```

## Running local mosquitto

### Install mosquitto

http://www.steves-internet-guide.com/install-mosquitto-linux/

```sh
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa &&
sudo apt-get update &&
sudo apt-get install mosquitto &&
sudo apt-get install mosquitto-clients
```

Use command to start or stop mosquitto service:
```sh
sudo service mosquitto stop
sudo service mosquitto start
```


### Development with mosquitto
It is recommended to use mosquitto with logging turned on from terminal/command line to understand connections and pub/sub events during development. To achive it stop service and run mosquitto with logging manually:

```sh
sudo service mosquitto stop &&
mosquitto -v
```
Run 'opcuamqtt-multi-ps-client' target configuration from IDE of your choice for debugging. CMake must be configured with option -DTHIRDPARTY=ON

# Test run: increasing payload

Make sure variable PROCSTAT is set, sample
```bash
export PROCSTAT=/media/psf/Home/git/ERT/olga/middleware_evaluation/linuxprocfs.py
```

## Test run brokerless, OPC UA Pub/Sub using UDP

Run server first and then client, better in separate terminals or machines

```bash
./opcua-perf-ps-server 50000 opc.udp://224.0.0.22:50000 opc.udp://224.0.0.22:10001
./opcua-perf-ps-client 
```

configuration for server `opcua-perf-ps-server <opc ua server port> <publish host> <subscribe host>`:
- 50000 is a server port
- opc.udp://224.0.0.22:50000 publish host
- opc.udp://224.0.0.22:10001 subscribe host 

client `opcua-perf-ps-client` has no parameters, both server and client require UDP multicast client uses 
`opc.udp://224.0.0.22:4841/` and `opc.udp://224.0.0.22:4840/` default addresses of OPS UA

Find results in executables location as `CSV` files. Or see `result`s folder for sample.

## Test run broker-mode, OPC UA Pub/Sub using MQTT broker

```bash
mosquitto -p 1883
opcuamqtt-perf-ps-client opc.mqtt://127.0.0.1:1883
```
Find results in executable location as `CSV` files. Or see `result`s folder for sample.

# Test run: multiple pub/sub, load test

## Test run brokerless,  OPC UA Pub/Sub using UDP
Start server first and than client, better to run in the separate terminals or different machines

```bash
./middleware_evaluation/opc-ua/run_multiple_pubsub.sh 10 opc.udp://224.0.0.22:50000
./middleware_evaluation/build/opc-ua/tests/opcua-multi-ps-client 50000 10001 opc.udp://224.0.0.22
```

Configuration for `opcua-multi-ps-client <SERVER_PORT> <CLIENT_START_PORT> <FORWARD_ENDPOINT>`:
- SEVRER_PORT=50000
- CLIENT_START_PORT=10001
- FORWARD_ENDPOINT=opc.udp://224.0.0.22

Configuration for `run_multiple_pubsub.sh <MAX NODES> <PUBLISH_ENDPOINT>`:
- MAX_NODES=10
- PUBLISH_ENDPOINT= opc.udp://224.0.0.22:50000

Capture results by redirecting console output. Or see `result`s folder for sample

## Test run broker-mode, OPC UA Pub/Sub using MQTT broker

First start server using sh script than client, better different terminals or machines

```bash
./middleware_evaluation/opc-ua/run_multiple_pubsub_mqtt.sh 50
./middleware_evaluation/build/opc-ua/tests/opcuamqtt-multi-ps-client opc.mqtt://127.0.0.1 1883 50 10240 1 
```

Configuration for `run_multiple_pubsub_mqtt.sh`: max number of Mosquitto instances, ports will be incremented by 1 
staring from 1883, e.g. 3 nodes start 1883, 1884, and 1885 mosquitto's.

Configuration for `opcuamqtt-multi-ps-client  <MQTT_ENDPOINT> <MQTT_START_PORT> <PARALLEL_FORWARD> <PAYLOAD_SIZE> <QOS>`:
- MQTT_ENDPOINT=opc.mqtt://127.0.0.1
- MQTT_START_PORT=1883 MQTT start port
- PARALLEL_FORWARD=50, number of publisher and subscriber instances sending in parallel
- PAYLOAD_SIZE=10240 in bytes
- QOS=1
QoS Levels described:

```
UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT = 1
UA_BROKERTRANSPORTQUALITYOFSERVICE_ATLEASTONCE = 2
UA_BROKERTRANSPORTQUALITYOFSERVICE_ATMOSTONCE = 3
UA_BROKERTRANSPORTQUALITYOFSERVICE_EXACTLYONCE = 4
```
Capture results by redirecting console output. Or see `result`s folder for sample
