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

# Sample test result

Sample test result if running application locally with local MQTT broker for RUNS = 100 and PARALLEL_RUNS = 1

```
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
[2019-10-12 11:26:11.698 (UTC+0200)] warn/userland	Total Elapsed = 2086.132000ms, Average = 21.072040ms
```
