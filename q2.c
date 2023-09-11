#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_PACKET_SIZE 70000
#define MAX_FLOWS 1000 // Maximum number of flows to track

// Structure to store TCP connection information
struct TcpConnection {
    char srcIpAddress[INET_ADDRSTRLEN];
    int srcPort;
    char destIpAddress[INET_ADDRSTRLEN];
    int destPort;
};

// Function to compare two connection tuples for equality
int areConnectionsEqual(const struct TcpConnection* conn1, const struct TcpConnection* conn2) {
    return strcmp(conn1->srcIpAddress, conn2->srcIpAddress) == 0 &&
           conn1->srcPort == conn2->srcPort &&
           strcmp(conn1->destIpAddress, conn2->destIpAddress) == 0 &&
           conn1->destPort == conn2->destPort;
}

// Function to identify TCP flags
void identifyTcpFlags(unsigned char flags) {
    printf("TCP Flags: ");
    if (flags & TH_FIN) printf("FIN ");
    if (flags & TH_SYN) printf("SYN ");
    if (flags & TH_RST) printf("RST ");
    if (flags & TH_PUSH) printf("PSH ");
    if (flags & TH_ACK) printf("ACK ");
    if (flags & TH_URG) printf("URG ");
    printf("\n");
}

// Function to process incoming packets with swapped source and destination IP and port
void processIncomingPacket(unsigned char* packetData, int packetSize, struct TcpConnection* flowArray, int* flowCount) {
    struct ip* ipHdr = (struct ip*)packetData;
    if (ipHdr->ip_p == IPPROTO_TCP) {
        struct tcphdr* tcpHdr = (struct tcphdr*)(packetData + ipHdr->ip_hl * 4);

        // Extract source and destination IP addresses
        char sourceIp[INET_ADDRSTRLEN];
        char destIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ipHdr->ip_dst), sourceIp, INET_ADDRSTRLEN); // Swap source and destination IP
        inet_ntop(AF_INET, &(ipHdr->ip_src), destIp, INET_ADDRSTRLEN);   // Swap source and destination IP

        // Extract source and destination ports
        int sourcePort = ntohs(tcpHdr->th_dport); // Swap source and destination port
        int destPort = ntohs(tcpHdr->th_sport);   // Swap source and destination port

        // Create a connection tuple for the current packet
        struct TcpConnection currentConnection;
        strcpy(currentConnection.srcIpAddress, sourceIp);
        currentConnection.srcPort = sourcePort;
        strcpy(currentConnection.destIpAddress, destIp);
        currentConnection.destPort = destPort;

        // Check if the flow already exists in the array
        int flowExists = 0;
        for (int i = 0; i < *flowCount; i++) {
            if (areConnectionsEqual(&currentConnection, &flowArray[i])) {
                flowExists = 1;
                break;
            }
        }

        // If the flow is new, add it to the array and increment the flow count
        if (!flowExists && *flowCount < MAX_FLOWS) {
            flowArray[*flowCount] = currentConnection;
            (*flowCount)++;
        }

        // Print the connection tuple with swapped source and destination
        printf("Connection Info: Source IP: %s, Source Port: %d, Destination IP: %s, Destination Port: %d\n",
               currentConnection.srcIpAddress, currentConnection.srcPort, currentConnection.destIpAddress, currentConnection.destPort);

        // Identify TCP flags
        identifyTcpFlags(tcpHdr->th_flags);
    }
}

int main() {
    int socketDesc;
    unsigned char packetBuffer[MAX_PACKET_SIZE];
    struct TcpConnection flowArray[MAX_FLOWS];
    int flowCount = 0;

    // Create a raw socket with superuser privileges
    socketDesc = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (socketDesc < 0) {
        perror("socket");
        return 1;
    }

    while (1) {
        int packetSize = recv(socketDesc, packetBuffer, sizeof(packetBuffer), 0);
        if (packetSize == -1) {
            perror("recv");
            break;
        }

        processIncomingPacket(packetBuffer, packetSize, flowArray, &flowCount);
        printf("Number of Unique Flows: %d\n", flowCount);
    }

    // Close the socket when done
    close(socketDesc);
    return 0;
}