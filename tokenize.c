#include "tokserver.h"
/*!
 * \brief This file contains various routines associated producing tokens from an input string
 * \author Kevin
 *  */

/*!
 * \defgroup Tokenize
 * \brief Tokenization routines
 *
 * 
*/

/*! 
 * \brief This function splits a string into terms based upon a dictionary's regular expression
 *
 * \returns A hash with terms as keys and counts as values.  Term_list stores the individual terms
 */
hash_t *termify(xmlrpc_env *const env, dict_t *dict, char *totok, list_t *term_list) 
{
  /* {{{ */
  int wcount;
  char *word; 
  int j,rc;
  struct stemmer * z  = create_stemmer();
  hash_t *term_hash   = hash_Create();
  pcre *token_re      = NULL;
  char *pattern       = NULL;
  char *tptr;
  size_t tptr_len;
  /* must be a multiple of 3, and I should always have one substring captured (the whole pattern) */
  int tovector[6];
  const char *pcre_errptr;
  int pcre_erroffset;
  pcre_extra re_extra;
  size_t word_len;

/*--------------------------------------------------------------------------------*/
  /* split buffer into terms and count. 
  Compile the required regexs */
  //sprintf(pattern, "<%s\\s++((?:\"[^\"]*+\"|\'[^\']*+\'|[^\'\">]*+)*+)>", tagname);
  pattern  = gk_strdup(dict->tokregex);
  token_re = pcre_compile(pattern, PCRE_CASELESS|PCRE_EXTENDED, &pcre_errptr, 
               &pcre_erroffset, NULL);
  if (token_re == NULL) {
    error_SetString("%s: pcre_compile failed for: %s. Error: %s\n", __func__,
         pattern, pcre_errptr);
    goto ERROR_EXIT;
  }
  // Should include this eventually (makes it faster):
  //       pcre_extra *pcre_study(const pcre *code, int options const char **errptr);

  /* Set the extra flags for the matches */
  re_extra.flags = PCRE_EXTRA_MATCH_LIMIT|PCRE_EXTRA_MATCH_LIMIT_RECURSION;
  re_extra.match_limit           = 100000;
  re_extra.match_limit_recursion = 4000;

  tptr     = totok; 
  tptr_len = strlen(tptr);

  while (1) {
    rc = pcre_exec(token_re, &re_extra, tptr, tptr_len, 0, 0, tovector, 6);
    if (rc == PCRE_ERROR_NOMATCH)
      break;

    /* Try to skip over the bad part of the string */
    if (rc == PCRE_ERROR_MATCHLIMIT || rc == PCRE_ERROR_RECURSIONLIMIT) {
      LOGMSG1("Recovering from pcre error of %d\n", rc);
      tptr++;
      tptr_len--;
      continue;
    }

    if (rc <= 0) {
      error_SetString("%s:token_re: pcre_exec failed. Error: %d\n", __func__, rc);
      goto ERROR_EXIT;
    }

    /* A match was found */
    if (tovector[1]-tovector[0] > 0) {
      word = PCRE_GET_MATCH(tptr, tovector, 0);
      //mprintf(50, "token: %s\n", word);
      //printf("token: %s\n", word);

      word_len = strlen(word);
      for(j=0; j<word_len; j++)
        word[j] = tolower(word[j]);
      
      if(dict->dostop == 0 || !stopWord(z,dict,word)) {

        /* if the word is in the token hash we've seen it before, at least */
        if(hash_Exists(term_hash, (void *)word, -1)) {
           // hash_Get returns a direct pointer into the hash, which is why this works
           (*(int *)(hash_Get(term_hash, (void *)word, -1, NULL)))++;
        }
        else {
          wcount = 1;
          hash_Put(term_hash, (void *)word, -1, &wcount, sizeof(int));
        }
        list_PutEnd(term_list, (char *)word, -1);
      }
      gk_free((void **)&word, LTERM);
    }
    tptr += tovector[1];
    tptr_len -= tovector[1];
  } // end tokeniztion loop

  if (token_re) pcre_free(token_re);
  free_stemmer(z);
  gk_free((void **)&pattern, LTERM);
  return(term_hash);

ERROR_EXIT:
  if (token_re) pcre_free(token_re);
  free_stemmer(z);
  gk_free((void **)&pattern, LTERM);
  return(term_hash);
  /* }}} */
}

/*!
 * \brief Converts terms from a hash into ids from a dictionary for all known terms.
 * 
 * \returns A hash with known terms as keys and ids as values.  nwhash has
 * unknown terms as keys and temporary ids as values.  If all terms are in the 
 * dictionary, *nwhash will be NULL;
 */
hash_t *idterms(xmlrpc_env *const env, dict_t *dict, hash_t *term_hash, hash_t **nwhash, int update) 
{
  /* {{{ */
  void *term           = NULL;
  int termsize         = 0;
  int has_lock         = 0;
  hash_t *term_id_hash = hash_Create();
  hash_t *newword_hash = hash_Create();
  int64_t term_id;
  int64_t nodictn = hhash_Size(dict->dict);
  int64_t unk_id  = -1;

  RDLOCK_OR_FAIL(&dict->dictstate_rwlock); has_lock = 1;

  /* iterate over terms in hash and get ids for known terms, putting unknown terms 
   * into the new word hash for later identification */
  hash_IterInit(term_hash);
  while((term = hash_IterGet(term_hash, &termsize))) {

//fprintf(stderr,"Getting id for %s\n",(char *)term);    

    if(hhash_Get(dict->dict, term, &term_id, -1)) {
      //printf("Got id: %"PRId64"\n",term_id);
      hash_Put(term_id_hash, term, termsize, &term_id, sizeof(int64_t));
    }
    else {
//fprintf(stderr,"new word %s\n",(char *)term);    
      //printf("Saving id: %"PRId64"\n",nodictn);
      if(update == 1) {
        hash_Put(newword_hash, term, termsize, &nodictn, sizeof(int64_t));
        nodictn++;
      }
      else {
        hash_Put(newword_hash, term, termsize, &unk_id, sizeof(int64_t));
      }
    }

//fprintf(stderr,"KDR tchdb error code: %s \n",tchdberrmsg(tchdbecode((TCHDB *)dict->dict->diskhash->dbp)));
  }
  RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock); has_lock = 0;

  if(hash_Size64(newword_hash) > 0) {
    *nwhash = newword_hash;
  }
  else {
    hash_Destroy(newword_hash);
    *nwhash = NULL;
  }
  return term_id_hash;

  ERROR_EXIT:
    if(has_lock) RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock); has_lock = 0;
    hash_Destroy(term_id_hash);
    hash_Destroy(newword_hash);
    return NULL;
  /* }}} */
}

fvector_t *tokenize_freq_known(dict_t *dict, xmlrpc_env *const env, char *totok, hash_t **nwhash, list_t *ret_tokens, int update) 
{ 
  /* {{{ */
  float value;
  char *word = NULL; 
  int i;
  int64_t dim;
  hash_t *term_count    = NULL;
  hash_t *known_terms   = NULL;
  list_t *uniq_termlist = NULL;
  fvector_t *v          = (fvector_t *)gk_malloc(sizeof(fvector_t),"tokenize_freq_known r_vec");
  int nuniq_terms       = 0;

  double hhash_time = 0.0;

  hhash_time = gk_WClockSeconds();
  term_count    = termify(env, dict, totok, ret_tokens);
  hhash_time = gk_WClockSeconds() - hhash_time;
  LOGMSG1("termify took: %lf",hhash_time);

  known_terms   = idterms(env, dict, term_count, nwhash, update);

  uniq_termlist = hash_GetKeys(term_count);
  nuniq_terms   = list_Size(uniq_termlist);

  if(nuniq_terms > 0) {
    v->vector  = (gk_lkiv_t *)gk_malloc(nuniq_terms*sizeof(gk_lkiv_t),"mkvectors vector");
    v->ndims   = nuniq_terms;
    v->ntokens = 0;
    for(i=0; uniq_termlist && i<nuniq_terms; i++) {

      word = list_GetIth(uniq_termlist,i,NULL);

      if(hash_Exists(known_terms, (void *)word, -1)) {
        dim = *(int64_t *)hash_Get(known_terms, (void *)word, -1, NULL);
        //printf("Looking for %s in known_terms\n",word);
      }
      else if(hash_Exists(*nwhash, (void *)word, -1)) {
        dim = *(int64_t *)hash_Get(*nwhash, (void *)word, -1, NULL);
      }
      else {
        FAIL_IFTRUE(1,"Word not found in either dictionary or new words.\n");
      }

      value = *(int *)hash_Get(term_count,(void *)word, -1, NULL);
      //printf("Storing: %"PRId64"\n",dim);
      v->vector[i].key = dim;
      v->vector[i].val = value;
      v->ntokens += value;
    }

    gk_lkivsorti(v->ndims, v->vector);
  }
  else {
    v->vector  = NULL;
    v->ndims   = 0;
    v->ntokens = 0;
  }
  list_Destroy(uniq_termlist);
  hash_Destroy(term_count);
  hash_Destroy(known_terms);

  return(v);

ERROR_EXIT:
  if (uniq_termlist) list_Destroy(uniq_termlist);
  hash_Destroy(*nwhash);
  hash_Destroy(term_count);
  hash_Destroy(known_terms);
  return(v);
/* }}} */
} 

/*!
 * \returns a list of ids, one per term
 */
list_t *tokenize_idlist_known(dict_t *dict, xmlrpc_env *const env, char *totok, hash_t **nwhash, list_t *ret_tokens, int update) 
{ 
  /* {{{ */
  char *word            = NULL;
  hash_t *term_count    = NULL;
  hash_t *known_terms   = NULL;
  list_t *idlist        = list_Create();
  int64_t dim;
  int nterms, i;


  term_count            = termify(env, dict, totok, ret_tokens);
  known_terms           = idterms(env, dict, term_count, nwhash, update);

  nterms = list_Size(ret_tokens);
  for(i=0; ret_tokens && i<nterms; i++) {
    word = list_GetIth(ret_tokens,i,NULL);
    if(hash_Exists(known_terms, (void *)word, -1)) {
      dim = *(int64_t *)hash_Get(known_terms, (void *)word, -1, NULL);
      //printf("Looking for %s in known_terms\n",word);
    }
    else if(hash_Exists(*nwhash, (void *)word, -1)) {
      dim = *(int64_t *)hash_Get(*nwhash, (void *)word, -1, NULL);
    }
    else {
      FAIL_IFTRUE(1,"Word not found in either dictionary or new words.\n");
    }
    list_PutEnd(idlist, (char *)&dim, sizeof(int64_t));
  }

  hash_Destroy(term_count);
  hash_Destroy(known_terms);
  return(idlist);

  ERROR_EXIT:
    hash_Destroy(term_count);
    hash_Destroy(known_terms);
    list_Destroy(idlist);
  return NULL;

/* }}} */
} 

void tokenize_freq_new(fvector_t *toupdate, hash_t *transdict) 
{ /* {{{ */
  int i;
  int rdsize;
  void *data;

  for(i=0; i<toupdate->ndims; i++) {
    data = (void *)hash_Get(transdict, (char *)&toupdate->vector[i].key, sizeof(int64_t), &rdsize);
    if(data != NULL) {
      toupdate->vector[i].key = *(int64_t *)data;
      //printf("Translating %ld to %ld count %d\n", toupdate->vector[i].key,(*(int64_t *)data),toupdate->vector[i].val);
    }
  }
  gk_lkivsorti(toupdate->ndims, toupdate->vector);
} /* }}} */

void tokenize_idlist_new(list_t *toupdate, hash_t *transdict)
{ /* {{{ */
  int i;
  int rdsize;
  void *data;
  char *id;

  if(list_Size(toupdate) > 0) {
    for(i=0; toupdate && i<list_Size(toupdate); i++) {
      id = list_GetIth(toupdate,i,NULL);
      //if(db_Exists(transdict, (char *)id, sizeof(long))) {
      data = (void *)hash_Get(transdict, (char *)id, sizeof(int64_t), &rdsize);
      if(data != NULL) {
        (*(int64_t *)id) = (*(int64_t *)data);
      }
    }
  }
  /* else {
    errexit("Empty list handed to tokenize_list_new\n");
  } */
} /* }}} */

fourpartvec *tokenize_quartfreq_new(list_t *toupdate, hash_t *transdict)
{ /* {{{ */
  int i,j,k;
  int rdsize;
  int nsplits = 4;
  int wcount;
  int quartsize,quart,value;
  fvector_t *dest = NULL;
  char *listelem;
  int64_t *wordid;
  void *data = NULL;
  list_t *keyslist = NULL;
  hash_t *qhash = hash_Create();
  int translate = 0;
  int64_t searchfor;

  fourpartvec *rdata = (fourpartvec *)gk_malloc(sizeof(fourpartvec),"quartfreq fourpart");

  if(transdict != NULL) {
    translate = 1;
  }

  if(list_Size(toupdate) < nsplits) {
    /* page too small to split.  ignore it for now? */
    gk_free((void **)&rdata, LTERM);
  }
  else {
    if(list_Size(toupdate)%nsplits == 3) {
      quartsize = (list_Size(toupdate)/nsplits)+1;
    }
    else {
      quartsize = (list_Size(toupdate)/nsplits);
    }
    quart = 0;
    for(i=0; i<list_Size(toupdate); i++) {
      listelem  = list_GetIth(toupdate,i,NULL);
      searchfor = (*(int64_t *)listelem);
      if(translate == 1)
        data = (void *)hash_Get(transdict, (void *)&searchfor, sizeof(int64_t), &rdsize);
      if(data != NULL) {
         /* listelem is a direct pointer into the list, so this should work  */
        *listelem = *(int64_t *)data;
      }
      //printf("%d %ld\n",quartsize,*(long *)listelem);
      if(i>0 && i%quartsize == 0 && quart < 3) {
        if(quart == 0) {
          rdata->q1 = (fvector_t *)gk_malloc(sizeof(fvector_t),"rdata->q1");
          dest = rdata->q1;
        }
        if(quart == 1) {
          rdata->q2 = (fvector_t *)gk_malloc(sizeof(fvector_t),"rdata->q2");
          dest = rdata->q2;
        }
        if(quart == 2) {
          rdata->q3 = (fvector_t *)gk_malloc(sizeof(fvector_t),"rdata->q3");
          dest = rdata->q3;
        }
        /* create the vector from the hash */
        dest->vector  = (gk_lkiv_t *)gk_malloc(hash_Size(qhash)*sizeof(gk_lkiv_t),"split_vector");
        dest->ndims   = hash_Size(qhash);
        dest->ntokens = 0;
 
        keyslist = hash_GetKeys(qhash);
        for(j=0, k=0; keyslist && j<list_Size(keyslist); j++) {
          
          wordid = (int64_t *)list_GetIth(keyslist,j,NULL);
          value  = (*(int *)(hash_Get(qhash, (void *)wordid, sizeof(int64_t), NULL)));
          dest->vector[k].key = (*(int64_t *)wordid);
          dest->vector[k].val = value;
          dest->ntokens += value;
          k++;
        }
        gk_lkivsorti(list_Size(keyslist), dest->vector);
        list_Destroy(keyslist);
 
        /* reset the hash */
        hash_Reset(qhash);
        quart++;
      }
      if(hash_Exists(qhash, (void *)listelem, sizeof(int64_t))) {
        (*(int *)(hash_Get(qhash, (void *)listelem, sizeof(int64_t), NULL)))++;
      }
      else {
        wcount = 1;
        hash_Put(qhash, (void *)listelem, sizeof(int64_t), &wcount, sizeof(int));
      }
    } 
    /* get the last quarter */
    rdata->q4 = (fvector_t *)gk_malloc(sizeof(fvector_t),"rdata->q4");
    dest      = rdata->q4;

    /* create the vector from the hash */
    dest->vector  = (gk_lkiv_t *)gk_malloc(hash_Size(qhash)*sizeof(gk_lkiv_t),"split_vector");
    dest->ndims   = hash_Size(qhash);
    dest->ntokens = 0;
    
    keyslist = hash_GetKeys(qhash);
    for(j=0, k=0; keyslist && j<list_Size(keyslist); j++) {
      
      wordid = (int64_t *)list_GetIth(keyslist,j,NULL);
      value  = (*(int *)(hash_Get(qhash, (void *)wordid, sizeof(int64_t), NULL)));
      dest->vector[k].key = (*(int64_t *)wordid);
      dest->vector[k].val = value;
      dest->ntokens += value;
      k++;
    }
    gk_lkivsorti(list_Size(keyslist), dest->vector);
    list_Destroy(keyslist);
  }

  hash_Destroy(qhash);

  return(rdata);
} /* }}} */

void fourpartFree(fourpartvec *f) {
  gk_free((void **)&f->q1->vector, &f->q1, &f->q2->vector, &f->q2, &f->q3->vector, 
                   &f->q3, &f->q4->vector, &f->q4, &f, LTERM);
}

/*! 
 * \ingroup Tokenize
 * This function will:
 * 1. Record in a journal the words to be added to the dictionary
 * 2. Update the dictionary with the new terms found in the newwords hash
 * 3. Fill a translation dictionary hash 
 *
 * We do NOT stem again, since we assume new words have already been stemmed,
 * stopped, and made lowercase.
 */
void update_dict(dict_t *dict, xmlrpc_env *const env, hash_t *newwords, hash_t **r_transdict) 
{ 
  /* {{{ */
  char *word;        
  int64_t nwords, tmpid, lastid;
  int64_t transid;
  int i;
  //int has_lock = 0;
  hash_t *transdict = hash_Create();
  list_t *keyslist  = NULL;
  char *journalfile = NULL;
  char *towrite     = (char *)gk_malloc(MAXLINELEN*sizeof(char),"update_dict line");
  //char addition[128];
  FILE *JFILE = NULL;
  struct timeval timestamp;
  double journal_time;

  RWLOCK_OR_FAIL(&dict->dictstate_rwlock); 
  //has_lock = 1;
  /* need the lock first so size gets read correctly */
  nwords = hhash_Size(dict->dict);
  lastid = nwords;
  asprintf(&journalfile,"%s/request.%"PRId64".dict.%s.toksrv.journal",ServerState.journaldir,nwords,dict->name); 
  JFILE = gk_fopen(journalfile, "w", journalfile);
  
  //printf("%ld Updating dictionary.  Next word will have id %ld\n",pthread_self(),nwords);
  // Scan the newwords hash and write out the journal
#if 0
  {{{
  //ServerState.JFILE = gk_fopen(ServerState.journalfile, "a", ServerState.journalfile);
  jwords   = nwords;
  keyslist = hash_GetKeys(newwords);
  if(list_Size(keyslist) > 0) {
    for(i=0; keyslist && i<list_Size(keyslist); i++) {
      word = list_GetIth(keyslist,i,NULL);
      tmpid   = (*(int64_t *)hash_Get(newwords, (void *)word, -1, NULL));
      if(!hhash_Exists(dict->dict, word, strlen(word)+1)) {
        FAILJ_IFTRUE(fprintf(ServerState.JFILE,"%"PRId64" %s\n",jwords,word) < 0, "Failed to write to journal."); 
        FAILJ_IFTRUE(fflush(ServerState.JFILE),"Failed flushing journal entry\n");
        jwords++;
      }
    }
  }
  }}}
#endif
#if 1
  keyslist   = hash_GetKeys(newwords);
  towrite[0] = 0;
  
  journal_time = gk_WClockSeconds();
  gettimeofday(&timestamp,NULL);
  FAILJ_IFTRUE(fprintf(JFILE,"%ld %ld %"PRId64,timestamp.tv_sec,timestamp.tv_usec,nwords) < 0, "Failed to write header to journal."); 
  FAILJ_IFTRUE(fflush(JFILE),"Failed flushing journal header\n");
  if(list_Size(keyslist) > 0) {
    for(i=0; keyslist && i<list_Size(keyslist); i++) {
      word = list_GetIth(keyslist,i,NULL);
      //tmpid   = (*(int64_t *)hash_Get(newwords, (void *)word, -1, NULL));
      if(!hhash_Exists(dict->dict, word, -1)) {
        //sprintf(addition," %s",word);
        if((strlen(towrite)+strlen(word))+1 > MAXLINELEN) {
           
          FAILJ_IFTRUE(fprintf(JFILE,"%s",towrite) < 0, "Failed to write data to journal."); 
          FAILJ_IFTRUE(fflush(JFILE),"Failed flushing journal data\n");
          /* put a null character at the beginning so the buffer starts over */
          towrite[0] = 0;
        }
        strcat(towrite," ");
        strcat(towrite,word);
        lastid++;
      }
    }
  }

  FAILJ_IFTRUE(fprintf(JFILE,"%s\n",towrite) < 0, "Failed to write footer to journal."); 
  FAILJ_IFTRUE(fflush(JFILE),"Failed flushing journal footer\n");
  gk_fclose(JFILE);
  JFILE = NULL;
  journal_time = gk_WClockSeconds() - journal_time;
#endif

  if(list_Size(keyslist) > 0) {
    for(i=0; keyslist && i<list_Size(keyslist); i++) {
      word  = list_GetIth(keyslist,i,NULL);
      tmpid = (*(int64_t *)hash_Get(newwords, (void *)word, -1, NULL));

      if(!hhash_Exists(dict->dict, word, -1)) {

        // KDR randomly corrupt the dictionary for testing
        //if(rand()%10 < 3) corrupt_dict_hhash();    
        if(0) {
          if(rand()%10 < 3) {
          FAILJ_IFTRUE(1,"Test Journal Fault\n");
          /* 
          corrupt_journal();    
          */
          corrupt_dict();
          //FAILJ_IFTRUE(1,"test journal failure");
          //FAILD_IFTRUE(1,"test dict failure");
          }
        }

        FAILD_IFTRUE(hhash_Put(dict->dict, word, -1, (char *)&nwords, sizeof(int64_t)) == 0,
                  "Failed to add word '%s' to the dictionary.\n",(char *)word);
        FAILD_IFTRUE(db_Insert(dict->rdict, (char *)&nwords, sizeof(int64_t), word, -1) == 0,
                  "Failed to add word '%s' to the reverse dictionary.\n",(char *)word);

        hash_Put(transdict, (char *)&tmpid, sizeof(int64_t), (char *)&nwords, sizeof(int64_t));
        dict->ninserts++;
        nwords++;
        /* Every 1000 inserts, sync the metadata with the disk */
        if(dict->ninserts%1000 == 0) {
          dict->ninserts = 0;

          hhash_disk_wr_lock(dict->dict);
          hhash_mem_wr_lock(dict->dict);

          tchdbsync(dict->dict->diskhash->dbp);

          hhash_disk_unlock(dict->dict);
          hhash_mem_unlock(dict->dict);
        }
      }
      else {
        /* before we got the chance to add this word ourselves, someone else added this word */
        /* Get the correct id from the dictionary */
        hhash_Get(dict->dict, word, &transid, -1);
        hash_Put(transdict, (char *)&tmpid, sizeof(int64_t), (char *)&transid, sizeof(int64_t));
      }
    }
  }
  list_Destroy(keyslist);
  RWUNLOCK_OR_FAIL(&dict->dictstate_rwlock); 
  //has_lock = 0;
  LOGMSG1("Journal writing time: %lf",journal_time);
//FAILD_IFTRUE(1,"Test Dict Fault\n");

  *r_transdict = transdict;
  gk_free((void **)&journalfile, &towrite, LTERM);
  return;

  DICT_ERROR_EXIT:
    /* signal all other threads to respond with an error code to the client */
    dict->dictvalid = 0;
    gk_free((void **)&journalfile, &towrite, LTERM);
    if(JFILE != NULL) gk_fclose(JFILE);
    errprintf("Dictionary failure: %p %p\n",dict,dict->dict);
    return;

  JOURNAL_ERROR_EXIT:
    //if(has_lock == 0) RWLOCK_OR_FAIL(&dict->dictstate_rwlock);
    //if(verify_journal() == 0)
    //backup_journal(); 
    //RWUNLOCK_OR_FAIL(&ServerState.srvstate_rwlock);

  ERROR_EXIT:    

    if(JFILE != NULL) gk_fclose(JFILE);
    gk_free((void **)&journalfile, &towrite, LTERM);

  return;
/* }}} */
} 

xmlrpc_value *tokenize_frequency(xmlrpc_env *const env, char *instring, dict_t *dict, int ret_tokens, int do_update)  
{
/* {{{ */
  hash_t *newwords        = NULL;
  hash_t *transdict       = NULL;
  xmlrpc_value *ret       = NULL;
  xmlrpc_value *xv_dims   = NULL;
  xmlrpc_value *xv_counts = NULL;
  xmlrpc_value *xv_tokens = NULL;
  list_t *tokens          = NULL;
  fvector_t *v            = NULL;

  tokens                  = list_Create();
  double new_time         = 0.0;
  double comms_time       = 0.0;
  double known_time       = 0.0;
  double update_dict_time = 0.0;

//  LOGMSG1("instring: %s\n",instring);

/*
  if (env->fault_occurred) {
    errprintf("[%s:%d] error before reaching tokenize_frequency!\n", __FILE__, __LINE__);
    errexit("KABAM!\n");
  }
  */

  if(do_update == 1) {
    known_time = gk_WClockSeconds();
    v = tokenize_freq_known(dict, env, instring, &newwords, tokens, do_update);
    known_time = gk_WClockSeconds() - known_time;

    if(newwords != NULL) {
      /*
  if (env->fault_occurred) {
    errprintf("[%s:%d] error before reaching update_dict!\n", __FILE__, __LINE__);
    errexit("KABAM!\n");
  }
  */
      update_dict_time = gk_WClockSeconds();
      update_dict(dict, env, newwords, &transdict);
      update_dict_time = gk_WClockSeconds() - update_dict_time;

      new_time = gk_WClockSeconds();
      tokenize_freq_new(v, transdict);
      new_time = gk_WClockSeconds() - new_time;

      hash_Destroy(transdict);
      hash_Destroy(newwords);
    }
  }
  else {
    known_time = gk_WClockSeconds();
    //v = tokenize_freq_only_known(dict, env, instring, tokens, do_update);
    v = tokenize_freq_known(dict, env, instring, &newwords, tokens, do_update);
    known_time = gk_WClockSeconds() - known_time;
  }

  comms_time = gk_WClockSeconds();
  create_xmlrpc_array_lkiv(env, v->ndims, v->vector, &xv_dims, &xv_counts);

  if(ret_tokens != 0) {
    xv_tokens = create_xmlrpc_array_stringlist(env, tokens);
    
    ret = xmlrpc_struct_new(env);

    xmlrpc_struct_set_value(env,ret,"tokens",xv_tokens);
    xmlrpc_DECREF(xv_tokens);

    xmlrpc_struct_set_value(env,ret,"ids",   xv_dims);
    xmlrpc_DECREF(xv_dims);

    xmlrpc_struct_set_value(env,ret,"freqs", xv_counts);
    xmlrpc_DECREF(xv_counts);
  }
  else {
    ret = xmlrpc_struct_new(env);
    
    xmlrpc_struct_set_value(env,ret,"ids",   xv_dims);
    xmlrpc_struct_set_value(env,ret,"freqs", xv_counts);

    xmlrpc_DECREF(xv_dims);
    xmlrpc_DECREF(xv_counts);
  }
  comms_time = gk_WClockSeconds() - comms_time;

  gk_free((void **)&v->vector, &v, LTERM);

  list_Destroy(tokens);

  LOGMSG1("Tokenizing known words time:   %lf",known_time);
  LOGMSG1("Tokenizing new words time:     %lf",new_time);
  LOGMSG1("Tokenizing update dict time:   %lf",update_dict_time);
  LOGMSG1("Tokenizing communication time: %lf",comms_time);

  return ret;
/* }}} */
}

xmlrpc_value *tokenize_idlist(xmlrpc_env *const env, char *instring, dict_t *dict, int ret_tokens, int do_update)  
{
/* {{{ */
  hash_t *newwords  = NULL;
  hash_t *transdict = NULL;
  list_t *l         = NULL;
  list_t *tokens    = NULL;
  xmlrpc_value *ret = NULL;

  xmlrpc_value *tokenslist = NULL;
  xmlrpc_value *idslist    = NULL;

  tokens = list_Create();
  if (do_update == 1) {
    l = tokenize_idlist_known(dict, env, instring, &newwords, tokens, do_update);
    if(newwords != NULL) {
      update_dict(dict,env,newwords,&transdict);
      tokenize_idlist_new(l, transdict);
      hash_Destroy(transdict);
      hash_Destroy(newwords);
    }
  }
  else {
    //l = tokenize_idlist_only_known(dict, instring, tokens);
    l = tokenize_idlist_known(dict, env, instring, &newwords, tokens, do_update);
  }

  idslist = create_xmlrpc_array_i64list(env, l);

  ret     = xmlrpc_struct_new(env);
  if(ret_tokens != 0) {
    tokenslist = create_xmlrpc_array_stringlist(env, tokens);
    xmlrpc_struct_set_value(env, ret, "tokens", tokenslist);
    xmlrpc_struct_set_value(env, ret, "ids",    idslist);
    xmlrpc_DECREF(tokenslist);
    xmlrpc_DECREF(idslist);
  }
  else {
    xmlrpc_struct_set_value(env, ret, "ids", idslist);
    xmlrpc_DECREF(idslist);
  }

  list_Destroy(l);
  list_Destroy(tokens);
  return ret;
/* }}} */
}

xmlrpc_value *tokenize_quarter_frequency(xmlrpc_env *const env, char *instring, dict_t *dict, int ret_tokens, int do_update)  
{
/* {{{ */
  hash_t *newwords           = NULL;
  hash_t *transdict          = NULL;
  fourpartvec *fourpart      = NULL;
  list_t *l                  = NULL;
  list_t *tokens             = NULL;
  xmlrpc_value *ret          = NULL;
  xmlrpc_value *xv_ids       = NULL;

  xmlrpc_value *xv_dims_q1   = NULL;
  xmlrpc_value *xv_counts_q1 = NULL;
  xmlrpc_value *xv_dims_q2   = NULL;
  xmlrpc_value *xv_counts_q2 = NULL;
  xmlrpc_value *xv_dims_q3   = NULL;
  xmlrpc_value *xv_counts_q3 = NULL;
  xmlrpc_value *xv_dims_q4   = NULL;
  xmlrpc_value *xv_counts_q4 = NULL;

  tokens = list_Create();
  if(do_update == 1) {
    l = tokenize_idlist_known(dict, env, instring, &newwords, tokens, do_update);

    if(newwords != NULL) {
      update_dict(dict,env,newwords,&transdict);
      hash_Destroy(newwords);
    }
  }
  else {
    //l = tokenize_idlist_only_known(dict, instring, tokens);
    l = tokenize_idlist_known(dict, env, instring, &newwords, tokens, do_update);
  }

  fourpart = tokenize_quartfreq_new(l, transdict);
  xv_ids   = create_xmlrpc_array_i64list(env, l);

  if(transdict != NULL) 
    hash_Destroy(transdict);

  list_Destroy(l);
  if(fourpart != NULL) {
    create_xmlrpc_array_lkiv(env, fourpart->q1->ndims, fourpart->q1->vector, &xv_dims_q1, &xv_counts_q1);
    create_xmlrpc_array_lkiv(env, fourpart->q2->ndims, fourpart->q2->vector, &xv_dims_q2, &xv_counts_q2);
    create_xmlrpc_array_lkiv(env, fourpart->q3->ndims, fourpart->q3->vector, &xv_dims_q3, &xv_counts_q3);
    create_xmlrpc_array_lkiv(env, fourpart->q4->ndims, fourpart->q4->vector, &xv_dims_q4, &xv_counts_q4);
    fourpartFree(fourpart);
  }
  else {
    xv_dims_q1   = xmlrpc_array_new(env);
    xv_counts_q1 = xmlrpc_array_new(env);

    xv_dims_q2   = xmlrpc_array_new(env);
    xv_counts_q2 = xmlrpc_array_new(env);

    xv_dims_q3   = xmlrpc_array_new(env);
    xv_counts_q3 = xmlrpc_array_new(env);

    xv_dims_q4   = xmlrpc_array_new(env);
    xv_counts_q4 = xmlrpc_array_new(env);

  }

  //ret = xmlrpc_struct_new(env);
  ret = tokenize_frequency(env, instring, dict, ret_tokens, do_update);

  xmlrpc_struct_set_value(env, ret, "ids_q1",    xv_dims_q1);
  xmlrpc_struct_set_value(env, ret, "ids_q2",    xv_dims_q2);
  xmlrpc_struct_set_value(env, ret, "ids_q3",    xv_dims_q3);
  xmlrpc_struct_set_value(env, ret, "ids_q4",    xv_dims_q4);
  xmlrpc_struct_set_value(env, ret, "count_q1",  xv_counts_q1);
  xmlrpc_struct_set_value(env, ret, "count_q2",  xv_counts_q2);
  xmlrpc_struct_set_value(env, ret, "count_q3",  xv_counts_q3);
  xmlrpc_struct_set_value(env, ret, "count_q4",  xv_counts_q4);

  
  xmlrpc_DECREF(xv_ids);
  xmlrpc_DECREF(xv_dims_q1);
  xmlrpc_DECREF(xv_dims_q2);
  xmlrpc_DECREF(xv_dims_q3);
  xmlrpc_DECREF(xv_dims_q4);
  xmlrpc_DECREF(xv_counts_q1);
  xmlrpc_DECREF(xv_counts_q2);
  xmlrpc_DECREF(xv_counts_q3);
  xmlrpc_DECREF(xv_counts_q4);

  list_Destroy(tokens);
  return ret;
/* }}} */
}

/*************************************************************************/
/*! \brief This function tokenizes a string.
 * \ingroup API
 * 
 *
    \param "dict_name"    is the dictionary to reference when tokenizing.
    \param "toTok"        is the string to be tokenized.
    \param "tokType"      specifies the output format.  Possible values are "freq"
                             "idlist", and "quartfreq"
    \param "returnTokens" 1/0 specifies whether tokens will be returned as well.  
                          1 means they will be returned, 0 means they will not be.

    \returns An xmlrpc response containing:
      - For tokType = "freq"
        - if "returnTokens" is 1
          - "tokens" is a space-separated list of tokens from the input string
          - "counts" is a space-separated list of tokenid:count pairs
        - if "returnTokens" is 0
          - "counts" is a space-separated list of tokenid:count pairs
      - For tokType = "idlist"
        - if "returnTokens" is 1
          - "tokens" is a space-separated list of tokens from the input string
          - "ids" is a space-separated list of tokenid values, one per token
        - if "returnTokens" is 0
          - "ids" is a space-separated list of tokenid values, one per token
      - For tokType = "quartfreq"
        - if "returnTokens" is 1
          - "tokens" is a space-separated list of tokens from the input string
          - "full"   is a space-separated list of tokenid:count pairs
          - "q1"     is a space-separated list of tokenid:count pairs for the first quarter of the input string
          - "q2"     is a space-separated list of tokenid:count pairs for the second quarter of the input string
          - "q3"     is a space-separated list of tokenid:count pairs for the third quarter of the input string
          - "q4"     is a space-separated list of tokenid:count pairs for the fourth quarter of the input string
        - if "returnTokens" is 0
          - "full"   is a space-separated list of tokenid:count pairs
          - "q1"     is a space-separated list of tokenid:count pairs for the first quarter of the input string
          - "q2"     is a space-separated list of tokenid:count pairs for the second quarter of the input string
          - "q3"     is a space-separated list of tokenid:count pairs for the third quarter of the input string
          - "q4"     is a space-separated list of tokenid:count pairs for the fourth quarter of the input string

*/
/**************************************************************************/
xmlrpc_value *Tokenize(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo) 
{ 
  /* {{{ */
  //xmlrpc_int status = -1;
  char *instring=NULL, *toktype=NULL, *dict_name=NULL;
  //int cachesize = 1000000;
  xmlrpc_value *ret   = NULL;
  void *p;
  dict_t *dict   = NULL;
  int haslock    = 0;
  int hasulock   = 0;
  int ret_tokens = 0;
  int do_update  = 0;
  char *r_string = NULL;

  double tot_time = gk_WClockSeconds();
  /* Get the parameters */
  xmlrpc_decompose_value(env, params, "({s:s,*})", "toTok",        &instring);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "tokType",      &toktype);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",         &dict_name);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "update",       &do_update);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "returnTokens", &ret_tokens);


  /* Various sanity checks */
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");
  FAIL_IFTRUE(strlen(instring) == 0, "Empty string supplied.");
  //LOGMSG1("Tokenize was called on dict %s.",dict_name);

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    ret = xmlrpc_build_value(env, "s", r_string);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }
  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));

  RDLOCK_OR_FAIL(&dict->inuse_rwlock);              hasulock = 1;
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock  = 0; 

  if(!strcmp("freq",toktype)) {
    ret = tokenize_frequency(env, instring, dict, ret_tokens, do_update);
  }
  else if(!strcmp("idlist",toktype)) {
    ret = tokenize_idlist(env, instring, dict, ret_tokens, do_update);
  }
  else if(!strcmp("quartfreq",toktype)) {
    ret = tokenize_quarter_frequency(env, instring, dict, ret_tokens, do_update);
  }
  else {
    if (!env->fault_occurred)
      xmlrpc_env_set_fault(env, -405, "Tokenization type must be either freq or idlist or quartfreq");
  }
  RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;

  tot_time = gk_WClockSeconds() - tot_time;

  LOGMSG1("Total tokenize call time: %lf",tot_time);

  gk_free((void **)&instring, &toktype, &dict_name, LTERM);
  return ret;


ERROR_EXIT:
  if(haslock  == 1) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
  if(hasulock == 1) RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;
  gk_free((void **)&instring, &toktype, &dict_name, LTERM);
  if (!env->fault_occurred)
    xmlrpc_env_set_fault(env, -404, "RPC failed at tokenize: dictionary does not exist.");

  return ret;
/* }}} */
}

xmlrpc_value *tokenize_segment(xmlrpc_env *const env, xmlNode * a_node, dict_t *dict, int ret_tokens, int do_update, char *toktype)
{
  xmlNode *cur_node = NULL;
  xmlrpc_value *seg = NULL;
  xmlrpc_value *ret = NULL;
  char *instring    = NULL;
  ret = xmlrpc_array_new(env);

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_TEXT_NODE) {
      //printf("node type: Element, name: %s content: %s\n", cur_node->name,cur_node->content);
      instring = (char *)cur_node->content;
      //printf("Tokenizing %s\n",instring);
      if(!strcmp("freq",toktype)) {
        seg = tokenize_frequency(env, instring, dict, ret_tokens, do_update);
      }
      else if(!strcmp("idlist",toktype)) {
        seg = tokenize_idlist(env, instring, dict, ret_tokens, do_update);
      }
      else if(!strcmp("quartfreq",toktype)) {
        seg = tokenize_quarter_frequency(env, instring, dict, ret_tokens, do_update);
      }
      else {
        if (!env->fault_occurred)
          xmlrpc_env_set_fault(env, -405, "Tokenization type must be either freq or idlist or quartfreq");
      }

      xmlrpc_array_append_item(env, ret, seg);
    }
    seg = tokenize_segment(env, cur_node->children, dict, ret_tokens, do_update, toktype);
    xmlrpc_array_append_item(env, ret, seg);
  }
  return ret;
} 

xmlrpc_value *TokenizeSegments(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo) 
{ 
  /* {{{ */
  //xmlrpc_int status = -1;
  char *instring=NULL, *toktype=NULL, *dict_name=NULL;
  //int cachesize = 1000000;
  xmlrpc_value *ret = NULL;
  void *p;
  dict_t *dict   = NULL;
  int haslock    = 0;
  int hasulock   = 0;
  int ret_tokens = 0;
  int do_update  = 0;
  char *r_string = NULL;
  char *doc_string = NULL;

  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  


  /* Get the parameters */
  xmlrpc_decompose_value(env, params, "({s:s,*})", "toTok",        &instring);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "tokType",      &toktype);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",         &dict_name);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "update",       &do_update);
  xmlrpc_decompose_value(env, params, "({s:i,*})", "returnTokens", &ret_tokens);

  /* Ok, at this point we have the segment string with the xml-style tags intact. */
  /* Just need to extract the strings to be tokenized and tokenize them */
  //printf("KDR_RECEIVED %s\n",instring);

  /* Has to be wrapped in an xml tag or the parser fails */
  asprintf(&doc_string,"<doc>%s</doc>",instring);
  doc          = xmlReadMemory(doc_string, strlen(doc_string), "noname.xml", NULL, 2);
  gk_free((void **)&doc_string, LTERM);
  root_element = xmlDocGetRootElement(doc);

  FAIL_IFTRUE(doc == NULL, "XML segment parsing failed");

  /* Various sanity checks */
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");
  FAIL_IFTRUE(strlen(instring) == 0, "Empty string supplied.");
  //LOGMSG1("Tokenize was called on dict %s.",dict_name);

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(dict_name == NULL || !hash_Exists(ServerState.dtionaries, dict_name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",dict_name);
    ret = xmlrpc_build_value(env, "s", r_string);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }
  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)dict_name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));

  RDLOCK_OR_FAIL(&dict->inuse_rwlock);              hasulock = 1;
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock  = 0; 

  ret = tokenize_segment(env, root_element, dict, ret_tokens, do_update, toktype);
  xmlCleanupParser();

  RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;

  gk_free((void **)&instring, &toktype, &dict_name, LTERM);
  return ret;


ERROR_EXIT:
  if(haslock  == 1) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
  if(hasulock == 1) RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;
  gk_free((void **)&instring, &toktype, &dict_name, LTERM);
  if (!env->fault_occurred)
    xmlrpc_env_set_fault(env, -404, "RPC failed at tokenize: dictionary does not exist.");

  return ret;
/* }}} */
} 

xmlrpc_value *ReverseTokenize(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo) 
{ 
  /* {{{ */
  char *instring=NULL, *name=NULL;
  xmlrpc_value *ret   = NULL;
  void *p;
  dict_t *dict   = NULL;
  int haslock    = 0;
  int hasulock   = 0;
  char *idstring = NULL;
  char *r_string = NULL;
  char *saveptr;
  char *word;
  int64_t id;
  int dsize;
  list_t *termlist = list_Create();
  list_t *idlist   = list_Create();
  xmlrpc_value *xv_termarray = NULL;
  xmlrpc_value *xv_idarray   = NULL;


  /* Get the parameters */
  xmlrpc_decompose_value(env, params, "({s:s,*})", "name",        &name);
  xmlrpc_decompose_value(env, params, "({s:s,*})", "query",      &instring);

  /* Various sanity checks */
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");
  FAIL_IFTRUE(strlen(instring) == 0, "Empty string supplied.");
  LOGMSG1("Reverse Tokenize was called on dict %s.",name);

  RDLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 1;
  if(name == NULL || !hash_Exists(ServerState.dtionaries, name, -1)) {
    asprintf(&r_string, "Dictionary %s does not exist.\n",name);
    ret = xmlrpc_build_value(env, "s", r_string);
    xmlrpc_env_set_fault_formatted(env, -404, "%s", r_string); 
    gk_free((void **)&r_string, LTERM);
    goto ERROR_EXIT;
  }
  p = (dict_t *)hash_Get(ServerState.dtionaries, (void *)name, -1, NULL);
  memcpy(&dict, p, sizeof(dict_t *));

  RDLOCK_OR_FAIL(&dict->inuse_rwlock);              hasulock = 1;
  RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock  = 0; 

  /* strtok the input string, and for each token get the id.  */
  idstring = strtok_r(instring, " ",&saveptr);
  while(idstring != NULL) {
    sscanf(idstring, "%"SCNd64, &id);
    if(db_Get(dict->rdict, (void *)&id, sizeof(int64_t), &word, &dsize)) {
      /* add to list of terms */
      list_PutEnd(termlist, word, -1);

      /* add to list of input ids */
      list_PutEnd(idlist, idstring, -1);
    }

    idstring = strtok_r(NULL, " ",&saveptr);
    db_FreeData(word);
  }

  ret = xmlrpc_struct_new(env);
  xv_termarray = create_xmlrpc_array_stringlist(env, termlist);
  xv_idarray   = create_xmlrpc_array_stringlist(env, idlist);
  xmlrpc_struct_set_value(env, ret, "terms", xv_termarray);
  xmlrpc_struct_set_value(env, ret, "ids",   xv_idarray);
  xmlrpc_DECREF(xv_termarray);
  xmlrpc_DECREF(xv_idarray);

  RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;
  gk_free((void **)&instring, &name, LTERM);
  list_Destroy(termlist);
  list_Destroy(idlist);
  return ret;


ERROR_EXIT:
  if(haslock  == 1) RWUNLOCK_OR_FAIL(&ServerState.dtionaries_rwlock); haslock = 0;
  if(hasulock == 1) RWUNLOCK_OR_FAIL(&dict->inuse_rwlock); hasulock = 0;
  gk_free((void **)&instring, &name, LTERM);
  if (!env->fault_occurred)
    xmlrpc_env_set_fault(env, -404, "RPC failed at Reverse tokenize: dictionary does not exist.");
  list_Destroy(termlist);
  list_Destroy(idlist);

  return ret;
/* }}} */
} 
