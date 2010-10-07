#ifndef SKIP_H
#define SKIP_H

#include <Python.h>

#define PROB 0.5
#define MAX_LEVELS 32

typedef struct _skipitem {
  PyObject *key;
  PyObject *value;
  struct _skipitem **next;
} skipitem;

typedef struct {
  PyObject_HEAD
  unsigned int level;
  unsigned int items_used;
  skipitem *header;
  PyTypeObject *type;
} SkipDict;

skipitem *skipitem_new(PyObject *, PyObject *, int);
void skipitem_free(skipitem *);

int skip_random_level(void);
inline int skip_length(SkipDict *);

#endif /* SKIP_H */
