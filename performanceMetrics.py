import re
import pprint
import sys


def main(argv):
    if len(argv) > 1:
        logfile = argv[1]
        resultdir = argv[2]
    else:
        resultdir = '/home/anirudha/final_year_project/results'
        logfile = '/home/anirudha/final_year_project/mrhof100log.txt'
    print('\nusing logfile: ', logfile)
    print('result directory: ', resultdir)
    energyconsumption(logfile, resultdir)
    networksetuptime(logfile, resultdir)
    networktraffic(logfile,resultdir)
    network_latency_reliability(logfile, resultdir)

def energyconsumption(logfile, resultdir):
    resultlog = resultdir + '/energy_consumption.txt'
    total_cpu, total_lpm, total_transmit, total_listen = 0, 0, 0, 0
    print('\nENERGY CONSUMPTION')
    print('==================')
    print('TODO')


def networksetuptime(logfile, resultdir):
    resultlog = resultdir + '/setup_time.txt'
    first_DIO_sent, last_DIO_joined_DAG = 0, 0
    node_count = 0
    with open(logfile, 'r') as readfile:
        for line in readfile:
            txt = re.findall(r'\d+', line)
            # print(txt)
            time = int(txt[0]) * 60000 + int(txt[1]) * 1000 + int(txt[2])
            if '-DIO with rank' in line:
                if first_DIO_sent == 0:
                    # print(line[:10])
                    first_DIO_sent = time
            elif 'RPL: Joined DAG with instance ID' in line:
                #print(line[:10])
                node_count += 1
                last_DIO_joined_DAG = time

    with open(resultlog, 'w') as resultfile:
        resultfile.write(str(last_DIO_joined_DAG-first_DIO_sent) + '\n')

    print('\nNETWORK SETUP TIME')
    print('number of nodes: ', node_count)
    print('===================================================')
    print('First DIO sent\tLast DIO joined DAG\tSetup time')
    print('---------------------------------------------------')
    print(f'{first_DIO_sent}\t\t{last_DIO_joined_DAG}\t\t\t{last_DIO_joined_DAG - first_DIO_sent}')
    print('===================================================')

def networktraffic(logfile, resultdir):
    DIO_sent, DAO_sent, DIS_sent = 0, 0, 0
    resultlog = resultdir + '/traffic.txt'
    with open(logfile) as file:
        for line in file:
            if 'RPL: Sending unicast-DIO with rank' in line or 'RPL: Sending a multicast-DIO with rank' in line:
                DIO_sent += 1
            elif 'RPL: Sending a DIS to' in line:
                DIS_sent += 1
            elif 'RPL: Sending DAO with prefix' in line:
                DAO_sent += 1

    with open(resultlog, 'w') as target:
        target.write(f'{DIO_sent}, {DIS_sent}, {DAO_sent}\n')
    print('\nNETWORK TRAFFIC')
    print('=======================================')
    print('DIO sent\tDIS sent\tDAO sent')
    print('---------------------------------------')
    print(f'{DIO_sent}\t\t{DIS_sent}\t\t{DAO_sent}')
    print('=======================================')

def network_latency_reliability(logfile, resultdir):
    resultlog = resultdir + '/latency.txt'
    recieved_packets = 0
    latency = 0
    message_data = {}
    sent_packets = 0
    with open(logfile) as file:
        for line in file:
            txt = re.findall(r"\d+", line)
            # print(txt)
            time = int(txt[0])*60000 + int(txt[1])*1000 + int(txt[2])
            if 'DATA send to' in line:
                # print(line)
                sent_packets += 1
                try:
                    #print(txt[0], txt[1], txt[2], txt[3], txt[4], line)
                    message_data[txt[3]][txt[4]] = time
                except KeyError:
                    message_data[txt[3]] = {txt[4]: time}
            elif 'DATA recv from' in line:
                # print(line)
                try:
                    #print(txt[0], txt[1], txt[2], txt[3], txt[4], line)
                    sendtime = message_data[txt[4]][txt[3]]
                    #del(message_data[txt[-2]][txt[-3]])
                    recieved_packets += 1
                    latency += time - sendtime
                except KeyError as E:
                    print('fail')
                # print(line);;


    lost_packets = sent_packets - recieved_packets
    with open(resultlog, 'w') as file:
        file.write(f'{sent_packets},{latency/recieved_packets:.2f},{(sent_packets - lost_packets)/sent_packets * 100:.2f},{lost_packets}')
    print('\nNETWORK LATENCY & PDR')
    print('=============================================================')
    print('sent packets\taverage latency\tPDR\tlost packets')
    print('-------------------------------------------------------------')
    print(f'{sent_packets}\t\t{latency/recieved_packets:.2f}\t\t{(sent_packets - lost_packets)/sent_packets * 100:.2f}\t{lost_packets}')
    print('=============================================================')
    # pprint.pp(message_data)

if __name__ == "__main__":
    main(sys.argv)

'''
problems:
count of lost packets is different from perl script
'''
