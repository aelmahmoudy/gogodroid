// **************************************************************************
// $Id: haccessmsgdata.h,v 1.1 2009/11/20 16:34:52 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   This include file describes the domain values and data that will be
//   exchanged between the gogoCLIENT GUI and gogoCLIENT
//   in the scope of the HACCESS project.
//
//   These structures must be defined in C-style for integration
//   in the gogoCLIENT.
//
//   You may extend the structures to include new data, - BUT -
//   Remember to do the following:
//   - Translate the new data.
//   - Encode then new data.
//   - *THINK* of backwards compatibility (Maybe crate a new message ID
//      for the extended structure and preserve functionnality).
//
// Author: Charles Nepveu
//
// Creation Date: February 2007
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocmessaging_haccessmsgdata_h__
#define __gogocmessaging_haccessmsgdata_h__



// HACCESS Configuration Information: HACCESSConfigInfo - (Data Structure)
//   - haccess_doc_root:       WWW document root.
//   - haccess_proxy_enabled:  Boolean value indicating if the proxy part of 
//                          HACCESS is enabled.
//   - haccess_web_enabled:    Boolean value indicating if the web part of 
//                          HACCESS is enabled.
//   - haccess_devmap_changed: Boolean value indicating whether the HACCESS device
//                          mappings have been changed.
//
typedef struct __HACCESS_CONFIG_INFO
{
  char*         haccess_doc_root;
  signed short  haccess_proxy_enabled;
  signed short  haccess_web_enabled;
  signed short  haccess_devmap_changed;
} HACCESSConfigInfo;


// HACCESS Device Mapping Statuses: HACCESSDevMapStts - (Enumeration)
//   - SUCCESS: Device mapping was successfully pushed to the DDNS server.
//   - ERROR: Device mapping failed to be pushed to the DDNS server.
//   - NEW: (GUI ONLY) New device mapping entered by the user.
//   - MODIFIED: (GUI ONLY) New or existing device mapping modified by user.
//   - UNKNOWN: (GUI ONLY) Device mapping default status.
// See "Appendix A" in HACCESS client integration design document.
//
typedef enum { HACCESS_DEVMAPSTTS_SUCCESS,
               HACCESS_DEVMAPSTTS_ERROR,
               HACCESS_DEVMAPSTTS_NEW,
               HACCESS_DEVMAPSTTS_MODIFIED,
               HACCESS_DEVMAPSTTS_UNKNOWN } HACCESSDevMapStts;


// gogoCLIENT broker list: gogocBrokerList - (Data structure)
//   - device_name: Name of the device.
//   - mapping_status: Status for the device mapping.
//   - next: Next element in the list: NULL if end.
//
struct __MAPPING_STATUS;
typedef struct __MAPPING_STATUS
{
  char*           device_name;
  HACCESSDevMapStts  mapping_status;
  struct __MAPPING_STATUS* next;
} MAPPING_STATUS, *PMAPPING_STATUS;


// HACCESS Feature Statuses: HACCESSFeatStts - (Enumeration)
//   - SUCCESS: HACCESS Feature has started and is functionning correctly.
//   - ERROR: HACCESS feature is unavailable; an error occured.
// See "Appendix A" in HACCESS client integration design document.
//
typedef enum { HACCESS_FEATSTTS_SUCCESS,
               HACCESS_FEATSTTS_ERROR } HACCESSFeatStts;


// HACCESS Status Information: HACCESSStatusInfo - (Data Structure)
//   - haccess_proxy_status:  Status of the HACCESS proxy.
//   - haccess_web_status:  Status of the HACCESS web server.
//   - haccess_devmapmod_status: Status of the Device Mapping Module.
//   - haccess_devmap_status: Linked list of mapping statuses.
//
typedef struct __HACCESS_STATUS_INFO
{
  HACCESSFeatStts    haccess_proxy_status;
  HACCESSFeatStts    haccess_web_status;
  HACCESSFeatStts    haccess_devmapmod_status;
  PMAPPING_STATUS haccess_devmap_statuses;
} HACCESSStatusInfo;


#endif
