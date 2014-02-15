/* *********************************************************************** */
/* $Id: haccess_devmap_c_wrap.h,v 1.1 2009/11/20 16:30:24 jasminko Exp $       */
/*                                                                         */
/* Copyright (c) 2007 gogo6 Inc. All rights reserved.                     */
/*                                                                         */
/*   For license information refer to CLIENT-LICENSE.TXT                   */
/*                                                                         */
/* Description:                                                            */
/*Wraps the HomeAccess Device Mapping configuration data to offer C access.*/
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: Febuary 2007                                             */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#ifndef __gogocconfig_haccess_devmap_c_wrap_h__
#define __gogocconfig_haccess_devmap_c_wrap_h__


#ifdef __cplusplus
extern "C" {
#endif


// Structure definition to hold HomeAccess device mappings
struct __DEVICE_MAPPING;
typedef struct __DEVICE_MAPPING
{
  char*             szName;
  char*             szAddress;
  struct __DEVICE_MAPPING* next;
} DEVICE_MAPPING, *PDEVICE_MAPPING;


/* ----------------------------------------------------------------------- */
/* HomeAccess Device Mapping management functions.                         */
/* ----------------------------------------------------------------------- */
int                 init_haccess_devmap      ( const char* );
int                 reload_haccess_devmap    ( void );
void                uninit_haccess_devmap    ( void );

/* ----------------------------------------------------------------------- */
/* HomeAccess Mapping data accessors.                                      */
/* ----------------------------------------------------------------------- */
int                 get_device_mapping    ( PDEVICE_MAPPING* ppDeviceMapping );
void                free_device_mapping   ( PDEVICE_MAPPING* ppDeviceMapping );


#ifdef __cplusplus
}
#endif

#endif
