/*!
\file  
\brief The driver for the CF xml-rpc server 

\date 11/24/2007
\author George
\version \verbatim $Id: main.c 2805 2007-12-01 23:08:55Z karypis $  \endverbatim
*/

#define MTCFSRV_MAIN 

#include "tokserver.h"

void init_defaults(params_t *params) {
  /* initialize the params data structure */
  params->port             = 3703;
  params->daemon           = 0;
  params->sthread          = 0;
  params->nocheck          = 1;
  params->loginfo          = 0;
  params->backup_enabled   = 1;
  params->logfile          = NULL;
  params->configfile       = gk_strdup("/safestore/tokenization_server/cfg/tokserver.cfg");
  params->journaldir       = gk_strdup("/safestore/tokenization_server/journals");
  params->dictdir          = gk_strdup("/safestore/tokenization_server/dict");
  params->logfile          = gk_strdup("/safestore/tokenization_server/log/tokserver.log");
  params->dbtest           = gk_strdup("/opt/bin/isdbcorrupt");
  params->default_stoplist = stoplist_default();
}

/*******************************************************************************/
/*! The main function for the server */
/*******************************************************************************/
int main(int argc, char **argv) 
{
  //printf("XXX pid: %ld\n",getpid());
  /* set up the default parameters */
  init_defaults(&ServerState);
printf("Read defaults\n");
  /* Parse command-line arguments */
  parse_cmdline(&ServerState, argc, argv);
printf("Parsed command line\n");

  /* Parse the config file */
  config_ReadConfigFile(&ServerState);
printf("Read config file\n");

  /* Initialize the server */
  InitializeServer();
  printf("Init server done\n");

  //read_config(ServerState);

  if (ServerState.daemon) {
    printf("turning into a daemon\n");
    Daemonize();
    printf("...done\n");
  }
    
  /* if (ServerState.sthread) {
    SingleThreadedServer();
  }
  else { */
    MultiThreadedServer();
  //}
}



/*******************************************************************************/
/*! This function initialize our own server state variables */
/*******************************************************************************/
void InitializeServer() 
{
  int rc;

  ServerState.dtionaries = hash_Create();

  /* Initialize the server's mutexes */
  if ((rc = pthread_mutex_init(&(ServerState.srvstate_mutex), NULL))) {
    fprintf(stderr, "Failed to initialize srvstate_mutex: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }
  if ((rc = pthread_mutex_init(&(ServerState.errprintf_mutex), NULL))) {
    fprintf(stderr, "Failed to initialize errprintf_mutex: %s\n", strerror(rc));
    exit(EXIT_FAILURE);
  }
  /* Initialize the server's rwlocks */
  if ((rc = pthread_rwlock_init(&(ServerState.srvstate_rwlock), NULL))) {
    fprintf(stderr, "Failed to initialize srvstate_rwlock: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }
  if ((rc = pthread_rwlock_init(&(ServerState.statslock), NULL))) {
    fprintf(stderr, "Failed to initialize statslock: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }
  if ((rc = pthread_rwlock_init(&(ServerState.testlock), NULL))) {
    fprintf(stderr, "Failed to initialize testlock: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }


  /* Initialize my error handling */
  gk_set_exit_on_error(0);

}
