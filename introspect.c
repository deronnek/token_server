/*************************************************************************/
/*!
\file introsepct.c
\brief XML-RPC Introspection routines
 
This file contains various routines associated with loging error messages.

\date Started 1/19/2009 \author Kevin
*/
/*************************************************************************/
/*! \mainpage The tokenization server supports xmlrpc introspection.  
 * The three standard introspection methods are defined:
 * -system.listMethods 
 * -system.methodSignature
 * -system.methodHelp
 *
 * For more information see http://xmlrpc-c.sourceforge.net/introspection.html
 *
 * As of 2010.01.19 system.methodHelp does nothing but send a pointer to 
 * this documentation.  
 *
 * system.methodSignature returns an array of arrays, one sub-array for 
 * each signature the server "knows" about.  The first entry of a sub-array 
 * is always the return type of the method.  In the case of a method taking 
 * a structure as a parameter, system.methodSignature will return a structure
 * containing name,type pairs.  For example, if a method takes a structure with
 * an entry name of type string, system.methodSignature will return a struct
 * with a "name","string" entry.  This is not covered in the usual introspection
 * description.
 *
 */
/*************************************************************************/

#include "tokserver.h"

xmlrpc_value *hirundo_MethodSignature(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo)  
{ 
/* {{{ */
  char *method_name = NULL;
  xmlrpc_value *ret = NULL;
  char r_string[2048];

  xmlrpc_decompose_value(env, params, "(s)", &method_name);
  FAIL_IFTRUE(env->fault_occurred, "Initial parameter passing failed.");

  if(strcmp(method_name,"ts.DictCreate")           == 0) {
    //ret = xmlrpc_build_value(env, "((s(ss{s:s,s:s,s:s,s:s,s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s},s:{s:s,s:s,s:s}}))",
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss),s:(sss),s:(sss),s:(sss),s:(sss),s:(sss),s:(sss),s:(sss),s:(sss)})))",

                              "struct",  
  
                              "Toplevel parameter","yes","struct",

                              "name",
                              "The name of the dictionary to delete","yes","string",

                              "tokregex",
                              "Regular expression to match a token","yes","string",

                              "cachesize",
                              "Number of terms to cache","yes","int",

                              "dostem",
                              "Stem tokens before storing (1=yes,0=no)","yes","int",

                              "dostop",
                              "Ignore stop words when tokenizing (1=yes,0=no)","yes","int",

                              "stopdelim",
                              "Character(s) that separate words in stopwords list", "no","string",

                              "stopwords",
                              "List of stop words to replace default list: +word means stop any word with same stem; *word means stop exactly word","no","text",

                              "addstopwords",
                              "List of stop words to add to default list: +word means stop any word with same stem; *word means stop exactly word", "no","text",

                              "delstopwords",
                              "List of stop words to remove from default list: +word means stop any word with same stem; *word means stop exactly word", "no","text"
                               );

  }                                              
  else if(strcmp(method_name,"ts.DictDelete")      == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to delete","yes","string");
  }                                              
  else if(strcmp(method_name,"ts.DictHeartBeat")      == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to check","yes","string");
  }
  else if(strcmp(method_name,"ts.DictList")        == 0) {
    ret = xmlrpc_build_value(env, "((ss))",
    "array",
    "null");
  }                                              
  else if(strcmp(method_name,"ts.DictGetMeta")     == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to get meta information for","yes","string");
  }
  else if(strcmp(method_name,"ts.ReverseTokenize")     == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss),s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to run reverse query on","yes","string",
    "query",
    "List of ids to get terms for","yes","string");
  }
  else if(strcmp(method_name,"ts.DictGetStopList") == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to get stop list for","yes","string");
  }
  else if(strcmp(method_name,"ts.DictGetStats")    == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to get cache stats for","yes","string");
  }
  else if(strcmp(method_name,"ts.Tokenize")     == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss),s:(sss),s:(sss),s:(sss),s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the dictionary to use","yes","string",

    "toTok",
    "String to be tokenized","yes","text",

    "tokType",
    "Requested output format: freq/idlist/quartfreq","yes","string",

    "returnTokens",
    "Return tokens with output (1=yes,0=no)","yes","int",

    "update",
    "Update dictionary?(1=yes,0=no)","yes","int");
  }
  else if(strcmp(method_name,"system.listMethods")     == 0) {
    ret = xmlrpc_build_value(env, "((ss))",
    "array",
    "null");
  }
  else if(strcmp(method_name,"system.methodHelp")     == 0) {
    ret = xmlrpc_build_value(env, "((s(sss{s:(sss)})))",
    "struct",
    "Toplevel parameter","yes","struct",
    "name",
    "Name of the method to get help for","yes","string");
  }
  else {
    sprintf(r_string,"undef");
    ret = xmlrpc_build_value(env, "s",r_string);
  }
  return ret;

ERROR_EXIT:
  return NULL;
/* }}} */
}
