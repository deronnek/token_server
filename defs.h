/*!
\file  defs.h
\brief Various constants used by the program

\date 6/24/2007
\author George
\version \verbatim $Id: defs.h 4171 2008-04-04 12:59:51Z karypis $ \endverbatim
*/


#ifndef _MTCFBIN_DEF_H_
#define _MTCFBIN_DEF_H_

/* Command-line options */
#define CMD_PORTNUM             1
#define CMD_NMODELS             2
#define CMD_DAEMON              3
#define CMD_LOGFILE             4
#define CMD_STHREAD             5
#define CMD_LOGINFO             6
#define CMD_CONFIG              7
#define CMD_NOCHECK             8

#define CMD_VERSION             100

#define CMD_HELP                101

//#define TOKSERVER_DBTYPE        DBI_BDBHASH
#define TOKSERVER_DBTYPE        DBI_TCHASH



/* Various constants for buffer specifications */
#define MAX_STRLEN              8192

#endif

/* from Sam: */
#define INDEXDELIM " \t\\/()\"[]{}\n'<>=|*"
#define QUERYDELIM " \t\\/()[]{}\n'<>=|"
#define SENTENCEDELIM ",;.?!"

#define WORDDELIM ":,;.?! \t\\/()\"[]{}\n'<>=|*"

