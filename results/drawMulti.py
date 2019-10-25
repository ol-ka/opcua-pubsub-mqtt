import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns



def drawChart(filename, title):
    df = pd.read_csv(filename, delimiter=";", skiprows=1, skipfooter=2)
    df = df.rename(columns={"Node": "Iteration", 'Microseconds': "RTT, usec"})

    print(df.median()[1])

    df.plot(x="Iteration", y="RTT, usec")

    plt.title(title, loc='center')
    plt.savefig(filename + "MULTI.png", dpi=600)
    plt.show()

drawChart(
    'test_run_2/MQTT/multi_results_manual_mqtt_1.csv',
    'Multi broker-mode MQTT, RTT')

drawChart(
    'test_run_2/OPCUA/multi_results_manual_opc_1.csv',
    'Multi OPC UA PubSub brokerless, RTT')

