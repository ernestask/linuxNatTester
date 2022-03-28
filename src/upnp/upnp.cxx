#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cassert>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

#include "miniwget.h"
#include "miniupnpc.h"
#include "upnpcommands.h"


using namespace std;
	//hier schreiben wir die sachen rein die wir ausgeben möchten
extern ofstream resultFile;
extern int verbose;

int DisplayInfos(struct UPNPUrls * urls,
                         struct IGDdatas * data)
{
	char externalIPAddress[16];
	char connectionType[64];
	char status[64];
	unsigned int uptime;
	unsigned int brUp, brDown;
	UPNP_GetConnectionTypeInfo(urls->controlURL,
	                           data->servicetype,
							   connectionType);
	if(connectionType[0])
		printf("Connection Type : %s\n", connectionType);
	else
		printf("GetConnectionTypeInfo failed.\n");
	UPNP_GetStatusInfo(urls->controlURL, data->servicetype, status, &uptime);
	printf("Status : %s, uptime=%u\n", status, uptime);
	UPNP_GetLinkLayerMaxBitRates(urls->controlURL_CIF, data->servicetype_CIF,
			&brDown, &brUp);
	printf("MaxBitRateDown : %u bps   MaxBitRateUp %u bps\n", brDown, brUp);
	UPNP_GetExternalIPAddress(urls->controlURL,
	                          data->servicetype,
							  externalIPAddress);
	if(externalIPAddress[0]){
			printf("ExternalIPAddress: %s", externalIPAddress);
			resultFile << "\nExternalIPAddress:";
			resultFile << externalIPAddress;
		return 1;
	}
	else{
			printf("\nExternalIPAddress failed");
			resultFile << "\nExternalIPAddress failed";
		return 0;
	}
}

static void ListRedirections(struct UPNPUrls * urls,
                             struct IGDdatas * data)
{
	int r;
	int i = 0;
	char index[6];
	char intClient[16];
	char intPort[6];
	char extPort[6];
	char protocol[4];
	char desc[80];
	char enabled[6];
	char rHost[64];
	char duration[16];
	/*unsigned int num=0;
	UPNP_GetPortMappingNumberOfEntries(urls->controlURL, data->servicetype, &num);
	printf("PortMappingNumberOfEntries : %u\n", num);*/
	do {
		sprintf(index, "%d", i);
		rHost[0] = '\0'; enabled[0] = '\0';
		duration[0] = '\0'; desc[0] = '\0';
		extPort[0] = '\0'; intPort[0] = '\0'; intClient[0] = '\0';
		r = UPNP_GetGenericPortMappingEntry(urls->controlURL, data->servicetype,
		                               index,
		                               extPort, intClient, intPort,
									   protocol, desc, enabled,
									   rHost, duration);
		if(r==0)
			printf("%02d - %s %s->%s:%s\tenabled=%s leaseDuration=%s\n"
			       "     desc='%s' rHost='%s'\n",
			       i, protocol, extPort, intClient, intPort,
				   enabled, duration,
				   desc, rHost);
		i++;
	} while(r==0);
}

int SetRedirectAndTest(struct UPNPUrls * urls,
                    struct IGDdatas * data,
					const char * iaddr,
					const char * iport,
					const char * eport,
                    const char * proto)
{
	char externalIPAddress[16];
	char intClient[16];
	char intPort[6];
	int r;

	if(!iaddr || !iport || !eport || !proto)
	{
		fprintf(stderr, "Wrong arguments\n");
		return 0;
	}
	
	UPNP_GetExternalIPAddress(urls->controlURL,
	                          data->servicetype,
							  externalIPAddress);
	if(externalIPAddress[0]) {
		if (verbose)
			printf("ExternalIPAddress = %s\n", externalIPAddress);
	}
	else
		return 0;
	
	r = UPNP_AddPortMapping(urls->controlURL, data->servicetype,
	                        eport, iport, iaddr, 0, proto);
	if(r==0)
		return 0;
	UPNP_GetSpecificPortMappingEntry(urls->controlURL,
	                                 data->servicetype,
    	                             eport, proto,
									 intClient, intPort);
	if(intClient[0]) {
		if (verbose)
			printf("InternalIP:Port = %s:%s\n", intClient, intPort);
	}
	else
		return 0;
	
	printf ("exernal IP %s:%s is redirected to internal %s:%s\n\n\n", externalIPAddress, eport, intClient, intPort);
	return 1;
}

static void
RemoveRedirect(struct UPNPUrls * urls,
               struct IGDdatas * data,
			   const char * eport,
			   const char * proto)
{
	if(!proto || !eport)
	{
		fprintf(stderr, "invalid arguments\n");
		return;
	}

	UPNP_DeletePortMapping(urls->controlURL, data->servicetype, eport, proto);
}


int addMapping(char* IP, char* intPort, char* extPort, char* protocol) {	
	struct UPNPDev * devlist;
	char lanaddr[16];
	devlist = upnpDiscover(2000);

	if(devlist)
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		
		for(device = devlist; device; device = device->pNext)
		{
		}

		if(UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))) {
			printf("\n\ntrying to add new mapping...\n");
						
			SetRedirectAndTest(&urls, &data, IP, intPort, extPort, protocol);
			freeUPNPDevlist(devlist); devlist = 0;
			return 1;
		}
	}
	freeUPNPDevlist(devlist); devlist = 0;
	return 0;
}

int removeMapping(char* extPort, char* protocol){
	struct UPNPDev * devlist;
	char lanaddr[16];
	devlist = upnpDiscover(2000);

	if(devlist)
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		
		for(device = devlist; device; device = device->pNext)
		{
		}

		if(UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))) {
			//printf("\n\nremoving mapping...\n");
			RemoveRedirect(&urls, &data, extPort, protocol);	
			freeUPNPDevlist(devlist); devlist = 0;
			return 1;
		}
	}
	freeUPNPDevlist(devlist); devlist = 0;
	return 0;
}

std::string getLanAddress() {	
	struct UPNPDev * devlist;
	char lanaddr[16];
	devlist = upnpDiscover(2000);

	if(devlist)
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		
		for(device = devlist; device; device = device->pNext)
		{
		}

		if(UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))) {
			freeUPNPDevlist(devlist); devlist = 0;
			return lanaddr;
		}
	}
	freeUPNPDevlist(devlist); devlist = 0;
	return "failed";
}

std::string getExtAddress() {	
	struct UPNPDev * devlist;
	char lanaddr[16];
	char externalIPAddress[16];
	devlist = upnpDiscover(2000);

	if(devlist)
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		
		for(device = devlist; device; device = device->pNext)
		{
		}

		if(UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))) {
				UPNP_GetExternalIPAddress(urls.controlURL,
	                          data.servicetype,
							  externalIPAddress);
				freeUPNPDevlist(devlist); devlist = 0;
				return externalIPAddress;
		}
	}
	freeUPNPDevlist(devlist); devlist = 0;
	return "failed";
}

int testForUPNP(){ 
	char command = 0;
	struct UPNPDev * devlist;
	
	char lanaddr[16];
	int i;

	
	devlist = upnpDiscover(2000);

	if(devlist)
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
				
		for(device = devlist; device; device = device->pNext)
		{
			printf("Found UPnP-Device at:\n");
			printf("desc: %s \n", device->descURL );
			printf("st: %s \n", device->st );
			
			resultFile << "Found UPnP-Device at:\n";
			resultFile << "desc: ";
			resultFile << device->descURL;
			resultFile << "\nst: ";
			resultFile << device->st;
			
		}

		if(UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)))
		{
			int ret = DisplayInfos(&urls, &data);
				//ok
			if (ret) {
				printf("\nLocal LAN ip address: %s ",lanaddr);
				resultFile <<  "\nLocal LAN ip address : ";
				resultFile << lanaddr;
				return 1;
			}
			else {
				return 0;
			}
			
		}
	}
	else {
		printf("No UPnP-Device found.\n");
		resultFile << "No UPnP-Device found.\n";
		freeUPNPDevlist(devlist); devlist = 0;
		return -1;
	}
	freeUPNPDevlist(devlist); devlist = 0;
	return -1;
}
