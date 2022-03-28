#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "keepalive.h"

#define ECHOMAX 6    				 // Longest string to echo 
#define STARTS 10
#define STARTL 100

extern char HelpersPublicIP[255];




/* External error handling function */
void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}
  
struct keepaliveresult keepalive()
{


float step = 20;								//STEPS FOR UDP TESTING
float step2 = 60;								//STEPS FOR UDP_STREAM TESTING
int udp_threshold = 100;					//WHEN TO STOP
int udp_stream_threshold = 300;			//WHEN TO STOP



keepaliveresult result;


int sock;                        		

int nf = 0;
float ls = 0, lf = 90;
struct sockaddr_in echoServAddr; 		// Echo server address 
struct sockaddr_in echoClntAddr; 		// Client address 
struct sockaddr_in fromAddr;     		// Source address of echo 

unsigned short echoServPort = 21000;		// Echo server port 
unsigned short echoClntPort = 21000;		// Echo server port
unsigned int fromSize;   	        	// In-out of address size for recvfrom() 
char *servIP = HelpersPublicIP;		// IP address of server 
char echoString[2] = "A";		      	// String to send to echo server 
char echoBuffer[ECHOMAX+1];      		// Buffer for receiving echoed string 
int echoStringLen;               		// Length of string to echo 
int respStringLen;             			// Length of received response 
float keepalive = STARTS;			// Time-out value variable 
char cka;
int to = 0;
int fail = 0;
int i=0;
int success = 0;

fd_set readfds, masterfds;
struct timeval tv;
long arg;


if ((echoStringLen = strlen(echoString)) > ECHOMAX)	// Check input length 
DieWithError("Echo word too long");


/* Construct the server address structure */
memset(&echoServAddr, 0, sizeof(echoServAddr));    	// Zero out structure 
echoServAddr.sin_family = AF_INET;                 	// Internet addr family 
echoServAddr.sin_addr.s_addr = inet_addr(servIP);  	// Server IP address 
echoServAddr.sin_port   = htons(echoServPort);     	// Server port 


/******************* TESTING UDP_TIMEOUT **********************/

printf("\nTesting for udp_timeout value...\n");

/* Run forever */
for (;;) { 						

foragain1:

	/* Create a datagram/UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	DieWithError("socket() failed");
	
	printf("\nKeep alive value to be tested is: %f seconds\n", keepalive);

	char keepalivetime[10];
	sprintf(keepalivetime, "%f",keepalive);

	/* Send the string to the server */
	sendto(sock, keepalivetime, 11, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) ;

	printf("Sending: %s, now waiting for %f seconds for a reply...",echoString, keepalive);
	fflush(stdout);	
	/* Recv a response */
	fromSize = sizeof(fromAddr);

	// Set to non-blocking 
	arg = fcntl(sock, F_GETFL, NULL); 
	arg |= O_NONBLOCK; 
	fcntl(sock, F_SETFL, arg);

	FD_ZERO(&masterfds);
	FD_SET(sock, &masterfds);
	tv.tv_sec = keepalive + 5; 
	tv.tv_usec = 0;

	memcpy(&readfds, &masterfds, sizeof(fd_set));

	if (select(sock+1, &readfds, NULL, NULL, &tv) < 0) {
		perror("\nError in select()");
		exit(1);
		}

	if (FD_ISSET(sock, &readfds)) {
		// read from the socket
		if((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) <= 0)
			printf("\nError in receive (new)");
		}

	else {
		printf("\nSocket timed out.\n");
		goto updateka1;
		} 

	// Set to Blocking again 
	arg = fcntl(sock, F_GETFL, NULL); 
	arg &= (~O_NONBLOCK); 
	fcntl(sock, F_SETFL, arg);

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
		fprintf(stderr,"\nError: received a packet from unknown source.\n");
		exit(1);
		}


	// null-terminate the received data 
	echoBuffer[respStringLen] = '\0';
	printf("\nReceived: %s\n", echoBuffer);    		// Print the echoed arg 
	
	strcpy(echoString, "B");
	cka = echoBuffer[0];

	if(cka == 'reply') {
		ls = keepalive;
		keepalive += step;
		}

	else {
		strcpy(echoString, "C");
		ls = keepalive;		
		if(nf == 1) {
			success++;
			if(success == 2 ) {
				printf("\nThe udp_timeout is in between %.2f and %.2f seconds.\n", ls, lf);
				result.a = ls;
				result.b = lf;
				goto streamcheck;
				}
			step /= 2;
			if (keepalive + step < lf)
				keepalive += step;
			else {
				printf("\nThe udp_timeout is in between %.2f and %.2f seconds.\n", ls, lf);
				result.a = ls;
				result.b = lf;
				goto streamcheck;
				}
			}
		else keepalive += step;
		}
	
	if(keepalive > udp_threshold) {
		printf("\nKeep-alive greater than %i seconds.\n", udp_threshold);
		result.a = udp_threshold;
		result.b = 0;
		goto streamcheck;
		}
close(sock);			
	
}

updateka1:
	
	fail++;
	
	if( keepalive <= STARTS) {
		printf("\nUnable to connect to server.\n");
		goto end;
		}

	lf = keepalive;
	step /= 2;

	if(fail == 3) {
		printf("\nThe udp_timeout is in between %.2f and %.2f seconds.\n", ls, lf);	
		result.a = ls;
		result.b = lf;		
		goto streamcheck;
		}

	if (keepalive - step > ls)
		keepalive -= step;

	else {
		printf("\nThe udp_timeout is in between %.2f and %.2f seconds.\n", ls, lf);	
		result.a = ls;
		result.b = lf;			
		goto streamcheck;
		}

	nf = 1;
	strcpy(echoString, "D");
	close(sock);
	goto foragain1;



/******************* TESTING UDP_TIMEOUT_STREAM **********************/

streamcheck:
	
	sendto(sock, "X", echoStringLen, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));

	printf("\n\nNow testing for udp_timeout_stream value...");

	nf = 0;
	ls = 0, lf = 90;
	strcpy(echoString,"A");			      		// String to send to echo server 
	keepalive = STARTL;					// Time-out value variable 
	cka = '\0';
	to = 0;
	fail = 0;
	
	/* Run forever */
	for (;;) { 					

	foragain2:

		// Create a datagram/UDP socket 
		if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

		// Construct the server address structure 
		memset(&echoServAddr, 0, sizeof(echoServAddr));    	// Zero out structure 
		echoServAddr.sin_family = AF_INET;                 	// Internet addr family 
		echoServAddr.sin_addr.s_addr = inet_addr(servIP);  	// Server IP address 
		echoServAddr.sin_port   = htons(echoServPort);     	// Server port 

		printf("\nKeep-alive time = %.2f", keepalive);

		printf("\nEstablishing UDP stream...");	
		fflush(stdout);
		sendto(sock, "R", echoStringLen, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));

		for(i=0; i<10; i++) {
			printf("\nSending S", i);
			fflush(stdout);
			sendto(sock, "S", echoStringLen, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
			respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize);
			echoBuffer[respStringLen] = '\0';
			printf("\t Receiving %s", echoBuffer);
			fflush(stdout);
			}

		printf("\nUDP stream established.");	
		fflush(stdout);


		char keepalivetime[100];
		sprintf(keepalivetime, "%f", keepalive);

		// Send the string to the server 
		sendto(sock, keepalivetime, 101, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));

		printf("\nSending: %s",echoString);
		fflush(stdout);	
		// Recv a response 
		fromSize = sizeof(fromAddr);

		// Set to non-blocking 
		arg = fcntl(sock, F_GETFL, NULL); 
		arg |= O_NONBLOCK; 
		fcntl(sock, F_SETFL, arg);

		FD_ZERO(&masterfds);
		FD_SET(sock, &masterfds);
		tv.tv_sec = keepalive + 5; 
		tv.tv_usec = 0;

		memcpy(&readfds, &masterfds, sizeof(fd_set));

		if (select(sock+1, &readfds, NULL, NULL, &tv) < 0) {
			perror("\nError in select()");
			exit(1);
		}

		if (FD_ISSET(sock, &readfds)) {
			// read from the socket
			if((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) <= 0)
				printf("\nError in receive (new)");
		}

		else {
			printf("\nSocket timed out.\n");
			goto updateka2;
		} 

		// Set to Blocking again 
		arg = fcntl(sock, F_GETFL, NULL); 
		arg &= (~O_NONBLOCK); 
		fcntl(sock, F_SETFL, arg);

		if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
			fprintf(stderr,"\nError: received a packet from unknown source.\n");
			exit(1);
		}


		// null-terminate the received data 
		echoBuffer[respStringLen] = '\0';
		printf("\nReceived: %s\n", echoBuffer);    		// Print the echoed arg 

		strcpy(echoString, "B");
		cka = echoBuffer[0];
		if(cka == 'reply') {
			ls = keepalive;
			keepalive += step2;
			}
		else {
			strcpy(echoString, "C");
			ls = keepalive;		
			if(nf == 1) {
				success++;
				if(success == 2 ) {
					printf("\nThe udp_timeout is in between %.2f and %.2f seconds.\n", ls, lf);
					result.c = ls;
					result.d = lf;	
					goto streamcheck;
					}
				step2 /= 2;
				if (keepalive + step2 < lf)
					keepalive += step2;
				else {
					printf("\nThe keep-alive time is in between %.2f and %.2f seconds.\n", ls, lf);
					result.c = ls;
					result.d = lf;	
					goto end;
					}
				}
			else keepalive += step2;
			}

		if(cka == 'S') {
			printf("\nThis shouldnt be happening");
			fflush(stdout);
			}

		if(keepalive > udp_stream_threshold) {
			printf("\nKeep-alive greater than %i seconds.\n", udp_stream_threshold);
			result.c = udp_stream_threshold;
			result.d = 0;	
			goto end;
			}
	close(sock);	

	}

updateka2:
	
	fail++;
	
	if( keepalive <= STARTL) {
		printf("\nThe udp_timeout_stream value is less than 100 seconds.");
		result.c = 0;
		result.d = 300;	
		goto end;
		}

	lf = keepalive;
	step2 /= 2;

	if(fail == 3) {
		printf("\nThe keep-alive time is in between %.2f and %.2f seconds.\n", ls, lf);			
		result.c = ls;
		result.d = lf;	
		goto end;
		}
	if (keepalive - step2 > ls)
		keepalive -= step2;
	else {
		printf("\nThe keep-alive time is in between %.2f and %.2f seconds.\n", ls, lf);			
		result.c = ls;
		result.d = lf;	
		goto end;
		}
	nf = 1;
	strcpy(echoString, "D");
	close(sock);
	goto foragain2;

end:
close(sock);
return result;
}
