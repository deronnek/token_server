/*!
\file  tokserver.h
\brief Top-level header file for the xml-rpc server 

\date 11/16/2007
\author George
\version \verbatim $Id: tokserver.h 2757 2007-11-26 14:26:32Z karypis $ \endverbatim
*/

#include <GKlib.h>
#include <common/common.h>
#include <tok.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>

#include <qdbm/cabin.h>
#include <qdbm/depot.h>

#include <xmlrpc.h>
#include <xmlrpc_server.h>
#include <xmlrpc_abyss.h>

//#include "../lib/defs.h"
//#include "../lib/struct.h"
//#include "../lib/proto.h"
#include <defs.h>
#include <struct.h>
#include <proto.h>
#include <macros.h>
#include <dirent.h>


/*******************************************************************************/
/*! This is a global variable for the server's state.
    In principle accessing it will not require locking */
/*******************************************************************************/
#ifdef MTCFSRV_MAIN
params_t ServerState;
#else
extern params_t ServerState;
#endif
