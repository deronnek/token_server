/*************************************************************************/
/*! \file config.c
\brief Reading and parsing the crawler's configuration file.

This file contains various routines for parsing the configuration file and
setting options based on it.

\par Format of the configuration file

The configuration file is a plain text file containing various options. The options
are of two types: scalar-value and list-value. 

Scalar-value options take a single value and their format is:
\code
option-name = value
\endcode

List-value options takes as input a list of values. Each value is stored on a 
separate line and corresponds to a perl-compatible regular expression. The 
entire list has the format:
\code
<option-name>
value1
value2
...
</option-name>
\endcode
where the start/ending delimiters of the option name appear in a line by 
themselves.

The configuration file also allows for inclusion of external files. This is 
done by using a <tt>\#include "filename"</tt> directive.

Any lines starting with a \c # are considered to be comments and are ignored.

\date Started 2/13/07
\author George
\version\verbatim $Id: config.c 3158 2009-10-19 14:37:50Z karypis $\endverbatim
*/
/*************************************************************************/

#include "tokserver.h"


/*************************************************************************/
/*! \brief Reads the configuration file

This function reads the various options from the configuration file. For
those options that can be specified both via the command-line and the
config file, the values specified at the command-line overide those 
specified at the configuration file.
*/
/**************************************************************************/
void config_ReadConfigFile(params_t *params)
{
  //idx_t i;
  char *value;

  if (params->configfile)
    config_PreProcessConfigFile(params);

  /*----------------------------------------------------------------------------
  * These are value-based options that can be specified in the config file and
  * at the command line
  *-----------------------------------------------------------------------------*/
  if ((value = config_GetValue(params->configfile, "port"))) {
    params->port = atoi(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(params->configfile, "daemon"))) {
    params->daemon = atoi(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(params->configfile, "sthread"))) {
    params->sthread = atoi(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(params->configfile, "loginfo"))) {
    params->loginfo = atoi(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(params->configfile, "logfile"))) {
    params->logfile = gk_strdup(value);
    gk_free((void **)&value, LTERM);
  }
  if ((value = config_GetValue(params->configfile, "journaldir"))) {
    gk_free((void **)&params->journaldir, LTERM);
    params->journaldir = value;
  }
  if ((value = config_GetValue(params->configfile, "dictdir"))) {
    gk_free((void **)&params->dictdir, LTERM);
    params->dictdir = value;
  }
  if ((value = config_GetValue(params->configfile, "dbtest"))) {
    gk_free((void **)&params->dbtest, LTERM);
    params->dbtest  = value;
  }

  /*----------------------------------------------------------------------------
  * These are list-based options that can be specified only in the config file 
  *-----------------------------------------------------------------------------*/
  //params->urls_valid_l         = config_GetList(params->configfile, "+urls");

  /*----------------------------------------------------------------------------
  * These are hash-based options that can be specified only in the config file
  *-----------------------------------------------------------------------------*/
  /*
  if (params->mode == MODE_FETCH || params->mode == MODE_SEGMENT) {
    params->servers_valid_h      = config_GetHash(params->configfile, "+servers");
    params->servers_invalid_h    = config_GetHash(params->configfile, "-servers");
  }
  */


  /* The following actually deletes the temporary configuration file */
  if (params->configfile)
    unlink(params->configfile);
}


/*************************************************************************/
/*! \brief Preprocess the contents of a configuration file.

This function uses gpp to process a pre-process a configuration file.
The resulting configuration file is stored in /tmp/gpp.pid, and is 
used as the input to the configuration routines.

*/
/**************************************************************************/
void config_PreProcessConfigFile(params_t *params)
{
  char cmd[1024];

  sprintf(cmd, "%s %s -o /tmp/gpp.%d", "gpp -C", params->configfile, (int)getpid());
  if (system(cmd) == -1)
    errexit("Failed on executing system command: %s\n", cmd);

  sprintf(cmd, "/tmp/gpp.%d", (int)getpid());
  gk_free((void **)&params->configfile, LTERM);
  params->configfile = gk_strdup(cmd);
}
  

/*************************************************************************/
/*! \brief Reads the values specified for a list option

This function reads a configuration file and returns the list of values 
associated with the specified option.

\param filename 
  Is the name of the configuration file.
\param optname 
  Is the name of the option to be read.
\return 
  A list of values, if the option is present in the config file, otherwise 
  it returns NULL. A NULL value is also returned if the configuration file 
  does not exist. The list of values is returned in the form of a list 
  using QDBM's CABIN API.

*/
/**************************************************************************/
list_t *config_GetList(char *filename, char *optname)
{
  int inList, rc;
  size_t lnlen;
  char *line=NULL, *str_start, *str_end;
  FILE *fpin;
  list_t *values=NULL;

  if (!filename && !gk_fexists(filename)) 
    return NULL;

  str_start = gk_malloc(sizeof(char)*(strlen(optname)+3), "config_GetList: str_start");
  str_end   = gk_malloc(sizeof(char)*(strlen(optname)+4), "config_GetList: str_end");

  sprintf(str_start, "<%s>", optname);
  sprintf(str_end, "</%s>", optname);
  gk_strtolower(str_start);
  gk_strtolower(str_end);

  fpin = gk_fopen(filename, "r", "config_GetList: filename");
  
  inList = 0;
  while ((rc = gk_getline(&line, &lnlen, fpin)) != -1) {
    gk_strtprune(line, " \t\n");
    gk_strhprune(line, " \t");

    if (strlen(line) == 0 || line[0] == '#')
      continue;

    if (!inList) {
      if (gk_strcasecmp(line, str_start)) {
        values = list_Create();
        inList = 1;
      }
    }
    else {
      if (gk_strcasecmp(line, str_end)) {
        inList = 0;

        /* Deal with the case that no actual values were provided */
        if (list_Size(values) == 0) {
          list_Destroy(values);
          values = NULL;
        }

        break;
      }

      if (strlen(line) > 0)
        list_PutEnd(values, line, -1);
    }
  }

  gk_fclose(fpin);
  gk_free((void **)&line, &str_start, &str_end, LTERM);

  if (rc == -1 && inList) 
    errexit("Incomplete configuration file for: %s.", optname);

  //printf("config_GetList: %s=%p [%d]\n", optname, values, (values ? list_Size(values) : 0));

  return values;
}


/*************************************************************************/
/*! \brief Reads the values specified for a hash option

This function reads a configuration file and returns the list of values 
associated with the specified option.

\param filename 
  Is the name of the configuration file.
\param optname 
  Is the name of the option to be read.
\return 
  A hash in which the values present in the config file are the keys.
  A NULL value is also returned if the configuration file does not exist. 

*/
/**************************************************************************/
hash_t *config_GetHash(char *filename, char *optname)
{
  int inList, rc;
  size_t lnlen;
  char *line=NULL, *str_start, *str_end;
  FILE *fpin;
  hash_t *values=NULL;

  if (!filename && !gk_fexists(filename)) 
    return NULL;

  str_start = gk_malloc(sizeof(char)*(strlen(optname)+3), "config_GetList: str_start");
  str_end   = gk_malloc(sizeof(char)*(strlen(optname)+4), "config_GetList: str_end");

  sprintf(str_start, "<%s>", optname);
  sprintf(str_end, "</%s>", optname);
  gk_strtolower(str_start);
  gk_strtolower(str_end);

  fpin = gk_fopen(filename, "r", "config_GetList: filename");
  
  inList = 0;
  while ((rc = gk_getline(&line, &lnlen, fpin)) != -1) {
    gk_strtprune(line, " \t\n");
    gk_strhprune(line, " \t");

    if (strlen(line) == 0 || line[0] == '#')
      continue;

    if (!inList) {
      if (gk_strcasecmp(line, str_start)) {
        values = hash_Create();
        inList = 1;
      }
    }
    else {
      if (gk_strcasecmp(line, str_end)) {
        inList = 0;

        /* Deal with the case that no actual values were provided */
        if (hash_Size(values) == 0) {
          hash_Destroy(values);
          values = NULL;
        }

        break;
      }

      if (strlen(line) > 0) {
        //printf("Adding %s in hash %s\n", line, optname);
        hash_Put(values, line, -1, NULL, 0);
      }
    }
  }

  gk_fclose(fpin);
  gk_free((void **)&line, &str_start, &str_end, LTERM);

  if (rc == -1 && inList) 
    errexit("Incomplete configuration file for: %s.", optname);

  return values;
}


/*************************************************************************/
/*! \brief Reads the value specified for an option

This function reads a configuration file and returns the value associated 
with the specified option.

\param filename 
  Is the name of the configuration file.
\param optname 
  is the name of the option to be read.
\returns 
  A point to a string that stores the value, if the option is present 
  in the config file, otherwise it returns NULL. The memory of the
  returned string needs to be freed by the calling routine using
  the gk_free() function. A NULL value is also returned if the 
  configuration file does not exist.
*/
/**************************************************************************/
char *config_GetValue(char *filename, char *optname)
{ /* {{{ */
  int rc;
  size_t lnlen=0;
  char *line=NULL, *pattern, *match=NULL;
  FILE *fpin;


  if (!filename || !gk_fexists(filename)) 
    return NULL;

  pattern = gk_cmalloc(100+strlen(optname), "config_GetValue: pattern");
  sprintf(pattern, "^%s\\s*[:=]\\s*(.+)", optname);

  fpin = gk_fopen(filename, "r", "config_GetValue: filename");
  
  while ((rc = gk_getline(&line, &lnlen, fpin)) != -1) {
    gk_strtprune(line, " \t\n");
    gk_strhprune(line, " \t");

    if (strlen(line) == 0 || line[0] == '#')
      continue;

    if ((match = util_GetMatch(line, pattern, 1)))
      break;
  }

  gk_fclose(fpin);
  gk_free((void **)&line, &pattern, LTERM);


  //printf("config_GetValue: %s=%s\n", optname, match);

  return match;
} /* }}} */

