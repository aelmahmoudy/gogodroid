// **************************************************************************
// $Id: namevalueparser.cc,v 1.1 2009/11/20 16:30:26 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
// 
//   For license information refer to CLIENT-LICENSE.TXT
//
// Implementation of the NameValueParser class
//
// Description:
//   Provides a means of parsing NAME=VALUE UNIX-style configuration files.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocconfig/namevalueparser.h>
#include <gogocconfig/gogocuistrings.h>
#include <iostream>
#include <fstream>
#include <pal.h>
#include <assert.h>


#define NV_SEPARATOR          "="                   // Name=value separator
#define NV_COMMENT_START      '#'                   // Comment line
#define NV_MAX_LINE_LENGTH    2048                  // Maximum line length
#define NV_COMMENT_GENERATE   "# Generated on: "    // Special comment.


// --------------------------------------------------------------------------
// Function : trim  [LOCAL]
//
// Description:
//  Will remove leading and trailing whitespaces from the string.
//
// Arguments:
//   s: string [IN], The string to trim. (not modified)
//
// Return values:
//   The string `s' without the leading and trailing whitespaces
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
#ifndef NO_STDLIBCXX
string trim( const string& s )
{
  if(s.length() == 0)
    return s;
  string::size_type b = s.find_first_not_of(" \t");
  string::size_type e = s.find_last_not_of(" \t");
  if(b == string::npos) // No non-spaces
    return "";
  return string(s, b, e - b + 1);
}
#endif

namespace gogocconfig
{
// --------------------------------------------------------------------------
// Function : NameValueParser constructor
//
// Description:
//   Will initialize a new NameValueParser object.
//
// Arguments:
//   aReadMode: enum [IN, facultative], Contains the read access mode.
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
NameValueParser::NameValueParser( const t_eReadMode aReadMode )
 : Parser(),
   m_eReadMode( aReadMode )
{
  m_NameValue.clear();
  m_FullFileData.clear();
}


// --------------------------------------------------------------------------
// Function : NameValueParser destructor
//
// Description:
//   Will perform clean-up tasks.
//
// Arguments: (N/A)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
NameValueParser::~NameValueParser( void )
{
}


// --------------------------------------------------------------------------
// Function : ReadConfigurationData
//
// Description:
//   Will read a configuration file, to extract the NAME=VALUE pairs. If read
//   mode is ALL(NVP_READ_ALL), the entire file will be read, including white
//   space lines and comments.
//
// Arguments:
//   aFilename: string [IN], Configuration data will be read from this file.
//
// Return values:
//   true upon successful read operation, false otherwise.
//
// Exceptions:
//   - When failed to open the filename provided.
//   - When a line (that is not a comment nor whitespaces) does not contain 
//     the `=' character.
//
// --------------------------------------------------------------------------
bool NameValueParser::ReadConfigurationData( const string& aFilename )
{
  std::ifstream inStream;
  char buf[NV_MAX_LINE_LENGTH];
  char *buffer = buf;
  char *name, *value;


  // Initialization.
  memset(&buf, 0, NV_MAX_LINE_LENGTH);


  // Open specified file.
  inStream.open( aFilename.c_str(), std::ios::in );
  if( !inStream.is_open() )
  {
    // Open failed.
    throw GOGOC_UIS__NMP_OPENFAIL;
  }

  // Clear the current configuration data, if any.
  m_NameValue.clear();
  m_FullFileData.clear();

  // Loop on the entire file until end.
  while( !inStream.eof() )
  {
    inStream.getline( buffer, NV_MAX_LINE_LENGTH );


    // Move to the first non-space character
    while( *buffer == ' ' )
      ++buffer;

    // Verify if whitespace.
    if( *buffer == '\0' )
    {
      AddEmptyLine();
      continue;
    }

    // Verify if comment.
    if( *buffer == NV_COMMENT_START )
    {
      AddComment( buffer );
      continue;
    }


    // Extract Name=Value pair
    if( (name = strtok(buffer, NV_SEPARATOR)) !=  NULL )
    {
      // Extract value (may be null).
      value = strtok(NULL, ""); // Get remainder of string.

      // Store the name value pair.
      AddNameValue( name, (value == NULL) ? "" : value );
    }
    else
    {
      // Line did not contain a "=" separator
      throw GOGOC_UIS__NMP_BADCONFIGFILE;
    }
  }

  // Close opened handle.
  inStream.close();

  return true;
}


// --------------------------------------------------------------------------
// Function : WriteConfigurationData
//
// Description:
//   Will write the internal configuration to the specified file. If read
//   mode is ALL(NVP_READ_ALL), the entire file will be overwritten with the
//   contents of the NAME=VALUE pairs, including comments and white space
//   lines.
//
// Arguments:
//   aFilename: string [IN], Configuration data will be written to this file.
//
// Return values:
//   true upon successful write operation, false otherwise.
//
// Exceptions:
//   - When failed to open file for read access.
//
// --------------------------------------------------------------------------
bool NameValueParser::WriteConfigurationData( const string& aFilename )
{
  std::ofstream outStream;
  time_t rawtime;
  struct tm * timeinfo;


  outStream.open( aFilename.c_str() );
  if( !outStream.is_open() ) 
  {
		// Open failed.
		throw GOGOC_UIS__NMP_OPENFAILWRITE;
  }

  // ---------------------------------------------------------
  // Format current time for timestamp in configuration file.
  // ---------------------------------------------------------
  pal_time( &rawtime );
  timeinfo = pal_localtime( &rawtime );


  // ------------------------------
  // Write the configuration data.
  // ------------------------------
  if( m_eReadMode == NVP_READ_ALL )
  {
    t_fullFileData::iterator iter;
    t_nameValueData::iterator iterMap;

    for( iter = m_FullFileData.begin(); iter != m_FullFileData.end(); iter++ )
    {
      // Verify what kind of data:
      switch( iter->first )
      {
        case NVP_DATA_NV:
          iterMap = m_NameValue.find( iter->second );
          if( iterMap != m_NameValue.end() )
            outStream << iter->second << NV_SEPARATOR << iterMap->second << endl;
          break;
        
        case NVP_DATA_COMMENT:
          if( strncmp( iter->second.c_str(), NV_COMMENT_GENERATE, strlen(NV_COMMENT_GENERATE) ) == 0 )
          {
            outStream << NV_COMMENT_GENERATE << 
                          (timeinfo->tm_year+1900) << "/" <<
                          (timeinfo->tm_mon+1) << "/" <<
                          timeinfo->tm_mday << " " <<
                          timeinfo->tm_hour << ":" <<
                          timeinfo->tm_min << ":" <<
                          timeinfo->tm_sec << endl;
          }
          else
            outStream << iter->second << endl;
          break;
        
        case NVP_DATA_EMPTYLINE:
          outStream << endl;
          break;
      }
    }
  }
  else
  {
    t_nameValueData::iterator iter;

    for(iter = m_NameValue.begin(); iter != m_NameValue.end(); iter++ )
      outStream << iter->first << NV_SEPARATOR << iter->second << endl << endl;  
  }

  // Close opened handle.
  outStream.close();

  return true;
}


// --------------------------------------------------------------------------
// Function : AddEmptyLine [PROTECTED]
//
// Description:
//   Will add an empty line to the configuration file data, only if read mode
//   is ALL(NVP_READ_ALL).
//
// Arguments: (none)
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueParser::AddEmptyLine( void )
{
  if( m_eReadMode == NVP_READ_ALL )
    m_FullFileData.push_back( pair<t_eDataType,string>(NVP_DATA_EMPTYLINE, "") );
}


// --------------------------------------------------------------------------
// Function : AddComment [PROTECTED]
//
// Description:
//   Will add a comment line to the configuration file data, only if read mode
//   is ALL(NVP_READ_ALL).
//
// Arguments:
//   aComment: string [in], A comment extracted from the configuration file.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueParser::AddComment( const string& aComment )
{
  assert( aComment.size() != 0 );

  if( m_eReadMode == NVP_READ_ALL )
    m_FullFileData.push_back( pair<t_eDataType,string>(NVP_DATA_COMMENT, aComment) );
}


// --------------------------------------------------------------------------
// Function : AddNameValue [PROTECTED]
//
// Description:
//   Will add the NAME=VALUE pair in the configuration data.
//
// Arguments:
//   aName: string [in], The name
//   aValue: string [in], the value
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueParser::AddNameValue( const string& aName, const string& aValue )
{
  assert( aName.size() != 0 );

  // Remove trailing and leading whitespaces
#ifndef NO_STDLIBCXX  
  string name  = trim( aName );
  string value = trim( aValue );
#else 
  string name = aName;
  string value = aValue;
#endif
 
  // The case someone would mischievously put a line like that in the config 
  // file: "<spaces>=<something>".
  if( !name.empty() )
  {
    if( m_eReadMode == NVP_READ_ALL )
      m_FullFileData.push_back( pair<t_eDataType,string>(NVP_DATA_NV, name) );

    m_NameValue[name] = value;
  }
}

}
