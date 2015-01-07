/*!
\file  
\brief Various routines associated with creating a daemon

\date 11/24/2007
\author George
\version \verbatim $Id: daemon.c 2760 2007-11-26 20:17:39Z karypis $  \endverbatim
*/

#include "tokserver.h"



/*******************************************************************************/
/*! The main function for the server */
/*******************************************************************************/
void Daemonize() 
{
  pid_t pid, sid; 
  time_t tstamp;
  char filename[MAX_STRLEN];
  FILE *PIDOUT;
  /* Fork off the parent process */
//printf("XXX Daemon.\n");

  pid = fork();
  if (pid < 0) {
    openlog("toksrv", LOG_CONS|LOG_PERROR|LOG_PID, LOG_DAEMON);
    syslog(LOG_ERR, "Failed to fork() the daemon.");
    closelog();
    exit(EXIT_FAILURE);
  }


  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    PIDOUT = gk_fopen("/var/run/toksrv.pid","w","/var/run/toksrv.pid");
    fprintf(PIDOUT,"%d\n",pid);
    gk_fclose(PIDOUT);
    exit(EXIT_SUCCESS);
  }

  /* Change the file mode mask */
  umask(0);       

  
  fprintf(stderr,"logfile: %s\n",ServerState.logfile);
        
  /* Open the syslog for login errors and success messages */
  openlog("toksrv", LOG_CONS|LOG_PERROR|LOG_PID, LOG_DAEMON);

  /* Close out the stdin and stdout file descriptors */
  close(STDIN_FILENO);
//  close(STDOUT_FILENO);

  /* Redirect the stderr to go to the logfile */
  if (gk_fexists(ServerState.logfile)) {
    tstamp = time(NULL);
    sprintf(filename, "%s_%d", ServerState.logfile, (int)tstamp);
    if (rename(ServerState.logfile, filename) == -1) {
      syslog(LOG_ERR, "Failed to rename the logfile: [%s, %s].", ServerState.logfile, filename);
      goto ERROR_EXIT;
    }
  }

  stderr = freopen(ServerState.logfile, "w", stderr);
  if (stderr == NULL) {
    syslog(LOG_ERR, "Failed to freopen the logfile: %s.", ServerState.logfile);
    goto ERROR_EXIT;
  }

  stdout = freopen("/dev/null", "w", stdout);
  if (stdout == NULL) {
    syslog(LOG_ERR, "Failed to freopen /dev/null for stdout");
    goto ERROR_EXIT;
  }

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    syslog(LOG_ERR, "Failed to create a new SID for the child process.");
    goto ERROR_EXIT;
  }
        
  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    syslog(LOG_ERR, "Failed to chdir to /.");
    goto ERROR_EXIT;
  }
  
        
  syslog(LOG_INFO, "Succeeded in installing the daemon.");
  return;

ERROR_EXIT:
  closelog();
  exit(EXIT_FAILURE);
}

