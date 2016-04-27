/*!
\file  
\brief Various utility functions for the server

\date 11/16/2007
\author George
\version \verbatim $Id: utils.c 2805 2007-12-01 23:08:55Z karypis $  \endverbatim
*/

#include "tokserver.h"

//GK_MKALLOC(w,   wgt_t)

/*******************************************************************************/
/*! This function generates a random string of letters.  It allocates memory, 
 *  which must be freed outside.
 *  \returns A char * with a random string of letters behind it. */
/*******************************************************************************/
char *rand_str(int len)
{
  int i;
  unsigned int seedp = 37;
  char *s = gk_malloc(len*sizeof(char), "rand_str");
  for (i = 0; i < len-1; i++) {
    s[i] = 'a' + (rand_r(&seedp) % 26);
  }
  s[len-1] = '\0';
  return s;
}

/*******************************************************************************/
/*! This function copies file src to file dest.  If the file needs to be created,
 *  it will be, with permissions set to u+rw.  Otherwise the permissions for dest
 *  will be the same as those for src.
 *  \returns 1 on error, 0 if everything went ok. */
/*******************************************************************************/
int copy_file(char *src, char *dest) {
  int inf, outf;
  int bytes;
  char line[512];
  if((inf  = open(src, O_RDONLY)) == -1){
    return 1;
  }
  if((outf = open(dest, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
    return 1;
  }
  while((bytes = read(inf, line, sizeof(line))) > 0)
    write(outf, line, bytes);

  fsync(outf);
  close(inf);
  close(outf);
  return 0;
}


#if 0
/* {{{ */
/*******************************************************************************/
/*! This function parses a variable length integer array and returns it. */
/*******************************************************************************/
int parse_xmlrpc_array_i(xmlrpc_env *const env, xmlrpc_value *const xv_array, 
                         idx_t *r_size, idx_t **r_array)
{
  idx_t i, *array=NULL;
  size_t size;
  xmlrpc_value *xv_value;
  xmlrpc_int value;

  *r_array = NULL;
  *r_size = -1;

  size = xmlrpc_array_size(env, xv_array);
  FAIL_IFTRUE(env->fault_occurred, "Failed to get the size of the array.");

  /* Return if the array is empty */
  if (size == 0) {
    *r_size = 0;
    return 1;
  }

  FAIL_IFTRUE((array = imalloc(size, "parse_xmlrpc_array_i: array")) == NULL,
              "Failed to allocate memory for an int parameter array.");

  for (i=0; i<size && !env->fault_occurred; i++) {
    xmlrpc_array_read_item(env, xv_array, i, &xv_value);
    if (!env->fault_occurred) {
      xmlrpc_read_int(env, xv_value, &value);
      array[i] = (idx_t)value;
      xmlrpc_DECREF(xv_value);
    }
  }
  FAIL_IFTRUE(env->fault_occurred, "Failed while parsing the array.");

  *r_array = array;
  *r_size  = (idx_t)size;

  return 1;

ERROR_EXIT:
  gk_free((void **)&array, LTERM);

  return -1;
}


/*******************************************************************************/
/*! This function parses a variable length integer array and returns it. */
/*******************************************************************************/
int parse_xmlrpc_array_d(xmlrpc_env *const env, xmlrpc_value *const xv_array, 
                         idx_t *r_size, wgt_t **r_array)
{
  idx_t i;
  wgt_t *array=NULL;
  size_t size;
  xmlrpc_value *xv_value;
  xmlrpc_double value;
  xmlrpc_int i_value;

  *r_array = NULL;
  *r_size = -1;

  size = xmlrpc_array_size(env, xv_array);
  FAIL_IFTRUE(env->fault_occurred, "Failed to get the size of the array.");

  /* Return if the array is empty */
  if (size == 0) {
    *r_size = 0;
    return 1;
  }

  FAIL_IFTRUE((array = wmalloc(size, "parse_xmlrpc_array_d: array")) == NULL,
              "Failed to allocate memory for a double parameter array.");

  for (i=0; i<size && !env->fault_occurred; i++) {
    xmlrpc_array_read_item(env, xv_array, i, &xv_value);
    if (!env->fault_occurred) {
      if (xmlrpc_value_type(xv_value) == XMLRPC_TYPE_INT) {
        xmlrpc_read_int(env, xv_value, &i_value);
        value = i_value;
      }
      else {
        xmlrpc_read_double(env, xv_value, &value);
      }

      array[i] = (wgt_t)value;
      xmlrpc_DECREF(xv_value);
    }
  }
  FAIL_IFTRUE(env->fault_occurred, "Failed while parsing the array.");

  *r_array = array;
  *r_size  = (idx_t)size;

  return 1;

ERROR_EXIT:
  gk_free((void **)&array, LTERM);

  return -1;
}
/* }}} */
#endif


/*******************************************************************************/
/*! This function converts a user supplied array into an xmlrpc_value array */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array_i(xmlrpc_env *const env, idx_t size, idx_t *array)
{
/* {{{ */
  idx_t i;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_array;

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_i failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_build_value(env, "i", (int)array[i]);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_i failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
/* }}} */
}

/*******************************************************************************/
/*! This function converts an array of strings into an xmlrpc_value array */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array_s(xmlrpc_env *const env, idx_t size, char **array)
{
/* {{{ */
  idx_t i;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_array;

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_s failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_build_value(env, "s", array[i]);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_s failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
/* }}} */
}

/*******************************************************************************/
/*! This function converts a list_t of int64_t into an xmlrpc_value array */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array_i64list(xmlrpc_env *const env, list_t *list)
{
/* {{{ */
  int i;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_array;
  int size = list_Size(list);

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_i64list failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_i8_new(env, (*(int64_t *)list_GetIth(list,i,NULL)));
//printf("%"PRId64"\n",(*(int64_t *)list_GetIth(list,i,NULL)));
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_i64list failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
/* }}} */
}

/*******************************************************************************/
/*! This function converts a list_t of int64_t into an xmlrpc_value array */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array_stringlist(xmlrpc_env *const env, list_t *list)
{
/* {{{ */
  int i;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_array;
  int size = list_Size(list);

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_stringlist failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_build_value(env, "s", list_GetIth(list,i,NULL));
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_stringlist failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
/* }}} */
}

/*******************************************************************************/
/*! This function converts the string keys from a hash_t into an xmlrpc_value array */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array_hash_t(xmlrpc_env *const env, hash_t *hash)
{
/* {{{ */
  int i=0;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_array;

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_hash_t failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  i=0;
  hash_IterInit(hash);
  while(i<hash_Size(hash) && !env->fault_occurred) {
    xv_value = xmlrpc_build_value(env, "s", (char *)hash_IterGet(hash, NULL));
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
    i++;
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_hash_t failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
/* }}} */
}

void create_xmlrpc_array_lkiv(xmlrpc_env *const env, int size, gk_lkiv_t *v, xmlrpc_value **r_key, xmlrpc_value **r_val)
{
/* {{{ */
  int i;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_key_array;
  xmlrpc_value *xv_val_array;

  /* build array for keys */
  xv_key_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_lkiv failed to create xv_key_array!\n", __FILE__, __LINE__);
    goto ERROR_EXIT;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_i8_new(env, v[i].key);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_key_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_lkiv failed while appending item %d into the xv_key_array!\n", __FILE__, __LINE__, i);
    xmlrpc_DECREF(xv_key_array);
    goto ERROR_EXIT;
  }

  /* build array for values */
  xv_val_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_lkiv failed to create xv_val_array!\n", __FILE__, __LINE__);
    goto ERROR_EXIT;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_build_value(env, "i", v[i].val);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_val_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_lkiv failed while appending items into the xv_val_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_key_array);
    goto ERROR_EXIT;
  }
  
  *r_key = xv_key_array;
  *r_val = xv_val_array;
  return;

  ERROR_EXIT:
  *r_key = NULL;
  *r_val = NULL;
  return;
/* }}} */
}


/*******************************************************************************/
/*! This function converts a user supplied array into an xmlrpc_value array */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array_d(xmlrpc_env *const env, idx_t size, wgt_t *array)
{
  /* {{{ */
  idx_t i;
  xmlrpc_value *xv_value;
  xmlrpc_value *xv_array;

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_d failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<size && !env->fault_occurred; i++) {
    xv_value = xmlrpc_build_value(env, "d", (double)array[i]);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_d failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
  /* }}} */
}


/*******************************************************************************/
/*! This function converts a user supplied array-of-arrays into the equivalent
    xmlrpc_value array-of-arrays */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array2_i(xmlrpc_env *const env, 
                                     idx_t nrows, idx_t *sizes, idx_t **array)
{
  idx_t i;
  xmlrpc_value *xv_value=NULL, *xv_array=NULL;

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array2_i failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<nrows && !env->fault_occurred; i++) {
    xv_value = create_xmlrpc_array_i(env, sizes[i], array[i]);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_i failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
}


/*******************************************************************************/
/*! This function converts a user supplied array-of-arrays into the equivalent
    xmlrpc_value array-of-arrays */
/*******************************************************************************/
xmlrpc_value *create_xmlrpc_array2_d(xmlrpc_env *const env, 
                                     idx_t nrows, idx_t *sizes, wgt_t **array)
{
  idx_t i;
  xmlrpc_value *xv_value=NULL, *xv_array=NULL;

  xv_array = xmlrpc_array_new(env);
  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array2_i failed to create xv_array!\n", __FILE__, __LINE__);
    return NULL;
  }

  for (i=0; i<nrows && !env->fault_occurred; i++) {
    xv_value = create_xmlrpc_array_d(env, sizes[i], array[i]);
    if (!env->fault_occurred) {
      xmlrpc_array_append_item(env, xv_array, xv_value);
      xmlrpc_DECREF(xv_value);
    }
  }

  if (env->fault_occurred) {
    errprintf("[%s:%d]create_xmlrpc_array_i failed while appending items into the xv_array!\n", __FILE__, __LINE__);
    xmlrpc_DECREF(xv_array);
    return NULL;
  }

  return xv_array;
}


/*******************************************************************************/
/*! This function looks for a structure with the specified key and returns its
    value as a double. The actual value can be provided either as int or as 
    double */
/*******************************************************************************/
xmlrpc_double get_params_value_double(xmlrpc_env *const env, xmlrpc_value *const params, char *key)
{
  xmlrpc_value *xv_struct=NULL, *xv_value=NULL;
  xmlrpc_int i_value;
  xmlrpc_double d_value;
  
  xmlrpc_decompose_value(env, params, "(S)", &xv_struct);
  FAIL_IFTRUE(env->fault_occurred, "Failed to get the parameter structure.");

  xmlrpc_struct_find_value(env, xv_struct, key, &xv_value);  
  FAIL_IFTRUE1(env->fault_occurred, "Failed to get the value for key: %s.", key);
  
  FAIL_IFTRUE1(xmlrpc_value_type(xv_value) != XMLRPC_TYPE_INT && 
               xmlrpc_value_type(xv_value) != XMLRPC_TYPE_DOUBLE,
               "The value for %s must be either an integer or a double.", key);

  if (xmlrpc_value_type(xv_value) == XMLRPC_TYPE_DOUBLE) {
    xmlrpc_read_double(env, xv_value, &d_value);
  }
  else {
    xmlrpc_read_int(env, xv_value, &i_value);
    d_value = i_value;
  }
  FAIL_IFTRUE1(env->fault_occurred, "Failed in getting the int/double value for key: %s.", key);
    
  xmlrpc_DECREF(xv_struct);
  xmlrpc_DECREF(xv_value);

  return d_value;


ERROR_EXIT:
  if (xv_struct)
    xmlrpc_DECREF(xv_struct);
  if (xv_value)
    xmlrpc_DECREF(xv_value);

  return 0.0;
}

