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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/sctp.h>


int testSCTP(){
	extern char HelpersPublicIP[255];

	char buf[1024];
	char *msg = "hello\n";

	int sockfd;
	int nread;
	struct sockaddr_in serv_addr;
	fd_set myset; 
	struct timeval tv; 
		 
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	long arg;
	 
	if (sockfd < 0) {
		perror("socket creation failed");
		exit(2); }

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(HelpersPublicIP);
		serv_addr.sin_port = htons(50000);
		
		// Set non-blocking	
	arg = fcntl(sockfd, F_GETFL, NULL); 
	arg |= O_NONBLOCK; 
	fcntl(sockfd, F_SETFL, arg); 

	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	tv.tv_sec = 10; 
	tv.tv_usec = 0; 
	FD_ZERO(&myset); 
	FD_SET(sockfd, &myset); 
	select(sockfd+1, NULL, &myset, NULL, &tv);

	if (FD_ISSET(sockfd,&myset)) {
		send(sockfd, msg, strlen(msg) + 1, 0);
		recv(sockfd, msg, strlen(msg)+1, 0);
				
		printf("SCTP Test successful, server returned: %s \n", msg);		
		close(sockfd);
		return 1;
	}  
	else {
		printf("timeout! SCTP Test not successful \n");		
		close(sockfd);
		return 0;			
	}
	return -1;

}

