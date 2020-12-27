use strict;
use warnings;
use autodie;
use Data::Dumper qw(Dumper);
my $nodes_count=100;

my $resultdir="/home/anirudha/final_year_project"; #Result directory path
my $logfile="/home/anirudha/loglistener.txt";#final_year_project/mrhof100log.txt"; #Logfile

energyconsumption($logfile,$resultdir,$nodes_count);
networksetuptime($logfile,$resultdir);
networktraffic($logfile,$resultdir);
networklatency_reliability($logfile,$resultdir);


sub energyconsumption{
	my $resultlog="$_[1]/network_energy_consumption.txt";
	my $logfile=$_[0];
	open (my $fh_resultlog,">>", $resultlog)or die $!;
	open (my $fh_logfile,$logfile) or die $!;

	my ($totalcpu,$totallpm, $totalTransmit,$totalListen)=(0,1,0,0);
	my $totalnodes=$_[2];
	#my $line;
	foreach my $line (<$fh_logfile>)
	{
		#printf $line;
		if($line =~ m/\d+:\d+:\w*\s+\d+\s+P\s+\d+\.\d+\s(\d+\s){7}(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/)
		{
			print $line;
			$totalcpu=$totalcpu+$2;
			$totallpm=$totallpm+$3;
			$totalTransmit=$totalTransmit+$4;
			$totalListen=$totalListen+$5;
		}
	}
	printf "ENERGY Consumption\n";
	printf "=============\n";
	printf"Nodes"."\t"."Total Transmit ticks"."\t"."Total Listen ticks"."\t". "Total Consumption(ticks)\tTotal cpu\tTotal lpm\tTotal Time\tRadio ON Time\n";
	my $row =sprintf "%-7s %-23s %-23f %-31f %-15f %-15f %-23f %-23.3f\n", $totalnodes, $totalTransmit, $totalListen, $totalTransmit + $totalListen, $totalcpu, $totallpm, $totalcpu + $totallpm, (($totalTransmit + $totalListen)/($totalcpu + $totallpm)) * 100;
	printf $row;
	$row = sprintf "%-.3f\r\n", (($totalTransmit + $totalListen)/($totalcpu + $totallpm)) * 100;
	print $fh_resultlog $row;
	printf "\n\n";
	close $fh_resultlog;
	close $fh_logfile;
}

#13297	ID:20	RPL: Sending a multicast-DIO with rank 844
#13340	ID:12	RPL: Sending a multicast-DIO with rank 718
#13430	ID:4	RPL: Sending a multicast-DIO with rank 844


#6287	ID:2	RPL: Joined DAG with instance ID 30, rank %hu, DAG ID 
#6327	ID:7	RPL: Joined DAG with instance ID 30, rank %hu, DAG ID 
#6434	ID:13	RPL: Joined DAG with instance ID 30, rank %hu, DAG ID 

sub networksetuptime{
	my $resultlog="$_[1]/network_setup_time.txt";
	my $logfile=$_[0];
	open (my $fh_resultlog,">>", $resultlog)or die $!;
	open (my $fh_logfile,$logfile) or die $!;
	my %Convergence=();
	my $hashCount=0;
	my $ConvergenceTime=0;
	my $ConvergFlag=1;
	my ($firstDIOsent,$lastDIOjoinedDAG)=0;
	foreach my $line (<$fh_logfile>)
	{
		#7211949:1:RPL : Prefix announced in DIO // RPL: Sending multicast-DIO with rank
		if ($line =~ m/(\d+)(\s)ID:\d+(\s)RPL: Sending unicast-DIO with rank (\d+)/ or $line =~ m/(\d+)(\s)ID:\d+(\s)RPL: Sending a multicast-DIO with rank (\d+)/)
		{
			if ($firstDIOsent eq 0)
			{
				print $line;
				$firstDIOsent = $1;
			}
		}
		else 
		{
			#4571266:6:RPL: Joined DAG with instance ID
			if ($line=~m/(\d+)(\s)ID:(\d+)(\s)RPL: Joined DAG with instance ID (\d+), rank %hu, DAG ID /)
			{
				#print $line;
				$lastDIOjoinedDAG=$1;
			}
		}
		
	}
	printf "NETWORK SETUP TIME \n";
	printf "============\n";
	printf "First DIO" . "\t" . "Last DIO joined DAG \t Setup Time(ms) \n";
	my $row = sprintf "%-.0f, %-.0f, %-.0f\r\n", $firstDIOsent,$lastDIOjoinedDAG,$lastDIOjoinedDAG-$firstDIOsent;
	# $row=sprintf "%-.0f, %-.0f, %-.0f\r\n", $firstDIOsent,$lastDIOjoinedDAG,$lastDIOjoinedDAG-$firstDIOsent;
	printf $row;
	my $row1=sprintf "%-.0f\r\n", $lastDIOjoinedDAG-$firstDIOsent;
	print $fh_resultlog $row1;
	printf "\n";
	close $fh_resultlog;
	close $fh_logfile;	
}

sub networktraffic {
    my $logfile=$_[0];
	# set the output (result) file name 
	my $resultlog="$_[1]/network_traffic.txt"; 
	open (my $fh_resultlog,">>",$resultlog) or die $!;
	open (my $fh_logfile,$logfile) or die $!;
	#print $logfile;
	# General Idea: Find the corresponding lines and get the value of DIS,DIO,5A0
	my ($DIOsent, $DISsent, $DAOsent) = (0,0,0); 
	#my %hash_dio=();
	foreach my $line(<$fh_logfile>){
		
		if ($line =~ m/(\d+)(\s)ID:\d+(\s)RPL: Sending unicast-DIO with rank (\d+)/ or $line =~ m/(\d+)(\s)ID:\d+(\s)RPL: Sending a multicast-DIO with rank (\d+)/){
			#print $line;
			$DIOsent = $DIOsent + 1;
			#$hash_dio{$1} = $hash_dio{$1} + 1;
		} else {
			if ($line=~ m/(\d+)(\s)ID:\d+(\s)RPL: Sending a DIS to/) {
				#print $line;
				$DISsent = $DISsent + 1;
			} else {
				if ($line=~ m/(\d+)(\s)ID:\d+(\s)RPL: Sending DAO with prefix/) {
					#print $line;
					$DAOsent = $DAOsent + 1;
				}
			}
		}
	}
	printf "\nNETWORK TRAFFIC\n";
	printf 	"==============\n";
	printf "DIO\t\tDIS\t\tDAO\n";
	my $row = sprintf "%-15d %-15d %-15d\n" ,$DIOsent, $DISsent, $DAOsent ; 
	printf $row;
	$row = sprintf "%-d, %-d, %-d \r\n" ,$DIOsent, $DISsent, $DAOsent ;
	print $fh_resultlog $row;
	printf "\n\n";
	close $fh_resultlog; close $fh_logfile
}

#*****************


#Network Latency
{# start of outer block
	my %send=(); my $num = 0;
	sub saveSendTime {
		# save nodenr,packetnr, time
		my $nodenr= $_[0]; my $packetnr= $_[1]; my $time = $_[2];
		if (exists $send{$nodenr}) {
			# if the element exists in send hash then add it to the 2nd hash only 
			$send{$nodenr}->{$packetnr} = $time;
		} else {
			# if the element does not exist in send hash, then add to both hashes 
			$send{$nodenr}= {$packetnr => $time};
		}
	}
	sub CleanHash {
		for (keys %send)
		{
			delete $send{$_};
		}
	}
	sub printLostPackets { 
		my	$lpackets = 0;
		foreach my $out (sort {$a <=> $b} keys %send) {
			print "i: $out \n";
			foreach my $key ( sort {$a <=> $b} keys %{$send{$out}}) {
				# printf "node: ;out packet: Skey time: csend{Out}{5key}\n"; 
				$lpackets = $lpackets + 1;
				print "j: $key\n";
			}
		}
		return $lpackets;
	}

	sub lookupSendTime{
		my $nodenr= $_[0]; my $packetnr= $_[1]; my $time = $_[2]; 
		# look
		if (exists $send{$nodenr}{$packetnr}) {
			my $sendTime = $send{$nodenr}{$packetnr}; # for compute latency 
			# if matches then delete, no need to keep it any more 
			delete($send{$nodenr}{$packetnr});
			return($sendTime);
		}
		return(-1);
	}
	print Dumper(\%send);
} # end of outer block


sub networklatency_reliability {
	CleanHash();
	my $resultlog="$_[1]/network_latency.txt";
	my $logfile=$_[0];
	# set the output (result) file name
	open (my $fh_resultlog,">>", $resultlog)or die $!;
	open (my $fh_logfile,$logfile) or die $!;
	my ($nodenr, $packetnr, $time, $sendTime)=0;  #, $noSendPackets, $noRecvPackets, $totalLatency, $lostpackets, $counter1, $counter2) = 0;
    my ($noSendPackets) = 0;
	my ($noRecvPackets) = 0;
	my ($totalLatency) = 0;
	my ($counter1) = 0;
	my ($counter2) = 0;
	my ($lostpackets) = 0;
	foreach my $line(<$fh_logfile>){	# line can be either sending or receiving
		#printf $line;
		if ($line=~ m/(\d+)(\s)ID:(\d+)(\s)DATA send to (\d+) 'Hello'/) {
			# print $1;
			$nodenr=$3; $packetnr=$5; $time = $1; # save nodenr,packetnr, time
            printf "node:$nodenr, packetnr:$packetnr, time:$time \n";
			$noSendPackets = $noSendPackets + 1;# save this sending time of each packet to the hash %send 
            printf "noSendPackets:$noSendPackets \n";			
            $counter1= $counter1 +1;
			saveSendTime($nodenr, $packetnr, $time );
		} 

#62706:ID:12:DATA send to 1 'Hello 1' ----$nodenr=$2; $packetnr=$3; $time = $1;
#62814:ID:1:DATA recv 'Hello 1 from the client' from 12 ---$nodenr=$4; $packetnr=$2; $time = $1;
#63013:ID:8:DATA send to 1 'Hello 1'
#63060:ID:1:DATA recv 'Hello 1 from the client' from 8
#63467:ID:20:DATA send to 1 'Hello 1'
#63556:ID:1:DATA recv 'Hello 1 from the client' from 20
#63631:ID:16:DATA send to 1 'Hello 1'

#LATENCY2.PL READS DATA in this form
#1867970	ID:1	DATA recv 'Hello 31 from the client' from 14   
#1875052	ID:2	RPL: Sending a multicast-DIO with rank 512
#1876176	ID:6	RPL: Sending a multicast-DIO with rank 640
#1879170	ID:7	DATA send to 1 'Hello 31'                    $nodenr=$3   here it is 7 ; $packetnr=$5  here 1 ; $time = $1 here 1879170;
#1879336	ID:1	DATA recv 'Hello 31 from the client' from 7
#1884613	ID:15	DATA send to 1 'Hello 31'                  node:15, packet:1, time:1884613 
#1884766	ID:1	DATA recv 'Hello 31 from the client' from 9     node:9, packet:1 , time:1884766   $nodenr=$6; $packetnr=$3; $time = $1;   
#1885132	ID:1	DATA recv 'Hello 31 from the client' from 15
#1888696	ID:18	DATA send to 1 'Hello 31'
#88279	ID:16	DATA send to 1 'Hello'
#88409	ID:1	30 0 87 0 4112 1 1 0 22 11204 0 4075 40704 229 564 257 128 844 1 131 252 1 189 182 65535 65535 0 0 0 0
#88411	ID:1	DATA recv from 16 ''
#93612	ID:13	DATA send to 1 'Hello'
#93685	ID:1	30 0 93 0 3341 1 1 0 22 11834 0 4927 42379 234 626 257 128 684 1 131 252 1 189 182 65535 65535 0 0 0 0
#93687	ID:1	DATA recv from 13 ''
   
    	else { #printf $line;
            if ($line=~ m/(\d+)(\s)ID:(\d+)(\s)DATA recv from (\d+)/) { 
				$nodenr=$5; $packetnr=$3; $time = $1; # save nodenr,packetnr, time
                printf "node:$nodenr, packetnr:$packetnr , time:$time \n";
				# check if send table has a corresponding sendTime, if yes(ofcourse) then calculate latency 
				$sendTime = lookupSendTime($nodenr, $packetnr,$time);
			    if ( $sendTime > -1) {
					# we have a match in sendTable
					$noRecvPackets = $noRecvPackets + 1;
                    printf "noRecvPackets: $noRecvPackets \n";

					$totalLatency = $totalLatency + ($time - $sendTime);
                    printf "latency: $totalLatency \n";
                    $counter2 =$counter2 +1;
				}
			}
		} 
    }# end of foreach
	# printf "lost packets are:\n"; 
	$lostpackets = printLostPackets(); 
	printf "NETWORK LATENCY & PDR\n"; 
	printf "=====================\n";
	my $row = "no of SendPackets\tAverage Latency(ms)\tPDR\tLost Packets\n";
	printf $row;
	# total packets sent, average latency in ms, PDR, lost packets
	$row = sprintf "%d,\t%-.2f,\t%-.2f,\t%d\r\n" ,$noSendPackets , ($totalLatency / ($noRecvPackets)), (($noSendPackets - $lostpackets)/$noSendPackets)*100,$lostpackets;
	printf $row;
	print $noSendPackets - $noRecvPackets;
	print $fh_resultlog $row;
	close $fh_resultlog; close $fh_logfile;
}




