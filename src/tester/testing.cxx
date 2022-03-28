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
#include <fstream>
#include <vector>		

#include "testing.h"
#include "ftpTesting.h"
#include "holePunching.h"
#include "../upnp/upnp.h"
#include "../udp/udp.h"
#include "../stun/stun.h"
#include "../tools/xmlRead.h"
#include "../tools/tools.h"
#include "sctpTesting.h"
#include "proxytest.h"
#include "keepalive.h"

using namespace std;

ofstream resultFile;
int result[11]; 
//0: NAT-Type, 1: Preserves Port, 2: Hairpining, 3: UPnP, 4: UDP-UPnP, 5: UDP-HP, 6: TCP-UPnP, 7: TCP-HP-low-TTL, 8: TCP-HP-high-TTL, 9:TCP-FTP, 10:SCTP, 11:PROXY 12:Keepalive

int TTL = 3;

extern int verbose;
extern char stunServer[255];
extern char HelpersPublicIP[255];
extern int HelpersPublicPort;
extern int HelpersPublicPort2;
extern int localTestingPort;


int startUDPListener(int sourcePort) {

	int sock;                     /* socket details */
	const int BUF_LEN=2048;         /* Buffer size for transfers */
	struct sockaddr_in local;
	struct hostent * host;           /* host stuff */
	int err;                         /* error trapping */
	char buffer[BUF_LEN];          /* file buffer */
  
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) 
		perror("ERROR opening socket");

		//set non blocking
	long arg;
	arg = fcntl(sock, F_GETFL, NULL); 
	arg |= O_NONBLOCK; 
	fcntl(sock, F_SETFL, arg); 

	local.sin_family=AF_INET;
	local.sin_port=htons(sourcePort);
	local.sin_addr.s_addr=INADDR_ANY;
	bind(sock,(struct sockaddr*)&local,sizeof(sockaddr_in));
	
	fd_set myset;
	struct timeval tv;
	tv.tv_sec = 15;
	tv.tv_usec = 0;
	FD_ZERO(&myset);
	FD_SET(sock, &myset);
	select (sock+1, NULL, &myset, NULL, &tv);

	if ( FD_ISSET(sock, &myset) ) {
		int t=0;
		while (t<15){
			int bytes = recvfrom(sock, buffer, BUF_LEN, 0, 0, 0);
			buffer[bytes] = '\0';
			if ( bytes > 0 && !strcmp(buffer, "aMoIC") ) {
				if (verbose)
					printf("received UDP Message behind NAT!\n\n");
				close(sock);
				return 1;
			}
			sleep(1);
			++t;
		}
	}
		close(sock);
		return 0;	
}

int startTCPListener(int sourcePort) {

	int sock;                     
	const int BUF_LEN=2048;         
	struct sockaddr_in local;
	struct hostent * host;           
	int err;                        
	char buffer[BUF_LEN];        
     
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
		perror("ERROR opening socket");

		//set non blocking
	long arg;
	arg = fcntl(sock, F_GETFL, NULL); 
	arg |= O_NONBLOCK; 
	fcntl(sock, F_SETFL, arg); 


	local.sin_family=AF_INET;
	local.sin_port=htons(sourcePort);
	local.sin_addr.s_addr=INADDR_ANY;
	bind(sock,(struct sockaddr*)&local,sizeof(sockaddr_in));
	
	listen(sock, 10);
	int connectedSocket = 0; 
	
	int t=0;
		//poll queue every second, exit after 15 seconds
	while ( t < 20 ){
		connectedSocket = accept(sock, NULL, NULL);
		if ( connectedSocket < 1 ) {
			sleep(1);
			++t;
		}
		else {
			int bytes = recv(connectedSocket, buffer, BUF_LEN, 0);
			close(connectedSocket);
			close(sock);
			return 1;
		}
	}

	close(connectedSocket);
	close(sock);
	return 0;
}


	//send request to helper and wait for holePunchingRequest
void sendToRelay(char* buffer, int port) {
	int sockfd, portno, n;
	long arg;
	socklen_t lon; 
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
		perror("ERROR opening socket");

		//use the tun-mapping
	server = gethostbyname(HelpersPublicIP);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
	serv_addr.sin_port = htons(port);

	if (verbose)
		printf("sending request to relay...\n");

		//connect to the given transport address to get source port
	connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

	send(sockfd, buffer, strlen(buffer), 0);

			//wait for HolePunching request
	char buf[1024];
	int bytes = recv(sockfd, buf, 1024, 0);
	buf[bytes] = '\0';
	
		//buffer now holds xml
	xmlDocPtr doc;
	char Protocol[255];
	char dPort[255];
	char sPort[255];
	char destIP[255];
	doc = openXMLBuffer(buf);
	char* testString = xmlGetRootName(doc);
	if ( !strcmp(testString, "NATS_HolePunchingRequest") ) {
		getXMLAttrib("/NATS_HolePunchingRequest/request", "Protocol", Protocol, 1, doc);
		getXMLAttrib("/NATS_HolePunchingRequest/request", "dPort", dPort, 1, doc);
		getXMLAttrib("/NATS_HolePunchingRequest/request", "sPort", sPort, 1, doc);
		getXMLAttrib("/NATS_HolePunchingRequest/request", "DestIP", destIP, 1, doc);

		if ( !strcmp("udp", Protocol) ) {
			if (verbose)
				printf("received Hole Punching Request (UDP)\n\n");
			punchHoleUDP(strtol(sPort, NULL, 10), strtol(dPort, NULL, 10), destIP);
		}
		if ( !strcmp("tcp", Protocol) ) {
			if (verbose)
				printf("received Hole Punching Request (TCP)\n\n");
			punchHoleTCP(strtol(sPort, NULL, 10), strtol(dPort, NULL, 10), destIP, TTL);
			TTL = 128;		
		}
	}

	close(sockfd);	
}


	//converts the candidate list into XML
char* createCandidateList(vector<CandidateList> list, char* protocol, char* purpose) {

	if ( verbose ){	
		cout<<"***************************************"<<endl;
		cout<<"creating candidate list..."<<endl;	
		cout<<"size of vector: "<<list.size()<<endl;
		cout<<"***************************************"<<endl<<endl;
	}

		///for converting to char*
	char temp[255], temp2[255];
	struct sockaddr_in ina;
	xmlDocPtr doc;
	sprintf(temp2,"NATS_Candidates_%s", purpose);
	doc = CreateXml(temp2);	

		///iterate through the vector and create xml-File
	int i = 0;

	while ( i < list.size() ) {
		sprintf(temp, "C%i", i);	
		AddElement(temp);	

			//convert BigEndian to dotted decimal	
			//original data (list[i].addr) is a BigEndian Integer
			//first it gets converted to a Little Endian Integer
			//finally to a dotted decimal char*
		ina.sin_addr.s_addr = ntohl(list[i].addr);
		sprintf(temp, inet_ntoa(ina.sin_addr)); 

		AddProperties("IP", temp);
		sprintf(temp, "%i", list[i].port);	
		AddProperties("Port", temp);	
		sprintf(temp, "%i", list[i].id);	
		AddProperties("Protocol", protocol);	
		AddProperties("ID", temp);
		AddProperties("Description", list[i].description);		

		++i;
	}
	
		///return the candidate-list
	return dumpXMLtoBuffer(doc);
}



	//called on a received request, gathers and returns candidate list
int getCandidates(char* protocol, int upnp, int helpersPort, int testingPort) {

		///now we know which Service is requested and have to create
		///an appropriate Candidate List for this service
		///this candidate list is implemented as a vector of structs
	char* candidateXML;
	CandidateList list;
	vector<CandidateList> candidates;


	/************************** STUN ***************************************/
		///local service on srcPort, get external Mapping from STUN-Server

		//we can only use real port if tcp, when using udp, port is already busy
	StunAddress4 stunCandidates;
	/*
	stunCandidates = stunCandidate(localTestingPort, stunServer);*/

	stunCandidates = test_mapping (testingPort, stunServer);
	if (verbose) {			
		cout<<"***************************************"<<endl;
		cout<<"ANSWER FROM STUN-SERVER"<<endl;
		cout<<"Destination: "<<stunServer<<endl;
		cout<<"Source Port: "<<localTestingPort<<endl;
		cout<<"Mapping: "<<stunCandidates<<endl;
		cout<<"***************************************"<<endl<<endl;
	}

	list.port = stunCandidates.port;	
	list.addr = stunCandidates.addr;
	list.id = 0;
	list.description = "STUN";
	candidates.push_back(list);
	/************************************************************************/
	
	/************************** UPnP ***************************************/
	if ( result[3] == 2 && upnp ) {

		string myIP = getLanAddress();
		string extIP = getExtAddress();

			//successful
		if ( myIP != "failed" ) {
			char port[255];
			sprintf(port, "%i",testingPort+1);
			addMapping((char*)myIP.c_str(), port, port, protocol);
			list.port = localTestingPort+1;
			list.addr = ntohl(inet_addr(extIP.c_str()));
			list.id = 1;
			list.description = "UPnP";
			candidates.push_back(list);	
		}	
	}
	/************************************************************************/

			/// finally create candidate XML-List
	candidateXML = createCandidateList(candidates, protocol, "testing");

	/**************************** END OF CREATING CANDIDATE LIST************/

		
	sendToRelay(candidateXML, helpersPort);

	

	return 0;
}


void startTester() {

	resultFile.open ("result.txt");
	
	int suc = 0;

	int doKeepalive = 0;

	printf("\nDo you also want to run the time consuming Keep Alive test which may take up to 15 minutes? (Y/N)\n");
	char* input;
	char word[2];
	while ( 1 ) {
		if (!strcmp(word, "Y")) {
			printf ("will run Keep Alive test \n");
			doKeepalive=1;
			break;
		}
		if (!strcmp(word, "N")) {
			printf ("will NOT run Keep Alive test \n");
			break;
		}

		else {
			printf ("please enter Y or N\n");
			scanf("%1s", word);
		}
	}



/* Keep alive*/
	keepaliveresult keepaliveStruct;
        keepaliveStruct.a=-1;
        keepaliveStruct.b=-1;
        keepaliveStruct.c=-1;
        keepaliveStruct.d=-1;


	if (doKeepalive){
		keepaliveStruct = keepalive();
		printf("result: %f-%f...%f-%f...\n", keepaliveStruct.a, keepaliveStruct.b, keepaliveStruct.c, keepaliveStruct.d);
	}




/***************STUN*********************************************************************/
	StunAddress4 stunServerAddr;
	
	stunParseServerName( stunServer, stunServerAddr);

	bool verbose = false;
	StunAddress4 sAddr;
	sAddr.port = 0;
	sAddr.addr = 0;
	bool preservePort;
	bool hairpin;
	int port=0;
	NatType stype = stunNatType( stunServerAddr, verbose, &preservePort, &hairpin, port, &sAddr );

	printf("\nYour NAT-Box has the following properties:\n\n");

	switch (stype)
	{
	case StunTypeOpen:
		result[0] = 0;
		printf("No NAT detected");
		resultFile << "No NAT detected\n";
		break;
	case StunTypeConeNat:
		result[0] = 1;
		printf("Cone Nat detect");
		resultFile << "Cone Nat detect\n";
		break;
	case StunTypeRestrictedNat:
		result[0] = 2;
		printf("Address restricted NAT detected");
		resultFile << "Address restricted NAT detected\n";
		break;
	case StunTypePortRestrictedNat:
		result[0] = 3;
		printf("Port restricted NAT detected");
		resultFile << "Port restricted NAT detected\n";
		break;
	case StunTypeSymNat:
		result[0] = 4;
		printf("Symetric");
		resultFile << "Symetric\n";
		break;
	case StunTypeSymFirewall:
		result[0] = 5;
		printf("Symetric firewall");
		resultFile << "Symetric firewall\n";
		break;
	case StunTypeBlocked:
		result[0] = 6;
		printf("Could not reach the stun server - check server name is correct");
		resultFile << "Could not reach the stun server - check server name is correct\n";
		break;
	default:
		result[0] = 7;
		printf("Unkown NAT type");
		resultFile << "Unkown NAT type\n";
		break;
	}

	printf("\n");

	if (preservePort)
	{
		result[1] = 1;
		printf("Preserves port number\r\n");
		resultFile << "Preserves port number\r\n";
	}
	else
	{
		result[1] = 0;
		printf("Does not preserve port number\r\n");
		resultFile << "Does not preserve port number\r\n";
	}

	if (hairpin)
	{
		result[2] = 1;
		printf("Supports hairpin of media\r\n");
		resultFile << "Supports hairpin of media\r\n\n";
	}
	else
	{
		result[2] = 0;
		printf("Does not support hairpin of media\r\n");
		resultFile << "Does not support hairpin of media\r\n\n";
	}

	printf("\n");
	printf("\n");

/***************UPnP*********************************************************************/
	int upnpWorks = 0;

	switch ( testForUPNP() ){
		case -1:
			result[3] = 0;
			printf("\n\nno UPnP device found\n\n");
			resultFile << "\n\nno UPnP device found\n\n";
			break;
		case 0:
			result[3] = 1;
			printf("\n\nUPnP device found but can't configure it\n\n");
			resultFile << "\n\nUPnP device found but can't configure it\n\n";
			break;
		case 1:
			result[3] = 2;
			printf("\n\nUPnP capable IGD seems to work\n\n");
			resultFile << "\nUPnP capable IGD seems to work\n\n";
			upnpWorks = 1;
			break;
		default:
			result[3] = 3;
			printf("\n\nunknown error while talking to UPnP device\n\n");
			resultFile << "\n\nunknown error while talking to UPnP device\n\n";
			break;
	}


	printf("performing connectivity tests...\n");
/***************UDP*********************************************************************/
		//gets candidates for udp, sends them to the relay and perfoms hole punching
	getCandidates("udp", 1, HelpersPublicPort, localTestingPort);

		//start Candidate test
	//STUN
	printf("starting UDP Listener for STUN\n");
	resultFile << "starting UDP Listener for STUN\n";
	suc = startUDPListener(localTestingPort);
	if (suc) {
		printf ("UDP Hole Punching successful\n\n",suc);
		resultFile << "UDP Hole Punching successful\n\n";
		result[5] = 1;
	}
	else {
		printf ("UDP Hole Punching NOT successful\n\n",suc);
		resultFile << "UDP Hole Punching NOT successful\n\n";
	}
	//UPnP
	if ( result[3] == 2 ) {
		printf("starting UDP Listener for UPnP\n");
		resultFile << "starting UDP Listener for UPnP\n";
		suc = startUDPListener(localTestingPort+1);
		if (suc) {
			printf ("UPnP UDP successful\n\n",suc);
			resultFile << "UPnP UDP successful\n\n";
			result[4] = 1;
		}
		else {
			printf ("UPnP UDP NOT successful\n\n",suc);
			resultFile << "UPnP UDP NOT successful\n\n";
		}
		char port[255];
		sprintf(port, "%i",localTestingPort+1);
		removeMapping(port, "udp");
	}

/***************TCP*********************************************************************/

		//gets candidates for udp, sends them to the relay and perfoms hole punching
	getCandidates("tcp", 1, HelpersPublicPort, localTestingPort);

		//start Candidate test
	//TCP low-TTL
	printf("starting TCP Listener (low-TTL)\n");
	resultFile << "starting TCP Listener\n";
	suc = startTCPListener(localTestingPort);
	if (suc) {
		printf ("TCP Hole Punching (low-TTL) successful\n\n",suc);
		resultFile << "TCP Hole Punching (low-TTL) successful\n\n";
		result[7] = 1;
	}
	else {
		printf ("TCP Hole Punching (low-TTL) NOT successful\n\n",suc);
		resultFile << "TCP Hole Punching (low-TTL) NOT successful\n\n";
	}

			
	//UPnP
	if ( result[3] == 2 ) {
		printf("starting TCP Listener for UPnP\n");
		resultFile << "starting TCP Listener for UPnP\n";
		suc = startTCPListener(localTestingPort+1);
		if (suc) {
			printf ("UPnP TCP successful\n\n",suc);
			resultFile << "UPnP TCP successful\n\n";
			result[6] = 1;
		}
		else {
			printf ("UPnP TCP NOT successful\n\n",suc);
			resultFile << "UPnP TCP NOT successful\n\n";
		}
		char port[255];
		sprintf(port, "%i",localTestingPort+1);
		removeMapping(port, "tcp");
	}

/*new RELAY*/
		//TCP-high TTL

	getCandidates("tcp", 0, HelpersPublicPort2, 17002);
	printf("starting TCP Listener\n");
	resultFile << "starting TCP Listener (high-TTL)\n";
	suc = startTCPListener(17002);
	if (suc) {
		printf ("TCP Hole Punching (high-TTL) successful\n\n",suc);
		resultFile << "TCP Hole Punching (high-TTL) successful\n\n";
		result[8] = 1;
	}
	else {
		printf ("TCP Hole Punching (high-TTL) NOT successful\n\n",suc);
		resultFile << "TCP Hole Punching (high-TTL) NOT successful\n\n";
	}
/*new RELAY*/

/*FTP*/
	ftpALG();
	printf("starting TCP Listener\n");
	suc = startTCPListener(17200);
	if (suc) {
		printf ("TCP FTP-ALG Hole Punching successful\n\n",suc);
		resultFile << "TCP FTP-ALG Hole Punching successful\n\n";
		result[9] = 1;
	}
	else {
		printf ("TCP FTP-ALG Hole Punching NOT successful\n\n",suc);
		resultFile << "TCP FTP-ALG Hole Punching NOT successful\n\n";
	}

	sleep(1);

/*SCTP*/
	suc = testSCTP();
	if (suc) {
		printf ("SCTP successful\n\n",suc);
		resultFile << "SCTP successful\n\n";
		result[10] = 1;
	}
	else {
		printf ("SCTP NOT successful\n\n",suc);
		resultFile << "SCTP NOT successful\n\n";
	}

/*Zwangsproxy*/
	

	suc = testforproxy(HelpersPublicIP);
	if (suc) {
		printf ("Proxy present\n\n",suc);
		resultFile << "Proxy present!\n\n";
		result[11] = 1;
	}
	else {
		printf ("no Proxy present\n\n",suc);
		resultFile << "no Proxy present\n\n";
	}

/* Keep alive*/
/*	keepaliveresult keepaliveStruct;
        keepaliveStruct.a=-1;
        keepaliveStruct.b=-1;
        keepaliveStruct.c=-1;
        keepaliveStruct.d=-1;


	if (doKeepalive){printf("test");
		keepaliveStruct = keepalive();
		//printf("result: %f-%f...%f-%f...\n", keepaliveStruct.a, keepaliveStruct.b, keepaliveStruct.c, keepaliveStruct.d);
	}
*/

	resultFile.close();

		//do an automatic backup
	//char urlBackup[255];
	//sprintf(urlBackup, "addResultBackground.php?a=%i&b=%i&c=%i&d=%i&e=%i&f=%i&g=%i&h=%i,$i=%i,$j=%i",
	//		result[0],result[1],result[2],result[3],result[4],result[5],result[6],result[7],result[8],result[9]);

	//updateDB(urlBackup);

	system("clear");

	char url[255];
//0: NAT-Type, 1: Preserves Port, 2: Hairpining, 3: UPnP, 4: UDP-UPnP, 5: UDP-HP, 6: TCP-UPnP, 7: TCP-HP-low-TTL, 8: TCP-HP-high-TTL, 9:TCP-FTP, 10:SCTP, 11:PROXY
	sprintf(url, "http://nattest.net.in.tum.de/addResult.php?a=%i&b=%i&c=%i&d=%i&e=%i&f=%i&g=%i&h=%i&i=%i&j=%i&k=%i&l=%i&m=%f&n=%f&o=%f&p=%f&z=linux",
			result[0],result[1],result[2],result[3],result[4],result[5],result[6],result[7],result[8],result[9],result[10],result[11],keepaliveStruct.a,keepaliveStruct.b, keepaliveStruct.c, keepaliveStruct.d);

	printf("\n\nZunächst mal vielen Dank für ihre Hilfe!\n");
	printf("\n\nUm den Test abzuschließen und die Ergebnisse zu übermitteln ");
	printf("öffnet sich nun ein Browser-Fenster(Firefox) in dem sie den Typ der getesteten NAT eintragen können.\n");
	printf("Dieser Schritt ist sehr WICHTIG, denn nur so können sie mir ihre Testergebnisse mitteilen!\n\n");
	printf("Wenn sich kein Browserfenster (oder ein neues Tab im vorhandenen Browserfenster) öffnet, öffnen sie bitte ");
	printf("manuell folgenden Link:\n");

	printf ("\n%s\n\n", url);
	
	printf ("\nAlternativ können Sie mir die Datei result.txt im Verzeichnis src per E-Mail zukommen lassen:\n");
	printf ("mueller@net.in.tum.de\n\n");

	
	char command[255];
	sprintf(command, "mozilla-firefox \"%s\" &", url);
	sprintf(command, "firefox \"%s\" &", url);
	system (command);
		
}
