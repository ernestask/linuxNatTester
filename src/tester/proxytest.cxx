#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <sys/wait.h>

#define HTTP_PORT 80


int testforproxy(char* servIP) {

    struct sockaddr_in server;
    struct hostent *host_info;
    unsigned long addr;
    int sock;
    char buffer[8192];
    int count = 0;
    

    /* Erzeuge das Socket */
    sock = socket( PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror( "failed to create socket");
        exit(1);
    }

    /* Erzeuge die Socketadresse des Servers 
     * Sie besteht aus Typ, IP-Adresse und Portnummer */
    memset( &server, 0, sizeof (server));
    addr = inet_addr(servIP);
    memcpy( (char *)&server.sin_addr, &addr, sizeof(addr));

    server.sin_family = AF_INET;
    server.sin_port = htons( HTTP_PORT);

    /* Baue die Verbindung zum Server auf */
    if ( connect( sock, (struct sockaddr*)&server, sizeof( server)) < 0) {
        perror( "can't connect to server");
        exit(1);
    }

    /* Erzeuge und sende den http GET request */
    sprintf( buffer, "GET %s HTTP/1.0\r\n\r\n", "testimage.jpg");
    send( sock, buffer, strlen( buffer), 0);

    int filesize = 0;

    /* Hole die Serverantwort und gib sie auf Konsole aus */
    do {
        count = recv( sock, buffer, sizeof(buffer), 0);
	filesize += count;
    }
    while (count > 0);

    //printf("filesize: %i\n", filesize);

    close( sock);
    
    if (filesize < 247433)
	return 1;
    else
	return 0;

}
