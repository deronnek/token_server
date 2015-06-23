#include "tokserver.h"
/*!
\defgroup API
\brief xml-rpc exported routines
*/

/*!
\brief This file contains various routines associated with dictionary handling
\author Kevin
 */

/*!
\defgroup Dictionary
\brief A term-id association object

These routines allow manipulation of dictionary objects.  Each object contains
its own stoplist, database and internal statistics.  Meta data about each 
dictionary is stored in a human-readable .cfg file. 

@{
*/

#if 0
/*******************************************************************************/
/*! \brief This function sets the stopping parameter on a dictionary.  
 * Not implemented yet. */
/*******************************************************************************/
xmlrpc_value *dict_SetStop(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo)  
{ /* {{{ */
  int dostopval;
  char *dict_name;
  void *p;
  char r_string[1024];
  int haslock  = 0;
  int hasdlock = 0;

  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);
  xmlrpc_decompose_value(env, params, "({s:d,*})", "dostop", &dostopval);

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  if(dostopval == 0) {
    sprintf(r_string,"Dict %s will now use stop words.\n",dict_name);
  }
  else {
    sprintf(r_string,"Dict %s will now ignore stop words.\n",dict_name);
  }

  RWLOCK_OR_FAIL(dict->dictstate_rwlock); hasdlock = 1;
  RWUNLOCK_OR_FAIL(dict->dictstate_rwlock); hasdlock = 0;


  ret = xmlrpc_build_value(env, "s", r_string);

  ERROR_EXIT:
    if(haslock) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  gk_free((void **)&dict_name, &dostopval, LTERM);

  return NULL;
}

/*******************************************************************************/
/*! \brief This function sets the stemming parameter on a dictionary.  
 * Not implemented yet. */
/*******************************************************************************/
xmlrpc_value *dict_SetStem(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo)  
{
  char *dict_name;
  void *p;
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  ret = xmlrpc_build_value(env, "s", r_string);

  ERROR_EXIT:
    if(haslock) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  gk_free((void **)&dict_name, LTERM);

  return NULL;
} /* }}} */
#endif 

/*******************************************************************************/
/*! \brief This function opens a new database object for storing delta terms for
           a dictionary since last closing a delta dictionary                  */
/*******************************************************************************/
db_t *deltadict_Open(dict_t *dict) 
{
  struct timeval tv;
  db_t *ret = NULL;
  char *delta_filename = NULL;
  gettimeofday(&tv, NULL);
  asprintf(&delta_filename, "%s_%ld.delta", dict->filename, (long)tv.tv_sec);
  ret = db_Open(delta_filename, DBI_FLUSHALL|TOKSERVER_DBTYPE);
  return ret;
}

/*******************************************************************************/
/*! \brief This function creates a dictionary object.
\ingroup API


\param  "name"         is the name of the dictionary to be created
\param  "tokregex"     is the regular expression specifying a token
\param  "cachesize"    is the number of elements to cache in memory
\param  "dostem"       is 1/0, turning stemming on (1) or off (0)
\param  "dostop"       is 1/0, indicating that stopwords should be ignored (1) or included (0)
\param  "stopwords"    is a list of stopwords to replace the default stopword list.  Can be "".
\param  "stopdelim"    is the character(s) used to separate stopwords in stopwords, addstopwords and delstopwords.  Can be "".
\param  "addstopwords" is a list of words to be added to the deault stopword list.  Can be "".
\param  "delstopwords" is a list of words to be remove from the default stopword list. Can be "".

\returns An xmlrpc response indicating whether the dictionary was created
            successfully.
\remark
The behavior based on the parameters is as follows:

If dostop = 1 and the stoplist parameter is empty, and either the addstopwords
parameter or the delstopwords parameter is not the empty string, the default
stoplist will be augmented.  Words in addstopwords will be added to the default
list and words in delstopwords will be removed from the default list.

If dostop = 1 and the stoplist parameter is nonempty, the default stoplist will
be replaced with the words from the supplied stoplist.  The addstopwords and
delstopwords parameters have no effect in this case.

If dostop = 0 the stoplist, stopdelim, addstopwords and delstopwords parameters will
have no effect.
*/
/*******************************************************************************/
xmlrpc_value *dict_Create(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo)  
{ /* {{{ */
  char *dict_name         = NULL;
  char *tokregex          = NULL;
  char *stoplist          = NULL;
  char *stopdelim         = NULL;
  char *add_stop          = NULL;
  char *del_stop          = NULL;
  char *dict_filename     = NULL;
  char *rdict_filename    = NULL;
  char *dict_cfg_filename = NULL;
  char *r_string = NULL;
  int dostem    = 1;
  int dostop    = 1;
  int cachesize = 1000000;
  int haslock   = 0;
  FILE *CFGFILE = NULL;
  dict_t *dict  = (dict_t *)gk_malloc(sizeof(dict_t),"dict create");
  dict->dict    = NULL; 
  dict->stophash= NULL;
  xmlrpc_value *ret = NULL;
  int rc;
  char *tmp = NULL;

  dict->dict     = NULL;
  dict->rdict    = NULL;
  dict->stophash = NULL;
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",             &dict_name);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "tokregex",         &tokregex);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "cachesize",        &cachesize);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "dostem",           &dostem);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "dostop",           &dostop);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "stopwords",        &stoplist);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "stopdelim",        &stopdelim);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "addstopwords",     &add_stop);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "delstopwords",     &del_stop);

  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  printf("Trying to get lock\n");
  RWLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  printf("Got lock\n");
  if(hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    asprintf(&r_string,"A dictionary named %s already exists.  Please choose a different name.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -300, "%s", r_string);
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto NOCLOSE_ERROR_EXIT;
  }

  if(strstr(dict_name,".toksrv.dict") != NULL || strstr(dict_name,".toksrv.journal") != NULL) {
    asprintf(&r_string,"A dictionary name cannot include the substring '.toksrv.dict or '.toksrv.journal'  Please choose a different name.\n");
    xmlrpc_env_set_fault_formatted(env, -310, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }

  printf("Stopwords: |%s|\n",stoplist);

  /* write a config file for this dictionary */
  asprintf(&dict_filename,"%s/%s.toksrv.dict", ServerState.dictdir, dict_name);
  asprintf(&rdict_filename,"%s/%s.toksrv.rdict", ServerState.dictdir, dict_name);
  asprintf(&dict_cfg_filename,"%s/%s.toksrv.dict.cfg", ServerState.dictdir, dict_name);

  CFGFILE = gk_fopen(dict_cfg_filename,"w",dict_cfg_filename);
  fprintf(CFGFILE,"tokregex=%s\n",     tokregex);
  fprintf(CFGFILE,"name=%s\n",         dict_name);
  fprintf(CFGFILE,"filename=%s\n",     dict_filename);
  fprintf(CFGFILE,"rfilename=%s\n",    rdict_filename);
  fprintf(CFGFILE,"cachesize=%d\n",    cachesize);
  fprintf(CFGFILE,"dostem=%d\n",       dostem);
  fprintf(CFGFILE,"dostop=%d\n",       dostop);
  fprintf(CFGFILE,"stopwords=%s\n",    stoplist);
  fprintf(CFGFILE,"stopdelim=%s\n",    stopdelim);
  fprintf(CFGFILE,"addstopwords=%s\n", add_stop);
  fprintf(CFGFILE,"delstopwords=%s\n", del_stop);
  gk_fclose(CFGFILE);

  dict->name         = dict_name;
  dict->dostem       = dostem;
  dict->dostop       = dostop;
  dict->filename     = dict_filename;
  dict->rfilename    = rdict_filename;
  dict->tokregex     = tokregex;
  dict->cachesize    = cachesize;
  dict->configfile   = dict_cfg_filename;
  dict->stopwords    = stoplist;
  dict->stopdelim    = stopdelim;
  dict->addstopwords = add_stop;
  dict->delstopwords = del_stop;

  dict->ninserts     = 0;
  dict->ninserts_at_backup = 0;
  dict->cachehit     = 0;
  dict->cachemiss    = 0;
  dict->cacherequest = 0;

  /* create an empty dictionary */
  dict->dict         = hhash_Open(dict->filename, dict->cachesize, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
  hhash_Close(dict->dict);

  /* create a backup dictionary so deltas can be merged properly */
  asprintf(&tmp,"%s.bak",dict->filename);

  if(gk_fexists(tmp) == 0) {
    gk_free((void **)&tmp, LTERM);
    asprintf(&tmp, "cp %s %s.bak", dict->filename, dict->filename);
    rc = system(tmp);
    asprintf(&tmp, "cp %s %s.bak", dict->configfile, dict->configfile);
    rc = system(tmp);
    gk_free((void **)&tmp, LTERM);

    if(rc == 1) 
      errexit("Failed to create backup dictionary.\n");
  }
  else {
    gk_free((void **)&tmp, LTERM);
  }

  dict->dict         = hhash_Open(dict->filename, dict->cachesize, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
  dict->rdict        = db_Open(dict->rfilename, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
  dict->deltadict    = deltadict_Open(dict);

  if(dostop == 1) {
    dict->stophash   = stoplist_create(gk_strdup(stoplist), stopdelim, gk_strdup(add_stop), gk_strdup(del_stop));
  }
  else {
    dict->stophash   = NULL;
  }

  if ((rc = pthread_rwlock_init(&(dict->dictstate_rwlock), NULL))) {
    fprintf(stderr, "Failed to initialize dictstate_rwlock: %s\n", gk_strerror(rc));
    goto ERROR_EXIT;
  }
  if ((rc = pthread_rwlock_init(&(dict->dict_statslock), NULL))) {
    fprintf(stderr, "Failed to initialize dict_statslock: %s\n", gk_strerror(rc));
    goto ERROR_EXIT;
  }
  if ((rc = pthread_rwlock_init(&(dict->inuse_rwlock), NULL))) {
    fprintf(stderr, "Failed to initialize inuse_rwlock: %s\n", gk_strerror(rc));
    goto ERROR_EXIT;
  }

  hash_Put(ServerState.dtionaries, (void *)dict_name, -1, &dict, sizeof(dict_t *));
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  asprintf(&r_string,"Dictionary %s created successfully.\n",dict_name);
  ret = xmlrpc_build_value(env, "s", r_string);

  gk_free((void **)&r_string, LTERM);
  LOGMSG1("Created dictionary %s",dict_name);
  return ret;

  ERROR_EXIT:
  printf("FAILED TO GET LOCK\n");
    if(dict->dict)     hhash_Close(dict->dict);
    if(dict->rdict)    db_Close(dict->rdict);
    if(dict->stophash) hash_Destroy(dict->stophash);

  NOCLOSE_ERROR_EXIT:
    if(haslock) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    if(dict_name) gk_free((void **)&dict_name, LTERM);
    if(tokregex)  gk_free((void **)&tokregex,  LTERM);
    if(stoplist)  gk_free((void **)&stoplist, LTERM);
    if(stopdelim) gk_free((void **)&stopdelim, LTERM);
    if(add_stop)  gk_free((void **)&add_stop,  LTERM);
    if(del_stop)  gk_free((void **)&del_stop,  LTERM);
    gk_free((void **)&dict_cfg_filename, &dict_filename, &rdict_filename, &dict, LTERM);

  return ret;
} /* }}} */

/*******************************************************************************/
/*! \brief This function deletes a dictionary.
\ingroup API
    \param  "name" - The name of the dictionary to be deleted

    \returns An xmlrpc response indicating whether the dictionary was deleted
            successfully.
*/
/*******************************************************************************/
xmlrpc_value *dict_Delete(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo)  
{ /* {{{ */
  char *dict_name;
  char *r_string = NULL;
  void *p;
  int haslock = 0, hasulock = 0;
  dict_t *dict = NULL;
  xmlrpc_value *ret = NULL;
  hash_t *tmphash = NULL;

  //printf("Attempting to delete %s\n",dict_name);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  RWLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }
  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));
  RWLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 1;

  hash_Del(ServerState.dtionaries, (void *)dict_name, -1);
  
  /* Clear any journals associated with this dictionary */
  dict_ClearJournals(env, dict);
  /*
  i=0;
  hash_IterInit(dict->stophash);
  while(i<hash_Size(dict->stophash)) {
    p = hash_IterGet(dict->stophash, NULL);
    memcpy(&tmphash, p, sizeof(hash_t *));
    hash_Destroy(tmphash);
    i++;
  }
  */

  p = (hash_t *)hash_Get(dict->stophash, (void *)"precise", -1, NULL);
  if(p)  {
    memcpy(&tmphash, p, sizeof(hash_t *));
    hash_Destroy(tmphash);
  }

  p = (hash_t *)hash_Get(dict->stophash, (void *)"stemmed", -1, NULL);
  if(p) {
    memcpy(&tmphash, p, sizeof(hash_t *));
    hash_Destroy(tmphash);
  }

  p = (hash_t *)hash_Get(dict->stophash, (void *)"plain", -1, NULL);
  if(p) {
    memcpy(&tmphash, p, sizeof(hash_t *));
    hash_Destroy(tmphash);
  }

  hhash_Close(dict->dict);
  db_Close(dict->rdict);
  db_Close(dict->deltadict);
  hash_Destroy(dict->stophash);

  unlink(dict->configfile);
  unlink(dict->filename);
  unlink(dict->rfilename);

  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
  /* should we delete the backup file as well? */

  asprintf(&r_string,"Dictionary %s deleted successfully.\n",dict_name);
  ret = xmlrpc_build_value(env, "s", r_string);
  gk_free((void **)&dict->stopwords, &dict->stopdelim, &dict->addstopwords, &dict->delstopwords, LTERM);
  gk_free((void **)&dict->configfile, &dict->name, &dict->filename, &dict->rfilename, &dict->tokregex, &dict, LTERM);
  gk_free((void **)&r_string, LTERM);

  LOGMSG1("Deleted dictionary %s",dict_name);

  ERROR_EXIT:
    if(haslock)          RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    if(hasulock && dict) RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;

  gk_free((void **)&dict_name, LTERM);
  
  return ret;
} /* }}} */

/*******************************************************************************/
/*! \brief This function lists the available dictionaries on the server
\ingroup API

    \returns An xmlrpc response containing the names of all available dictionaries.
*/
/*******************************************************************************/
xmlrpc_value *dict_List(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo)  
{ /* {{{ */
  int i;
  int haslock        = 0;
  list_t *dicts      = NULL;
  xmlrpc_value *ret  = NULL;
  xmlrpc_value *dict = NULL;
  char r_string[64];

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock);   haslock = 1;
  dicts = hash_GetKeys(ServerState.dtionaries);
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  ret = xmlrpc_array_new(env);

  if(list_Size(dicts) == 0) {
    sprintf(r_string, "No dictionaries on server.\n");
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    goto ERROR_EXIT;
  }

  for(i=0; i<list_Size(dicts); i++) {
    dict = xmlrpc_string_new(env, list_GetIth(dicts, i, NULL));
    xmlrpc_array_append_item(env, ret, dict);
    xmlrpc_DECREF(dict);
  }

  list_Destroy(dicts);
  return ret;

  ERROR_EXIT:
    if(haslock) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    if(dicts) list_Destroy(dicts);

  return ret;
} /* }}} */

/*******************************************************************************/
/*! \brief This function returns meta information about a specific dictionary
\ingroup API

    \param  "name" - The dictionary name subject to the request

    \returns An xmlrpc response containing:
      - "name"     - The dictionary name from the request
      - "size"     - The number of elements in the dictionary 
      - "dostop"   - Whether or not stop words are ignored
      - "dostem"   - Whether or not stemming is in use
      - "tokregex" - The regular expression defining a dictionary
*/
/*******************************************************************************/
xmlrpc_value *dict_Get_Meta(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo)  
{ /* {{{ */
  char *dict_name = NULL;
  char *r_string  = NULL;
  void *p;
  int haslock = 0, hasdlock = 0;
  dict_t *dict;
  xmlrpc_value *ret=NULL;
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;

  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }

  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));

  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  RDLOCK_OR_FAIL(&dict->dictstate_rwlock); hasdlock = 1;
                          
  ret = xmlrpc_build_value(env, "{s:s,s:i,s:i,s:i,s:s,s:i,s:s}",
                          "name",       dict->name,
                          "size",       hhash_Size(dict->dict),
                          "dostop",     dict->dostop,
                          "dostem",     dict->dostem,
                          "tokregex",   dict->tokregex,
                          "cachesize",  dict->cachesize,
                          "configfile", dict->configfile);  
  RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock); hasdlock = 0;
  gk_free((void **)&dict_name, LTERM);

  return ret;
      
  ERROR_EXIT:
    if(haslock)  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    if(hasdlock) RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock);       hasdlock = 0;
    gk_free((void **)&dict_name, LTERM);

  return ret;
} /* }}} */

/*******************************************************************************/
/*! \brief This function returns the stopwords used by a dictionary
\ingroup API
 *
    \param  "name" - The dictionary name subject to the request

    \returns An xmlrpc response containing:
      - "name"      - The dictionary name from the request
      - "stopwords" - The stopwords used by the dictionary "name", one per line.  
                      If no stopwords are in use, returns the empty string.
*/
/*******************************************************************************/
xmlrpc_value *dict_Get_Stoplist(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo)  
{ /* {{{ */
  char *dict_name = NULL;
  char *r_string  = NULL;
  void *p;
  int haslock = 0, hasdlock = 0;
  dict_t *dict;
  xmlrpc_value *ret            = NULL;
  xmlrpc_value *xv_name        = NULL;
  xmlrpc_value *xv_plainstop   = NULL;
  xmlrpc_value *xv_precisestop = NULL;
  xmlrpc_value *xv_stemmedstop = NULL;

  xmlrpc_value * xv_stopwords  = NULL;
  xmlrpc_value * xv_addstop    = NULL;
  xmlrpc_value * xv_delstop    = NULL;

  hash_t *plainstop         = NULL;
  hash_t *stemmedstop       = NULL;
  hash_t *precisestop       = NULL;
  char *plainkey            = "plain";
  char *precisekey          = "precise";
  char *stemmedkey          = "stemmed";

  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }

  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  RDLOCK_OR_FAIL(&dict->dictstate_rwlock); hasdlock = 1;

  if(dict->dostop != 1) {
    asprintf(&r_string, "Dictionary %s does not use a stop list.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }

  if(dict->dostop == 1) {
    
    p = (hash_t *)hash_Get(dict->stophash, (void *)plainkey, -1, NULL);
    memcpy(&plainstop, p, sizeof(hash_t *));

    p = (hash_t *)hash_Get(dict->stophash, (void *)precisekey, -1, NULL);
    memcpy(&precisestop, p, sizeof(hash_t *));

    p = (hash_t *)hash_Get(dict->stophash, (void *)stemmedkey, -1, NULL);
    memcpy(&stemmedstop, p, sizeof(hash_t *));

    ret            = xmlrpc_struct_new(env);
    xv_name        = xmlrpc_build_value(env, "s", dict->name);
    xv_plainstop   = create_xmlrpc_array_hash_t(env, plainstop);
    xv_precisestop = create_xmlrpc_array_hash_t(env, precisestop);
    xv_stemmedstop = create_xmlrpc_array_hash_t(env, stemmedstop);

    xv_stopwords   = xmlrpc_build_value(env, "s", dict->stopwords);
    xv_addstop     = xmlrpc_build_value(env, "s", dict->addstopwords);
    xv_delstop     = xmlrpc_build_value(env, "s", dict->delstopwords);

    xmlrpc_struct_set_value(env, ret, "name",                  xv_name);

    xmlrpc_struct_set_value(env, ret, "supplied stopwords",    xv_stopwords);
    xmlrpc_struct_set_value(env, ret, "supplied addstopwords", xv_addstop);
    xmlrpc_struct_set_value(env, ret, "supplied delstopwords", xv_delstop);

    xmlrpc_struct_set_value(env, ret, "plain stopwords",       xv_plainstop);
    xmlrpc_struct_set_value(env, ret, "precise stopwords",     xv_precisestop);
    xmlrpc_struct_set_value(env, ret, "stemmed stopwords",     xv_stemmedstop);



    xmlrpc_DECREF(xv_name);
    xmlrpc_DECREF(xv_stopwords);
    xmlrpc_DECREF(xv_addstop);
    xmlrpc_DECREF(xv_delstop);

    xmlrpc_DECREF(xv_plainstop);
    xmlrpc_DECREF(xv_precisestop);
    xmlrpc_DECREF(xv_stemmedstop);

  }

  RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock); hasdlock = 0;

  ERROR_EXIT:
    if(haslock)  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    if(hasdlock) RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock);       hasdlock = 0;
    gk_free((void **)&dict_name, LTERM);

  return ret;
} /* }}} */

/*******************************************************************************/
/*! \brief This function checks to see if a dictionary is avaiable and healthy.
\ingroup API
    \param  "name" - The dictionary name subject to the request
 *  \returns An xmlrpc response containing status:ok status:bad                */
/*******************************************************************************/
xmlrpc_value *dict_heartbeat(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo) 
{ 
  /* {{{ */
  int haslock     = 0; 
  char *dict_name = NULL;
  void *p = NULL;
  char *r_string  = NULL;
  int read_ok  = 0;
  int write_ok = 0;
  dict_t *dict;
  xmlrpc_value *ret = NULL;
  char *test_str;
  int64_t nwords;
  int64_t *r_data;
  int r_dsize;

  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  
  /*  {{{ Lock and get dict pointer set properly based on the name sent */
  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }

  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
  /* }}} */

/*
  1. Get a write lock on the dictionary
  2. Generate a string we believe to be unique
  3. Check to see if it's in the database
  4. If it is, goto #2
  5. If it isn't, write it to disk (test write) then read it back out (test read).
  6. Delete it from disk
  7. Unlock
  8. Return status
*/
  //fprintf(stderr,"KDR heartbeat testing start\n");


  hhash_disk_wr_lock(dict->dict);

  //fprintf(stderr,"lock obtained\n");
  nwords   = db_Nrecords(dict->dict->diskhash);

  test_str = rand_str(100); 
  while(db_Get(dict->dict->diskhash, test_str, -1, (char **)&r_data, &r_dsize)) {
    gk_free((void **)&test_str, &r_data, LTERM);
    test_str = rand_str(100); 
  }


  //fprintf(stderr,"KDR heartbeat testing with: %s %"PRId64"\n",test_str,nwords);
  write_ok = db_Insert(dict->dict->diskhash, test_str, -1, (char *)&nwords, sizeof(int64_t));
  read_ok  = db_Get(dict->dict->diskhash,    test_str, -1, (char **)&r_data, &r_dsize);

  // FOR TESTING ONLY: (should force a "bad" to be returned)
  //db_Close(dict->dict->diskhash);

  if(read_ok && *r_data == nwords) {
    read_ok = 1;
  }
  else {
    read_ok = 0;
  }

  if(write_ok) {
    write_ok = db_Delete(dict->dict->diskhash, test_str, -1);
  }

  gk_free((void **)&test_str, &r_data, LTERM);

  hhash_disk_unlock(dict->dict);

  return xmlrpc_build_value(env, "{s:s}", "status",   (read_ok&&write_ok ? "ok" : "bad"));

ERROR_EXIT:
    if (haslock) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    gk_free((void **)&dict_name, LTERM);

  return ret;
  /* }}} */
}

/*******************************************************************************/
/*! \brief This function returns cache hit stats for a specific dictionary.
\ingroup API


    \param  "name" - The dictionary name subject to the request

    \returns An xmlrpc response containing:
    - "cache_hitrate"     - The number of requests serviced by the dictionary cache for "name"
    - "cache_missrate"    - The number of requests serviced by the dictionary cache for "name"
    - "tokens_requested"  - The number of token requests made to the dictionary "name"
    - "dictionary_size"   - The number of elements in the dictionary "name"

*/
/*******************************************************************************/
xmlrpc_value *dict_GetStats(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo) 
{ /* {{{ */
  int64_t dictsize;
  float hitrate   = 0.0;
  float missrate  = 0.0;
  int haslock     = 0, hasdlock = 0;
  char *dict_name = NULL;
  void *p = NULL;
  xmlrpc_value *ret = NULL;
  char *r_string  = NULL;
  dict_t *dict;

  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",   &dict_name);
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  /* Need to lock, and figure out which dictionary we want from the name */

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    ret = xmlrpc_build_value(env, "s", r_string);
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }

  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;

  RDLOCK_OR_FAIL(&dict->dict_statslock); hasdlock = 1;
  if(dict->cacherequest == 0) {
    hitrate  = 0;
    missrate = 0;
  }
  else {
    hitrate        = (float)dict->cachehit/((float)dict->cacherequest);
    missrate       = (float)dict->cachemiss/((float)dict->cacherequest);
  }
  dictsize = hhash_Size(dict->dict);
  RWUNLOCK_OR_FAIL(&dict->dict_statslock); hasdlock = 0;

  gk_free((void **)&dict_name, LTERM);

  return xmlrpc_build_value(env, "{s:d,s:d,s:i,s:i}",
                          "cache_hitrate",   hitrate,
                          "cache_missrate",  missrate,
                          "tokens_requested",   dict->cacherequest,
                          "dictionary_size", dictsize);
  ERROR_EXIT:
    if (hasdlock) RWUNLOCK_OR_FAIL(&dict->dict_statslock); hasdlock = 0;
    if (haslock) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
    gk_free((void **)&dict_name, LTERM);

  return ret;
} /* }}} */

/*************************************************************************/
/*! \brief Preprocess the contents of a configuration file.

This function uses gpp to process a pre-process a configuration file.
The resulting configuration file is stored in /tmp/gpp-dict.pid, and is 
used as the input to the configuration routines.

*/
/**************************************************************************/
char *dict_PreProcessConfigFile(char *configfile)
{ /* {{{ */
  char *cmd = NULL;

  asprintf(&cmd, "%s %s -o /tmp/gpp-dict.%d", "gpp -C", configfile, (int)getpid());
  if (system(cmd) == -1)
    errexit("Failed on executing system command: %s\n", cmd);

  gk_free((void **)&cmd, LTERM);
  asprintf(&cmd, "/tmp/gpp-dict.%d", (int)getpid());
  return cmd;
} /* }}} */

void dict_ReadConfigFile(char *configfile, dict_t *dict)
{ /* {{{ */
  char *value;
  char *tmpconfig;

  if (configfile)
    tmpconfig = dict_PreProcessConfigFile(configfile);


  dict->configfile = gk_strdup(configfile);
  /*----------------------------------------------------------------------------
  * These are value-based options that can be specified in the config file and
  * at the command line.  For any parameter that's not a string, we must free 
  * the variable value.
  *-----------------------------------------------------------------------------*/
  if ((value = config_GetValue(tmpconfig, "tokregex"))) {
    dict->tokregex = value;
  }
  if ((value = config_GetValue(tmpconfig, "name"))) {
    dict->name = value;
  }
  if ((value = config_GetValue(tmpconfig, "filename"))) {
    dict->filename = value;
  }
  if ((value = config_GetValue(tmpconfig, "rfilename"))) {
    dict->rfilename = value;
  }
  if ((value = config_GetValue(tmpconfig, "cachesize"))) {
    dict->cachesize = atoi(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(tmpconfig, "dostem"))) {
    dict->dostem = atoi(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(tmpconfig, "dostop"))) {
    dict->dostop = atoi(value);
    gk_free((void **)&value, LTERM);
  }

  if ((value = config_GetValue(tmpconfig, "stopwords"))) {
    gk_free((void **)&dict->stopwords, LTERM);
    dict->stopwords    = value;
  }
  if ((value = config_GetValue(tmpconfig, "stopdelim"))) {
    gk_free((void **)&dict->stopdelim, LTERM);
    dict->stopdelim    = value;
  }
  if ((value = config_GetValue(tmpconfig, "addstopwords"))) {
    gk_free((void **)&dict->addstopwords, LTERM);
    dict->addstopwords = value;
  }
  if ((value = config_GetValue(tmpconfig, "delstopwords"))) {
    gk_free((void **)&dict->delstopwords, LTERM);
    dict->delstopwords = value;
  }
  if (tmpconfig) {
    unlink(tmpconfig);
    gk_free((void **)&tmpconfig, LTERM);
  }
} /* }}} */

static int dselect(const struct dirent64 *totest) 
{ /* {{{ */
  if(strstr(totest->d_name,".toksrv.dict.cfg") != NULL) {
    if(strstr(totest->d_name,".toksrv.dict.cfg.bak") == NULL) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
} /* }}} */

static int jselect(const struct dirent64 *totest) 
{ /* {{{ */
  if(strstr(totest->d_name,".toksrv.journal") != NULL) {
    return 1;
  }
  else {
    return 0;
  }
} /* }}} */

int timestamp_compare_increasing(const void *a, const void *b) 
{ /* {{{ */
  int rc = 0;
  journal_t *aa = (journal_t *)a;
  journal_t *bb = (journal_t *)b;

  if(aa->time_sec < bb->time_sec) {
    rc = -1;
  }
  else if(aa->time_sec == bb->time_sec) {
    if(aa->time_usec < bb->time_usec) {
      rc = -1;
    }
    else if(aa->time_usec == bb->time_usec) {
      rc = 0;
    }
    else if(aa->time_usec > bb->time_usec) {
      rc = 1;
    }
  }
  else if(aa->time_sec > bb->time_sec) {
    rc = 1;
  }
  return rc;
} /* }}} */

int dict_replay_journal(dict_t *dict, xmlrpc_env *env) 
{ 
  /* {{{ */
  struct dirent64 **eps;
  int64_t n, i;
  FILE *JFILE;
  int64_t jid, time_sec, time_usec, firstid;
  uint64_t mll = MAXLINELEN;
  char *term   = NULL;
  char *torm   = NULL;
  /* this gets automatically re-allocated by getline */
  char *line   = (char *)gk_malloc(MAXLINELEN*sizeof(char),"replay_journal line");
  int matched;
  int64_t nentries = 0;
  char *needle = NULL;
  char *tmp    = NULL;
  journal_t *journal;
  char *saveptr1;

  asprintf(&needle,"%s.toksrv.journal",dict->name);

  //printf("Restoring to dictionary that already has %"PRId64" words\n",nwords);
  n = scandir64(ServerState.journaldir,&eps,jselect,alphasort64);
  for(i=0; i<n; i++) {
    if(strstr(eps[i]->d_name, needle) != NULL) {
      nentries++;
    }
  }

  /* allocate, store, sort */
  journal  = (journal_t *)gk_malloc(nentries*sizeof(journal_t),"replay_journal journal");
  nentries = 0;
  for(i=0; i<n; i++) {
    if(strstr(eps[i]->d_name, needle) != NULL) {
      printf("Processing journal file %s\n",eps[i]->d_name);
      asprintf(&tmp,"%s/%s",ServerState.journaldir,eps[i]->d_name);
      JFILE = gk_fopen(tmp,"r","replay journal");
      gk_free((void **)&tmp, LTERM);

      matched = fscanf(JFILE,"%"PRId64" %"PRId64" %"PRId64,&time_sec,&time_usec,&firstid); 
      FAIL_IFTRUE(matched != 3, "Failure reading journal entry.\n");

      getline(&line, &mll, JFILE);

      gk_fclose(JFILE);

      journal[nentries].terms     = gk_strdup(line);
      journal[nentries].firstid   = firstid;
      journal[nentries].time_sec  = time_sec;
      journal[nentries].time_usec = time_usec;
      nentries++;
    }
  }
  qsort(journal, nentries, sizeof(journal_t), timestamp_compare_increasing); 
  JFILE = NULL;

/*
  for(i=0; i<nentries; i++) {
    printf("%"PRId64" %"PRId64" %"PRId64" %"PRId64" %s",journal[i].firstid,journal[i].nextid,journal[i].time_sec,journal[i].time_usec,journal[i].terms);
  }
  */

  /* insert terms into dictionary */
  for(i=0; i<nentries; i++) {
    /* nextid gets set to the term of the last id + 1 */
    jid   = journal[i].firstid;

    term = strtok_r(journal[i].terms, " \n", &saveptr1);
    while(term != NULL) {
      //fscanf(JFILE," %s ",term);
      /* FAIL_IFTRUE(sscanf(termspt," %s ",term) != 1,
                "Number of terms did not match number of ids in journal\n");  */
      //printf("Inserting: %s\n",term); 
      FAIL_IFTRUE(hhash_Put(dict->dict, term, -1, (char *)&jid, sizeof(int64_t)) == 0,
                "Failed to add word to the dictionary.\n");
      term = strtok_r(NULL, " \n", &saveptr1);
      jid++;
    }
    gk_free((void **)&journal[i].terms, LTERM);
  }
  gk_free((void **)&journal, LTERM);

  /* delete journals and free memory */
  for(i=0; i<n; i++) {
    asprintf(&torm,"%s/%s",ServerState.journaldir,eps[i]->d_name);
    if(strstr(eps[i]->d_name, needle) != NULL) {
      unlink(torm);
    }
    free(eps[i]);
    gk_free((void **)&torm, LTERM);
  }
  free(eps);

  gk_free((void **)&needle, &line, LTERM);

  return 1;


  ERROR_EXIT:
    if(JFILE != NULL) 
      gk_fclose(JFILE);
    gk_free((void **)&term, &torm, &needle, &line, LTERM);
  return 0;
} /* }}} */

void dict_restore(xmlrpc_env *env, dict_t *dict)
{ /* {{{ */
  /* 
   * close the dictionary if necessary
   * copy from backup, or create new if we only have a journal
   * replay journal
   * close dictionary
   * copy to backup
   * delete journal
   * re-open dictionary
   */
  char *tmp = NULL;
  int retry = 1;
  int rc;

  LOGMSG1("Attempting to restore dictionary: %s \n",dict->name);

  if(dict->dict != NULL) {
    retry = 1;
    while(retry == 1) {
      /* close the disk hash */
      //retry = !db_Close(dict->dict->diskhash);
      retry = hhash_Close(dict->dict);
      if(retry == 1) {
        LOGMSG1("Failed to close dictionary %s.  Trying again in 3 seconds.",dict->name);
        pthread_sleep(3);
      }
      else {
        dict->dict->diskhash = NULL;
      }
    }
  }

  /* open dictionary */
  retry = 1;
  while(retry == 1) {
    //dict->dict->diskhash = db_Open(dict->filename, DBI_FLUSHALL|TOKSERVER_DBTYPE);
    dict->dict = hhash_Open(dict->filename, dict->cachesize, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
    retry = (dict->dict->diskhash == NULL);

    if(retry == 1) {
      LOGMSG1("Failed to re-open dictionary %s.  Trying again in 3 seconds.",dict->name);
      pthread_sleep(3);
    }
  }
  LOGMSG1("Replaying journal on %s\n",dict->name);
  /* replay journal */
  dict_replay_journal(dict,env);

  if(dict->dict->diskhash != NULL) {
    /* close the dictionary */
    retry = 1;
    while(retry == 1) {
      retry = !db_Close(dict->dict->diskhash);
      if(retry == 1) {
        LOGMSG1("Failed to close dictionary %s.  Trying again in 3 seconds.",dict->name);
        pthread_sleep(3);
      }
      else {
        dict->dict->diskhash = NULL;
      }
    }
  }

  retry = 1;
  asprintf(&tmp,"cp %s %s.bak", dict->configfile, dict->configfile);
  while(retry == 1) {
    retry = system(tmp);
    if(retry == 1) {
      LOGMSG1("Failed to backup dictionary %s.  Trying again in 3 seconds.",dict->name);
      pthread_sleep(3);
    }
  }
  gk_free((void **)&tmp, LTERM);

  asprintf(&tmp, "%s %s", ServerState.dbtest, dict->filename);
  rc = system(tmp);
  LOGMSG1("DB check return code: %d ",rc);

  if(rc == 0) {
    /* re-open dictionary */
    retry = 1;
    while(retry == 1) {
      dict->dict->diskhash = db_Open(dict->filename, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
      retry = (dict->dict->diskhash == NULL);
 
      if(retry == 1) {
        LOGMSG("Failed to re-open dictionary.  Trying again in 3 seconds.");
        pthread_sleep(3);
      }
    }
    LOGMSG1("Restored dictionary %s",dict->name);
  }
  else {
    errexit("Could not restore: %s from backup or journals.\n",dict->filename);
  }
  gk_free((void **)&tmp, LTERM);
} /* }}} */

xmlrpc_value *dict_pause_backup(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo) 
{
  LOCK_OR_FAIL(&ServerState.srvstate_mutex);
  ServerState.backup_enabled = 0;
  UNLOCK_OR_FAIL(&ServerState.srvstate_mutex);

  LOGMSG("Dictionary backup paused.");
  return xmlrpc_build_value(env, "{s:s}", "status", "Dictionary backup paused.");

  ERROR_EXIT:
    return NULL;
}

xmlrpc_value *dict_resume_backup(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo) 
{
  LOCK_OR_FAIL(&ServerState.srvstate_mutex);
  ServerState.backup_enabled = 1;
  UNLOCK_OR_FAIL(&ServerState.srvstate_mutex);

  LOGMSG("Dictionary backup resumed.");
  return xmlrpc_build_value(env, "{s:s}", "status", "Dictionary backup resumed.");

  ERROR_EXIT:
    return NULL;
}

/*******************************************************************************
*! Every 30 minutes (1800 seconds), lock a database, close out the database, copy 
*  the database to a backup file, clear its respective journals, re-open the 
*  database, unlock, repeat for each database, and go back to sleep. 
*******************************************************************************/
void *dict_backup(void *void_env) 
{ /* {{{ */
  xmlrpc_env *const env = (xmlrpc_env *const)void_env;
  char *tmp = NULL;
  struct dirent64 **eps;
  int64_t n, i, j;
  int retry;
  list_t *keyslist = NULL;
  dict_t *dict;
  void *p;
  char *needle = NULL;
  char *toremove = NULL;

  while(1) {
    //pthread_sleep(10);
    //
    LOCK_OR_FAIL(&ServerState.srvstate_mutex);
    if(ServerState.backup_enabled) {
      RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock);
      //LOGMSG("Auto-backing up dictionaries");
      keyslist = hash_GetKeys(ServerState.dtionaries);
      if(list_Size(keyslist) > 0) {
        for(i=0; keyslist && i<list_Size(keyslist); i++) {

          p = hash_Get(ServerState.dtionaries, (void *)list_GetIth(keyslist,i,NULL), -1, NULL);
          /* hash_Get returns a pointer to an address.  this sets dict equal to that address */
          memcpy(&dict, p, sizeof(dict_t *));

          if(dict->ninserts_at_backup != dict->ninserts) {
            printf("Backing up %s: %"PRId64" %"PRId64"\n", dict->name, dict->ninserts, dict->ninserts_at_backup);
            asprintf(&needle,"%s.toksrv.journal",dict->name);
            /* only get the disk lock, allowing cache hits to continue */
            FAIL_IFTRUE(hhash_disk_wr_lock(dict->dict),"Failed to get disk rw lock");
            retry = 1;
            while(retry == 1) {
              retry = !db_Close(dict->deltadict);
              if(retry == 1) {
                LOGMSG("Failed to close dictionary.  Trying again in 3 seconds.");
                pthread_sleep(3);
              }
              else {
                dict->deltadict = deltadict_Open(dict);
              } 
            }
            
            /*
            retry = 1;
            asprintf(&tmp, "cp -a %s %s.bak", dict->filename, dict->filename);
            while(retry == 1) {
              retry = system(tmp);
              if(retry == 1) {
                LOGMSG("Failed to backup dictionary.  Trying again in 3 seconds.");
                pthread_sleep(3);
              }
            }
            gk_free((void **)&tmp, LTERM);
            */

            retry = 1;
            asprintf(&tmp, "cp %s %s.bak", dict->configfile, dict->configfile);
            while(retry == 1) {
              retry = system(tmp);
              if(retry == 1) {
                LOGMSG("Failed to backup dictionary.  Trying again in 3 seconds.");
                pthread_sleep(3);
              }
            }
            gk_free((void **)&tmp, LTERM);
      
            /*
            retry = 1;
            while(retry == 1) {
              dict->dict->diskhash = db_Open(dict->filename, DBI_FLUSHALL|TOKSERVER_DBTYPE);
              retry = (dict->dict->diskhash == NULL);
      
              if(retry == 1) {
                LOGMSG("Failed to re-open dictionary.  Trying again in 3 seconds.");
                pthread_sleep(3);
              }
            }
            */
            //LOGMSG1("Dictionary %s backed up successfully.",dict->name);
      
            /* delete journals and free memory */
            n = scandir64(ServerState.journaldir,&eps,jselect,alphasort64);
            for(j=0; j<n; j++) {
              if(strstr(eps[j]->d_name, needle) != NULL) {
                asprintf(&toremove, "%s/%s", ServerState.journaldir, eps[j]->d_name);
                unlink(toremove);
              }
              free(eps[j]);
            }
            free(eps);
      
            hhash_disk_unlock(dict->dict);
      
            //printf("-------Backed up Dictionary to %s.bak\n",dict->filename);
            gk_free((void **)&needle, LTERM);
            dict->ninserts_at_backup = dict->ninserts;
          }
          else {
            printf("No inserts since last backup of %s\n", dict->name);
          }
        }
      }
      RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock);
    }
    else {
      //LOGMSG("Skipping auto-backup of dictionaries (disabled)");
    }
    UNLOCK_OR_FAIL(&ServerState.srvstate_mutex);
    /* sleep last so we automatically create a backup at startup */
    pthread_sleep(1800);
  }

  ERROR_EXIT:
    return NULL;

} /* }}} */

dict_t *dict_load(xmlrpc_env *env, char *dict_config) 
{ /* {{{ */
  int rc;
  char *tmp = NULL;
  char *rdict_filename = NULL;
  dict_t *dict = (dict_t *)gk_malloc(sizeof(dict_t),"dictionary allocate");
  FILE *CFGFILE;
  char *data;
  char *key;
  int ksize, dsize;
  cursor_t *cursor;

  if ((rc = pthread_rwlock_init(&(dict->dictstate_rwlock), NULL))) {
    fprintf(stderr, "Failed to initialize dictstate_rwlock: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }
  if ((rc = pthread_rwlock_init(&(dict->dict_statslock), NULL))) {
    fprintf(stderr, "Failed to initialize dict_statslock: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }
  if ((rc = pthread_rwlock_init(&(dict->inuse_rwlock), NULL))) {
    fprintf(stderr, "Failed to initialize inuse_rwlock: %s\n", gk_strerror(rc));
    exit(EXIT_FAILURE);
  }

  dict->dict               = NULL;
  dict->stopdelim          = gk_strdup("");
  dict->stopwords          = gk_strdup("");
  dict->addstopwords       = gk_strdup("");
  dict->delstopwords       = gk_strdup("");
  dict->rfilename          = NULL;
  dict->ninserts           = 0;
  dict->ninserts_at_backup = 0;

  dict_ReadConfigFile(dict_config, dict);
  
  if(ServerState.nocheck == 0) {
    asprintf(&tmp, "%s %s", ServerState.dbtest, dict->filename);
    LOGMSG1("Running: %s\n",tmp);
    rc = system(tmp);
    LOGMSG1("DB check return code: %d ",rc);
    gk_free((void **)&tmp, LTERM);
  }
  else {
    rc = 0;
  }

  if(rc != 0) {
    dict_restore(env, dict);
  }
  else {
    
  /* create a backup dictionary if we don't have one */
    asprintf(&tmp,"%s.bak",dict->filename);

    if(gk_fexists(tmp) == 0) {
      gk_free((void **)&tmp, LTERM);
      asprintf(&tmp, "cp %s %s.bak", dict->filename, dict->filename);
      rc = system(tmp);
      asprintf(&tmp, "cp %s %s.bak", dict->configfile, dict->configfile);
      rc = system(tmp);
      gk_free((void **)&tmp, LTERM);

      if(rc == 1) 
        errexit("Failed to create backup dictionary.\n");
    }
    else {
      gk_free((void **)&tmp, LTERM);
    }

    if(dict->dostop == 1) {
      dict->stophash = stoplist_create(gk_strdup(dict->stopwords), dict->stopdelim, gk_strdup(dict->addstopwords), gk_strdup(dict->delstopwords));
    }
    else {
      dict->stophash = NULL;
    }
    dict->dict       = hhash_Open(dict->filename,  dict->cachesize, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
    dict->deltadict  = deltadict_Open(dict);

    /* if we don't have a reverse-dictionary, migrate this dict to the new version*/
    if(dict->rfilename == NULL) {
      asprintf(&rdict_filename,"%s/%s.toksrv.rdict", ServerState.dictdir, dict->name);
      CFGFILE = gk_fopen(dict_config,"a",dict_config);
      fprintf(CFGFILE,"rfilename=%s\n",    rdict_filename);
      gk_fclose(CFGFILE);
      dict->rfilename = rdict_filename;
      dict->rdict     = db_Open(dict->rfilename, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);

      /* insert everything into the reverse dictionary */
      cursor = db_CursorOpen(dict->dict->diskhash);
      while(db_CursorGet(cursor, &key, &ksize, &data, &dsize)) {
        //printf("Inserting %"PRId64" %s\n",(*(int64_t *)data), key);
        db_Insert(dict->rdict, data, dsize, key, ksize);
        db_FreeData(key);
        db_FreeData(data);
      }
      db_CursorClose(cursor);
    }
    else {
      dict->rdict = db_Open(dict->rfilename, DBI_FLUSHALL|TOKSERVER_DBTYPE|DBI_LARGESIZE);
    }
    LOGMSG1("Opened reverse dictionary: %s\n",dict->rfilename);
  }
  dict_ClearJournals(env, dict);

  return dict;
} /* }}} */

void dict_ClearJournals(xmlrpc_env *env, dict_t *dict) 
{
  /* {{{ */
  char *needle = NULL;
  struct dirent64 **eps;
  int64_t n;
  int j;
  char *toremove = NULL;

  asprintf(&needle,"%s.toksrv.journal",dict->name);
  /* delete journals and free memory */
  n = scandir64(ServerState.journaldir,&eps,jselect,alphasort64);
  for(j=0; j<n; j++) {
    if(strstr(eps[j]->d_name, needle) != NULL) {
      asprintf(&toremove, "%s/%s", ServerState.journaldir, eps[j]->d_name);
      unlink(toremove);
    }
    free(eps[j]);
  }
  free(eps);
  gk_free((void **)&needle, LTERM);
  /* }}} */
}

/*******************************************************************************
*! This function scans the directory specified in ServerState.dictdir for 
*  dictionary files.  For each dictionary found we:
*  1. create a backup if needed
*  2. read any relevant configuration with dict_load
*  3. connect the dictionary to the dtionaries hash
*
* dict_load calls dict_restore if the working copy is corrupt.
* dict_restore, in turn, creates a dictionary if all we have is journals 
*
*  This function is only called once, at server start-up, which is why I feel
*  comfortable using an errexit for now.
*******************************************************************************/
void dict_loadall(xmlrpc_env *env) 
{ /* {{{ */
  int64_t i, n;
  struct dirent64 **eps;
  char *tmp = NULL;
  char *dictfile;
  dict_t *dict;

  if(!gk_dexists(ServerState.dictdir)) {
    errexit("Dictionary directory does not exist: %s\n",ServerState.dictdir);
  }
  if(!gk_dexists(ServerState.journaldir)) {
    errexit("Journal directory does not exist: %s\n",ServerState.journaldir);
  }
  n = scandir64(ServerState.dictdir,&eps,dselect,alphasort64);
  for(i=0; i<n; i++) {
    dictfile = eps[i]->d_name;

    asprintf(&tmp, "%s/%s", ServerState.dictdir, dictfile);
    dict = dict_load(env, tmp);

    /* no need to lock here as we're still single-threaded */    
    hash_Put(ServerState.dtionaries, (void *)dict->name, -1, &dict, sizeof(dict_t *));
    LOGMSG1("Processed dictionary config file %s",dictfile);
    free(eps[i]);
    gk_free((void **)&tmp, LTERM);
  }
  free(eps);
} /* }}} */

/*! @}*/


/*
 * You must obtain a read-write lock on ServerState.srvstate_rwlock before entering
 * this function.  Creates a backup journal file, but has to close ServerState.JFILE 
 * to do this. 
 */


/* 
  Should not need to be rewritten for the new journal techniques, as we don't
  maintain journal backups anymore.
*/


/* probably don't need this either... */
#if 0
xmlrpc_value *dictClearJournals(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo)
{ /* {{{ */
  int64_t n, i;
  struct dirent64 **eps;
  int has_lock;
  xmlrpc_value *ret;
  char *torm = gk_malloc(MAXLINELEN*sizeof(char),"clearjournals filename");

  RWLOCK_OR_FAIL(&ServerState.srvstate_rwlock); has_lock = 1;
  n = scandir64(ServerState.journaldir,&eps,jselect,alphasort64);
  /* delete journals and free memory */
  for(i=0; i<n; i++) {
    sprintf(torm,"%s/%s",ServerState.journaldir,eps[i]->d_name);
    unlink(torm);
    free(eps[i]);
  }
  free(eps);
  ret = xmlrpc_build_value(env, "s","Journals cleared.");

  RWUNLOCK_OR_FAIL(&ServerState.srvstate_rwlock); has_lock = 0;
  gk_free((void **)&torm, LTERM);
  return ret;

  ERROR_EXIT:
    gk_free((void **)&torm, LTERM);
    if(has_lock ==1) 
      RWUNLOCK_OR_FAIL(&ServerState.srvstate_rwlock); has_lock = 0;
    return ret;
} /* }}} */
#endif

/* journals are basically atomic now, so this is most likely unnecessary */
/*
 * You must obtain a read-write lock on ServerState.srvstate_rwlock before entering
 * this function.
 *
 * This function ensures that the journal is not corrupt.
 * Returns: 1 if it's valid, 0 if it's corrupt.  Attempts to restore from backup if
 * the working copy is corrupt.
 */
#if 0
int verify_journal(xmlrpc_env *env) 
{ /* {{{ */
  /* test the journal integrity */
  errexit("Write verify_journal\n");
#if 0
  int valid = 1;
  long tmpid;
  char *word = (char *)gk_malloc(2048*sizeof(char),"verify journal");
  if(ServerState.JFILE != NULL) 
    gk_fclose(ServerState.JFILE);

  ServerState.JFILE  = gk_fopen(ServerState.journalfile,"r","verify_journal read");
  fseek(ServerState.JFILE, 0, SEEK_END);
  if (ftell(ServerState.JFILE) == 0) {
    valid = 1;
  }
  else {
    rewind(ServerState.JFILE);
    while(!feof(ServerState.JFILE) && valid == 1) {
      if(fscanf(ServerState.JFILE,"%ld %s\n",&tmpid,word) != 2) {
        printf("Invalid: %ld %s\n",tmpid,word);
        valid = 0;
        break;
      }
    }
    /* restore the journal if it's unreadable.  */
    if(valid == 0) {
      /* restore_journal() closes and re-opens the journal file on ServerState.JFILE */
      restore_journal(env);
      gk_fclose(ServerState.JFILE);
      ServerState.JFILE  = gk_fopen(ServerState.journalfile,"r","verify journal read");
      valid = 1;
      /* check again to make sure the backup isn't corrupt as well */
      fseek(ServerState.JFILE, 0, SEEK_END);
      if (ftell(ServerState.JFILE) == 0) {
        valid = 1;
      }
      else {
        rewind(ServerState.JFILE);
        while(!feof(ServerState.JFILE) && valid == 1) {
          if(fscanf(ServerState.JFILE,"%ld %s\n",&tmpid,word) != 2) {
            valid = 0;
            break;
          }
        }
      }
    }
  }
  gk_fclose(ServerState.JFILE);
  ServerState.JFILE  = gk_fopen(ServerState.journalfile,"a","verify journal append");
  gk_free((void **)&word, LTERM);
#endif
  return 1;
} /* }}} */
#endif 

#if 0
int verify_dict(xmlrpc_env *env) 
{ /* {{{ */
  int retry;
  int rc;
  char tmp[2048];
  //printf("%p %p\n",ServerState.dict,ServerState.dict->diskhash);
  if(ServerState.dict->diskhash != NULL) {
    FAIL_IFTRUE(hhash_disk_wr_lock(ServerState.dict),"Failed to get disk rw lock");
    retry = 1;
    while(retry == 1) {
      retry = !db_Close(ServerState.dict->diskhash);
      if(retry == 1) {
        LOGMSG("Failed to close dictionary.  Trying again in 3 seconds.");
        pthread_sleep(3);
      }
      else {
        ServerState.dict->diskhash = NULL;
      }
    }
  }

  sprintf(tmp, "%s %s", ServerState.dbtest, ServerState.dictfile);
  rc = system(tmp);
  printf("DB check return code: %d\n",rc);

  if(rc != 0) {
    restore_dict(env);
  }
  else {
    ServerState.dict->diskhash = db_Open(ServerState.dictfile, DBI_FLUSHALL|TOKSERVER_DBTYPE);
  }
  hhash_disk_unlock(ServerState.dict);
  /* either we got hung up in restore_dict(), or we now have a good copy of the dictionary */
  return 1;

  ERROR_EXIT:
    return 0;
} /* }}} */
#endif

#if 0
void initialize_dict(xmlrpc_env *env) 
{ /* {{{ */
  //printf("Initializing dict 1\n");
  ServerState.dict = hhash_Open(ServerState.dictfile, ServerState.cachesize, DBI_FLUSHALL|TOKSERVER_DBTYPE);
  //printf("Initializing dict 2\n");
  verify_dict(env);
  //printf("Initializing dict 3\n");
  FAILD_IFTRUE(ServerState.dict->diskhash == NULL, "Initial open of Dictionary failed.");
  return;

  DICT_ERROR_EXIT:
    errexit("Error initializing dictionary: %s\n",ServerState.dictfile);
} /* }}} */
#endif 

