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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "tools/xmlRead.h"
#include "tester/testing.h"


	//variables for the xml-File
	//make them global so we can use them everywhere
char stunServer[255];
char HelpersPublicIP[255];
int HelpersPublicPort;
int HelpersPublicPort2;
int localTestingPort;

char* device;

using namespace std;

	///defined here
	///all other files use "extern int verbose"
int verbose;

void usage() {
	printf("usage: ./NATHelper verbose(1 or 0)\n\n");
	exit(0);
}


	/* parse the xml File and get all config variables */
int init_context(char* config_file) {
	char HelpersPublicPortc[255] = {0};
	char HelpersPublicPortc2[255] = {0};
	char localTestingPortc[255] = {0};
	
	openXML(config_file);	
	if( 0 != getXMLAttrib("/NATS_Framework/NATS_config", "StunServer", stunServer, 1, NULL)) return -2;
	if( 0 != getXMLAttrib("/NATS_Framework/NATS_config", "HelpersPublicIP", HelpersPublicIP, 1, NULL)) return -2;
	if( 0 != getXMLAttrib("/NATS_Framework/NATS_config", "HelpersPublicPort", HelpersPublicPortc, 1, NULL)) return -2;
	if( 0 != getXMLAttrib("/NATS_Framework/NATS_config", "HelpersPublicPort2", HelpersPublicPortc2, 1, NULL)) return -2;
	if( 0 != getXMLAttrib("/NATS_Framework/NATS_config", "localTestingPort", localTestingPortc, 1, NULL)) return -2;

		//cast vars to int
	HelpersPublicPort = strtol(HelpersPublicPortc, NULL, 10);
	HelpersPublicPort2 = strtol(HelpersPublicPortc2, NULL, 10);
	localTestingPort = strtol(localTestingPortc, NULL, 10);


	if (verbose) {	
		printf("***************************************\n");
		printf("XML-CONFIG\n");
		printf("Using Helper at: %s:%i....local Port: %i\n", HelpersPublicIP, HelpersPublicPort, localTestingPort);
		printf("***************************************\n\n");
	}

	destroyXML(NULL);
	return 0;
}


int main(int argc, char *argv[])
{
	verbose = 0;
		//usage: verbose(0 or 1)
	if ( argc < 2 ) {
		printf("usage: NatTester device(e.g. eth0 or wlan0)\n");
		exit(0);
	}
	device = argv[1];



		//open XML-Config-File	
	init_context("framework.cfg");

		//start Listener
	startTester();

	return 0;
}
