/*
 * macros.h 
 *
 * This file contains various macros
 *
 * Started 11/21/2007
 * George
 *
 * $Id: macros.h 2812 2007-12-03 01:46:37Z karypis $
 */

#ifndef _MTCFSRV_MACROS_H_
#define _MTCFSRV_MACROS_H_


#define FAIL_IFTRUE(expr, msg) \
  do { \
    if (expr) { \
      errprintf("[%s:%d]Error: [%s] was true! [Message: %s]\n", __FILE__, __LINE__, #expr, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] " msg, oldstr); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)

#define FAILJ_IFTRUE(expr, msg) \
  do { \
    if (expr) { \
      errprintf("[%s:%d]Error: [%s] was true! [Message: %s]\n", __FILE__, __LINE__, #expr, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] " msg, oldstr); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, msg); \
      goto JOURNAL_ERROR_EXIT;\
    }\
  }\
  while(0)

#define FAILD_IFTRUE(expr, msg, arg1) \
  do { \
    if (expr) { \
      errprintf("[%s:%d]Error: [%s] was true! [Message: " msg "]\n", __FILE__, __LINE__,#expr,arg1); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] " msg, oldstr, arg1); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1); \
      goto DICT_ERROR_EXIT;\
    }\
  }\
  while(0)

#define FAIL_IFTRUE1(expr, msg, arg1) \
  do { \
    if (expr) { \
      errprintf("[%s:%d]Error: [" #expr "] was true! [Message: " msg "]\n", __FILE__, __LINE__, arg1); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] " msg, oldstr, arg1); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)


#define FAIL_IFTRUE2(expr, msg, arg1, arg2) \
  do { \
    if (expr) { \
      errprintf("[%s:%d]Error: [" #expr "] was true! [Message: " msg "]\n", __FILE__, __LINE__, arg1, arg2); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] " msg, oldstr, arg1, arg2); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1, arg2); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1, arg2); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)


#define FAIL_IFTRUE3(expr, msg, arg1, arg2, arg3) \
  do { \
    if (expr) { \
      errprintf("[%s:%d]Error: [" #expr "] was true! [Message: " msg "]\n", __FILE__, __LINE__, arg1, arg2, arg3); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] " msg, oldstr, arg1, arg2, arg3); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1, arg2, arg3); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, msg, arg1, arg2, arg3); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)


#define LOGMSG(msg) \
  do { \
    if (ServerState.loginfo) \
      errprintf("[%s:%d] Info: [" msg "]\n", __FILE__, __LINE__); \
  }\
  while(0)

#define LOGMSG1(msg, arg1) \
  do { \
    if (ServerState.loginfo) \
      errprintf("[%s:%d] Info: [" msg "]\n", __FILE__, __LINE__, arg1); \
  }\
  while(0)

#define LOGMSG2(msg, arg1, arg2) \
  do { \
    if (ServerState.loginfo) \
      errprintf("[%s:%d] Info: [" msg "]\n", __FILE__, __LINE__, arg1, arg2); \
  }\
  while(0)


#define LOCK_OR_FAIL(lock) \
  do { \
    int rc;\
    if (lock == NULL) continue; \
    rc = pthread_mutex_lock(lock); \
    if (rc) { \
      char msg[MAX_STRLEN]; \
      snprintf(msg, MAX_STRLEN, "[Failed to get the lock: " #lock "]! [Message: %s]", gk_strerror(rc)); \
      errprintf("[%s:%d]Error: %s\n", __FILE__, __LINE__, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] %s", oldstr, msg); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)

#define UNLOCK_OR_FAIL(lock) \
  do { \
    int rc;\
    if (lock == NULL) continue; \
    rc = pthread_mutex_unlock(lock); \
    if (rc) { \
      char msg[MAX_STRLEN]; \
      snprintf(msg, MAX_STRLEN, "[Failed to release the lock: [%s]! [Message: %s]", #lock, gk_strerror(rc)); \
      errprintf("[%s:%d]Error: %s\n", __FILE__, __LINE__, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] %s", oldstr, msg); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)


#define WRLOCK_OR_FAIL(lock) \
  do { \
    int rc;\
    if (lock == NULL) continue; \
    rc = pthread_rwlock_wrlock(lock); \
    if (rc) { \
      char msg[MAX_STRLEN]; \
      snprintf(msg, MAX_STRLEN, "[Failed to get the wrlock: " #lock " for writing]! [Message: %s]", gk_strerror(rc)); \
      errprintf("[%s:%d]Error: %s\n", __FILE__, __LINE__, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] %s", oldstr, msg); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)

#define RDLOCK_OR_FAIL(lock) \
  do { \
    int rc;\
    if (lock == NULL) continue; \
    rc = pthread_rwlock_rdlock(lock); \
    if (rc) { \
      char msg[MAX_STRLEN]; \
      snprintf(msg, MAX_STRLEN, "[Failed to get the wrlock: %s for reading]! [Message: %s]", #lock, gk_strerror(rc)); \
      errprintf("[%s:%d]Error: %s\n", __FILE__, __LINE__, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] %s", oldstr, msg); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)

#define RWLOCK_OR_FAIL(lock) \
  do { \
    int rc;\
    if (lock == NULL) continue; \
    rc = pthread_rwlock_wrlock(lock); \
    if (rc) { \
      char msg[MAX_STRLEN]; \
      snprintf(msg, MAX_STRLEN, "[Failed to get the rwlock: %s for writing]! [Message: %s]", #lock, gk_strerror(rc)); \
      errprintf("[%s:%d]Error: %s\n", __FILE__, __LINE__, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] %s", oldstr, msg); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)

#define RWUNLOCK_OR_FAIL(lock) \
  do { \
    int rc;\
    if (lock == NULL) continue; \
    rc = pthread_rwlock_unlock(lock); \
    if (rc) { \
      char msg[MAX_STRLEN]; \
      snprintf(msg, MAX_STRLEN, "[Failed to release the rwlock: %s]! [Message: %s]", #lock, gk_strerror(rc)); \
      errprintf("[%s:%d]Error: %s\n", __FILE__, __LINE__, msg); \
      if (env->fault_occurred && env->fault_string != NULL) {\
        char *oldstr=NULL;\
        oldstr = strdup(env->fault_string);\
        if (oldstr) {\
           xmlrpc_env_set_fault_formatted(env, -200, "[%s] %s", oldstr, msg); \
           free(oldstr);\
        }\
        else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      }\
      else xmlrpc_env_set_fault_formatted(env, -200, "%s", msg); \
      goto ERROR_EXIT;\
    }\
  }\
  while(0)

#endif 

/* Get a match from PCRE */
#define PCRE_GET_MATCH(subject, offsets, mnum)\
 strncpy(gk_csmalloc(offsets[2*(mnum)+1]-offsets[2*(mnum)]+1, '\0', "pcre-get-match"),\
         subject+offsets[2*(mnum)], offsets[2*(mnum)+1]-offsets[2*(mnum)])

