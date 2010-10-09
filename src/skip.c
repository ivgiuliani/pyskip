#include <Python.h>

#include "skipdict.h"

/* Python API Type definition */
PyMappingMethods skipdict_as_mapping = {
  (lenfunc)SkipDict_length_map,    /* mp_length */
  (binaryfunc)SkipDict_get,        /* mp_subscript */
  (objobjargproc)SkipDict_setItem, /* mp_ass_subscript */
};

PyMethodDef
SkipDict_methods[] = {
  { "clear", (PyCFunction)SkipDict_clear, METH_NOARGS,
           "Clear the whole dictionary" },
  { "get", (PyCFunction)SkipDict_get, METH_O,
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

PyTypeObject
SkipDict_Type = {
  PyObject_HEAD_INIT(NULL)
  0,                               /* ob_size */
  "SkipDict",                      /* tp_name */
  sizeof(SkipDict),                /* tp_basicsize */
  0,                               /* tp_itemsize */
  (destructor)SkipDict_dealloc,    /* tp_dealloc */
  0,                               /* tp_print */
  0,                               /* tp_getattr */
  0,                               /* tp_setattr */
  0,                               /* tp_compare */
  (reprfunc)SkipDict_repr,         /* tp_repr */
  0,                               /* tp_as_number */
  0,                               /* tp_as_sequence */
  &skipdict_as_mapping,            /* tp_as_mapping */
  0,                               /* tp_hash */
  0,                               /* tp_call */
  0,                               /* tp_str */
  0,                               /* tp_getattro */
  0,                               /* tp_setattro */
  0,                               /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags*/
  "Dictionary based on skiplists", /* tp_doc */
  0,                               /* tp_traverse */
  0,                               /* tp_clear */
  0,                               /* tp_richcompare */
  0,                               /* tp_weaklistoffset */
  0,                               /* tp_iter */
  0,                               /* tp_iternext */
  SkipDict_methods,                /* tp_methods */
  NULL,                            /* tp_members */
  0,                               /* tp_getset */
  0,                               /* tp_base */
  0,                               /* tp_dict */
  0,                               /* tp_descr_get */
  0,                               /* tp_descr_set */
  0,                               /* tp_dictoffset */
  (initproc)SkipDict_init,         /* tp_init */
  PyType_GenericAlloc,             /* tp_alloc */
  PyType_GenericNew,               /* tp_new */
  PyObject_GC_Del,                 /* tp_free */
};

void
initskip(void) {
  PyObject *module = Py_InitModule3("skip", NULL, "Python dictionary based on skip lists");
  if (!module) return;

  if (PyType_Ready(&SkipDict_Type) < 0) return;

  Py_INCREF(&SkipDict_Type);
  PyModule_AddObject(module, "SkipDict", (PyObject *)&SkipDict_Type);
}

