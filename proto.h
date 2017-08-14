 /*
 * proto.h
 *
 * This file conatins function prototypes
 *
 *
 */

/* main.c */
void InitializeServer();

/* daemon.c */
void Daemonize();

/* error.c */
void errprintf(const char *fmt,...);

/* server.c */
int pthread_sleep (int seconds);
int SingleThreadedServer();
int MultiThreadedServer();
int MultiThreadedServer2();

/* introspect.c */
xmlrpc_value *system_ListMethods(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *system_MethodSignature(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *system_methodHelp(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *callInfo);
xmlrpc_value *hirundo_MethodSignature(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);

/* createsrv.c */
xmlrpc_value *Create(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo);

/* destroy.c */
xmlrpc_value *Destroy(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo);

/* dict.c */
/* exported methods */
xmlrpc_value *dict_heartbeat    (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_List         (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_Create       (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_Delete       (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_SetStop      (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_SetStem      (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_GetStats     (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_Get_Meta     (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_Get_Stoplist (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo); 
xmlrpc_value *dict_pause_backup (xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *dict_resume_backup(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);

/* internal methods */
int  verify_dict();
void restore_dict();
int  verify_journal();
void backup_journal();
void restore_journal();
void *backup_dict_immediate();
void *dict_backup(void *void_env);
int  replay_journal(dict_t *dict, xmlrpc_env *env);
void dict_loadall(xmlrpc_env *env);
void dict_ClearJournals(xmlrpc_env *env, dict_t *dict);

/* ping.c */
xmlrpc_value *Ping(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo);

/* getservers.c */
xmlrpc_value *GetServers(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo);

/* getserverinfo.c */
xmlrpc_value *GetServerInfo(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo);



/* utils.c */
char *rand_str(int len);
int copy_file(char *src, char *dest);
int parse_xmlrpc_array_i(xmlrpc_env *const env, 
                         xmlrpc_value *const xv_array,
                         idx_t *r_size, idx_t **r_array);
int parse_xmlrpc_array_d(xmlrpc_env *const env, 
                         xmlrpc_value *const xv_array,
                         idx_t *r_size, wgt_t **r_array);
xmlrpc_value *create_xmlrpc_array_i(xmlrpc_env *const env, idx_t size, idx_t *array);
xmlrpc_value *create_xmlrpc_array_s(xmlrpc_env *const env, idx_t size, char **array);
xmlrpc_value *create_xmlrpc_array_i64list(xmlrpc_env *const env, list_t *list);
xmlrpc_value *create_xmlrpc_array_stringlist(xmlrpc_env *const env, list_t *list);
xmlrpc_value *create_xmlrpc_array_hash_t(xmlrpc_env *const env, hash_t *hash);
void create_xmlrpc_array_lkiv(xmlrpc_env *const env, int size, gk_lkiv_t *v, xmlrpc_value **r_key, xmlrpc_value **r_val);
xmlrpc_value *create_xmlrpc_array_d(xmlrpc_env *const env, idx_t size, wgt_t *array);
xmlrpc_value *create_xmlrpc_array2_i(xmlrpc_env *const env, idx_t nrows, idx_t *sizes, idx_t **array);
xmlrpc_value *create_xmlrpc_array2_d(xmlrpc_env *const env, idx_t nrows, idx_t *sizes, wgt_t **array);
xmlrpc_double get_params_value_double(xmlrpc_env *const env, xmlrpc_value *const params, char *key);





/* cmdline.c */
void parse_cmdline(params_t *params, int argc, char *argv[]);


/* config.c */
void config_ReadConfigFile(params_t *params);
void config_PreProcessConfigFile(params_t *params);
char *config_GetValue(char *filename, char *optname);

/******************************************************************************* 
 * Prototypes
*******************************************************************************/ 

/* stemmer.c */
struct stemmer;
extern struct stemmer * create_stemmer(void);
extern void free_stemmer(struct stemmer * z);
extern int stem(struct stemmer * z, char * b, int k);

/* tokenize.c */
xmlrpc_value *Tokenize(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *TokenizeSegments(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
xmlrpc_value *ReverseTokenize(xmlrpc_env *const env, xmlrpc_value *const params, void *const srvinfo, void *const callInfo);
void update_dict(dict_t *dict, xmlrpc_env *const env, hash_t *newwords, hash_t **r_transdict);

/* stoplist.c */
hash_t *stoplist_default();
hash_t *stoplist_create(char *stoplist, char *stopdelim, char *add_stop, char *del_stop);
int stopWord(struct stemmer *z, dict_t *dict, char *query);

/* test.c */
void corrupt_journal();
void corrupt_dict();

