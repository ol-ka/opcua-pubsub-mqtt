#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
    echo "Usage: run_multiple.sh NUMBER_OF_MQTT_BROKERS"
    exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

BUILD_DIR=$DIR/../../build/testing/opc-ua/tests


pid_list=()
run=1
max_nodes=$1
final_endpoint=$2
start_port=1882

pkill -f -9 "opcua-perf-ps-server"

run_program () {
    port=$(($start_port + $1))
    # port_tcp=$(($start_port + $1 + 10000))
    port_fwd=$(($port + 1))

    endpoint=""
    if [ $port_fwd -lt $(($start_port + $max_nodes)) ]; then
        endpoint="opc.mqtt://127.0.0.1:${port_fwd}"
    else
        endpoint=$final_endpoint
    fi

   # echo "Starting node $1 with port $port, endpoint $endpoint and opc.mqtt://127.0.0.1:$port"
   # $BUILD_DIR/opcua-perf-ps-server $endpoint "opc.mqtt://127.0.0.1:$port" &
   echo "running mosquitto -v -p $port"
   mosquitto -p $port &

   pid_list=("${pid_list[@]}" $!)
}

trap ctrl_c INT

function ctrl_c() {
    echo -e "** Closing all child processes\n"
    for i in "${pid_list[@]}"; do
        echo -e "*** Killing PID $i\n"
        kill -SIGINT $i
    done
    echo -e "** All children killed\n"
    run=0
}



counter=1
while [ $counter -le $max_nodes ] && [ $run -eq 1 ]; do
    run_program $counter
    ((counter++))
done

echo "All instances started. Kill with Ctrl+C"

while [ $run -eq 1 ]; do
    sleep 1
done
