/*
 *
 *  Copyright (C) 2014 Jürg Müller, CH-5524
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see http://www.gnu.org/licenses/ .
 */

#include <Python.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NTypes.h"

#include "NUtils.h"
#include "KElsterTable.h"
#include "elster_instance.h"

static PyObject * elsterError;

static bool get_params(PyObject *args, const char * & params)
{
  if (!PyArg_ParseTuple(args, "s", &params))
    return false;

  if (!params)
    return false;

  while (*params == ' ')
    params++;

  return true;
}

// siehe: https://docs.python.org/3/extending/extending.html

#ifdef __cplusplus
extern "C" {
#endif

#define PY_RETURN_NONE Py_INCREF(Py_None), Py_None

static PyObject * elster_getname(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  return PyBytes_FromString(elster_instance::GetName(params));
}

static PyObject * elster_gettype(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  return PyBytes_FromString(elster_instance::GetType(params));
}

static PyObject * elster_getvalue(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  return PyBytes_FromString(elster_instance::GetValue(params));
}

static PyObject * elster_setvalue(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  if (elster_instance::SetValue(params))
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE;
}

static PyObject * elster_setbits(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  if (elster_instance::SetBits(params))
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE;
}

static PyObject * elster_clrbits(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  if (elster_instance::ClrBits(params))
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE;
}

static PyObject * elster_getstring(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  return PyBytes_FromString(elster_instance::GetString(params));
}
  
static PyObject * elster_setstring(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  if (elster_instance::SetString(params))
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE;
}

static PyObject * elster_trace(PyObject *self, PyObject *args)
{
  return PyBytes_FromString(elster_instance::ToggleTrace());
}

static PyObject * elster_setdev(PyObject *self, PyObject *args)
{
  const char * params;
  
  if (!get_params(args, params))
    return PY_RETURN_NONE;
  
  if (elster_instance::SetDev(params))
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE;
}

static PyObject * elster_setcs(PyObject *self, PyObject *args)
{
  if (elster_instance::SetCS())
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE; 
}

static PyObject * elster_set_can232(PyObject *self, PyObject *args)
{
  if (elster_instance::SetCAN232())
    return PyBytes_FromString("ok");
  else
    return PY_RETURN_NONE;
}

static PyObject * elster_system(PyObject *self, PyObject *args)
{
  const char * command;
  int sts;

  if (!PyArg_ParseTuple(args, "s", &command))
    return PY_RETURN_NONE;

  sts = system(command);
  if (sts < 0)
  {
    PyErr_SetString(elsterError, "System command failed");
    return PY_RETURN_NONE;
  }
  return PyLong_FromLong(sts);
}
  
static PyObject * elster_geterrmsg(PyObject *self, PyObject *args)
{
  return PyBytes_FromString(elster_instance::GetErrMsg());
}
  
static PyMethodDef elsterMethods[] =
{
  {"system",    elster_system,    METH_VARARGS, "Execute a shell command."},
  {"getvalue",  elster_getvalue,  METH_VARARGS, "Get Elster value"},
  {"setvalue",  elster_setvalue,  METH_VARARGS, "Set Elster value"}, // setvalue, setbits und clrbits sind noch nicht getestet
  {"setbits",   elster_setbits,   METH_VARARGS, "Set bits"},
  {"clrbits",   elster_clrbits,   METH_VARARGS, "Clear bits"},
  {"getstring", elster_getstring, METH_VARARGS, "Get Elster string"},
  {"setstring", elster_setstring, METH_VARARGS, "Set Elster string"},
  {"getname",   elster_getname,   METH_VARARGS, "Get Elster name"},
  {"gettype",   elster_gettype,   METH_VARARGS, "Get Elster type"},
  {"trace",     elster_trace,     METH_VARARGS, "Set tracing"},
  {"setdev",    elster_setdev,    METH_VARARGS, "Set can device"},
  {"setcs",     elster_setcs,     METH_VARARGS, "Set optical inteface (ComfortSoft)"},
  {"set_can232",elster_set_can232,METH_VARARGS, "Set CAN232 inteface (SUBtin)"},
  {"geterrmsg", elster_geterrmsg, METH_VARARGS, "Get last error message"},
  {NULL, NULL, 0, NULL}
};

////////////////////////////////////////////////////////////////////////////////

struct module_state
{
  PyObject * error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static int elster_traverse(PyObject *m, visitproc visit, void *arg)
{
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int elster_clear(PyObject *m)
{
  Py_CLEAR(GETSTATE(m)->error);

  return 0;
}

static void elster_free(void *)
{
  elster_instance::Undef();
}

static struct PyModuleDef moduledef =
{
  PyModuleDef_HEAD_INIT,
  "elster",
  NULL,
  sizeof(struct module_state), // Wenn hier -1 steht, dann wird elster_free nicht ausgefuehrt?
  elsterMethods,
  NULL,
  elster_traverse,
  elster_clear,
  elster_free
};

PyObject * PyInit_elster(void)
{
  PyObject * m;

  m = PyModule_Create(&moduledef);
  if (!m)
    return NULL;

  elsterError = PyErr_NewException("elster.error", NULL, NULL);
  Py_INCREF(elsterError);
  PyModule_AddObject(m, "error", elsterError);

  return m;
}

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[])
{
  wchar_t name[1024];

  for (unsigned i = 0; i <= strlen(argv[0]); i++)
    name[i] = argv[0][i];

  Py_SetProgramName(name);
  Py_Initialize();

  return 0;
}


