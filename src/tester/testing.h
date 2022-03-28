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
#include <vector>
#include "../stun/stun.h"

typedef unsigned short UInt16;
typedef unsigned int   UInt32;


///ids:
///0: Hole Punching
///1: UPnP
///2: local IP
typedef struct 
{
   UInt32 addr;
	UInt16 port;
	int id;
	char* description;
} CandidateList;

typedef struct 
{
	char* protocol;
	UInt16 port;
} CandidateRequest;

void startTester();

