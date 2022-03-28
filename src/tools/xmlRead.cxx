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
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <ctype.h>
#include "xmlRead.h"

	//only for the xml-Config File which is opened only once
xmlDocPtr config_doc;
xmlNodePtr root_node, newnode;

char xmlFile[255]= {0};

void stripWS(char* str) {
  char tstr[200];
  int lindex = 0;
  int rindex = 0;

  while (str[lindex] != 0 && isspace(str[lindex]))
    lindex++;

  rindex = ((int) strlen(str)) -1;
  while ( rindex >= 0 && isspace(str[rindex]) )
    rindex--;

  if ( lindex == strlen(str))
  {
    strcpy(str,"");
    return;
  }

  str[rindex+1] = 0;
  strcpy(tstr,str+lindex);
  strcpy(str,tstr);
}



void AddElement(char* name) {
	newnode = xmlNewTextChild (root_node, NULL, (const xmlChar*)name, NULL);
}

	//adds a Properties within the above Element
void AddProperties(char* name, char* value) {
	xmlAttrPtr newattr;
	newattr = xmlNewProp (newnode, (const xmlChar*)name, (const xmlChar*)value);
}


xmlXPathObjectPtr getnode(xmlChar *xpath, xmlDocPtr doca) {
  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;

  context = xmlXPathNewContext(doca);
  result = xmlXPathEvalExpression(xpath, context);
  
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    return NULL;
  }

  xmlXPathFreeContext(context);
  return result;
}

int getXMLAttrib(char *elementPath, char *attribName, char *str, int required, xmlDocPtr doca) {
 
	if (doca == NULL)
		doca = config_doc;


  xmlChar *xpath;
  xmlXPathObjectPtr result;
  xmlNodeSetPtr nodeset;
  xmlChar *resultstr;
  char tmpBuf[1000];
  char buf[255];
  
  sprintf(tmpBuf, "%s[@%s]", elementPath, attribName);

  xpath = xmlCharStrdup(tmpBuf);
  result = getnode(xpath, doca);
  xmlFree(xpath);

  required=1;

  if ( required && result == NULL )  {
    sprintf(buf, "%s: %s required in configuration file",xmlFile, elementPath);
   
    return -1;
  }

  if ( result == NULL )
    return -1;
    
  nodeset = result->nodesetval;

  if ( nodeset->nodeNr >= 2 )  {
   	sprintf(buf, "%s: %s should appear atmost once in configuration file\n", xmlFile, elementPath);
   	
   	return -1;
  }

  resultstr = xmlGetProp(nodeset->nodeTab[0], (xmlChar*)attribName);
  strcpy(str,(char*)resultstr);
  stripWS(str);
  xmlFree(resultstr);
  xmlXPathFreeObject(result);
  
  
  return 0;
}

char* xmlGetRootName(xmlDocPtr doc) {
	xmlNodePtr root;
  	root = xmlDocGetRootElement(doc);

  	if (root == NULL)  {
    	exit(0);
    }
	return (char*)root->name;
}


xmlDocPtr openXMLBuffer(char* buffer) {
	xmlDocPtr doc;
	
	xmlNodePtr root;
	doc = xmlReadMemory(buffer, strlen(buffer), "noname.xml", NULL, 0);
	root = xmlDocGetRootElement(doc);

  	if (root == NULL)  {
    	exit(0);
    }
	return doc;
}

	//create an XML File
xmlDocPtr CreateXml(char* rootNodeName) {
		//new doc and root node		
	xmlDocPtr doc;
   doc = xmlNewDoc ((const xmlChar*)"1.0");
   root_node = xmlNewNode (NULL,(const xmlChar*)rootNodeName);
	xmlDocSetRootElement (doc, root_node); 
	return doc;  
}


char* dumpXMLtoBuffer(xmlDocPtr doc) {
 	xmlNodePtr n;
    xmlChar *xmlbuff;
    int buffersize;
    
    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    return((char *) xmlbuff);

    xmlFree(xmlbuff);
    xmlFreeDoc(doc);
}

void destroyXML(xmlDocPtr doc) {
	if ( doc == NULL )
		doc = config_doc;
	xmlFreeDoc(doc);
	xmlCleanupParser();
}

char* openXML(char* filename) {
	char version[255];
	xmlNodePtr root;

	strcpy(xmlFile,filename);

	config_doc = xmlParseFile(filename);

	if (config_doc == NULL ) {
   		exit(0);
	}

  	root = xmlDocGetRootElement(config_doc);

  	if (root == NULL)  {
    	exit(0);
    }

	return (char*)root->name;
}
