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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <sys/wait.h>
#include <iostream>

int send_request(const int sock, const char *hostname, char *requestParameters)
{
    char request[4096];

    snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: Close\r\n\r\n", requestParameters, hostname);

    if (send(sock, request, strlen(request), 0) == -1)
    {
        perror("send() failed");
        return 1;
    }
    return 0;
}



int updateDB(char* request)
{
    struct hostent *host;
    struct sockaddr_in addr;
    int s;

   
    host = gethostbyname("gex.cs.uni-tuebingen.de");

    addr.sin_addr = *(struct in_addr*)host->h_addr;
   

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        perror("socket() failed");
        return 3;
    }

    addr.sin_port = htons(80);
    addr.sin_family = AF_INET;

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("connect() failed");
        return 4;
    }

    if (send_request(s, "gex.cs.uni-tuebingen.de", request ))
        return 5;

    close(s);

    return 0;
}
