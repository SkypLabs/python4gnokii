#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Python.h>
#include <gnokii.h>

struct gn_statemachine *state = NULL;
static gn_data *data;
static PyObject *GnokiiError;

static int connected = 0;

/* All */

static PyObject *gnokii_open(PyObject *self, PyObject *args)
{
	if (connected == 1)
		Py_RETURN_NONE;

	gn_error error;
	
	error = gn_lib_phoneprofile_load(NULL, &state);

	if (GN_ERR_NONE == error)
		error = gn_lib_phone_open(state);
	
	if (GN_ERR_NONE != error)
	{
		PyErr_SetString(GnokiiError, "Connection failed");
		return NULL;
	}

	data = &state->sm_data;
	connected = 1;

	Py_RETURN_NONE;
}

static PyObject *gnokii_close(PyObject *self, PyObject *args)
{
	if (state == NULL)
		Py_RETURN_NONE;

	gn_lib_phone_close(state);
	gn_lib_phoneprofile_free(&state);
	gn_lib_library_free();

	connected = 0;

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
		PyErr_SetString(GnokiiError, "Not connected");
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
		PyErr_SetString(GnokiiError, "Call failed");
		return NULL;
	}

	return Py_BuildValue("i", call_info.call_id);
}

static PyObject *gnokii_answercall(PyObject *self, PyObject *args)
{
	const char callid;
    	gn_call_info callinfo;
	gn_error error;

	if (!connected)
	{
		PyErr_SetString(GnokiiError, "Not connected");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "b", &callid))
		return NULL;

	memset(&callinfo, 0, sizeof(callinfo));
	callinfo.call_id = callid;

	if (callinfo.call_id < 0)
	{
		PyErr_SetString(GnokiiError, "Invalid call id");
		return NULL;
	}

	gn_data_clear(data);
	data->call_info = &callinfo;

	error = gn_sm_functions(GN_OP_AnswerCall, data, state);

	if (error != GN_ERR_NONE)
	{
		PyErr_SetString(GnokiiError, "Answer call failed");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *gnokii_senddtmf(PyObject *self, PyObject *args)
{
	const char *cmd;
	gn_error error;

	if (!connected)
	{
		PyErr_SetString(GnokiiError, "Not connected");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "s", &cmd))
		return NULL;

	gn_data_clear(data);
	data->dtmf_string = cmd;

	error = gn_sm_functions(GN_OP_SendDTMF, data, state);

	if (error != GN_ERR_NONE)
	{
		PyErr_SetString(GnokiiError, "Sending DTMF failed");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject *gnokii_hangup(PyObject *self, PyObject *args)
{
	const char callid;
	gn_call_info callinfo;
	gn_error error;

	if (!connected)
	{
		PyErr_SetString(GnokiiError, "Not connected");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "b", &callid))
		return NULL;

	memset(&callinfo, 0, sizeof(callinfo));
	callinfo.call_id = callid;

	if (callinfo.call_id < 0)
	{
		PyErr_SetString(GnokiiError, "Invalid call id");
		return NULL;
	}

	gn_data_clear(data);
	data->call_info = &callinfo;

	error = gn_sm_functions(GN_OP_CancelCall, data, state);

	if (error != GN_ERR_NONE)
	{
		PyErr_SetString(GnokiiError, "Hung up failed");
		return NULL;
	}
	
	Py_RETURN_NONE;
}

/* SMS */

static PyObject *gnokii_getnumber(PyObject *self, PyObject *args)
{
	const char *number;
	gn_gsm_number_type type;

	if (!PyArg_ParseTuple(args, "s", &number))
		return NULL;

	if (!number)
		return Py_BuildValue("i", GN_GSM_NUMBER_Unknown);
	if (*number == '+')
	{
		type = GN_GSM_NUMBER_International;
		number++;
	}
	else
		type = GN_GSM_NUMBER_Unknown;

	while (*number)
	{
		if (!isdigit(*number))
			Py_BuildValue("i", GN_GSM_NUMBER_Alphanumeric);

		number++;
	}

	return Py_BuildValue("i", type);
}

static PyObject *gnokii_deletesms(PyObject *self, PyObject *args)
{
	gn_sms message;
	gn_sms_folder folder;
	gn_sms_folder_list folderlist;
	char *memory_type_string = NULL;
	int start, end, count;
	gn_error error = GN_ERR_NONE;

	if (!connected)
	{
		PyErr_SetString(GnokiiError, "Not connected");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "s i i", memory_type_string, &start, &end))
	{
		if (!PyArg_ParseTuple(args, "s i", memory_type_string, &start))
			return NULL;
	}

	message.memory_type = gn_str2memory_type(memory_type_string);

	if (message.memory_type == GN_MT_XX)
        {
		PyErr_SetString(GnokiiError, "Unknown memory type");
		return NULL;
        }

	if (errno || start < 0)
	{
		PyErr_SetString(GnokiiError, "Invalid start message");
		return NULL;
	}

	if (errno || end < 0)
	{
		PyErr_SetString(GnokiiError, "Invalid end message");
		return NULL;
	}

	/* Now delete the requested entries. */
	for (count = start; count <= end; count++)
	{
		message.number = count;
		data->sms = &message;
		data->sms_folder = &folder;
		data->sms_folder_list = &folderlist;
		error = gn_sms_delete(data, state);

		if (error == GN_ERR_NONE)
			Py_RETURN_NONE;
		else
		{
			if ((error == GN_ERR_INVALIDLOCATION) && (end == INT_MAX) && (count > start))
				return GN_ERR_NONE;

			PyErr_SetString(GnokiiError, "Deleting SMS failed");
			return NULL;
                }
        }

	Py_RETURN_NONE;
}

/* Settings */

static PyMethodDef GnokiiMethods[] = {
	{"open", gnokii_open, METH_VARARGS, "Initiate connection to phone."},
	{"close", gnokii_close, METH_VARARGS, "Close connection to phone."},
	{"dialvoice", gnokii_dialvoice, METH_VARARGS, "Initiate voice call."},
	{"answercall", gnokii_answercall, METH_VARARGS, "Answer an incoming call."},
	{"senddtmf", gnokii_senddtmf, METH_VARARGS, "Send DTMF sequence."},
	{"hangup", gnokii_hangup, METH_VARARGS, "Hangup an incoming call or an already established call."},
	{"getnumber", gnokii_getnumber, METH_VARARGS, "Return the type of the number."},
	{"deletesms", gnokii_deletesms, METH_VARARGS, "Delete SMS message."},
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

	PyModule_AddIntConstant(m, "gsm_number_unknown", GN_GSM_NUMBER_Unknown);
	PyModule_AddIntConstant(m, "gsm_number_international", GN_GSM_NUMBER_International);
	PyModule_AddIntConstant(m, "gsm_number_national", GN_GSM_NUMBER_National);
	PyModule_AddIntConstant(m, "gsm_number_network", GN_GSM_NUMBER_Network);
	PyModule_AddIntConstant(m, "gsm_number_subscriber", GN_GSM_NUMBER_Subscriber);
	PyModule_AddIntConstant(m, "gsm_number_alphanumeric", GN_GSM_NUMBER_Alphanumeric);
	PyModule_AddIntConstant(m, "gsm_number_abbreviated", GN_GSM_NUMBER_Abbreviated);
}
