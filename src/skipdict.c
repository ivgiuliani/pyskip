#include <Python.h>

#include "skipdict.h"

/* Returns a pointer to the first item in the SkipDict */
inline skipitem *
SkipDict_first(SkipDict *self) {
  return self->header->next[0];
}

/* Checks if the list is empty or not */
static inline int
SkipDict_is_empty(SkipDict *self) {
  return SkipDict_first(self) == NULL;
}

/* Move to the next item */
static inline skipitem *
skipitem_get_next(skipitem *item) {
  return item->next[0];
}

int
SkipDict_init(SkipDict *self, PyObject *args, PyObject *kwargs) {
  srand(time(NULL));
  int i;

  self->level = 1;
  self->items_used = 0;
  self->type = NULL;

  /* We need to allocate just enough memory to hold pointers to real
   * items for the various levels so we can start our search from there.
   * There's no need to assign a key/value here since we'll never
   * consider those values
   */
  self->header = PyMem_New(skipitem, 1);
  if (self->header == NULL)
    return -1;

  self->header->next = PyMem_Malloc(MAX_LEVELS * sizeof(skipitem *));
  if (self->header->next == NULL)
    return -1;

  for (i = 0; i < MAX_LEVELS; i++)
    self->header->next[i] = NULL;

  return 0;
}

void
SkipDict_dealloc(SkipDict *self) {
  SkipDict_clear(self);

  PyMem_Free(self->header->next);
  PyMem_Free(self->header);
}

/* Deletes the item with the key `key` from the skiplist */
PyObject *
SkipDict_del(SkipDict *self, PyObject *key) {
  skipitem *item = self->header;
  skipitem *update[MAX_LEVELS];
  int i;

  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i, key);
    update[i] = item;
  }

  item = skipitem_get_next(item);

  /* delete the item only if the key already exists, otherwise
   * raise a KeyError
   */
  if (item && PyObject_RichCompareBool(item->key, key, Py_EQ)) {
    for (i = 0; i < self->level; i++) {
      if (update[i]->next[i] != item) break;
      update[i]->next[i] = item->next[i];
    }

    Py_DECREF(item->key);
    skipitem_free(item);

    while (self->level > 1 && self->header->next[self->level - 1] == NULL) {
      self->level--;
    }
    self->items_used--;

    /* if we deleted the last item reset the type uniformity */
    if (self->items_used == 0) {
      self->type = NULL;
    }

    Py_RETURN_NONE;
  }

  PyErr_SetObject(PyExc_KeyError, key);
  return NULL;
}

/* Set the key `key` to value `value` */
PyObject *
SkipDict_set(SkipDict *self, PyObject *args) {
  PyObject *key, *value;
  skipitem *item = self->header;
  skipitem *update[MAX_LEVELS];
  int i, level;

  if (!PyArg_ParseTuple(args, "OO", &key, &value)) {
    PyErr_SetString(PyExc_ValueError, "invalid value");
    return NULL;
  }

  /* If there's no type set (first key to be inserted), save it */
  if (!self->type) self->type = key->ob_type;

  /* Verify that the dict contains only one type (or that the new key is
   * a subtype of self->type
   */
  if ((key->ob_type != self->type) && !PyType_IsSubtype(key->ob_type, self->type)) {
    PyErr_SetString(PyExc_TypeError, "skipdict keys can't be of different types");
    return NULL;
  }

  level = generate_random_level();

  if (SkipDict_is_empty(self)) {
    for (i = 0; i < self->level; i++) update[i] = self->header;
  } else {
    for (i = self->level - 1; i >= 0; i--) {
      SKIP_FIND_PREV(item, i, key);
  
      /* Keep a list of the rightmost pointers to the various levels
       * before the key we're searching for (we need this list to
       * link the newly inserted item)
       */
      update[i] = item;
    }
  
    item = skipitem_get_next(item);

    /* the key already exists, just update its value */
    if (item && PyObject_RichCompareBool(item->key, key, Py_EQ)) {
      Py_DECREF(item->value);
      item->value = value;
      Py_RETURN_NONE;
    }
  }
  
  /* if the new item's level is higher than the current list level
   * just update the level by one
   */
  if (level > self->level) {
    update[self->level] = self->header;
    self->level++;
    level = self->level;
  }

  /* create the item and update the pointers */
  item = skipitem_new(key, value, level);
  for (i = 0; i < level; i++) {
    item->next[i] = update[i]->next[i];
    update[i]->next[i] = item;
  }
  /* increment items count */
  self->items_used++;

  Py_RETURN_NONE;
}

int
SkipDict_setItem(SkipDict *self, PyObject *key, PyObject *value) {
  if (!value) {
    if (SkipDict_del(self, key) != Py_None) return -1;
  } else {
    Py_INCREF(key);
    Py_INCREF(value);
    if (!SkipDict_set(self, Py_BuildValue("OO", key, value))) return -1;
  }
  return 0;
}

PyObject *
SkipDict_clear(SkipDict *self) {
  skipitem *item = SkipDict_first(self);
  skipitem *next;

  while (item) {
    Py_DECREF(item->key);
    Py_DECREF(item->value);

    next = skipitem_get_next(item);
    skipitem_free(item);
    item = next;
  }

  /* reinit the dict */
  SkipDict_init(self, NULL, NULL);

  Py_RETURN_NONE;
}

/* Returns the value of the key `key` or raise a KeyError if it doesn't exist */
PyObject *
SkipDict_get(SkipDict *self, PyObject *key) {
  skipitem *item = self->header;
  int i;

  /* Even though this may seem O(n^2) it's O(n) in the worst case
   * and O(logn) in the average case
   */
  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i, key);
  }

  item = skipitem_get_next(item);

  if (item && PyObject_RichCompareBool(item->key, key, Py_EQ)) {
    Py_INCREF(item->value);
    return item->value;
  }

  /* the key we're looking for is not in the skipdict */
  PyErr_SetObject(PyExc_KeyError, key);
  return NULL;
}

PyObject *
SkipDict_pop(SkipDict *self, PyObject *args) {
  PyObject *key, *ret;

  if (!PyArg_ParseTuple(args, "O", &key)) {
    PyErr_SetString(PyExc_SyntaxError, "Parameters missing");
    return NULL;
  }

  ret = Py_BuildValue("O", SkipDict_get(self, key));
  SkipDict_del(self, key);

  return ret;
}

PyObject *
SkipDict_keys(SkipDict *self, PyObject *args) {
  /* Returns a tuple with all the skipdict's keys */
  PyObject *list = PyList_New(self->items_used);
  Py_ssize_t i;
  skipitem *item = SkipDict_first(self);

  for (i = 0; i < self->items_used; i++) {
    Py_INCREF(item->key);
    PyList_SetItem(list, i, item->key);
    item = skipitem_get_next(item);
  }

  return list;
}

PyObject *
SkipDict_length(SkipDict *self) {
  return Py_BuildValue("i", self->items_used);
}

PyObject *
SkipDict_has_key(SkipDict *self, PyObject *key) {
  int i;
  skipitem *item = self->header;
  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i, key);
  }
  item = skipitem_get_next(item);
  if (item && PyObject_RichCompareBool(item->key, key, Py_EQ)) {
    return Py_True;
  } else return Py_False;
}

Py_ssize_t
SkipDict_length_map(SkipDict *self) {
  return (Py_ssize_t)self->items_used;
}

PyObject *
SkipDict_repr(SkipDict *self) {
  return PyString_FromFormat("<SkipDict: %zd items/%d levels>",
                             self->items_used, self->level);
}

/* Allocates a new item of level `level`, returns a pointer
 * to the newly allocated memory area
 */
skipitem *
skipitem_new(PyObject *key, PyObject *value, int level) {
  int i;
  skipitem *item = PyMem_New(skipitem, 1);
  if (item == NULL)
    return NULL;

  Py_INCREF(value);
  Py_INCREF(key);

  item->key = key;
  item->value = value;

  item->next = PyMem_Malloc((level + 1) * sizeof(skipitem *));
  if (item->next == NULL)
    return NULL;
  for (i = 0; i < level; i++)
    item->next[i] = NULL;

  return item;
}

/* Deallocates the item pointed by `item`. Note that items pointed
 * by `next` are not deallocated
 */
void
skipitem_free(skipitem *item) {
  PyMem_Free(item->next);
  PyMem_Free(item);
}

/* Generate a new random level */
int
generate_random_level() {
  int level = 1;
  while (((double)rand() / (RAND_MAX + 1.0) < PROB) && (level < MAX_LEVELS))
    level++;
  return level;
}

