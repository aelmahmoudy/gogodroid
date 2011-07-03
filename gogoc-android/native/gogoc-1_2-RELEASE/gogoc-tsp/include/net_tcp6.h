/*
-----------------------------------------------------------------------------
 $Id: net_tcp6.h,v 1.1 2009/11/20 16:53:16 jasminko Exp $
-----------------------------------------------------------------------------
* This source code copyright (c) gogo6 Inc. 2002-2004,2007.
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License (GPL) Version 2, 
* June 1991 as published by the Free  Software Foundation.
* 
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY;  without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License 
* along with this program; see the file GPL_LICENSE.txt. If not, write 
* to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
* MA 02111-1307 USA
-----------------------------------------------------------------------------
*/

#ifndef _NET_TCP6_H_
#define _NET_TCP6_H_

extern sint32_t      NetTCP6Connect        (pal_socket_t *, char *, uint16_t);
extern sint32_t      NetTCP6Close         (pal_socket_t);

extern sint32_t      NetTCP6ReadWrite     (pal_socket_t, char *, sint32_t, char *, sint32_t);

extern sint32_t      NetTCP6Write         (pal_socket_t, char *, sint32_t);
extern sint32_t      NetTCP6Printf        (pal_socket_t, char *, sint32_t, char *, ...);

extern sint32_t      NetTCP6Read          (pal_socket_t, char *, sint32_t);

#endif
