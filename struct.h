/*
 * struct.h
 *
 * This file contains various data structure definitions used by the server
 *
 * Started 11/16/2007
 * George
 *
 * $Id: struct.h 2812 2007-12-03 01:46:37Z karypis $
 */

#ifndef _MTCFSRV_STRUCT_H_
#define _MTCFSRV_STRUCT_H_

typedef struct {
  fvector_t *q1;
  fvector_t *q2;
  fvector_t *q3;
  fvector_t *q4;
} fourpartvec;

typedef struct {
  int cachesize;
  int dictvalid;
  //char *dictfile;
  char *configfile;
  char *name;
  char *filename;
  char *rfilename;
  hhash_t *dict;
  db_t *rdict;
  char *tokregex;
  hash_t *stophash;

  int dostem;
  int dostop;

  int64_t cachehit;
  int64_t cachemiss;
  int64_t cacherequest;
  int64_t ninserts;

  char *stopwords;
  char *stopdelim;
  char *addstopwords;
  char *delstopwords;
  
  pthread_rwlock_t dictstate_rwlock;   /* This lock protects the dictionary itself */
  pthread_rwlock_t dict_statslock;     /* This lock protects the dictionary's statistics variables */
  pthread_rwlock_t inuse_rwlock;       /* This ensures the dictionary doesn't get deleted at a bad time */

} dict_t;

typedef struct {
  int64_t firstid;
  int64_t time_sec;
  int64_t time_usec;
  char *terms;
} journal_t;

/*************************************************************************
* This data structure stores the state of the server
**************************************************************************/
typedef struct {
  /* The following three fields are used to implement a simple internal
     to external mapping of the handles returned by the CF engine */

  int port;             /* Server information */
  int daemon;           /* Indicates if the server will be run as a daemon */
  char *logfile;        /* Log file */
  int sthread;          /* Indicates that a single-threaded server will be run */
  int loginfo;          /* Indicates that info messages will be logged */

  //int toktype;    

  int verbosity;
  int nocheck;
  int backup_enabled;

  char *dictdir;
  char *journaldir;
  char *dbtest;
  char *configfile;

  //FILE *JFILE;

  hash_t *dtionaries;
  hash_t *default_stoplist;

  /* Thread synchronization constructs */
  pthread_mutex_t errprintf_mutex;      /* This mutex protects the error reporting output */
  pthread_mutex_t srvstate_mutex;       /* This mutex protects the server's state variables */
  pthread_rwlock_t srvstate_rwlock;     /* This lock protects the server's state variables */

  pthread_rwlock_t testlock;            /* This lock is used to for testing */
  pthread_rwlock_t statslock;           /* This lock is used to protect cache stats */

  pthread_rwlock_t dtionaries_rwlock;   /* This lock is used to protect the hash of dict_t pointers */

} params_t;
#endif 

