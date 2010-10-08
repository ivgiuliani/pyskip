#include <Python.h>

#include "skip.h"

/* Returns a pointer to the first item in the list */
static inline skipitem *
skip_first(SkipDict *self) {
  return self->header->next[0];
}

/* Checks if the list is empty or not */
static inline int
skip_is_empty(SkipDict *self) {
  return skip_first(self) == NULL;
}

/* Find the previous pointer to the item we're looking for
 * or where the item should be inserted
 */
#define SKIP_FIND_PREV(item, level, key) do {\
  while ((item->next[level] != NULL) && \
         (PyObject_RichCompareBool(item->next[level]->key, key, Py_LT))) { \
    item = item->next[level]; \
  } \
} while(0);

/* Returns the value of the key `key` or raise a KeyError if it doesn't exist */
static PyObject *
skip_get(SkipDict *self, PyObject *key) {
  skipitem *item = self->header;
  int i;

  /* Even though this may seem O(n^2) it's O(n) in the worst case
   * and O(logn) in the average case
   */
  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i, key);
  }

  item = item->next[0];

  if (item && PyObject_RichCompareBool(item->key, key, Py_EQ)) {
    Py_INCREF(item->value);
    return item->value;
  }

  /* the key we're looking for is not in the skipdict */
  PyErr_SetObject(PyExc_KeyError, key);
  return NULL;
}

/* Deletes the item with the key `key` from the skiplist */
static PyObject *
skip_del(SkipDict *self, PyObject *key) {
  skipitem *item = self->header;
  skipitem *update[MAX_LEVELS];
  int i;

  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i, key);
    update[i] = item;
  }

  item = item->next[0];

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
    Py_RETURN_NONE;
  }

  PyErr_SetObject(PyExc_KeyError, key);
  return NULL;
}

static int
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

static void
SkipDict_dealloc(SkipDict *self) {
  skipitem *item = self->header->next[0];
  skipitem *tmp;

  while (item != NULL) {
    tmp = item;
    item = item->next[0];

    skipitem_free(tmp);
  }

  PyMem_Free(self->header->next);
  PyMem_Free(self->header);
}

/* Set the key `key` to value `value` */
static PyObject *
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

  level = skip_random_level();

  if (skip_is_empty(self)) {
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
  
    item = item->next[0];

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

static int
SkipDict_setItem(SkipDict *self, PyObject *key, PyObject *value) {
  if (!value) {
    if (skip_del(self, key) != Py_None) return -1;
  } else {
    Py_INCREF(key);
    Py_INCREF(value);
    if (!SkipDict_set(self, Py_BuildValue("OO", key, value))) return -1;
  }
  return 0;
}

static PyObject *
SkipDict_get(SkipDict *self, PyObject *key) {
  return skip_get(self, key);;
}

static PyObject *
SkipDict_pop(SkipDict *self, PyObject *args) {
  PyObject *key, *ret;

  if (!PyArg_ParseTuple(args, "O", &key)) {
    PyErr_SetString(PyExc_SyntaxError, "Parameters missing");
    return NULL;
  }

  ret = Py_BuildValue("O", skip_get(self, key));
  skip_del(self, key);

  return ret;
}

static PyObject *
SkipDict_keys(SkipDict *self, PyObject *args) {
  /* Returns a tuple with all the skipdict's keys */
  PyObject *tuple = PyTuple_New(self->items_used);
  Py_ssize_t i;
  skipitem *item = skip_first(self);

  for (i = 0; i < self->items_used; i++) {
    Py_INCREF(item->key);
    PyTuple_SetItem(tuple, i, item->key);
    item = item->next[0];
  }

  return tuple;
}

static PyObject *
SkipDict_length(SkipDict *self) {
  return Py_BuildValue("i", skip_length(self));
}

static PyObject *
SkipDict_has_key(SkipDict *self, PyObject *key) {
  int i;
  skipitem *item = self->header;
  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i, key);
  }
  item = item->next[0];
  if (item && PyObject_RichCompareBool(item->key, key, Py_EQ)) {
    return Py_True;
  } else return Py_False;
}

static Py_ssize_t
SkipDict_length_map(SkipDict *self) {
  return (Py_ssize_t)skip_length(self);
}

static PyObject *
SkipDict_repr(SkipDict *self) {
  return PyString_FromFormat("<SkipDict: %zd items/%d levels>",
                             self->items_used, self->level);
}

static PyMethodDef
SkipDict_methods[] = {
  { "get", (PyCFunction)SkipDict_get, METH_O | METH_COEXIST,
           "Get key's value" },
  { "set", (PyCFunction)SkipDict_set, METH_VARARGS,
           "Set the value of a key" },
  { "pop", (PyCFunction)SkipDict_pop, METH_VARARGS,
           "Remove specified key and return the corresponding value.\n"
           "If key is not found, KeyError is raised."},
  { "keys", (PyCFunction)SkipDict_keys, METH_VARARGS,
            "Returns an iterator over the keys of the dictionary" },
  { "has_key", (PyCFunction)SkipDict_has_key, METH_O, 0 },

  /* special methods */
  { "__len__", (PyCFunction)SkipDict_length, METH_NOARGS | METH_COEXIST, 0 },
  { "__getitem__", (PyCFunction)SkipDict_get, METH_O | METH_COEXIST, 0 },
  { "__setitem__", (PyCFunction)SkipDict_set, METH_VARARGS | METH_COEXIST, 0 },
  { NULL }
};

static PyMappingMethods skipdict_as_mapping = {
  (lenfunc)SkipDict_length_map,    /* mp_length */
  (binaryfunc)SkipDict_get,        /* mp_subscript */
  (objobjargproc)SkipDict_setItem, /* mp_ass_subscript */
};

static PyTypeObject
SkipDict_Type = {
  PyObject_HEAD_INIT(NULL)
  0,                            /* ob_size */
  "SkipDict",                   /* tp_name */
  sizeof(SkipDict),             /* tp_basicsize */
  0,                            /* tp_itemsize */
  (destructor)SkipDict_dealloc, /* tp_dealloc */
  0,                            /* tp_print */
  0,                            /* tp_getattr */
  0,                            /* tp_setattr */
  0,                            /* tp_compare */
  (reprfunc)SkipDict_repr,      /* tp_repr */
  0,                            /* tp_as_number */
  0,                            /* tp_as_sequence */
  &skipdict_as_mapping,         /* tp_as_mapping */
  0,                            /* tp_hash */
  0,                            /* tp_call */
  0,                            /* tp_str */
  0,                            /* tp_getattro */
  0,                            /* tp_setattro */
  0,                            /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags*/
  "A dictionary which works over skiplists", /* tp_doc */
  0,                            /* tp_traverse */
  0,                            /* tp_clear */
  0,                            /* tp_richcompare */
  0,                            /* tp_weaklistoffset */
  0,                            /* tp_iter */
  0,                            /* tp_iternext */
  SkipDict_methods,             /* tp_methods */
  NULL,                         /* tp_members */
  0,                            /* tp_getset */
  0,                            /* tp_base */
  0,                            /* tp_dict */
  0,                            /* tp_descr_get */
  0,                            /* tp_descr_set */
  0,                            /* tp_dictoffset */
  (initproc)SkipDict_init,      /* tp_init */
  PyType_GenericAlloc,          /* tp_alloc */
  0,                            /* tp_new */
  PyObject_GC_Del,              /* tp_free */
};

void
initskip(void) {
  PyObject *module = Py_InitModule3("skip", NULL, "A dictionary with skiplists.");
  if (module == NULL) return;

  SkipDict_Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&SkipDict_Type) < 0) return;

  Py_INCREF(&SkipDict_Type);
  PyModule_AddObject(module, "SkipDict", (PyObject*)&SkipDict_Type);
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
skip_random_level() {
  int level = 1;
  while (((double)rand() / (RAND_MAX + 1.0) < PROB) && (level < MAX_LEVELS))
    level++;
  return level;
}

/* Returns skipdict's length (number of keys in the skipdict) */
inline int
skip_length(SkipDict *skip) {
  return skip->items_used;
}

