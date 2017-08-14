#include "tokserver.h"

/* Yes, actually corrupts the dictionary.  Use only for testing unless you're crazy. */
void corrupt_dict(dict_t *dict) {
  char tmp[2048];
  int64_t bad = 373737373; 
  sprintf(tmp,"corrupting");

  printf("\n\nCorrupting dictionary...\n\n");
  //hhash_t *one = hhash_Open(ServerState.dictfile,100,0);
  //hhash_t *two = hhash_Open(ServerState.dictfile,100,0);

  hhash_Put(dict->dict, (void *)&bad, 20, (void *)tmp, 20);
  //bad = 16987;
  //hhash_Put(two, (void *)&bad, 20, (void *)tmp, 20);
  //hhash_Close(one);
  //hhash_Close(two);
  //unlink(ServerState.dictfile);
}

void corrupt_journal() {
  printf("\n\nCorrupting journal...\n\n");
  fprintf(ServerState.JFILE,"badDAtea\n");
}
