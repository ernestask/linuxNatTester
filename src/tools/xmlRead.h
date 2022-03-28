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

#include <libxml/parser.h>

/**
 * \brief open a XML-File
 * 
 *  \param filename	filename of the XML-File
 * 
 *  \return Name of the Root-Node
 * 
 * This method opens a XML-File
 */
char* openXML(char* filename);

/**
 * \brief destroy XML
 * 
 * \param doc XML-doc
 * 
 * This method destroys the XML-Context
 */
void destroyXML(xmlDocPtr doc);

/**
 * \brief Read an attribute 
 * 
 *  \param *elementPath	error message
 *  \param *attribName	error message
 *  \param *str	error message
 *  \param required	error message
 *  \xmlDocPtr doca Pointer to the XML-Element
 * 
 * 
 * This method reads an Attribut of a XML-Identifier
 */
int getXMLAttrib(char *elementPath, char *attribName, char *str, int required, xmlDocPtr doca);

/**
 * \brief adds an XML-Element
 * 
 *  \param name	name of the element which is to be added
 * 
 * 
 * This method adds an XML-Element
 */
void AddElement(char* name);

/**
 * \brief adds a property and a value
 * 
 *  \param *name	name of the property
 *  \param *value	value of the property
 * 
 * 
 * This method adds a property/value combination
 * to the active XML-Element
 */
void AddProperties(char* name, char* value);

/**
 * \brief reads a xml-File from a Buffer
 * 
 *  \param char*	buffer holding the XML-Content
 * 
 *  \return XML Doc
 * 
 * This method reads a buffer containing a XML-File
 * and returns a xmlDocPtr to its XML-content
 */
xmlDocPtr openXMLBuffer(char* buffer);

/**
 * \brief returns the Name of the Root-Node
 * 
 *  \param char*	XML Doc
 * 
 *  \return Name of the Root Node
 * 
 * This method gets the name of the root node
 * from a XML-Doc
 */
char* xmlGetRootName(xmlDocPtr doc);

/**
 * \brief returns a XML-Buffer
 * 
 *  \param char*	XML Doc
 * 
 *  \return buffer holding the new XML-File
 * 
 * This method dumps the content of a doc
 * to a char* Buffer.
 */
char* dumpXMLtoBuffer(xmlDocPtr doc);

/**
 * \brief Creates a new XML-Document
 * 
 *  \param char*	Name of the Root Node
 * 
 *  \return buffer holding the new XML-File
 * 
 * This method creates a new XML-Document in Memory.
 *
 */
xmlDocPtr CreateXml(char* rootNodeName);




