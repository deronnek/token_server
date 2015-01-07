/*************************************************************************/
/*!
\file error.c
\brief Error login module
 
This file contains various routines associated with loging error messages.

\date Started 11/17/2007
\author George
*/
/*************************************************************************/

#include "tokserver.h"



/*************************************************************************/
/*! Print an error to a log-file. The access to this log file is controled
    via a server-wide global lock.
 
\param fmt is the format string for printing the error message.
\param ... is a variable number of arguments based on the specification 
       at \e fmt.
*/
/**************************************************************************/
void errprintf(const char *fmt,...)
{
  int rc;
  char timestr[32], msg[MAX_STRLEN];
  time_t currentTime;
  va_list argp;

  va_start(argp, fmt);
  vsnprintf(msg, MAX_STRLEN, fmt, argp);
  va_end(argp);

  currentTime = time(NULL);
  ctime_r(&currentTime, timestr);

  /* Get the lock for the output file */
  if ((rc = pthread_mutex_lock(&ServerState.errprintf_mutex))) {
    char lockerr[MAX_STRLEN];
    snprintf(lockerr, MAX_STRLEN, "Failed on locking the errprintf_mutex: %s", gk_strerror(rc));
    fprintf(stderr, "%s:%lu: %s", gk_strtprune(timestr, " \t\n"), (unsigned long int)pthread_self(), lockerr);
    fflush(stderr);
  }

  fprintf(stderr, "%s:%lu: %s", gk_strtprune(timestr, " \t\n"), (unsigned long int)pthread_self(), msg);
  fflush(stderr);

  /* Release the lock for the output file */
  if ((rc = pthread_mutex_unlock(&ServerState.errprintf_mutex))) {
    char lockerr[MAX_STRLEN];
    snprintf(lockerr, MAX_STRLEN, "Failed on unlocking the errprintf_mutex: %s", gk_strerror(rc));
    fprintf(stderr, "%s: %s", gk_strtprune(timestr, " \t\n"), lockerr);
    fflush(stderr);
  }
}


