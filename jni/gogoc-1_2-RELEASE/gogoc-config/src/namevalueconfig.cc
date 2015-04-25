// **************************************************************************
// $Id: namevalueconfig.cc,v 1.1 2009/11/20 16:30:26 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Implementation of the NameValueConfig class
//
// Description:
//   Extends the Config class to provide NAME=VALUE variables access.
//   To perform LOAD, APPLY, CANCEL, OVERRIDE operations, refer to the
//   Config class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocconfig/namevalueconfig.h>
#include <gogocconfig/namevalueparser.h>
#include <assert.h>


namespace gogocconfig
{
// --------------------------------------------------------------------------
// Function : NameValueConfig constructor
//
// Description:
//   Will initialize a new NameValueConfig object.
//
// Arguments:
//   aFileName: string [IN], The file name of the configuration data.
//   aEAccessMode: enum [IN], The access mode to the configuration data.
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
NameValueConfig::NameValueConfig( const string& aFileName, const t_accessMode aEAccessMode ) :
  Config( aFileName, aEAccessMode )
{
  // Create the parser.
  m_pParser = new NameValueParser( NVP_READ_ALL );
}


// --------------------------------------------------------------------------
// Function : NameValueConfig destructor
//
// Description:
//   Will destroy the class.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
NameValueConfig::~NameValueConfig( void )
{
  assert( m_pParser != NULL );

  // Delete the parser object.
  delete m_pParser;
  m_pParser = NULL;
}


// --------------------------------------------------------------------------
// Function : GetVariableValue
//
// Description:
//   Will return the value associated with the variable name specified.
//   This information is fetched from the configuration data.
//   If the variable name can't be found in the configuration data, an empty
//   string is returned.
//
// Arguments:
//   aName: string [IN], The name of the variable.
//   aValue: string [OUT], The value associated with the variable name.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueConfig::GetVariableValue( const string& aName, string& aValue ) const
{
  t_nameValueData& nv = ((NameValueParser*)m_pParser)->GetConfigurationData();
  t_nameValueData::const_iterator iter;

  // Clear the value string.
  aValue="";

  // Find the requested variable.
  iter = nv.find(aName);
  if( iter != nv.end() )
    aValue = iter->second;    // retrieve the value.
}


// --------------------------------------------------------------------------
// Function : SetVariableValue
//
// Description:
//   Will set a value associated to the variable name specified.
//   This operation will change the in-memory configuration data. If the
//   variable name can't be found in the configuration data, a new variable
//   is created to hold the value.
// NOTE:
//   To apply the change made, call the Config::ApplyConfiguration() function.
//
// Arguments:
//   aName: string [IN], The name of the variable.
//   aValue: string [IN], The value associated with the variable name.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueConfig::SetVariableValue( const string& aName, const string& aValue )
{
  t_nameValueData& nv = ((NameValueParser*)m_pParser)->GetConfigurationData();
  t_fullFileData& ffd = ((NameValueParser*)m_pParser)->GetFullFileData();
  bool bFound = false;
  t_fullFileData::const_iterator iter;


  // Insert in full file data, if it wasn't already present.
  for( iter=ffd.begin(); iter!=ffd.end() && !bFound; iter++ )
    bFound = iter->second == aName;

  if( !bFound )
    ffd.push_back( pair<t_eDataType,string>(NVP_DATA_NV, aName) );

  // Set the value for the name.
  nv[aName] = aValue;
}


// --------------------------------------------------------------------------
// Function : RemoveVariable
//
// Description:
//   Will remove the variable from the configuration data. This operation
//   will change the in-memory configuration data.
// NOTE:
//   To apply the change made, call the Config::ApplyConfiguration() function.
//
// Arguments:
//   aName: string [IN], The name of the variable.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueConfig::RemoveVariable( const string& aName )
{
  t_nameValueData& nv = ((NameValueParser*)m_pParser)->GetConfigurationData();
  t_fullFileData& ffd = ((NameValueParser*)m_pParser)->GetFullFileData();

  ffd.remove( pair<t_eDataType,string>(NVP_DATA_NV, aName) );
  nv.erase( aName );
}


// --------------------------------------------------------------------------
// Function : GetVariableNameList
//
// Description:
//   Will retrieve all the variable names from the configuration data.
//
// Arguments:
//   aList: list<string> [OUT], Will contain a list of strings, containing
//          all variable names, from the configuration data.
//
// Return values: (none)
//
// Exceptions: (none)
//
// --------------------------------------------------------------------------
void NameValueConfig::GetVariableNameList( stringlist& aList ) const
{
  t_nameValueData& nv = ((NameValueParser*)m_pParser)->GetConfigurationData();
  t_nameValueData::const_iterator iter;

  // Empty the list first.
  aList.clear();

  // Iterate on every variable name, and store it in the list.
  for( iter=nv.begin(); iter!=nv.end(); iter++)
    aList.push_back( iter->first );
}

}

