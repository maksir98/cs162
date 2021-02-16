#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2)
    {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], "ftp", &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP address for %s: \n\n", argv[1]);

    for ( p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char *ipver;

        unsigned short int port;

        if (p->ai_family == AF_INET) /* ipv4 */
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            port = (int) ntohs(ipv4->sin_port);
            ipver = "IPv4";
        } else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        
        
        printf("\t%s: %s port: %d\n", ipver, ipstr, port);
    }  
    freeaddrinfo(res);

    return 0;
}
