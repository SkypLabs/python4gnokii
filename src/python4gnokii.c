#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <Python.h>
#include <gnokii.h>

struct gn_statemachine *state = NULL;
static gn_data *data;
static PyObject *GnokiiError;

/* ---------- All ---------- */

/*
 * Establishes the connection with the device.
 *
 * Returns 1 if succeed, else 0.
 */
unsigned char gnokii_open(void)
{
    gn_error error;

    error = gn_lib_phoneprofile_load(NULL, &state);

    if (GN_ERR_NONE == error)
        error = gn_lib_phone_open(state);

    if (GN_ERR_NONE != error)
        return 1;

    data = &state->sm_data;

    return 0;
}

/*
 * Closes the connection with the device.
 */
void gnokii_close(void)
{
    if (state == NULL)
        return;

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();
}

/*
 * Returns the type of the number given as parameter.
 */
gn_gsm_number_type get_number_type(const char *number)
{
    gn_gsm_number_type type;

    if (!number)
        return GN_GSM_NUMBER_Unknown;
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
            return GN_GSM_NUMBER_Alphanumeric;

        number++;
    }

    return type;
}

/* ---------- Dial ---------- */

/*
 * Initiates a voice call.
 *
 * Returns the call ID.
 */
static PyObject *gnokii_dialvoice(PyObject *self, PyObject *args)
{
    const char *number;
    gn_call_info call_info;
    gn_error error;
    int call_id;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
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
        gnokii_close();
        PyErr_SetString(GnokiiError, "Call failed");
        return NULL;
    }

    gnokii_close();

    return Py_BuildValue("i", call_info.call_id);
}

/*
 * Answers an incoming call using the call ID.
 */
static PyObject *gnokii_answercall(PyObject *self, PyObject *args)
{
    const char callid;
        gn_call_info callinfo;
    gn_error error;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "b", &callid))
        return NULL;

    memset(&callinfo, 0, sizeof(callinfo));
    callinfo.call_id = callid;

    if (callinfo.call_id < 0)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Invalid call id");
        return NULL;
    }

    gn_data_clear(data);
    data->call_info = &callinfo;

    error = gn_sm_functions(GN_OP_AnswerCall, data, state);

    if (error != GN_ERR_NONE)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Answer call failed");
        return NULL;
    }

    gnokii_close();

    Py_RETURN_NONE;
}

/*
 * Sends the DTMF sequence given as parameter.
 */
static PyObject *gnokii_senddtmf(PyObject *self, PyObject *args)
{
    const char *cmd;
    gn_error error;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s", &cmd))
        return NULL;

    gn_data_clear(data);
    data->dtmf_string = cmd;

    error = gn_sm_functions(GN_OP_SendDTMF, data, state);

    if (error != GN_ERR_NONE)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Sending DTMF failed");
        return NULL;
    }

    gnokii_close();

    Py_RETURN_NONE;
}

/*
 * Hangs up an incoming call or one already established using the call ID.
 */
static PyObject *gnokii_hangup(PyObject *self, PyObject *args)
{
    const char callid;
    gn_call_info callinfo;
    gn_error error;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "b", &callid))
        return NULL;

    memset(&callinfo, 0, sizeof(callinfo));
    callinfo.call_id = callid;

    if (callinfo.call_id < 0)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Invalid call id");
        return NULL;
    }

    gn_data_clear(data);
    data->call_info = &callinfo;

    error = gn_sm_functions(GN_OP_CancelCall, data, state);

    if (error != GN_ERR_NONE)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Hung up failed");
        return NULL;
    }

    gnokii_close();

    Py_RETURN_NONE;
}

/* ---------- SMS ---------- */

/*
 * Sends an SMS message.
 */
static PyObject *gnokii_sendsms(PyObject *self, PyObject *args)
{
    gn_sms sms;
    gn_error error;
    char *dest = NULL, *message = NULL;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "ss", &dest, &message))
        return NULL;

    gn_sms_default_submit(&sms);

    if ((strlen(message) + 1) > sizeof(sms.user_data[0].u.text))
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Message too long");
        return NULL;
    }

    snprintf(sms.remote.number, sizeof(sms.remote.number), "%s", dest);
    sms.remote.type = get_number_type(sms.remote.number);

    if (sms.remote.type == GN_GSM_NUMBER_Alphanumeric)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Wrong data format");
        return NULL;
    }

    data->message_center = calloc(1, sizeof(gn_sms_message_center));
    data->message_center->id = 1;

    if (gn_sm_functions(GN_OP_GetSMSCenter, data, state) == GN_ERR_NONE)
    {
        snprintf(sms.smsc.number, sizeof(sms.smsc.number), "%s", data->message_center->smsc.number);
        sms.smsc.type = data->message_center->smsc.type;
        free(data->message_center);
    }
    else
    {
        free(data->message_center);
        gnokii_close();
        PyErr_SetString(GnokiiError, "Cannot read the SMSC number from your phone");
        return NULL;
    }

    if (!sms.smsc.type)
        sms.smsc.type = GN_GSM_NUMBER_Unknown;

    sms.user_data[0].length = strlen(message) + 1;
    strncpy(sms.user_data[0].u.text, message, strlen(message) + 1);
    sms.user_data[0].type = GN_SMS_DATA_Text;

    if ((sms.dcs.u.general.alphabet != GN_SMS_DCS_8bit) && !gn_char_def_alphabet(sms.user_data[0].u.text))
        sms.dcs.u.general.alphabet = GN_SMS_DCS_UCS2;

    sms.user_data[1].type = GN_SMS_DATA_None;

    data->sms = &sms;

    error = gn_sms_send(data, state);

    if (error != GN_ERR_NONE)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Sending SMS failed");
        return NULL;
    }

    free(sms.reference);
    gnokii_close();

    Py_RETURN_NONE;
}

/*
 * Gets SMS messages from the specified memory type.
 */
static PyObject *gnokii_getsms(PyObject *self, PyObject *args)
{
    gn_sms_folder folder;
    gn_sms_folder_list folderlist;
    gn_sms message;
    gn_error error = GN_ERR_NONE;
    PyObject *remote_num = NULL, *smsc_num = NULL;
    PyObject *msg_num = NULL, *nb_msg = NULL;
    PyObject *message_text = NULL;
    int start_message, end_message = 0, count;
    unsigned char cont = 1, all = 0, messages_read = 0;
    unsigned char i = 0;
    char *memory_type_string = NULL;
    char folder_count = -1;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "si|i", &memory_type_string, &start_message, &end_message))
        return NULL;

    message.memory_type = gn_str2memory_type(memory_type_string);

    if (message.memory_type == GN_MT_XX)
        {
            gnokii_close();
        PyErr_SetString(GnokiiError, "Unknown memory type");
        return NULL;
        }

    if (start_message < 0)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Invalid start message");
        return NULL;
    }

    if (end_message < 0)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Invalid end message");
        return NULL;
    }
    else if (end_message == 0)
        end_message = start_message;
    else if (end_message < start_message)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "End value is less than start value");
        return NULL;
    }
    else if (end_message == INT_MAX)
    {
        unsigned char j;
        gn_error e;

        all = 1;
        memset(&folderlist, 0, sizeof(folderlist));
        gn_data_clear(data);
        data->sms_folder_list = &folderlist;

        e = gn_sm_functions(GN_OP_GetSMSFolders, data, state);

        if (e == GN_ERR_NONE)
        {
            for (j = 0; j < folderlist.number; j++)
            {
                data->sms_folder = folderlist.folder + j;

                if (folderlist.folder_id[j] == gn_str2memory_type(memory_type_string))
                {
                    e = gn_sm_functions(GN_OP_GetSMSFolderStatus, data, state);

                    if (e == GN_ERR_NONE)
                        folder_count = folderlist.folder[j].number;
                }
            }
        }
    }

    folder.folder_id = 0;
    data->sms_folder = &folder;
    data->sms_folder_list = &folderlist;
    count = start_message;
    PyObject* messages = PyTuple_New(end_message - start_message + 1);

    while (cont)
    {
        memset(&message, 0, sizeof(gn_sms));
        message.memory_type = gn_str2memory_type(memory_type_string);
        message.number = count;
        data->sms = &message;
        error = gn_sms_get(data, state);

        if (error == GN_ERR_NONE)
        {
            messages_read++;

            switch (message.type)
            {
                case GN_SMS_MT_StatusReport:
                    message_text = Py_BuildValue("s", message.user_data[0].u.text);
                    break;
                case GN_SMS_MT_Picture:
                case GN_SMS_MT_PictureTemplate:
                    message_text = Py_BuildValue("s", message.user_data[1].u.text);
                    break;
                default:
                    if (!message.udh.number)
                        message.udh.udh[0].type = GN_SMS_UDH_None;

                    if (message.udh.udh[0].type != GN_SMS_UDH_Ringtone)
                        message_text = Py_BuildValue("s", message.user_data[0].u.text);
                    break;
            }

            msg_num = Py_BuildValue("i", message.udh.udh[0].u.concatenated_short_message.current_number);
            nb_msg = Py_BuildValue("i", message.udh.udh[0].u.concatenated_short_message.maximum_number);
            remote_num = Py_BuildValue("s", message.remote.number);
            smsc_num = Py_BuildValue("s", message.smsc.number);

            PyTuple_SetItem(messages, i, Py_BuildValue("OOOOO", msg_num, nb_msg, remote_num, smsc_num, message_text));
        }
        else if (error == GN_ERR_INVALIDMEMORYTYPE)
        {
            gnokii_close();
            PyErr_SetString(GnokiiError, "Unknown memory type");
            return NULL;
        }
        else
        {
            Py_INCREF(Py_None);
            PyTuple_SetItem(messages, i, Py_None);
        }

        if (count >= end_message)
            cont = 0;
        if ((folder_count > 0) && (messages_read >= (folder_count - start_message + 1)))
            cont = 0;
        if (all && error != GN_ERR_NONE && error != GN_ERR_EMPTYLOCATION)
            cont = 0;

        count++;
        i++;
    }

    gnokii_close();

    return messages;
}

/*
 * Deletes SMS messages from the specified memory type.
 */
static PyObject *gnokii_deletesms(PyObject *self, PyObject *args)
{
    gn_sms message;
    gn_sms_folder folder;
    gn_sms_folder_list folderlist;
    char *memory_type_string = NULL;
    int start_message, end_message = 0, count;
    gn_error error = GN_ERR_NONE;

    if (gnokii_open() != 0)
    {
        PyErr_SetString(GnokiiError, "Connection failed");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "si|i", &memory_type_string, &start_message, &end_message))
        return NULL;

    message.memory_type = gn_str2memory_type(memory_type_string);

    if (message.memory_type == GN_MT_XX)
        {
            gnokii_close();
        PyErr_SetString(GnokiiError, "Unknown memory type");
        return NULL;
        }

    if (start_message < 0)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Invalid start message");
        return NULL;
    }

    if (end_message < 0)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "Invalid end message");
        return NULL;
    }
    else if (end_message == 0)
        end_message = start_message;
    else if (end_message < start_message)
    {
        gnokii_close();
        PyErr_SetString(GnokiiError, "End value is less than start value");
        return NULL;
    }

    for (count = start_message; count <= end_message; count++)
    {
        message.number = count;
        data->sms = &message;
        data->sms_folder = &folder;
        data->sms_folder_list = &folderlist;
        error = gn_sms_delete(data, state);

        if (error != GN_ERR_NONE)
        {
            gnokii_close();

            if ((error == GN_ERR_INVALIDLOCATION) && (end_message == INT_MAX) && (count > start_message))
                Py_RETURN_NONE;

            PyErr_SetString(GnokiiError, "Deleting SMS failed");
            return NULL;
        }
        }

    gnokii_close();

    Py_RETURN_NONE;
}

/* ---------- Settings ---------- */

/*
 * Python module's methods.
 */
static PyMethodDef GnokiiMethods[] = {
    {"dialvoice", gnokii_dialvoice, METH_VARARGS, "Initiate voice call."},
    {"answercall", gnokii_answercall, METH_VARARGS, "Answer an incoming call."},
    {"senddtmf", gnokii_senddtmf, METH_VARARGS, "Send DTMF sequence."},
    {"hangup", gnokii_hangup, METH_VARARGS, "Hangup an incoming call or an already established call."},
    {"sendsms", gnokii_sendsms, METH_VARARGS, "Send an SMS message."},
    {"getsms", gnokii_getsms, METH_VARARGS, "Get an SMS message."},
    {"deletesms", gnokii_deletesms, METH_VARARGS, "Delete SMS message."},
    {NULL, NULL, 0, NULL}
};

/*
 * Python module itself.
 */
static struct PyModuleDef gnokii_module =
{
    PyModuleDef_HEAD_INIT,
    "gnokii",
    "Python bindings for the Gnokii library",
    -1,
    GnokiiMethods
};

/*
 * Python module initialization.
 */
PyMODINIT_FUNC PyInit_gnokii(void)
{
    PyObject *m;

    m = PyModule_Create(&gnokii_module);

    if (m == NULL)
        return NULL;

    GnokiiError = PyErr_NewException("gnokii.error", NULL, NULL);
    Py_INCREF(GnokiiError);
    PyModule_AddObject(m, "error", GnokiiError);

    return m;
}
