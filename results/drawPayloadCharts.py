import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns



def drawChart(filename, title, prefix = ""):
    payloads = np.genfromtxt(filename, delimiter=',', skip_header=1, dtype=(int, int, int), usecols=1).astype(int)

    rtt = np.genfromtxt(filename, delimiter=',', skip_header=1, dtype=(int, int, int), usecols=2).astype(int)

    df = pd.DataFrame({'x': payloads, 'y': rtt })
    df = df[df.y < 30000]
    median_df = df.groupby('x', axis = 0).median()
    #median_df.to_csv(filename + prefix + "AGGR.csv")

    median_df.reset_index(inplace=True)

    df.sort_values(by=['x'])
    median_df.sort_values(by=['x'])

    # using dictionary to convert specific columns
    convert_dict = {'x': str, 'y': float }

    df = df.astype(convert_dict)
    median_df = median_df.astype(convert_dict)




    fig, ax = plt.subplots(figsize = (8,6))
    sns.boxplot(x="x", y="y", data=df, color="silver", ax=ax, linewidth=0.75,
                order=['2', '4', '8', '16', '32', '64', '128', '256', '512', '1024', '2048', '4096','8192', '16384',
                       '32768'])  # whis=range
    sns.lineplot(x=median_df.index, y="y", data=median_df, ax=ax)
                # order=['2', '4', '8', '16', '32', '64', '128', '256', '512', '1024', '2048', '4096','8192', '16384', '32768'])

    xticks=ax.xaxis.get_major_ticks()
    ax.set(xlabel='Payload size, bytes', ylabel='RTT, usec')
    for i in range(len(xticks)):
        if i%2==1:
            xticks[i].set_visible(False)

    plt.title(title, loc='center')
    plt.savefig(filename + prefix + ".png", dpi=600)
    plt.show()

drawChart(
    'test_run_2/MQTT/opcua-ps-2019-10-19_124520_ack.csv',
    'OPC UA Pub/Sub, borker-mode MQTT, RTT ACK', prefix="SAMPLE30000")

#drawChart(
#    'test_run_2/MQTT/opcua-ps-2019-10-19_124520_echo.csv',
#    'OPC UA Pub/Sub, borker-mode MQTT, RTT ECHO')

#drawChart(
#    'test_run_2/OPCUA/opcua-ps-2019-10-19_125652_ack.csv',
#    'OPC UA Pub/Sub, borkerless, RTT ACK')

#drawChart(
#    'test_run_2/OPCUA/opcua-ps-2019-10-19_125652_echo.csv',
#    'OPC UA Pub/Sub, borkerless, RTT ECHO')
