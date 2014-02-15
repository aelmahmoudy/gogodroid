/* *********************************************************************** */
/* $Id: gogoc_c_wrapper.h,v 1.2 2010/02/08 21:40:06 jasminko Exp $           */
/*                                                                         */
/* Copyright (c) 2007 gogo6 Inc. All rights reserved.                     */
/*                                                                         */
/*   For license information refer to CLIENT-LICENSE.TXT                   */
/*                                                                         */
/* Description:                                                            */
/*   Wraps the gogoCLIENT Configuration subsystem to offer C access.  */
/*                                                                         */
/* Author: Charles Nepveu                                                  */
/*                                                                         */
/* Creation Date: November 2006                                            */
/* _______________________________________________________________________ */
/* *********************************************************************** */
#ifndef __gogocconfig_gogoc_c_wrapper_h__
#define __gogocconfig_gogoc_c_wrapper_h__


#ifndef TBOOLEAN_DECLARED
#define TBOOLEAN_DECLARED
/* ----------------------------------------------------------------------- */
/* tBoolean is also declared in the gogoCLIENT. This is to avoid      */
/* redefinition.                                                           */
/* ----------------------------------------------------------------------- */
#undef FALSE
#undef TRUE
typedef enum { FALSE=0, TRUE } tBoolean;
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* ----------------------------------------------------------------------- */
/* Configuration management functions.                                     */
/* ----------------------------------------------------------------------- */
int                 initialize            ( const char* );
void                un_initialize         ( void );
void                get_config_errors     ( int*, unsigned int*[] );
int                 save_config           ( void );

/* ----------------------------------------------------------------------- */
/* Configuration accessors.                                                */
/* ----------------------------------------------------------------------- */
void                get_user_id           ( char** );
void                set_user_id           ( char * );
void                get_passwd            ( char** );
void                set_passwd            ( char * );
void                get_server            ( char** );
void                set_server            ( char * );
void                get_host_type         ( char** );
void                get_prefixlen         ( int* );
void                get_ifprefix          ( char** );
void                get_dns_server        ( char** );
void                get_gogoc_dir         ( char** );
void                get_auth_method       ( char** );
void                get_auto_retry_connect( tBoolean* );
void                get_retry_delay       ( int* );
void                get_retry_delay_max   ( int* );
void                get_keepalive         ( tBoolean* );
void                get_keepalive_interval( int* );
void                get_tunnel_mode       ( char** );
void                get_if_tun_v6v4       ( char** );
void                get_if_tun_v6udpv4    ( char** );
void                get_if_tun_v4v6       ( char** );
void                get_client_v4         ( char** );
void                get_client_v6         ( char** );
void                get_template          ( char** );
void                get_proxy_client      ( tBoolean* );
void                get_broker_list_file  ( char** );
void                get_last_server_file  ( char** );
void                get_always_use_last_server( tBoolean* );
void                get_log               ( const char*, short* );
void                get_log_filename      ( char** );
void                get_log_rotation      ( tBoolean* );
void                get_log_rotation_sz   ( int* );
void                get_log_rotation_del  ( tBoolean* );
void                get_syslog_facility   ( char** );
void                get_haccess_proxy_enabled( tBoolean* );
void                get_haccess_web_enabled  ( tBoolean* );
void                get_haccess_document_root( char** );
void                get_dslite_server     ( char** );
void                get_dslite_client     ( char** );

#ifdef __cplusplus
}
#endif

#endif
