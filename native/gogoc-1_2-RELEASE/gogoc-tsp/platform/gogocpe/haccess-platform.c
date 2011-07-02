/*
----------------------------------------------------------------------
 haccess.c - HACCESS Module -- Dongle6/Linux Specific
----------------------------------------------------------------------
 $Id: haccess-platform.c,v 1.1 2010/03/07 19:39:08 carl Exp $
----------------------------------------------------------------------
Copyright (c) 2007 gogo6 Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with gogo6 Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
----------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "hex_strings.h"
#include "haccess-private.h"


/*
================================================
================================================
                PRIVATE VARIABLES
================================================
================================================
*/

/* A condition variable that can be waited on and */
/* that will be signaled by the DDNS request */
/* processing thread when the processing of the */
/* DDNS deletion requests that are generated when */
/* the devices are hidden at client shutdown has */
/* completed. */
pthread_cond_t _haccess_ddns_device_hiding_done_event;

/* A flag indicating that DDNS device hiding is done. */
/* It is enabled when we receive a signal that */
/* the DDNS request processing queue is empty while */
/* in the device hiding state. */
/* It is used as part of the boolean predicate checked */
/* to protect from spurious exits of the pthread_cond_timedwait */
/* function. A simple check of whether or not the DDNS */
/* request processing queue is empty is not a sufficient */
/* predicate as the spurious exit might occur in the */
/* timeframe during which the last DDNS request has been popped */
/* from the queue - making it empty - but not yet processed. */
/* Adding this flag to the check ensures that the queue is */
/* empty AND that the last request has been processed */
/* because the signal is sent only after the last */
/* request has been fully processed. */
_haccess_boolean _haccess_ddns_device_hiding_done;

/* The mutex associated with the above condition */
/* variable. */
pthread_mutex_t _haccess_ddns_device_hiding_done_mutex;

/* A flag indicating whether the client is in */
/* the device hiding state. In other words, */
/* it will be true if the client is shutting down */
/* and generating DDNS deletion requests. This is */
/* used to make sure that signals indicating */
/* that the DDNS request queue is empty are */
/* otherwise ignored. */
_haccess_boolean _haccess_ddns_device_hiding_active;

/* The mutex used to protect the shared variable. */
pthread_mutex_t _haccess_ddns_device_hiding_active_mutex;

/* The saved handle to the DDNS processing thread. */
pthread_t _haccess_ddns_processing_thread_handle;


/*
================================================
================================================
         PRIVATE FUNCTION IMPLEMENTATIONS
================================================
================================================
*/


/*
================================================
FUNCTION _haccess_platform_ddns_processing_thread

DESCRIPTION

  The function that is executed by the DDNS
  processing thread.

ARGUMENTS

  void *arguments
    The thread arguments (actually a tTunnel *).

RETURN VALUE

  Just conforming to the pthread API.

NOTES

TODO
================================================
*/
void *
_haccess_platform_ddns_processing_thread(void *arguments)
{
  haccess_status status = HACCESS_STATUS_OK;

  /* Iterate until the HACCESS module signals that */
  /* it is destructing. */
  while (!_haccess_platform_module_is_destructing())
  {
    /* Call the thread function from the HACCESS module */
    /* that will do the real thread work. */
    status = _haccess_ddns_processing_thread(arguments);

    if (status != HACCESS_STATUS_OK)
    {
      _HACCESS_LOG_ERROR(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_PROCESSING_DDNS_REQS)
    }

    /* Sleep until the next iteration. */
    errno = 0;
    if (sleep(_HACCESS_DDNS_PROCESSING_THREAD_DELAY) || errno == EINTR)
        break;
  }

  pthread_exit(PTHREAD_CANCELED);
  return (void *)NULL;
}

/*
================================================
FUNCTION _haccess_platform_start_ddns_processing_thread

DESCRIPTION

  Platform-specific function to start the DDNS
  processing thread.

ARGUMENTS

  void *arguments
    The thread arguments (actually a tTunnel *).

RETURN VALUE

  haccess_status
    Function execution status.

NOTES

TODO
================================================
*/
haccess_status
_haccess_platform_start_ddns_processing_thread(void *arguments)
{
  haccess_status status = HACCESS_STATUS_OK;

  _HACCESS_ENTER_FUNCTION

  /* Start the thread, and save the handle to be able to use it later on. */
  if (pthread_create(&_haccess_ddns_processing_thread_handle, NULL, &_haccess_platform_ddns_processing_thread, (void *)arguments) != 0)
  {
    /* The thread failed to start, that's an error. */
    _HACCESS_LOG_DEBUG(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_START_PLATFORM_THREAD)

    status = HACCESS_STATUS_ERR;

    _HACCESS_LEAVE_FUNCTION(status)
  }

  pthread_detach(_haccess_ddns_processing_thread_handle);

  _HACCESS_LEAVE_FUNCTION(status)
}

/*
=======================================================
FUNCTION _haccess_platform_signal_ddns_request_queue_empty

DESCRIPTION

  Platform-specific function called to indicate
  that the DDNS request processing queue is empty.
  It will be called multiple times.

ARGUMENTS

  None.

RETURN VALUE

  haccess_status
    Function execution status.

NOTES

TODO
================================================
*/
haccess_status
_haccess_platform_signal_ddns_request_queue_empty()
{
  _haccess_boolean ready_for_signal = _HACCESS_FALSE;
  haccess_status status = HACCESS_STATUS_OK;

  _HACCESS_ENTER_FUNCTION

  pthread_mutex_lock(&_haccess_ddns_device_hiding_active_mutex);

  ready_for_signal = _haccess_ddns_device_hiding_active;

  pthread_mutex_unlock(&_haccess_ddns_device_hiding_active_mutex);

  if (ready_for_signal == _HACCESS_TRUE)
  {
    pthread_mutex_lock(&_haccess_ddns_device_hiding_done_mutex);

    _haccess_ddns_device_hiding_done = _HACCESS_TRUE;

    if (pthread_cond_signal(&_haccess_ddns_device_hiding_done_event) != 0)
    {
      _HACCESS_LOG_DEBUG(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_SIGNAL_HIDING_EVENT)

      status = HACCESS_STATUS_ERR;
    }

    pthread_mutex_unlock(&_haccess_ddns_device_hiding_done_mutex);

    pthread_mutex_lock(&_haccess_ddns_device_hiding_active_mutex);

    _haccess_ddns_device_hiding_active = _HACCESS_FALSE;

    pthread_mutex_unlock(&_haccess_ddns_device_hiding_active_mutex);
  }

  _HACCESS_LEAVE_FUNCTION(status)
}

/*
=======================================================
FUNCTION _haccess_platform_wait_ddns_hiding_done

DESCRIPTION

  Platform-specific function called to wait
  until the DDNS request processing thread has
  finished processing the deletion requests
  or a timeout expires.

ARGUMENTS

  int hiding_done_timeout
    The maximum number of seconds to
    wait if we aren't otherwise notified.

RETURN VALUE

  haccess_status
    Function execution status.

NOTES

TODO
================================================
*/
haccess_status
_haccess_platform_wait_ddns_hiding_done(int hiding_done_timeout)
{
  struct timespec timeout;
  _haccess_platform_event_wait_state state = _HACCESS_PLATFORM_HIDING_WAIT_LOOP;
  haccess_status status = HACCESS_STATUS_OK;

  _HACCESS_ENTER_FUNCTION

  pthread_mutex_lock(&_haccess_ddns_device_hiding_active_mutex);

  _haccess_ddns_device_hiding_active = _HACCESS_TRUE;

  pthread_mutex_unlock(&_haccess_ddns_device_hiding_active_mutex);

  clock_gettime(CLOCK_REALTIME, &timeout);

  timeout.tv_sec += hiding_done_timeout;

  while (state == _HACCESS_PLATFORM_HIDING_WAIT_LOOP)
  {
    pthread_mutex_lock(&_haccess_ddns_device_hiding_done_mutex);

    switch (pthread_cond_timedwait(&_haccess_ddns_device_hiding_done_event,
                                   &_haccess_ddns_device_hiding_done_mutex,
                                   &timeout))
    {
      case 0:
        if (_haccess_ddns_device_hiding_done == _HACCESS_TRUE)
        {
          state = _HACCESS_PLATFORM_HIDING_WAIT_PROCEED;
        }

        break;
      case ETIMEDOUT:
        if (_haccess_ddns_device_hiding_done == _HACCESS_TRUE)
        {
          state = _HACCESS_PLATFORM_HIDING_WAIT_PROCEED;
        }
        else
        {
          state = _HACCESS_PLATFORM_HIDING_WAIT_TIMEOUT;
        }

        break;
      default:
        state = _HACCESS_PLATFORM_HIDING_WAIT_ERROR;
        break;
    }

    pthread_mutex_unlock(&_haccess_ddns_device_hiding_done_mutex);
  }

  if (state == _HACCESS_PLATFORM_HIDING_WAIT_TIMEOUT)
  {
    _HACCESS_LOG_ERROR(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_HIDING_TIMEOUT)
  }
  else if (state == _HACCESS_PLATFORM_HIDING_WAIT_ERROR)
  {
    _HACCESS_LOG_DEBUG(HACCESS_LOG_PREFIX_ERROR GOGO_STR_HACCESS_ERR_CANT_WAIT_HIDING_EVENT)
  }

  _HACCESS_LEAVE_FUNCTION(status)
}


/*
================================================
FUNCTION _haccess_platform_module_is_destructing

DESCRIPTION

  The function called by the DDNS processing
  thread to see if it should stop iterating
  because the HACCESS module is destructing.

ARGUMENTS

  None.

RETURN VALUE

  int
    Whether or not the module is destructing.

NOTES

TODO
  Process possible errors!
================================================
*/
int
_haccess_platform_module_is_destructing()
{
  int destructing = 0;

  /* Always return as if destruction is NOT occuring */
  /* to make sure the DDNS request processing thread */
  /* doesn't stop which, with uClibc's pthread implementation */
  /* would kill the main thread. */

  return destructing;
}

/*
================================================
FUNCTION _haccess_platform_set_module_destructing

DESCRIPTION

  The function used by the HACCESS module to indicate
  that it is destructing.

ARGUMENTS

  int thread_wait_delay
    The number of milliseconds to wait for the
    DDNS processing thread to terminate after
    destruction has been signaled.

RETURN VALUE

  haccess_status
    Function execution status.

NOTES

TODO
================================================
*/
haccess_status
_haccess_platform_set_module_destructing(int thread_wait_delay)
{
  haccess_status status = HACCESS_STATUS_OK;

  _HACCESS_ENTER_FUNCTION

  /* Not stopping the DDNS request processing thread here */
  /* as it seems that, with uClbic's pthread implementation, */
  /* it would also stop and kill the parent thread. */

  _HACCESS_LEAVE_FUNCTION(status)
}


/*
================================================
FUNCTION _haccess_platform_initialize

DESCRIPTION

  Platform-specific initialization function.

ARGUMENTS

  None.

RETURN VALUE

  haccess_status
    Function execution status.

NOTES

TODO
================================================
*/
haccess_status
_haccess_platform_initialize()
{
  haccess_status status = HACCESS_STATUS_OK;

  _HACCESS_ENTER_FUNCTION

  /* Initialize general global variables. */
  _haccess_ddns_device_hiding_done = _HACCESS_FALSE;
  _haccess_ddns_device_hiding_active = _HACCESS_FALSE;

  /* Initialize the condition variables. */
  if (pthread_cond_init(&_haccess_ddns_device_hiding_done_event, NULL) != 0)
  {
    status = HACCESS_STATUS_ERR;
  }
  /* Initialize the mutexes. */
  else if (pthread_mutex_init(&_haccess_ddns_device_hiding_done_mutex, NULL) != 0)
  {
    status = HACCESS_STATUS_ERR;
  }
  else if (pthread_mutex_init(&_haccess_ddns_device_hiding_active_mutex, NULL) != 0)
  {
    status = HACCESS_STATUS_ERR;
  }

  _HACCESS_LEAVE_FUNCTION(status)
}


/*
================================================
FUNCTION _haccess_platform_destruct

DESCRIPTION

    Platform-specific destruction function.

ARGUMENTS

  None.

RETURN VALUE

  haccess_status
    Function execution status.

NOTES

TODO
================================================
*/
haccess_status
_haccess_platform_destruct()
{
  int cancel_status = 0;
  haccess_status status = HACCESS_STATUS_OK;

  _HACCESS_ENTER_FUNCTION

  /* Cancel the DDNS request processing thread. */
  cancel_status = pthread_cancel(_haccess_ddns_processing_thread_handle);

  if ((cancel_status != 0) && (cancel_status != ESRCH))
  {
    status = HACCESS_STATUS_ERR;
  }

  /* Destroy the mutexes. */
  if (pthread_mutex_destroy(&_haccess_ddns_device_hiding_active_mutex) != 0)
  {
    status = HACCESS_STATUS_ERR;
  }
  else if (pthread_mutex_destroy(&_haccess_ddns_device_hiding_done_mutex) != 0)
  {
    status = HACCESS_STATUS_ERR;
  }
  /* Destroy the condition variables. */
  else if (pthread_cond_destroy(&_haccess_ddns_device_hiding_done_event) != 0)
  {
    status = HACCESS_STATUS_ERR;
  }

  _HACCESS_LEAVE_FUNCTION(status)
}
