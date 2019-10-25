import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns



def drawChart(filename, title):
    df = pd.read_csv(filename)
    df = df.rename(index={0: "id", 1: "rtt"})
    df = df.rename(columns={'Test Number': 'id', ' Duration [usec]': "RTT, usec"})

    df.median()[1]

    df.plot(x="id", y="RTT, usec")

    plt.title(title, loc='center')
    plt.savefig(filename + ".png", dpi=600)
    plt.show()

drawChart(
    'test_run_2/MQTT/opcua-ps-2019-10-19_124520_random.csv',
    'OPC UA Pub/Sub, borker mode MQTT, random test')

drawChart(
    'test_run_2/OPCUA/opcua-ps-2019-10-19_125652_random.csv',
    'OPC UA Pub/Sub, borkerless, random test')

