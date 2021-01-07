'''
plots output of rpl metric logs
'''

import csv
import os
import numpy as np
import matplotlib.pyplot as plt

plt.style.use('seaborn')


labels = [
    'PDR', 'average latency', 'DIO sent', 'DAO sent',
    'DIS sent', 'CPU 1e7', 'lpm 1e8', 'comsumption 1e6', 'radio on time'
    ]
X = np.array([1, 3, 5, 7, 9, 11])
WIDTH = 0.5
os.chdir('./resultdir')
files = os.listdir()
for file in files:
    OF0 = []
    MRHOF = []

    N = OF0, MRHOF
    with open(file) as logfile:
        csvreader = csv.DictReader(logfile)
        fields = csvreader.fieldnames
        for row, nodes in zip(csvreader, N):
            nodes.append(float(row['PDR']))
            nodes.append(float(row['average latency']))
            nodes.append(float(row['DIO sent']))
            nodes.append(float(row['DAO sent']))
            nodes.append(float(row['DIS sent']))
            nodes.append(float(row['total cpu'])/1e7)
            nodes.append(float(row['total lpm'])/1e8)
            nodes.append(float(row['total consumption'])/1e6)
            nodes.append(float(row['radio on time']))
            nodes.append(float(row['total transmit ticks']))


    fig, ax = plt.subplots(3, 3)
    MET = 0
    for row in ax:
        fig.suptitle(file)
        for col in row:
            col.bar(X[:1] - WIDTH, OF0[MET:MET+1], WIDTH)
            col.bar(X[:1], MRHOF[MET:MET+1], WIDTH)
            #col.bar(X[:1] + WIDTH, N100[MET:MET+1], WIDTH)

            col.set_xticks(X[:1])
            col.set_xticklabels(labels[MET:MET+1])
            col.legend(['MRHOF', 'OF0'])
            MET += 1
plt.tight_layout()
plt.show()
