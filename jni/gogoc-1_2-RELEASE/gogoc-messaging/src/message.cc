// **************************************************************************
// $Id: message.cc,v 1.1 2009/11/20 16:34:56 jasminko Exp $
//
// Copyright (c) 2007 gogo6 Inc. All rights reserved.
//
//   For license information refer to CLIENT-LICENSE.TXT
//
// Description:
//   Implementation of the Message union.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gogocmessaging/message.h>
#include <stddef.h>
#include <assert.h>


namespace gogocmessaging
{
// --------------------------------------------------------------------------
// Function : CreateMessage     [ STATIC ]
//
// Description:
//   Will create a new Message object.
//
// Arguments:
//   aMsgId: uint16_t [IN], The message identifier.
//   aDataLen: uint16_t [IN], The length of data pointed by `aData'.
//   aData: uint8_t* [IN], The message data.
//
// Return values:
//   Pointer to newly created message. NULL is returned on error.
//
// --------------------------------------------------------------------------
Message* Message::CreateMessage( const uint16_t aMsgId, const uint32_t aDataLen, const uint8_t* aData )
{
  const uint32_t cnMsgSize = (MSG_HEADER_LEN) + aDataLen;
  Message* pMsg = NULL;


  // Only create messages that are within message data length bounds.
  if( aDataLen < MSG_MAX_USERDATA )
  {
    // Create enough space to copy user data.
    pMsg = (Message*) new uint8_t[ cnMsgSize ];
    assert( pMsg != NULL );

    // Set message members.
    pMsg->msg.header._msgid = aMsgId;
    pMsg->msg.header._datalen = (uint16_t)aDataLen;
    memcpy( pMsg->msg._data, aData, aDataLen );
  }

  // return the new message.
  return pMsg;
}


// --------------------------------------------------------------------------
// Function : FreeMessage       [ static ]
//
// Description:
//   Will delete the space used by the message.
//
// Arguments:
//   pMsg: Message* [IN,OUT], The message to be deleted.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void Message::FreeMessage( Message* pMsg )
{
  uint8_t* pBuf = (uint8_t*)pMsg;

  delete [] pBuf;
  pMsg = NULL;
}


// --------------------------------------------------------------------------
// Function : AssertMessage       [ static ]
//
// Description:
//   Will perform verification on the message content.
//
// Arguments:
//   pMsg: Message* [IN], The message to be verified.
//
// Return values:
//
//
// --------------------------------------------------------------------------
void Message::AssertMessage( const Message* pMsg )
{
  assert( pMsg->msg.header._datalen <= MSG_MAX_USERDATA );
}


// --------------------------------------------------------------------------
// Function : GetRawSize
//
// Description:
//   Will return the byte size of the data managed by this class:
//   - Message header length + user data length.
//  This is useful to know the amount of bytes to copy from member `rawdata'.
//
// Arguments: (none)
//
// Return values:
//   The size of this class' data.
//
// --------------------------------------------------------------------------
uint32_t Message::GetRawSize( void ) const
{
  return (MSG_HEADER_LEN) + this->msg.header._datalen;
}

}
