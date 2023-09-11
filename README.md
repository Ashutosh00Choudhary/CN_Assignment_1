# CN_Assignment_1
Authors: Ashutosh Choudhary & Chaitanya Rao

How to implement the repository in your system

Install the following dependencies:

libpcap - https://github.com/the-tcpdump-group/libpcap/blob/master/INSTALL.md

Clone the repository: https://github.com/Ashutosh00Choudhary/CN_Assignment_1

git clone 

Compile the C file:

gcc -o pcap_parser pcap_parser.c -lpcap

Run the C file:

./pcap_parser <file_name>.pcap

Run .pcap file:

sudo tcpreplay -i <network_interface> --mbps= <path_to_pcap_file>


If possible, disconnect your wired ethernet connection in your VM while replaying the packets using tcpreplay.

For reverse DNS lookup, run the vi file(dig shell script):

vi <file_name>
