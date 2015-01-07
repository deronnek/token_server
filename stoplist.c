#include "tokserver.h"

/* TC_res is an unsigned int that will be set to the hashed value */
#define SWHASH(TC_res, TC_kbuf, TC_ksiz) \
  do { \
    const unsigned char *_TC_p = (const unsigned char *)(TC_kbuf); \
    int _TC_ksiz = TC_ksiz; \
    for((TC_res) = 19780211; _TC_ksiz--;){ \
      (TC_res) = (TC_res) * 37 + *(_TC_p)++; \
    } \
  } while(0)

#define NSTOPWORDS 390
#define STOPWORDARRAYSIZE 1600
hash_t *stoplist_default() 
{
char stoplist[][100] = { /* {{{ */
    "",
    "a",
    "about",
    "above",
    "according",
    "across",
    "actually",
    "adj",
    "after",
    "afterwards",
    "again",
    "against",
    "all",
    "almost",
    "alone",
    "along",
    "already",
    "also",
    "although",
    "always",
    "am",
    "among",
    "amongst",
    "an",
    "and",
    "another",
    "any",
    "anyhow",
    "anyone",
    "anything",
    "anywhere",
    "are",
    "aren't",
    "around",
    "as",
    "at",
    "b",
    "be",
    "became",
    "because",
    "become",
    "becomes",
    "becoming",
    "been",
    "before",
    "beforehand",
    "begin",
    "beginning",
    "behind",
    "being",
    "below",
    "beside",
    "besides",
    "between",
    "beyond",
    "billion",
    "both",
    "but",
    "by",
    "c",
    "can",
    "can't",
    "cannot",
    "caption",
    "co",
    "co.",
    "could",
    "couldn't",
    "d",
    "did",
    "didn't",
    "do",
    "does",
    "doesn't",
    "don't",
    "down",
    "during",
    "e",
    "each",
    "eg",
    "eight",
    "eighty",
    "either",
    "else",
    "elsewhere",
    "end",
    "ending",
    "enough",
    "etc",
    "even",
    "ever",
    "every",
    "everyone",
    "everything",
    "everywhere",
    "except",
    "f",
    "few",
    "fifty",
    "first",
    "five",
    "for",
    "former",
    "formerly",
    "forty",
    "found", 
    "four",
    "from",
    "further",
    "g",
    "h",
    "had",
    "has",
    "hasn't",
    "have",
    "haven't",
    "he",
    "he'd",
    "he'll",
    "he's",
    "hence",
    "her",
    "here",
    "here's",
    "hereafter",
    "hereby",
    "herein",
    "hereupon",
    "hers",
    "herself",
    "him",
    "himself",
    "his",
    "how",
    "however",
    "hundred",
    "i",
    "i'd",
    "i'll",
    "i'm",
    "i've",
    "ie",
    "if",
    "in",
    "inc.",
    "indeed",
    "instead",
    "into",
    "is",
    "isn't",
    "it",
    "it's",
    "its",
    "itself",
    "j",
    "k",
    "l",
    "last",
    "later",
    "latter",
    "latterly",
    "least",
    "less",
    "let",
    "let's",
    "like",
    "likely",
    "ltd",
    "m",
    "made",
    "make",
    "makes",
    "many",
    "maybe",
    "me",
    "meantime",
    "meanwhile",
    "might",
    "million",
    "miss",
    "more",
    "moreover",
    "most",
    "mostly",
    "mr",
    "mrs",
    "much",
    "must",
    "my",
    "myself",
    "n",
    "namely",
    "neither",
    "never",
    "nevertheless",
    "next",
    "nine",
    "ninety",
    "no",
    "nobody",
    "none",
    "nonetheless",
    "noone",
    "nor",
    "not",
    "nothing",
    "now",
    "nowhere",
    "o",
    "of",
    "off",
    "often",
    "on",
    "once",
    "one",
    "one's",
    "only",
    "onto",
    "or",
    "other",
    "others",
    "otherwise",
    "our",
    "ours",
    "ourselves",
    "out",
    "over",
    "overall",
    "own",
    "p",
    "per",
    "perhaps",
    "q",
    "r",
    "rather",
    "recent",
    "recently",
    "s",
    "same",
    "seem",
    "seemed",
    "seeming",
    "seems",
    "seven",
    "seventy",
    "several",
    "she",
    "she'd",
    "she'll",
    "she's",
    "should",
    "shouldn't",
    "since",
    "six",
    "sixty",
    "so",
    "some",
    "somehow",
    "someone",
    "something",
    "sometime",
    "sometimes",
    "somewhere",
    "still",
    "stop",
    "such",
    "t",
    "taking",
    "ten",
    "than",
    "that",
    "that'll",
    "that's",
    "that've",
    "the",
    "their",
    "them",
    "themselves",
    "then",
    "thence",
    "there",
    "there'd",
    "there'll",
    "there're",
    "there's",
    "there've",
    "thereafter",
    "thereby",
    "therefore",
    "therein",
    "thereupon",
    "these",
    "they",
    "they'd",
    "they'll",
    "they're",
    "they've",
    "thirty",
    "this",
    "those",
    "though",
    "thousand",
    "three",
    "through",
    "throughout",
    "thru",
    "thus",
    "to",
    "together",
    "too",
    "toward",
    "towards",
    "trillion",
    "twenty",
    "two",
    "u",
    "under",
    "unless",
    "unlike",
    "unlikely",
    "until",
    "up",
    "upon",
    "us",
    "used",
    "using",
    "v",
    "very",
    "via",
    "w",
    "was",
    "wasn't",
    "we",
    "we'd",
    "we'll",
    "we're",
    "we've",
    "well",
    "were",
    "weren't",
    "what",
    "what'll",
    "what's",
    "what've",
    "whatever",
    "when",
    "whence",
    "whenever",
    "where",
    "where's",
    "whereafter",
    "whereas",
    "whereby",
    "wherein",
    "whereupon",
    "wherever",
    "whether",
    "which",
    "while",
    "whither",
    "who",
    "who'd",
    "who'll",
    "who's",
    "whoever",
    "whole",
    "whom",
    "whomever",
    "whose",
    "why",
    "will",
    "with",
    "within",
    "without",
    "won't",
    "would",
    "wouldn't",
    "x",
    "y",
    "yes",
    "yet",
    "you",
    "you'd",
    "you'll",
    "you're",
    "you've",
    "your",
    "yours",
    "yourself",
    "yourselves",
    "z"
}; /* }}} */
  int i;
  char *t = "1\0";
  hash_t *stoplist_hash = hash_Create();
  for(i=0; i<NSTOPWORDS; i++) {
    hash_Put(stoplist_hash, &stoplist[i], -1, &t, -1);
  }
  return stoplist_hash;
}

#if 0
hash_t *stoplist_create(char *stoplist, char *stopdelim, char *add_stop, char *del_stop) 
{ 
  /* {{{ */
  char *token;
  char *t = "1\0";
  int free_stopdelim = 0;
  hash_t *stoplist_hash = NULL;

  if((stoplist != NULL || add_stop != NULL || del_stop != NULL) && (stopdelim == NULL || !strcmp(stopdelim,""))) {
printf("Set stopdelim to space\n");
    stopdelim = gk_strdup(" ");
    free_stopdelim = 1;
  }
printf("Stopdelim: --%s--\n",stopdelim);

  /* stoplist string  == "" */
  if(stoplist != NULL && !strcmp(stoplist,"")) {
    stoplist_hash = hash_Copy(ServerState.default_stoplist);

    if(add_stop != NULL && strcmp(add_stop,"") && stopdelim != NULL) {
      token = strtok_r(add_stop,stopdelim);
      while(token != NULL) {
        hash_Put(stoplist_hash, token, -1, &t, -1);
        token = strtok_r(NULL, stopdelim);
      }
    }

    if(del_stop != NULL && strcmp(del_stop,"") && stopdelim != NULL) {
      token = strtok_r(del_stop,stopdelim);
      while(token != NULL) {
        hash_Del(stoplist_hash, token, -1);
        token = strtok_r(NULL, stopdelim);
      }
    }
  }
  /* nonempty stoplist string -- replace default */
  else if(stoplist != NULL && strcmp(stoplist,"") && stopdelim != NULL){
    stoplist_hash = hash_Create();
    token = strtok_r(stoplist,stopdelim);
    while(token != NULL) {
      hash_Put(stoplist_hash, token, -1, &t, -1);
      token = strtok_r(NULL, stopdelim);
    }
  }
  /* empty stoplist string */
  else {
    stoplist_hash = hash_Copy(ServerState.default_stoplist);
  }

  if(free_stopdelim) gk_free((void **)&stopdelim, LTERM);

  return stoplist_hash;
  /* }}} */
}
#endif

hash_t *stoplist_create(char *stoplist, char *stopdelim_ext, char *add_stop, char *del_stop) 
{ 
  /* {{{ */
  char *token;
  char *t = "1\0";   /* 'true' value stored as value for all stop words (I just need keys) */
  int stemsize;
  char *stopdelim       = NULL;

  struct stemmer * z    = create_stemmer();
  hash_t *stoplist_hash = hash_Create();

  hash_t *plain         = NULL;
  hash_t *precise       = hash_Create();
  hash_t *stemmed       = hash_Create();
                        
  char *toknodelim      = NULL;
  char *stemtok         = NULL;
  char *saveptr1;       
                        
  char *plainkey        = "plain";
  char *precisekey      = "precise";
  char *stemmedkey      = "stemmed";

  /* Always use at least a space as a stopword delimiter */
  if((stoplist != NULL || add_stop != NULL || del_stop != NULL) && (stopdelim == NULL || !strcmp(stopdelim,""))) {
//printf("Set stopdelim to space\n");
    stopdelim = gk_strdup(" ");
  }
  else {
    asprintf(&stopdelim, "%s ", stopdelim_ext);
  }
//printf("Stopdelim: --%s--\n",stopdelim);

  /* stoplist string  == "" */
  if(stoplist != NULL && !strcmp(stoplist,"")) {
    plain   = hash_Copy(ServerState.default_stoplist);

    if(add_stop != NULL && strcmp(add_stop,"") && stopdelim != NULL) {
      token = strtok_r(add_stop,stopdelim,&saveptr1);
      while(token != NULL) {
        if(strlen(token) > 1 && token[0] == '+') {
          stemtok  = gk_strdup(token+1);
          stemsize = stem(z, stemtok, strlen(stemtok)-1)+1;
          stemtok[stemsize] = 0;
          hash_Put(stemmed, stemtok, -1, &t, -1);
          gk_free((void **)&stemtok, LTERM);
        }
        else if(strlen(token) > 1 && token[0] == '*') {
          toknodelim = gk_strdup(token+1);
          hash_Put(precise, toknodelim, -1, &t, -1);
          gk_free((void **)&toknodelim, LTERM);
        }
        else {
          hash_Put(plain, token, -1, &t, -1);
        }
        token = strtok_r(NULL, stopdelim,&saveptr1);
      }
    }

    if(del_stop != NULL && strcmp(del_stop,"") && stopdelim != NULL) {
      token = strtok_r(del_stop,stopdelim,&saveptr1);
      while(token != NULL) {
        if(strlen(token) > 1 && token[0] == '+') {
          stemtok  = gk_strdup(token+1);
          stemsize = stem(z, stemtok, strlen(stemtok)-1)+1;
          stemtok[stemsize] = 0;
          hash_Del(stemmed, stemtok, -1);
          gk_free((void **)&stemtok, LTERM);
        }
        else if(strlen(token) > 1 && token[0] == '*') {
          toknodelim = gk_strdup(token+1);
          hash_Del(precise, toknodelim, -1);
          gk_free((void **)&toknodelim, LTERM);
        }
        else {
          hash_Del(plain, token, -1);
        }
        token = strtok_r(NULL, stopdelim,&saveptr1);
      }
    }
  }
  /* nonempty stoplist string -- replace default */
  else if(stoplist != NULL && strcmp(stoplist,"") && stopdelim != NULL){
    plain   = hash_Create();
    token = strtok_r(stoplist,stopdelim,&saveptr1);
    while(token != NULL) {
      if(token[0] == '+') {
        stemtok  = gk_strdup(token+1);
        stemsize = stem(z, stemtok, strlen(stemtok)-1)+1;
        stemtok[stemsize] = 0;
        hash_Put(stemmed, stemtok, -1, &t, -1);
        gk_free((void **)&stemtok, LTERM);
      }
      else if(token[0] == '*') {
        toknodelim = gk_strdup(token+1);
        hash_Put(precise, toknodelim, -1, &t, -1);
        gk_free((void **)&toknodelim, LTERM);
      }
      else {
        hash_Put(plain, token, -1, &t, -1);
      }
      token = strtok_r(NULL, stopdelim,&saveptr1);
    }
  }
  /* empty stoplist string */
  else {
    plain   = hash_Copy(ServerState.default_stoplist);
  }

  gk_free((void **)&stopdelim, LTERM);

  /* put the three stoplist hash pointers into the return hash */
  hash_Put(stoplist_hash, (void *)plainkey,   -1,   &plain, sizeof(hash_t *));
  hash_Put(stoplist_hash, (void *)precisekey, -1, &precise, sizeof(hash_t *));
  hash_Put(stoplist_hash, (void *)stemmedkey, -1, &stemmed, sizeof(hash_t *));

  gk_free((void **)&stoplist, &add_stop, &del_stop, LTERM);
  return stoplist_hash;
  /* }}} */
}

int stopWord(struct stemmer *z, dict_t *dict, char *query) 
{
  /* {{{ */
  int rc = 0;
  hash_t *plain    = NULL;
  hash_t *precise  = NULL;
  hash_t *stemmed  = NULL;
  void *p;
  int stemsize;     
  char *querystem  = gk_strdup(query);
  char *plainkey   = "plain";
  char *precisekey = "precise";
  char *stemmedkey = "stemmed";

  /* the funciton stem modifies the query string, so we do it on a copy
   * first, and again on the actual query */
  //LOGMSG1("before stemming: %s\n",querystem);
  stemsize            = stem(z, querystem, strlen(querystem)-1)+1;
  //LOGMSG1("after stemming: %s\n",querystem);
  querystem[stemsize] = 0;



  /* The three stoplists are going to be called plain, precise and stem */
  p = (hash_t *)hash_Get(dict->stophash, (void *)plainkey, -1, NULL);
  memcpy(&plain, p, sizeof(hash_t *));

  p = (hash_t *)hash_Get(dict->stophash, (void *)precisekey, -1, NULL);
  memcpy(&precise, p, sizeof(hash_t *));

  p = (hash_t *)hash_Get(dict->stophash, (void *)stemmedkey, -1, NULL);
  memcpy(&stemmed, p, sizeof(hash_t *));

  /* Before we stem, check precise (*) words */
  if(hash_Exists(precise, query, -1)) {
    rc = 1;
  }

  /* Check the stem of the query against the stemmed stop words */
  if(hash_Exists(stemmed, querystem, -1)) {
    rc = 1;
  }

  /* "before and after" (stemming) stopping */
  if(hash_Exists(plain, query, -1)) {
    rc = 1;
  }
  /* if rc == 1 we're ignoring query anyway.  So if we 
   * reach this point and stemming is enabled, the word
   * will always be stemmed before it gets stored. */
  if(rc != 1) {
    if(dict->dostem == 1) {
      stemsize        = stem(z, query, strlen(query)-1)+1;
      query[stemsize] = 0;
    }
    if(hash_Exists(plain, query, -1)) {
      rc = 1;
    }
  }
  gk_free((void **)&querystem, LTERM);
  return rc;
  /* }}} */
}

#if 0
int stopWord(char **stoplist, char *query) {
  int len = strlen(query);
  unsigned int hashval;
  int slot;
  SWHASH(hashval, query, len);
  slot = hashval % NSTOPWORDS;
  if(stoplist[slot] != NULL && !strcmp(stoplist[slot],query)) {
    return 1;
  }
  slot  = hashval % 998;
  slot += NSTOPWORDS;
  if(stoplist[slot] != NULL && !strcmp(stoplist[slot],query)) {
    return 1;
  }
  slot++;
  if(stoplist[slot] != NULL && !strcmp(stoplist[slot],query)) {
    return 1;
  }
  return 0;
}
#endif

#if 0
char **hashStopList() 
{ /* {{{ */ 
char stoplist[][100] = { /* {{{ */
    "",
    "a",
    "about",
    "above",
    "according",
    "across",
    "actually",
    "adj",
    "after",
    "afterwards",
    "again",
    "against",
    "all",
    "almost",
    "alone",
    "along",
    "already",
    "also",
    "although",
    "always",
    "am",
    "among",
    "amongst",
    "an",
    "and",
    "another",
    "any",
    "anyhow",
    "anyone",
    "anything",
    "anywhere",
    "are",
    "aren't",
    "around",
    "as",
    "at",
    "b",
    "be",
    "became",
    "because",
    "become",
    "becomes",
    "becoming",
    "been",
    "before",
    "beforehand",
    "begin",
    "beginning",
    "behind",
    "being",
    "below",
    "beside",
    "besides",
    "between",
    "beyond",
    "billion",
    "both",
    "but",
    "by",
    "c",
    "can",
    "can't",
    "cannot",
    "caption",
    "co",
    "co.",
    "could",
    "couldn't",
    "d",
    "did",
    "didn't",
    "do",
    "does",
    "doesn't",
    "don't",
    "down",
    "during",
    "e",
    "each",
    "eg",
    "eight",
    "eighty",
    "either",
    "else",
    "elsewhere",
    "end",
    "ending",
    "enough",
    "etc",
    "even",
    "ever",
    "every",
    "everyone",
    "everything",
    "everywhere",
    "except",
    "f",
    "few",
    "fifty",
    "first",
    "five",
    "for",
    "former",
    "formerly",
    "forty",
    "found", 
    "four",
    "from",
    "further",
    "g",
    "h",
    "had",
    "has",
    "hasn't",
    "have",
    "haven't",
    "he",
    "he'd",
    "he'll",
    "he's",
    "hence",
    "her",
    "here",
    "here's",
    "hereafter",
    "hereby",
    "herein",
    "hereupon",
    "hers",
    "herself",
    "him",
    "himself",
    "his",
    "how",
    "however",
    "hundred",
    "i",
    "i'd",
    "i'll",
    "i'm",
    "i've",
    "ie",
    "if",
    "in",
    "inc.",
    "indeed",
    "instead",
    "into",
    "is",
    "isn't",
    "it",
    "it's",
    "its",
    "itself",
    "j",
    "k",
    "l",
    "last",
    "later",
    "latter",
    "latterly",
    "least",
    "less",
    "let",
    "let's",
    "like",
    "likely",
    "ltd",
    "m",
    "made",
    "make",
    "makes",
    "many",
    "maybe",
    "me",
    "meantime",
    "meanwhile",
    "might",
    "million",
    "miss",
    "more",
    "moreover",
    "most",
    "mostly",
    "mr",
    "mrs",
    "much",
    "must",
    "my",
    "myself",
    "n",
    "namely",
    "neither",
    "never",
    "nevertheless",
    "next",
    "nine",
    "ninety",
    "no",
    "nobody",
    "none",
    "nonetheless",
    "noone",
    "nor",
    "not",
    "nothing",
    "now",
    "nowhere",
    "o",
    "of",
    "off",
    "often",
    "on",
    "once",
    "one",
    "one's",
    "only",
    "onto",
    "or",
    "other",
    "others",
    "otherwise",
    "our",
    "ours",
    "ourselves",
    "out",
    "over",
    "overall",
    "own",
    "p",
    "per",
    "perhaps",
    "q",
    "r",
    "rather",
    "recent",
    "recently",
    "s",
    "same",
    "seem",
    "seemed",
    "seeming",
    "seems",
    "seven",
    "seventy",
    "several",
    "she",
    "she'd",
    "she'll",
    "she's",
    "should",
    "shouldn't",
    "since",
    "six",
    "sixty",
    "so",
    "some",
    "somehow",
    "someone",
    "something",
    "sometime",
    "sometimes",
    "somewhere",
    "still",
    "stop",
    "such",
    "t",
    "taking",
    "ten",
    "than",
    "that",
    "that'll",
    "that's",
    "that've",
    "the",
    "their",
    "them",
    "themselves",
    "then",
    "thence",
    "there",
    "there'd",
    "there'll",
    "there're",
    "there's",
    "there've",
    "thereafter",
    "thereby",
    "therefore",
    "therein",
    "thereupon",
    "these",
    "they",
    "they'd",
    "they'll",
    "they're",
    "they've",
    "thirty",
    "this",
    "those",
    "though",
    "thousand",
    "three",
    "through",
    "throughout",
    "thru",
    "thus",
    "to",
    "together",
    "too",
    "toward",
    "towards",
    "trillion",
    "twenty",
    "two",
    "u",
    "under",
    "unless",
    "unlike",
    "unlikely",
    "until",
    "up",
    "upon",
    "us",
    "used",
    "using",
    "v",
    "very",
    "via",
    "w",
    "was",
    "wasn't",
    "we",
    "we'd",
    "we'll",
    "we're",
    "we've",
    "well",
    "were",
    "weren't",
    "what",
    "what'll",
    "what's",
    "what've",
    "whatever",
    "when",
    "whence",
    "whenever",
    "where",
    "where's",
    "whereafter",
    "whereas",
    "whereby",
    "wherein",
    "whereupon",
    "wherever",
    "whether",
    "which",
    "while",
    "whither",
    "who",
    "who'd",
    "who'll",
    "who's",
    "whoever",
    "whole",
    "whom",
    "whomever",
    "whose",
    "why",
    "will",
    "with",
    "within",
    "without",
    "won't",
    "would",
    "wouldn't",
    "x",
    "y",
    "yes",
    "yet",
    "you",
    "you'd",
    "you'll",
    "you're",
    "you've",
    "your",
    "yours",
    "yourself",
    "yourselves",
    "z"
}; /* }}} */
  int len;
  unsigned int hashval;
  int slot;
  int i;
  int arraysize = STOPWORDARRAYSIZE;
  int success   = 0;
  char **ha = (char **)gk_malloc(arraysize*sizeof(char *),"stoplist hash");

  for(i=0; i<arraysize; i++)
    ha[i] = NULL;

  for(i=0; i<NSTOPWORDS; i++) {
    len = strlen(stoplist[i]);
    SWHASH(hashval, stoplist[i], len);
    slot = hashval % NSTOPWORDS; 
    success = 0;

    if(ha[slot] == NULL) {
      ha[slot] = strdup(stoplist[i]);
      success = 1;
    }
    else { 
      slot = hashval % 998; 
      slot += NSTOPWORDS;
      if(ha[slot] == NULL) {
        ha[slot] = strdup(stoplist[i]);
        success = 1;
      } 
      else if(ha[slot+1] == NULL) {
        ha[slot+1] = strdup(stoplist[i]);
        success = 1;
      } 
      if(success == 0) { 
        errexit("Didn't have enough slots in stoplist hash for %s\n",stoplist[i]);
      }
    }
  }

  /* debug -- we only use 1386 slots, but I'm willing to let it go */
  /* for(i=0; i<arraysize; i++) {
    if(ha[i] != NULL) {
      printf("%d :: %s\n",i,ha[i]);
    }
  } */
  
  return ha;
} /* }}} */

void destroyStopList(char **stoplist) {
  int i;
  for(i=0; i<STOPWORDARRAYSIZE; i++) {
    gk_free((void **)&stoplist[i], LTERM);
  }
  gk_free((void **)&stoplist, LTERM);
}
#endif 
