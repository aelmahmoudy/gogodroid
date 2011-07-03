// **************************************************************************
// $Id: namevalueparser.h,v 1.1 2009/11/20 16:30:25 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Provides a means of parsing NAME=VALUE UNIX-style configuration files.
//   This class extends the generic Parser class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#ifndef __gogocconfig_namevalueparser_h__
#define __gogocconfig_namevalueparser_h__


#include <gogocconfig/parser.h>
#include <map>
#include <list>
using namespace std;


namespace gogocconfig
{
  // Type definitions.
  typedef enum { NVP_READ_ALL, NVP_READ_NAMEVALUE } t_eReadMode;
  typedef enum { NVP_DATA_NV, NVP_DATA_COMMENT, NVP_DATA_EMPTYLINE } t_eDataType;

  typedef map<string,string> t_nameValueData;
  typedef list<pair<t_eDataType,string> > t_fullFileData;


  // ------------------------------------------------------------------------
  class NameValueParser : public Parser
  {
  protected:
    t_eReadMode     m_eReadMode;          // Read mode.
    t_nameValueData m_NameValue;          // Holds the Name=Value data.
    t_fullFileData  m_FullFileData;       // Holds the full file data.

  public:
    // Construction / destruction
                    NameValueParser       ( const t_eReadMode aReadMode = NVP_READ_NAMEVALUE );
    virtual         ~NameValueParser      ( void );

    // Overrides
    virtual bool    ReadConfigurationData ( const string& aFilename );
    virtual bool    WriteConfigurationData( const string& aFilename );

    // Public configuration accessor
    t_nameValueData& GetConfigurationData ( void ) { return m_NameValue; }
    t_fullFileData& GetFullFileData       ( void ) { return m_FullFileData; }

  protected:
    virtual void    AddEmptyLine          ( void );
    virtual void    AddComment            ( const string& aComment );
    virtual void    AddNameValue          ( const string& aName, const string& aValue );
  };

}

#endif
