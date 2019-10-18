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
Run 'opcuamqtt-multi-ps-client' target configuration from IDE of your choice for debugging. CMake must be configured with option -DTHIRDPARTY=ON

# JSON payload sample
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

# Test results payload

Make manual edit, comment logging on line 285 in file `libs/open62541-plugins/ua_network_pubsub_mqtt.cc` and rebuild.
Logging makes tests run longer

## Test runs with increasing payload and cpu/ram stat

### Test runs brokerless

Make sure variable PROCSTAT is set, sample
```bash
export PROCSTAT=/media/psf/Home/git/ERT/olga/middleware_evaluation/linuxprocfs.py
```
in build folder run on different machines, or without arguments on local:
```./opcua-perf-ps-server```
```./opcua-perf-ps-client```

## Test runs broker
1. Run mosquitto
2. Run `opcuamqtt-perf-ps-client`

# Test result multiple publisher/subscriber on same machine

## Test run brokerless
1. Start client
```bash
./middleware_evaluation/build/opc-ua/tests/opcua-multi-ps-client 50000 10001 opc.udp://224.0.0.22
```

client will publish messages to CLIENT_START_PORT=10001 and subscribe to messages from SEVRER_PORT=50000, client RUNS in 100 iterations, PAYLOAD_SIZE 10240, PARALLEL_FORWARD 10


SEVRER_PORT=50000
CLIENT_START_PORT=10001
URL=opc.udp://224.0.0.22

2. Start server, server works in echo mode, starts 10 listenders on ports from 10001-10010 and republishes all messages to 50000 

```bash
./middleware_evaluation/opc-ua/run_multiple_pubsub.sh 10 opc.udp://224.0.0.22:50000
```

Sample log output

```
middleware_evaluation/opc-ua$ ./run_multiple_pubsub.sh 10 opc.udp://224.0.0.22:50000
Starting node 1 with port 10001
Starting node 2 with port 10002
Starting node 3 with port 10003
Starting node 4 with port 10004
Starting node 5 with port 10005
WARNING: Could not set process priorityStarting node 6 with port 10006
Starting node 7 with port 10007
Starting node 8 with port 10008
[2019-10-13 13:45:36.011 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
Starting node 9 with port 10009
Starting node 10 with port 10010
All instances started. Kill with Ctrl+C
WARNING: Could not set process priorityWARNING: Could not set process priorityWARNING: Could not set process priorityWARNING: Could not set process priority[2019-10-13 13:45:36.015 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
WARNING: Could not set process priority[2019-10-13 13:45:36.017 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
WARNING: Could not set process priorityWARNING: Could not set process priority[2019-10-13 13:45:36.015 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.015 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.019 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.021 (UTC+0200)] info/userland	PubSub channel requested
WARNING: Could not set process priority[2019-10-13 13:45:36.016 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.027 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.028 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.029 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.021 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.021 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.030 (UTC+0200)] info/userland	PubSub channel requested
WARNING: Could not set process priority[2019-10-13 13:45:36.030 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20004/
[2019-10-13 13:45:36.030 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.030 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.031 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20006/
[2019-10-13 13:45:36.031 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.032 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20007/
[2019-10-13 13:45:36.023 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.025 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.026 (UTC+0200)] info/session	Connection 0 | SecureChannel 0 | Session g=00000001-0000-0000-0000-000000000000 | AddNodes: No TypeDefinition for i=15303; Use the default TypeDefinition for the Variable/Object
[2019-10-13 13:45:36.033 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.026 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.034 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20008/
[2019-10-13 13:45:36.032 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.034 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.035 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20005/
[2019-10-13 13:45:36.032 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.032 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20001/
[2019-10-13 13:45:36.035 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.035 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.035 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20010/
[2019-10-13 13:45:36.036 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.036 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20009/
[2019-10-13 13:45:36.036 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20002/
[2019-10-13 13:45:36.036 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.037 (UTC+0200)] info/userland	PubSub channel requested
[2019-10-13 13:45:36.037 (UTC+0200)] info/network	TCP network layer listening on opc.tcp://parallels-Parallels-Virtual-Platform:20003/
Subscribe to opc.udp://224.0.0.22:10004 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10006 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10007 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10008 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10005 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10001 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10010 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10009 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10002 and publish to opc.udp://224.0.0.22:50000
Subscribe to opc.udp://224.0.0.22:10003 and publish to opc.udp://224.0.0.22:50000
```

### Sample test results for brokerless mode

```bash
-------------
Node;Microseconds
0;1223
1;1050
2;958
3;1072
4;1938
5;1083
6;982
7;839
8;903
9;2385
10;969
11;1218
12;953
13;916
14;918
15;1038
16;956
17;3655
18;1214
19;1005
20;947
21;1368
22;969
23;1056
24;1182
25;1004
26;965
27;879
28;806
29;822
30;733
31;826
32;1203
33;920
34;822
35;773
36;892
37;872
38;971
39;1110
40;1089
41;784
42;923
43;717
44;889
45;979
46;3508
47;1172
48;948
49;955
50;938
51;1041
52;791
53;1320
54;980
55;1444
56;1144
57;1054
58;846
59;820
60;983
61;1019
62;821
63;960
64;854
65;786
66;796
67;788
68;802
69;746
70;1157
71;855
72;974
73;835
74;851
75;3759
76;1191
77;771
78;831
79;802
80;838
81;770
82;837
83;746
84;804
85;1234
86;881
87;960
88;897
89;793
90;728
91;803
92;851
93;758
94;803
95;672
96;927
97;772
98;779
99;798
---------------
[2019-10-13 13:12:53.813 (UTC+0200)] warn/userland Total Elapsed = 102.746000ms, Average = 1.037838ms
```
### Sample test results for broker mode with MQTT

Sample test result if running application locally with local MQTT broker for RUNS = 100 and PARALLEL_RUNS = 1

```bash
-------------
Node;Microseconds
0;31557
1;20452
2;20487
3;20473
4;20461
5;22159
6;23018
7;20494
8;21109
9;20929
10;20576
11;20503
12;21210
13;20762
14;21095
15;21106
16;21238
17;21066
18;20452
19;20918
20;20848
21;20477
22;22126
23;22795
24;21195
25;21424
26;20488
27;20772
28;20435
29;20526
30;20738
31;21143
32;20818
33;21057
34;20924
35;21161
36;20461
37;20588
38;20466
39;20546
40;20600
41;20736
42;26589
43;21099
44;21047
45;20595
46;20605
47;20553
48;20470
49;20709
50;20475
51;20452
52;20814
53;21173
54;28454
55;20600
56;21397
57;20678
58;21054
59;20498
60;21901
61;20673
62;20642
63;20541
64;20501
65;20524
66;20545
67;20502
68;20627
69;20512
70;20636
71;20605
72;20795
73;20529
74;21635
75;20514
76;20609
77;20826
78;20518
79;20801
80;21289
81;20843
82;20562
83;29514
84;20338
85;20402
86;22177
87;20669
88;20650
89;20592
90;20654
91;20642
92;20605
93;20491
94;21606
95;20955
96;20614
97;21073
98;21473
99;20453
---------------
[2019-10-12 11:26:11.698 (UTC+0200)] warn/userland Total Elapsed = 2086.132000ms, Average = 21.072040ms
```
## Test result using multiple publisher/subscribers on the same machine

1. Use `run_multiple_pubsub_mqtt.sh` and provide first argument - number of mqtt brokers, it will start N brokers
with incrementing port number, example

```bash
middleware_evaluation/opc-ua$ ./run_multiple_pubsub_mqtt.sh 100
```

2. Run client which starts multiple publishers and subscribers

```bash
./opcuamqtt-multi-ps-client  opc.mqtt://127.0.0.1 1883 30 1024 1
```

- first argument is URL of MQTT broker machine - opc.mqtt://127.0.0.1
- 1883 is starting port, so it will start on ports 1883, 1884, 1885, etc. brokers
- 30 is a number of parallel pub/subs using inside one run, total RUNS = 100
- 1024 is a payload size
- 1 is QoS, following QoSes can be used

```
UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT = 1
UA_BROKERTRANSPORTQUALITYOFSERVICE_ATLEASTONCE = 2
UA_BROKERTRANSPORTQUALITYOFSERVICE_ATMOSTONCE = 3
UA_BROKERTRANSPORTQUALITYOFSERVICE_EXACTLYONCE = 4
```