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
Run 'olga-opc-mqtt' target configuration from IDE of your choice for debugging. CMake must be configured with option -DTHIRDPARTY=ON

# JSON payload
## Format
```js
{
  "MessageId": "7FFC8E30-77C3-20F7-87B8-6B966C16F19F",
  "MessageType": "ua-data",
  "Messages": [
    {
      "DataSetWriterId": 62541,
      "SequenceNumber": 0,
      "MetaDataVersion": {
        "MajorVersion": 2746103942,
        "MinorVersion": 0
      },
      "Timestamp": "2019-09-14T17:56:55.234Z",
      "Payload": {
        "x-axis": {
          "Type": 6,
          "Body": 42
        },
        "Server localtime": {
          "Type": 13,
          "Body": "2019-09-14T17:56:55.234Z"
        }
      }
    }
  ]
}
```
