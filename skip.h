#ifndef SKIP_H
#define SKIP_H

#include <Python.h>

#define PROB 0.5
#define MAX_LEVELS 32

typedef struct _skipitem {
  long key_hash;
  PyObject *value;
  struct _skipitem **next;
} skipitem;

typedef struct {
  unsigned int level;
  skipitem *header;
} skiplist;

void skip_printall(skiplist *);

skipitem *skipitem_new(long, PyObject *, int);
void skipitem_free(skipitem *);

int skip_random_level(void);

#endif /* SKIP_H */
