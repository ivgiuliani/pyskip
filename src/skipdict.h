#ifndef SKIPDICT_H
#define SKIPDICT_H

#include <Python.h>

/* Keep this to the size of a cpu register, the random level
 * generation function relies on this number to be contained
 * into a single register
 */
#define SKIPDICT_MAX_LEVELS 32

typedef struct _skipitem {
  PyObject *key;
  PyObject *value;
  struct _skipitem **next;
} skipitem;

typedef struct {
  PyObject_HEAD
  unsigned int level;
  Py_ssize_t items_used;
  skipitem *header;
  PyTypeObject *type;
} SkipDict;

inline skipitem *SkipDict_first(SkipDict *);
skipitem *skipitem_new(PyObject *, PyObject *, int);
void skipitem_free(skipitem *);
unsigned short int generate_random_level(void);
int SkipDict_init(SkipDict *, PyObject *, PyObject *);
void SkipDict_dealloc(SkipDict *);
PyObject *SkipDict_repr(SkipDict *);
PyObject *SkipDict_clear(SkipDict *);
Py_ssize_t SkipDict_length_map(SkipDict *);
PyObject *SkipDict_set(SkipDict *, PyObject *);
int SkipDict_setItem(SkipDict *, PyObject *, PyObject *);
PyObject *SkipDict_get(SkipDict *, PyObject *);
PyObject *SkipDict_del(SkipDict *, PyObject *);
PyObject *SkipDict_pop(SkipDict *, PyObject *);
PyObject *SkipDict_keys(SkipDict *, PyObject *);
PyObject *SkipDict_has_key(SkipDict *, PyObject *);
PyObject *SkipDict_length(SkipDict *);

/* Find the previous pointer to the item we're looking for
 * or where the item should be inserted
 */
#define SKIP_FIND_PREV(item, level, key) do {\
  while ((item->next[level] != NULL) && \
         (PyObject_RichCompareBool(item->next[level]->key, key, Py_LT))) { \
    item = item->next[level]; \
  } \
} while(0);

#endif /* SKIPDICT_H */
