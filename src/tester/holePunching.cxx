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
#include <fcntl.h>
#include <iostream>
#include <vector>		

void punchHoleUDP(int sourcePort, int destPort, char* destIP) {
	int sock;                     	/* socket details */
	const int BUF_LEN=10000;         /* Buffer size for transfers */
	struct sockaddr_in address;      /* socket address stuff */
	struct sockaddr_in local;
	struct hostent * host;           /* host stuff */
	int err;                         /* error trapping */
	char File_Buf[BUF_LEN];          /* file buffer */
  	    
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) 
		perror("ERROR opening socket");

	local.sin_family=AF_INET;
	local.sin_port=htons(sourcePort);
	local.sin_addr.s_addr=INADDR_ANY;
	bind(sock,(struct sockaddr*)&local,sizeof(sockaddr_in));

	address.sin_family=AF_INET;
	address.sin_port=htons(destPort);
	address.sin_addr.s_addr=inet_addr(destIP);

	
	strcpy(File_Buf, "lalala");
	err = sendto(sock, File_Buf,strlen(File_Buf),0,(sockaddr*)&address,sizeof(sockaddr_in)); /* send testing request */
	close(sock);

}

void punchHoleTCP(int sourcePort, int destPort, char* destIP, int ttl_value) {

	int tcp_socket;
	struct sockaddr_in peer, local;
	long arg;
	//int ttl_value = 3;

	peer.sin_family = AF_INET;
	peer.sin_port = htons(destPort);
	peer.sin_addr.s_addr = inet_addr(destIP);

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) 
		perror("ERROR opening socket");

	arg = fcntl(tcp_socket, F_GETFL, NULL); 
	arg |= O_NONBLOCK; 
	fcntl(tcp_socket, F_SETFL, arg); 

	local.sin_family=AF_INET;
	local.sin_port=htons(sourcePort);
	local.sin_addr.s_addr=INADDR_ANY;
	bind(tcp_socket,(struct sockaddr*)&local,sizeof(sockaddr_in));

	setsockopt(tcp_socket, IPPROTO_IP, IP_TTL, (char*)&ttl_value, sizeof(ttl_value) );
	
	connect(tcp_socket, (sockaddr*)&peer, sizeof(sockaddr_in));
	close(tcp_socket);
}
