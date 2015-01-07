/*!
\file  
\brief Command-line parsing module

This file parses the command line arguments and provides a short help.

\date 11/24/2007
\author George
\version\verbatim $Id: cmdline.c 4172 2008-04-04 13:11:36Z karypis $ \endverbatim
*/


#include "tokserver.h"


/*-------------------------------------------------------------------*/
/* Command-line options */
/*-------------------------------------------------------------------*/
static struct gk_option long_options[] = {
  {"port",         1,      0,      CMD_PORTNUM},
  {"config",       1,      0,      CMD_CONFIG},
  {"daemon",       0,      0,      CMD_DAEMON},
  {"nocheck",      0,      0,      CMD_NOCHECK},
  {"sthread",      0,      0,      CMD_STHREAD},
  {"loginfo",      0,      0,      CMD_LOGINFO},
  {"logfile",      1,      0,      CMD_LOGFILE},
  {"help",         0,      0,      CMD_HELP},
  {0,              0,      0,      0}
};



/*-------------------------------------------------------------------*/
/* Mini help */
/*-------------------------------------------------------------------*/
static char helpstr[][100] =
{
" ",
"Usage: toksrv [options]",
" ",
" Optional parameters",
"  -port=int",
"     Specifies the port number where the XML-RPC server will be listening",
"     to. The default is 8080.",
" ",
"  -config=filename",
"     Specifies the config file to be read.",
" ",
"  -dict=filename",
"     Specifies the dictionary filename to be used.",
" ",
"  -nocheck",
"     Do not check dictionaries at startup.  *USE AT YOUR OWN RISK*",
" ",
"  -logfile=string",
"     Specifies the fully qualifed filename that will store the logfile",
"     for the server. The default depends if the -daemon option has been",
"     specified or not. If it has, then it is /tmp/toksrv.log, otherwise",
"     it writes all log messages to stderr.",
" ",
"  -daemon",
"     Installs the program as a daemon.",
" ",
"  -sthread",
"     Runs in single-threaded mode.",
" ",
"  -loginfo",
"     Info messages will be logged as well.",
" ",
" ",
"  -help",
"     Prints this message.",
""
};

 


/*************************************************************************/
/*! This is the entry point of the command-line argument parser */
/*************************************************************************/
void parse_cmdline(params_t *params, int argc, char *argv[])
{
  int i;
  int c, option_index;

  /* Parse the command line arguments  */
  while ((c = gk_getopt_long_only(argc, argv, "", long_options, &option_index)) != -1) {
    switch (c) {
      case CMD_PORTNUM:
        if (gk_optarg) params->port = atoi(gk_optarg);
        break;

      case CMD_CONFIG:
        if (gk_optarg) {
          gk_free((void **)&params->configfile, LTERM);
          params->configfile = gk_strdup(gk_optarg);
        }
        break;

      case CMD_LOGFILE:
        if (gk_optarg) params->logfile = gk_strdup(gk_optarg);
        break;

      case CMD_DAEMON:
        params->daemon = 1;
        break;

      case CMD_STHREAD:
        params->sthread = 1;
        break;

      case CMD_NOCHECK:
        params->nocheck = 1;
        break;

      case CMD_LOGINFO:
        params->loginfo = 1;
        break;

      case CMD_VERSION:
        printf("Build information: %s\n", SVNINFO);
        exit(0);
        break;

      case CMD_HELP:
        for (i=0; strlen(helpstr[i]) > 0; i++)
          printf("%s\n", helpstr[i]);
        exit(0);
        break;
      case '?':
      default:
        printf("Illegal command-line option(s)\nUse %s -help for a summary of the options.\n", argv[0]);
        exit(0);
    }
  }

  if (argc-gk_optind != 0) {
    printf("Unrecognized parameters. argc: %d optind %d\n",argc,gk_optind);
    for (i=0; strlen(helpstr[i]) > 0; i++)
      printf("%s\n", helpstr[i]);
    printf("About to exit\n");
    exit(0);
  }

  if (params->logfile == NULL && params->daemon)
    params->logfile = gk_strdup("/tmp/toksrv.log");
}
