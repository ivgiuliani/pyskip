#include <Python.h>

#include "skip.h"

#define PYOBJ_EQ(ob1, ob2) PyObject_RichCompareBool(ob1, ob2, Py_EQ)

typedef struct {
  PyObject_HEAD
  unsigned int level;
  skipitem *header;
} SkipDict;

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
#define SKIP_FIND_PREV(item, level) do {\
  while ((item->next[level] != NULL) && \
         (PyObject_RichCompareBool(item->next[level]->key, key, Py_LT))) { \
    item = item->next[level]; \
  } \
} while(0);

/* Returns the value of the key `key` or -1 if it doesn't exist */
static PyObject *
skip_get(SkipDict *self, register PyObject *key) {
  skipitem *item = self->header;
  int i;

  if (skip_is_empty(self)) {
    Py_RETURN_NONE;
  }

  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i);
  }

  item = item->next[0];

  if (item && (PYOBJ_EQ(item->key, key))) {
    return item->value;
  } else {
    Py_RETURN_NONE;
  }
}

/* Deletes the item with the key `key` from the skiplist */
static void
skip_del(SkipDict *self, PyObject *key) {
  skipitem *item = self->header;
  skipitem *update[MAX_LEVELS];
  int i;

  if (skip_is_empty(self)) return;

  for (i = self->level - 1; i >= 0; i--) {
    SKIP_FIND_PREV(item, i);
    update[i] = item;
  }

  item = item->next[0];

  /* delete the item only if the key already exists, otherwise just ignore it */
  if (item && (PYOBJ_EQ(item->key, key))) {
    for (i = 0; i < self->level; i++) {
      if (update[i]->next[i] != item) break;
      update[i]->next[i] = item->next[i];
    }

    skipitem_free(item);

    while (self->level > 1 && self->header->next[self->level - 1] == NULL) {
      self->level--;
    }
  }
}


static int
SkipDict_init(SkipDict *self, PyObject *args, PyObject *kwargs) {
  srand(time(NULL));
  int i;

  self->level = 1;

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

/* Set a the key `key` to value `value` */
static PyObject *
SkipDict_set(SkipDict *self, PyObject *args) {
  PyObject *key, *value;
  skipitem *item = self->header;
  skipitem *update[MAX_LEVELS];
  int i, level;

  if (!PyArg_ParseTuple(args, "OO", &key, &value)) {
    PyErr_SetString(PyExc_SyntaxError, "Assignment parameters missing");
    return NULL;
  }

  level = skip_random_level();

  if (skip_is_empty(self)) {
    for (i = 0; i < self->level; i++) update[i] = self->header;
  } else {
    for (i = self->level - 1; i >= 0; i--) {
      SKIP_FIND_PREV(item, i);
  
      /* Keep a list of the rightmost pointers to the various levels
       * before the key we're searching for (we need this list to
       * link the newly inserted item)
       */
      update[i] = item;
    }
  
    item = item->next[0];

    /* the key already exists, just update its value */
    if (item && (PYOBJ_EQ(item->key, key))) {
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

  Py_RETURN_NONE;
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
  /* TODO */
  Py_RETURN_NONE;
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

  /* slice methods */
  { "__getitem__", (PyCFunction)SkipDict_get, METH_O | METH_COEXIST, 0 },
  { "__setitem__", (PyCFunction)SkipDict_set, METH_VARARGS | METH_COEXIST, 0 },
  { NULL }
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
  0,                            /* tp_repr */
  0,                            /* tp_as_number */
  0,                            /* tp_as_sequence */
  0,                            /* tp_as_mapping */
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
  0,                            /* tp_alloc */
  0,                            /* tp_new */
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

  Py_INCREF(key);
  Py_INCREF(value);

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
//  Py_DECREF(item->key);
//  Py_DECREF(item->value);

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

