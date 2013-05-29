#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gnokii.h>

struct gn_statemachine *state = NULL;
static gn_data *data;
static PyObject *GnokiiError;

static int connected = 0;

/* All */

static PyObject *gnokii_open(PyObject *self, PyObject *args)
{
	gn_error error;
	
	error = gn_lib_phoneprofile_load(NULL, &state);

	if (GN_ERR_NONE == error)
		error = gn_lib_phone_open(state);
	
	if (GN_ERR_NONE != error)
	{
		PyErr_SetString(GnokiiError, "Open command failed");
		return NULL;
	}

	data = &state->sm_data;
	connected = 1;

	Py_RETURN_NONE;
}

static PyObject *gnokii_close(PyObject *self, PyObject *args)
{
	gn_lib_phone_close(state);
	gn_lib_phoneprofile_free(&state);
	gn_lib_library_free();

	Py_RETURN_NONE;
}

/* Dial */

static PyObject *gnokii_dialvoice(PyObject *self, PyObject *args)
{
	const char *number;
	gn_call_info call_info;
	gn_error error;
	int call_id;

	if (!connected)
	{
		PyErr_SetString(GnokiiError, "Not connected !");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "s", &number))
		return NULL;

	memset(&call_info, 0, sizeof(call_info));
	snprintf(call_info.number, sizeof(call_info.number), "%s", number);
	call_info.type = GN_CALL_Voice;
	call_info.send_number = GN_CALL_Default;

	gn_data_clear(data);
	data->call_info = &call_info;

	if ((error = gn_call_dial(&call_id, data, state)) != GN_ERR_NONE)
	{
		PyErr_SetString(GnokiiError, "Dialvoice failed");
		return NULL;
	}

	return Py_BuildValue("i", call_info.call_id);

	Py_RETURN_NONE;
}

static PyMethodDef GnokiiMethods[] = {
	{"open", gnokii_open, METH_VARARGS, "Initiate connection to phone."},
	{"close", gnokii_close, METH_VARARGS, "Close connection to phone."},
	{"dialvoice", gnokii_dialvoice, METH_VARARGS, "Initiate voice call."},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initgnokii(void)
{
	PyObject *m;

	m = Py_InitModule("gnokii", GnokiiMethods);

	if (m == NULL)
		return;

	GnokiiError = PyErr_NewException("gnokii.error", NULL, NULL);
	Py_INCREF(GnokiiError);
	PyModule_AddObject(m, "error", GnokiiError);
}
