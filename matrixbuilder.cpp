#include <Python.h> //有的是#include<Python.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <vector>
#include <algorithm>
#include <iostream>
#include "matrixwriter.h"
#include "matrixreader.h"

using namespace std;

typedef struct{
    PyObject_HEAD
    MatrixWriter *matrix_writer;
}writer;
 
static PyObject *writer_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    writer *m;
    m = (writer *)type->tp_alloc(type, 0);
    m->matrix_writer = new MatrixWriter();
    return (PyObject *)m;
}

static void writer_delete(writer *self) {
    self->matrix_writer->Close();
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int writer_traverse(writer *self, visitproc visit, void *arg) {
    return 0;
}

static int writer_clear(writer *self) {
    return 0;
}

static int writer_init(writer *self, PyObject *args) {
    char *string1;
    char *string2;
    int max;
    if (!PyArg_ParseTuple(args, "ssi", &string1, &string2, &max)) {
        return -1;
    }
    self->matrix_writer->Open(string1, string2, max);
    return 0;
}

static PyObject *writer_append(writer *self, PyObject *args) {
    int id;
    int len;
    char *value;
    if (!PyArg_ParseTuple(args, "isi", &id, &value, &len)) {
        return Py_BuildValue("i", -1);
    }
    self->matrix_writer->Append(id, value, len);
    return Py_BuildValue("i", 0);
}

static PyMethodDef writer_methods[] = {
    {"append", (PyCFunction)writer_append, METH_VARARGS,"append to matrix file"},
    {NULL}  /* Sentinel */
};

static PyTypeObject writer_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder3.writer",    /* tp_name */
    sizeof(writer),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)writer_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "writer builder v1.0",     /* tp_doc */
    (traverseproc)writer_traverse,/* tp_traverse */
    (inquiry)writer_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    writer_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)writer_init,     /* tp_init */
    0,                         /* tp_alloc */
    writer_new,                /* tp_new */
};

typedef struct{
    PyObject_HEAD
    MatrixReader *matrix_reader;
}reader;
 
static PyObject *reader_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    reader *m;
    m = (reader *)type->tp_alloc(type, 0);
    m->matrix_reader = new MatrixReader();
    return (PyObject *)m;
}

static void reader_delete(reader *self) {
    self->matrix_reader->Close();
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int reader_traverse(reader *self, visitproc visit, void *arg) {
    return 0;
}

static int reader_clear(reader *self) {
    return 0;
}

static int reader_init(reader *self, PyObject *args) {
    char *string1, *string2;
    if (!PyArg_ParseTuple(args, "ss", &string1, &string2)) {
        return -1;
    }
    self->matrix_reader->Open(string1, string2);
    return 0;
}

static PyObject *reader_get(reader *self, PyObject *args) {
    int uid, len;
    if (!PyArg_ParseTuple(args, "ii", &uid, &len)) {
        return PyList_New(0);
    }
    IndexBody idx = self->matrix_reader->GetIndex((uint32_t)uid);
    uint64_t offset = idx.offset;
    uint64_t size = idx.count;
    if (size == 0 || len <= 0) {
        return PyList_New(0);
    }
    if (size > (uint64_t)len) {
        size = len;
    }
    vector<MatrixBody> *vec = self->matrix_reader->GetData(offset, size);
    int count = 0,length = vec->size();
    PyObject *list = PyList_New(length);
    if (list == NULL) {
        return PyList_New(0);
    }
    for (vector<MatrixBody>::iterator iter = vec->begin();iter != vec->end();iter++) {
        PyObject *dict = PyDict_New();
        PyObject *rid = Py_BuildValue("i", iter->rid);
        //PyTuple_SET_ITEM(tuple, 0, rid);
        PyMapping_SetItemString(dict, "id", rid);
        PyObject *value = Py_BuildValue("i", int(iter->value));
        //PyTuple_SET_ITEM(tuple, 1, value);
        PyMapping_SetItemString(dict, "score", value);
        //PyList_SET_ITEM(list, count, tuple);
        PyList_SET_ITEM(list, count, dict);
        count++;
    }
    for (count = 0;count < length;count++) {
        PyObject *item = PyList_GET_ITEM(list, count);
        if (item == NULL) {
            PyList_SET_ITEM(list, count, Py_None);
        }
    }
    
    return list;
}

static PyMethodDef reader_methods[] = {
    {"get", (PyCFunction)reader_get, METH_VARARGS,"append to matrix file"},
    {NULL}  /* Sentinel */
};

static PyTypeObject reader_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder3.reader",    /* tp_name */
    sizeof(reader),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)reader_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "reader builder v1.0",     /* tp_doc */
    (traverseproc)reader_traverse,/* tp_traverse */
    (inquiry)reader_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    reader_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)reader_init,     /* tp_init */
    0,                         /* tp_alloc */
    reader_new,                /* tp_new */
};


static PyMethodDef module_methods[] = {
    {NULL}
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initmatrixbuilder3(void) {
    PyObject *m;
    
    if (PyType_Ready(&writer_obj) < 0)
        return;
    if (PyType_Ready(&reader_obj) < 0)
        return;

    m = Py_InitModule3("matrixbuilder3", module_methods, "matrix builder module v1.0");

    if (m == NULL)
        return;

    Py_INCREF(&writer_obj);
    PyModule_AddObject(m, "writer", (PyObject *)&writer_obj);
    Py_INCREF(&reader_obj);
    PyModule_AddObject(m, "reader", (PyObject *)&reader_obj);
}
