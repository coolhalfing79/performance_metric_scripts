'''
calculates rpl metrics from contiki cooja logfiles
'''
import csv
import sys
import os
import re

NODES = 0


def main(argv):
    'main function'
    try:
        logfile = argv[1]
        resultfile = argv[2]
    except IndexError:
        print('no log files provided')
    if 'resultdir' not in os.listdir():
        os.mkdir('resultdir')
    resultdir = './resultdir'

    boldline = '=' * (len(logfile)+16)
    thinline = '-' * (len(logfile)+16)
    print(boldline)
    print('RESULT DIRECTORY: ', resultdir)
    print('USING LOGFILE: ', logfile)
    print(boldline)

    metrics = (
        (networksetuptime(logfile), 'NETWORK SETUP TIME'),
        (network_latency_reliability(logfile), 'NETWORK LATENCY & PDR'),
        (energyconsumption(logfile), 'ENERGY CONSUMPTION'),
        (networktraffic(logfile), 'NETWORK TRAFFIC')
    )

    result = {'node count': NODES}
    for metric, name in metrics:
        result.update(metric)
        print('\n'+boldline+'\n'+name+'\n' + thinline)
        for key, value in metric.items():
            print(key, '\t:', value)

    resultfile = resultdir + f'/{resultfile}'
    with open(resultfile, 'a') as csvfile:
        csvwriter = csv.DictWriter(csvfile, result.keys())
        if not os.stat(resultfile).st_size:
            csvwriter.writeheader()
        csvwriter.writerow(result)


def networksetuptime(logfile):
    """ calculates network setup time as last DIO joined - first DIO sent"""
    node_count = 0
    metric = {
        'first DIO sent': 0,
        'last DIO joined DAG': 0,
        'network setup time': 0
    }
    with open(logfile, 'r') as readfile:
        for line in readfile:
            txt = re.findall(r'\d+', line)
            if 'RPL' in line:
                if len(txt) < 4:
                    time = int(txt[0])
                else:
                    time = int(txt[0]) * 60000 + \
                        int(txt[1]) * 1000 + int(txt[2])
                if '-DIO with rank' in line:
                    if metric['first DIO sent'] == 0:
                        metric['first DIO sent'] = time
                elif 'RPL: Joined DAG with instance ID' in line:
                    node_count += 1
                    metric['last DIO joined DAG'] = time
    metric['network setup time'] = metric['last DIO joined DAG'] - \
        metric['first DIO sent']

    global NODES
    NODES += node_count

    return metric


def networktraffic(logfile):
    """calculates number of DIO DIS and DAO sent"""
    metric = {
        'DIO sent': 0,
        'DIS sent': 0,
        'DAO sent': 0
    }
    with open(logfile) as file:
        for line in file:
            if 'DIO with rank' in line:
                metric['DIO sent'] += 1
            elif 'RPL: Sending a DIS to' in line:
                metric['DIS sent'] += 1
            elif 'RPL: Sending DAO with prefix' in line:
                metric['DAO sent'] += 1

    return metric


def network_latency_reliability(logfile):
    """calculates PDR, average latency, send and recieved packets """
    metric = {
        'PDR': 0,
        'lost packets': 0,
        'sent packets': 0,
        'recieved packets': 0,
        'average latency': 0
    }
    latency = 0
    message_data = {}
    with open(logfile) as file:
        for line in file:
            txt = re.findall(r"\d+", line)
            if 'DATA' in line:
                time = int(txt[0])*60000 + int(txt[1])*1000 + int(txt[2])
                if 'send to' in line:
                    metric["sent packets"] += 1
                    try:
                        message_data[txt[-2]][txt[-1]] = time
                    except KeyError:
                        message_data[txt[-2]] = {txt[-1]: time}
                    #print('sent', txt[-3], '->', txt[-2])
                elif 'recv ' in line:
                    try:
                        sendtime = message_data[txt[-1]][txt[-2]]
                        metric["recieved packets"] += 1
                        latency += time - sendtime
                    except KeyError:
                        pass
                #print('recv', txt[-2], '<-', txt[-3])

    metric['lost packets'] = metric["sent packets"] - \
        metric["recieved packets"]
    try:
        metric['average latency'] = latency/metric["recieved packets"]
        metric['PDR'] = (metric["sent packets"] -
                         metric['lost packets'])/metric["sent packets"] * 100
    except ZeroDivisionError:
        sys.exit('wrong log file')  # wrong log file may corrupt data

    return metric


def energyconsumption(logfile):
    """
    energy comsumption is obtained by using powertrace
    powertrace and collect view cannot work simultaneousy
    without causing overflow so this is done separately and
    saved in a separate log file manually
    """
    metric = {'total transmit ticks': 0,
              'total listen ticks': 0,
              'total consumption': 0,
              'total cpu': 0,
              'total lpm': 0,
              'radio on time': 0}
    with open(logfile) as infile:
        for row in infile:
            if '#E' in row:
                digits = re.findall(r'\d+', row)
                metric['total cpu'] += int(digits[4])
                metric['total lpm'] += int(digits[5])
                metric['total transmit ticks'] += int(digits[6])
                metric['total listen ticks'] += int(digits[7])

    try:
        metric['radio on time'] += ((metric['total transmit ticks'] + metric['total listen ticks'])/(
            metric['total cpu'] + metric['total lpm'])) * 100
    except ZeroDivisionError:
        sys.exit("wrong energy log file")  # wrong log file may corrupt data

    metric['total consumption'] += metric['total transmit ticks'] + \
        metric['total listen ticks']

    return metric


if __name__ == "__main__":
    main(sys.argv)
