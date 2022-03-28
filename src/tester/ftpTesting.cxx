/********************************************************************************
*                                                                               *
* Copyright (c) 2007, Computer Networks and Internet, University of Tübingen,   *
* Sand 13, 72076 Tübingen, Germany                                              *
*                                                                               *
* All rights reserved.                                                          *
*                                                                               *
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU              *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA       *
*                                                                               *
********************************************************************************/

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
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sys/ioctl.h>
#include <net/if.h>

extern char* device;


void error(char *msg)
{
    perror(msg);
    exit(1);
}


char* replace(const char* s, const char* find, const char* replace)
{
	const char* p;
	char* ret;

	p = strstr(s, find);

	if (p == NULL)
		return NULL;

	ret = (char*)malloc(strlen(s) + strlen(replace) + 1 - strlen(find));

	if (ret == NULL)
		return NULL;

	if (p != s)
		strncpy(ret, s, p-s);

	strcpy(ret + (p-s), replace);
	strcat(ret, p + strlen(find));

	return ret;
}

char* getIP(){

	struct ifreq ifa;
	struct sockaddr_in *i;
	int fd;

	strcpy (ifa.ifr_name, device);

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror ("socket");
		exit (EXIT_FAILURE);
	}

	if(ioctl(fd, SIOCGIFADDR, &ifa)) {
		return "0";
	}

	i = (struct sockaddr_in*)&ifa.ifr_addr;
	close(fd);
	return inet_ntoa(i->sin_addr);
}



void ftpALG()
{
 int newsockfd;
 int laenge;
 int anzahl;
 int ergebnis;
 struct sockaddr_in adresse;
 char empfangene_zeichen[65000];

 unsigned short int portnummer = 21;
 char ip_adresse[] = "131.159.15.247";

 newsockfd = socket(AF_INET, SOCK_STREAM, 0);
 
 adresse.sin_family = AF_INET;
 adresse.sin_addr.s_addr = inet_addr(ip_adresse);
 adresse.sin_port = htons(portnummer);
 laenge = sizeof(adresse);
 
 ergebnis = connect(newsockfd, (struct sockaddr *)&adresse, laenge);
 
 //printf("\nConnection to %s",ip_adresse);
 //printf(" Port %d",portnummer);

 if (ergebnis == -1)
   {
    perror("Keine Verbindung erfolgt ");
   }
 else
   {
    //printf("\nestablished connection... fake FTP\n");

int n = 0;
char buffer[256];
//initial
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     //printf("ok %s\n",buffer);

//user
     n = write(newsockfd,"USER nattester\r\n",strlen("USER nattester\r\n"));
     if (n < 0) error("ERROR writing to socket");
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     //printf("ok %s\n",buffer);

//pw
     n = write(newsockfd,"PASS nattester\r\n",strlen("PASS nattester\r\n"));
     if (n < 0) error("ERROR writing to socket");
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     //printf("ok %s\n",buffer);

//syst
     n = write(newsockfd,"SYST\r\n",strlen("SYST\r\n"));
     if (n < 0) error("ERROR writing to socket");
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     //printf("ok %s\n",buffer);


	 //get local IP and replace . by ,
	char* ip = replace(getIP(), ".", ",");
	ip = replace(ip, ".", ",");
	ip = replace(ip, ".", ",");

	char to_send1[255]={0};
	char to_send2[255]={0};
	sprintf(to_send1,"PORT %s,67,48\r\n\0",ip);



//port
     n = write(newsockfd,to_send1, strlen(to_send1));
     if (n < 0) error("ERROR writing to socket");
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     //printf("ok %s\n",buffer);

//port
	  sprintf(to_send2,"LIST %s,67,48\r\n\0",ip);
     n = write(newsockfd,to_send2,strlen(to_send2));

   }  

	printf("now trying to use FTP-ALG to receive a message behind the NAT...\n");
   
 close(newsockfd);  

}
