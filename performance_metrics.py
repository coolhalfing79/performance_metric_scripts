'''
calculates rpl metrics from contiki cooja logfiles
'''
import sys
import os
import re


def main(argv):
    'main function'
    try:
        logfile = argv[1]
        energylogfile = argv[2]
    except IndexError:
        logfile = '/home/anirudha/final_year_project/mrhof100log.txt'
        energylogfile = '/home/anirudha/final_year_project/test.txt'

    if 'resultdir' not in os.listdir():
        os.mkdir('resultdir')
    resultdir = './resultdir'

    boldline = '='* (len(logfile)+16)
    thinline = '-'* (len(logfile)+16)
    print(boldline)
    print('RESULT DIRECTORY: ', resultdir)
    print('USING LOGFILE: ', logfile)
    print(boldline)

    metrics = (
        (network_latency_reliability(logfile, resultdir), 'NETWORK LATENCY & PDR'),
        (energyconsumption(energylogfile, resultdir), 'ENERGY CONSUMPTION'),
        (networksetuptime(logfile, resultdir), 'NETWORK SETUP TIME'),
        (networktraffic(logfile, resultdir), 'NETWORK TRAFFIC')

    )

    for metric, name in metrics:
        print('\n'+boldline+'\n'+name+'\n' + thinline)
        for key, value in metric.items():
            print(key, '\t:', value)


def networksetuptime(logfile, resultdir):
    """ calculates network setup time as last DIO joined - first DIO sent"""
    resultlog = resultdir + '/setup_time.txt'
    metric = {
        'node count': 0,
        'first DIO sent': 0,
        'last DIO joined DAG': 0
    }
    with open(logfile, 'r') as readfile:
        for line in readfile:
            txt = re.findall(r'\d+', line)
            time = int(txt[0]) * 60000 + int(txt[1]) * 1000 + int(txt[2])
            if '-DIO with rank' in line:
                if metric['first DIO sent'] == 0:
                    metric['first DIO sent'] = time
            elif 'RPL: Joined DAG with instance ID' in line:
                metric['node count'] += 1
                metric['last DIO joined DAG'] = time

    with open(resultlog, 'w') as resultfile:
        resultfile.write(
            f'{metric["first DIO sent"] - metric["last DIO joined DAG"]}\n')

    return metric


def networktraffic(logfile, resultdir):
    """calculates number of DIO DIS and DAO sent"""
    metric = {
        'DIO sent': 0,
        'DIS sent': 0,
        'DAO sent': 0
    }
    resultlog = resultdir + '/traffic.txt'
    with open(logfile) as file:
        for line in file:
            if 'DIO with rank' in line:
                metric['DIO sent'] += 1
            elif 'RPL: Sending a DIS to' in line:
                metric['DIS sent'] += 1
            elif 'RPL: Sending DAO with prefix' in line:
                metric['DAO sent'] += 1

    with open(resultlog, 'w') as target:
        target.write(
            f'{metric["DIO sent"]}, {metric["DIS sent"]}, {metric["DAO sent"]}\n')

    return metric


def network_latency_reliability(logfile, resultdir):
    """calculates PDR, average latency, send and recieved packets """
    resultlog = resultdir + '/latency.txt'
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
            time = int(txt[0])*60000 + int(txt[1])*1000 + int(txt[2])
            if 'DATA send to' in line:
                metric["sent packets"] += 1
                try:
                    message_data[txt[3]][txt[4]] = time
                except KeyError:
                    message_data[txt[3]] = {txt[4]: time}
            elif 'DATA recv ' in line:
                try:
                    sendtime = message_data[txt[4]][txt[3]]
                    metric["recieved packets"] += 1
                    latency += time - sendtime
                except KeyError:
                    print(line)

    metric['lost packets'] = metric["sent packets"] - \
        metric["recieved packets"]
    try:
        metric['average latency'] = latency/metric["recieved packets"]
        metric['PDR'] = (metric["sent packets"] -
                     metric['lost packets'])/metric["sent packets"] * 100
    except ZeroDivisionError:
        sys.exit('wrong log file') # wrong log file may corrupt data 


    with open(resultlog, 'w') as file:
        file.write(f'{0},{1},{2},{3}'.format(
            metric["sent packets"],
            metric["average latency"],
            metric["PDR"],
            metric["lost packets"]))

    return metric


def energyconsumption(logfile, resultdir):
    """
    energy comsumption is obtained by using powertrace
    powertrace and collect view cannot work simultaneousy
    without causing overflow so this is done separately and
    saved in a separate log file manually
    """
    resultlog = resultdir + '/energy_consumption.txt'
    metric = {'total transmit ticks': 0,
              'total listen ticks': 0,
              'total comsumption': 0,
              'total cpu': 0,
              'total lpm': 0,
              'radio on time': 0}
    with open(logfile) as infile:
        for row in infile:
            if '#P' in row:
                digits = re.findall(r'\d+', row)
                metric['total cpu'] += int(digits[8])
                metric['total lpm'] += int(digits[9])
                metric['total transmit ticks'] += int(digits[10])
                metric['total listen ticks'] += int(digits[11])

    try:
        metric['radio on time'] += (metric['total transmit ticks'] + metric['total listen ticks'])/(
            metric['total cpu'] + metric['total lpm']) * 100
    except ZeroDivisionError:
        sys.exit("wrong log file") # wrong log file may corrupt data

    metric['total comsumption'] += metric['total transmit ticks'] + \
        metric['total listen ticks']
    with open(resultlog, 'w') as file:
        file.write(f'{metric["radio on time"]}')

    return metric


if __name__ == "__main__":
    main(sys.argv)
