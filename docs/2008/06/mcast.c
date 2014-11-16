#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <pcap.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/sockio.h>

#define INTERFACE "pcn0"
#define MYIP "10.10.3.197"
#define IPPROTO_CARP 112

void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
   printf("received carp packet\n");
}

int main(void)
{
    pcap_t *pcapfd;
    char errbuf[PCAP_ERRBUF_SIZE];
    char rule[256];
    struct bpf_program bpfp;
    int fd;
    struct ip_mreq mreq;   
    struct in_addr srcip;
    int fd2;

    /* pcap */
    if ((pcapfd = pcap_open_live(INTERFACE, 1518, 0, 1000, errbuf)) == NULL) {
        perror("pcap open");
        exit(1);
    }    

    snprintf(rule, sizeof rule, "ether multicast and proto %u and src host not %s",
             (unsigned int) IPPROTO_CARP, MYIP);
    if (pcap_compile(pcapfd, &bpfp, rule, 1, (bpf_u_int32) 0) != 0) {
        perror("compiling pcap rule");
        exit(1);
    }
    pcap_setfilter(pcapfd, &bpfp);

    /* join mcast group */
    /*if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {*/
    if ((fd = socket(PF_INET, SOCK_RAW, IPPROTO_CARP)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&mreq, 0, sizeof mreq);
    mreq.imr_multiaddr.s_addr = inet_addr("224.0.0.18");
    inet_pton(AF_INET, MYIP, &srcip);
    mreq.imr_interface.s_addr = srcip.s_addr; /* or maybe INADDR_ANY */
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		   &mreq, sizeof mreq) < 0) {
        perror("joining multicast group");
        exit(1);
    }

#if 0
    /* sniff */
    pcap_loop(pcapfd, -1, got_packet, NULL);
#endif

    {
        struct sockaddr_in addr;
        int addrlen = sizeof(addr);
        int nbytes;
        char msgbuf[256];

        for (;;) {
            if ((nbytes = recvfrom(fd, msgbuf, sizeof msgbuf, 0, (struct sockaddr *) &addr, &addrlen)) < 0) {
                perror("recvfrom");
                exit(1);
    	  }
          printf("received %d bytes\n", nbytes);
    	  /*puts(msgbuf);*/
        }
    }

    return 0;
}
