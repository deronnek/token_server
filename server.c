/*!
\file  
\brief The driver for the tokenization xml-rpc server 

\date 10/21/2009
\author Kevin
\version \verbatim $Id: server.c 4258 2008-04-15 23:21:56Z karypis $  \endverbatim
*/

#include "tokserver.h"

void close_dictionaries(xmlrpc_env *const env) 
{ 
  /* {{{ */
  int64_t i;
  int retry;
  list_t *keyslist = NULL;
  dict_t *dict;
  void *p;

    
  WRLOCK_OR_FAIL(&ServerState.dtionaries_rwlock);
  keyslist = hash_GetKeys(ServerState.dtionaries);
  if(list_Size(keyslist) > 0) {
    for(i=0; keyslist && i<list_Size(keyslist); i++) {

      p = hash_Get(ServerState.dtionaries, (void *)list_GetIth(keyslist,i,NULL), -1, NULL);
      /* hash_Get returns a pointer to an address.  this sets dict equal to that address */
      memcpy(&dict, p, sizeof(dict_t *));

      hhash_disk_wr_lock(dict->dict);
      hhash_mem_wr_lock(dict->dict);
      retry = 1;
      while(retry == 1) {
        retry = !db_Close(dict->dict->diskhash);
        if(retry == 1) {
          LOGMSG("Failed to close dictionary.  Trying again in 3 seconds.");
          pthread_sleep(3);
        }
        else {
          LOGMSG("Closed dictionary successfully.");
          dict->dict->diskhash = NULL;
        } 
      }
    }
  }
  return;

ERROR_EXIT:
  LOGMSG("Failed to get lock on dictionaries hash\n");
  return;

 /* }}} */
}

/*******************************************************************************/
/*! This is the system.shutdown method */
/*******************************************************************************/
static xmlrpc_server_shutdown_fn requestShutdown;

static void requestShutdown(xmlrpc_env *const env, 
                            void *const context, 
                            const char *const comment, 
                            void *const callInfo)
{
  //int *const terminationRequested = context;
  TServer *server = context;

  xmlrpc_env_init(env);

  LOGMSG1("Termination requested: %s", comment);

  if (!strcmp(comment, "stop@now")) {
    //*terminationRequested = 1;
    close_dictionaries(env);
    hash_Destroy(ServerState.dtionaries);
    hash_Destroy(ServerState.default_stoplist);
    gk_free((void **)&ServerState.configfile, &ServerState.journaldir, &ServerState.dictdir, &ServerState.dbtest, LTERM); 
    ServerTerminate(server);
    //system("killall -r toksrv*");
    exit(0);
  }
}

/*
xmlrpc_value *Shutdown(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo) {
    system("killall -r toksrv.*");
    return NULL;
}
*/

/*******************************************************************************/
/*! In UNIX, when you try to write to a socket that has been closed from the
    other end, your write fails, but you also get a SIGPIPE signal. That
    signal will kill you before you even have a chance to see the write fail
    unless you catch, block, or ignore it.  If a client should connect to us
    and then disconnect before we've sent our response, we see this
    socket-closed behavior.  We obviously don't want to die just because a
    client didn't complete an RPC, so we ignore SIGPIPE.
*/
/*******************************************************************************/
static void setupSignalHandlers(void)
{
  struct sigaction mysigaction;

  sigemptyset(&mysigaction.sa_mask);
  mysigaction.sa_flags = 0;
  mysigaction.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &mysigaction, NULL);
}


/*******************************************************************************/
/*! This is the single-threaded server */
/*******************************************************************************/
#if 0
int SingleThreadedServer()
{ /* {{{ */
  TServer abyssServer;
  xmlrpc_registry *registry;
  xmlrpc_env env;
  int terminationRequested;  /* A boolean value */


  /* Initialize the xml-rpc server */
  xmlrpc_env_init(&env);

  /* Create a new method's registry */
  registry = xmlrpc_registry_new(&env);

  /* Disable introspection */
  xmlrpc_registry_disable_introspection(registry);

  /* Add the termination method */
  xmlrpc_registry_set_shutdown(registry, &requestShutdown, &terminationRequested);
  //xmlrpc_registry_set_shutdown(registry, &requestShutdown, );

  /* Add the various methods of the server */

  xmlrpc_registry_add_method(&env, registry, NULL, "tok.Tokenize",    &Tokenize,      NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "dict.dictGetStats",   &dict_GetStats, NULL);

  /* Setup the loop server */
  ServerCreate(&abyssServer, "XmlRpcServer", ServerState.port, NULL, NULL);

  xmlrpc_server_abyss_set_handlers2(&abyssServer, "/RPC2", registry);

  ServerInit(&abyssServer);

  setupSignalHandlers();

  terminationRequested = 0;

  while (!terminationRequested) {
    //LOGMSG("Waiting for next RPC...");

    ServerRunOnce(&abyssServer);
  }

  ServerFree(&abyssServer);

  return 1;
} /* }}} */
#endif

int pthread_sleep (int seconds)
{ /* {{{ */
  pthread_mutex_t mutex;
  pthread_cond_t conditionvar;
  struct timespec timetoexpire;

  if(pthread_mutex_init(&mutex,NULL)) {
    return -1;
  }

  if(pthread_cond_init(&conditionvar,NULL)) {
    return -1;
  }

  timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
  timetoexpire.tv_nsec = 0;

  return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
} /* }}} */


/*******************************************************************************/
/*! This is the multi-threaded server */
/*******************************************************************************/
int MultiThreadedServer()
{
  TServer abyssServer;
  xmlrpc_registry *registry;
  xmlrpc_env env;
  //int terminationRequested;  /* A boolean value */
  uint64_t threadid; /* not used; required for pthread_create */

  /* Initialize the xml-rpc server */
  xmlrpc_env_init(&env);

  /* Create a new method's registry */
  registry = xmlrpc_registry_new(&env);

  /* Disable introspection */
  /* xmlrpc_registry_disable_introspection(registry); */

  /* Add the termination method */
  //xmlrpc_registry_set_shutdown(registry, &requestShutdown, &terminationRequested);
  xmlrpc_registry_set_shutdown(registry, &requestShutdown, &abyssServer);


  /* Add the various methods of the server */
  /*
  void xmlrpc_registry_add_method2(xmlrpc_env *      envP,
                              xmlrpc_registry * registryP,
                              const char *      methodName,
                              xmlrpc_method     methodFunction,
                              const char *      signature,
                              const char *      help,
                              void *            serverInfo); */

  xmlrpc_registry_add_method2(&env, registry, "ts.Tokenize", &Tokenize, 
                              "S:S", 
                              "This method tokenizes a string using a specific dictionary.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.TokenizeSegments", &TokenizeSegments, 
                              "S:S", 
                              "This method tokenizes multiple strings using a specific dictionary.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.ReverseTokenize", &ReverseTokenize, 
                              "S:S", 
                              "This method takes a list of ids and returns their corresponding terms using a specific dictionary.",
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictGetStats", &dict_GetStats, 
                              "S:S", 
                              "This method returns information regarding a dictionary's cache.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictCreate", &dict_Create, 
                              "S:S", 
                              "This method creates a dictionary.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictDelete", &dict_Delete, 
                              "S:S", 
                              "This method deletes a dictionary.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictGetMeta", &dict_Get_Meta, 
                              "S:S", 
                              "This method returns meta information about a dictionary.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictGetStopList", &dict_Get_Stoplist, 
                              "S:S", 
                              "This method returns the stopwords in use by a dictionary.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "hirundo.methodSignature", &hirundo_MethodSignature, 
                              "S:S", 
                              "This method returns the hirundo-style signature for a method.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictList", &dict_List, 
                              "A:n", 
                              "This method lists the available dictionaries.", 
                              NULL);
  xmlrpc_registry_add_method2(&env, registry, "ts.DictHeartBeat", &dict_heartbeat, 
                              "S:S", 
                              "This method tests a dictionary to see if it is available and healthy.", 
                              NULL);

/* the libxmlrpc v1.06 version way: */
/*  xmlrpc_registry_add_method(&env, registry, NULL, "ts.Tokenize",         &Tokenize,                  NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "ts.DictGetStats",     &dict_GetStats,             NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "ts.DictCreate",       &dict_Create,               NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "ts.DictDelete",       &dict_Delete,               NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "ts.DictList",         &dict_List,                 NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "ts.DictGetMeta",      &dict_Get_Meta,             NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "ts.DictGetStopList",  &dict_Get_Stoplist,         NULL);
  xmlrpc_registry_add_method(&env, registry, NULL, "hirundo.methodSignature",&hirundo_MethodSignature, NULL); */

  //xmlrpc_registry_add_method(&env, registry, NULL, "update.Tokenize",    &Tokenize_update,    NULL);
  
  
  
  /* Introspection methods -- these are in the library now that I upgraded libxmlrpc to v1.16 */

  //xmlrpc_registry_add_method(&env, registry, NULL, "system.listMethods",    &system_ListMethods,     NULL);
  //xmlrpc_registry_add_method(&env, registry, NULL, "system.methodSignature",&system_MethodSignature, NULL);
  //xmlrpc_registry_add_method(&env, registry, NULL, "system.methodHelp",     &system_MethodHelp,      NULL);
  //xmlrpc_registry_add_method2(&env, registry, "system.methodHelp",     &system_methodHelp, "(s)" ,"stuff",   NULL);


  //xmlrpc_registry_add_method(&env, registry, NULL, "dict.clearJournals", &dictClearJournals,  NULL);
  //xmlrpc_registry_add_method(&env, registry, NULL, "dict.setStopping",   &dict_SetStop,        NULL);
  //xmlrpc_registry_add_method(&env, registry, NULL, "dict.setStemming",   &dict_SetStem,        NULL);
  //xmlrpc_registry_add_method(&env, registry, NULL, "tok.Destroy",       &Shutdown,           NULL);

  /* make sure we can receive very large documents.  The numeric argument is
  the max number of characters the server will allow in. */
  xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, 5e9);

  dict_loadall(&env);

  LOGMSG("Going multi-threaded");
  LOGMSG1("Listening on port %d",ServerState.port);

  /* Setup the loop server */
  ServerCreate(&abyssServer, "XmlRpcServer", ServerState.port, NULL, NULL);
  xmlrpc_server_abyss_set_handlers2(&abyssServer, "/RPC2", registry);
  ServerInit(&abyssServer);
  setupSignalHandlers();

  pthread_create(&threadid, NULL, dict_backup, (void *)&env);
  //terminationRequested = 0;
  ServerRun(&abyssServer);

  //LOGMSG("Kabam!\n");
  //exit(1);
  return 1;
}
