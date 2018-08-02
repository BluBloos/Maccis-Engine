#include <vendor/python/Python.h>
#define PY_FLOAT_FROM_LIST(list, index) (float)PyFloat_AsDouble(PyList_GetItem(list, index))

loaded_bitmap LoadBMP(platform_read_file *ReadFile, char *path)
{
	loaded_bitmap bitmap = {};
	bitmap.scale = 1;
	read_file_result fileResult = ReadFile(path);
	if (fileResult.contentSize != 0)
	{
		bitmap_header *header = (bitmap_header *)fileResult.content;
		bitmap.pixelPointer = (unsigned int *) ( (unsigned char *)fileResult.content + header->BitmapOffset );
		bitmap.height = header->Height;
		bitmap.width = header->Width;
    bitmap.container = fileResult.content;
	}
	return bitmap;
}

int loadPythonModule(char *moduleName, char *functionName, char *arg)
{
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;

    Py_Initialize(); //initialize the python interpreter

    pName = PyString_FromString(moduleName);

    pModule = PyImport_Import(pName);
    Py_DECREF(pName); //decref kills it, but you gotta be sure it isn't NULL

    if (pModule != NULL) //check whether or not we actually loaded the module
    {
        pFunc = PyObject_GetAttrString(pModule, functionName); //attempts to find the function
        if (pFunc && PyCallable_Check(pFunc)) //these conditions confirm that there is a function
        {
            pArgs = PyTuple_New(1);
            pValue = PyString_FromString(arg);
            PyTuple_SetItem(pArgs, 0, pValue);
            pValue = PyObject_CallObject(pFunc, pArgs); //this calls the function and pValue is set to the return of the function
            Py_DECREF(pArgs); //gotta make sure to decref that shit
            if (pValue != NULL)
            {
                printf("Result of call: %ld\n", PyInt_AsLong(pValue));
                Py_DECREF(pValue); //kill that shit if its not null
            } else
            {
                //kill everything
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Call failed\n");
                return 1;
            }
        } else //could not find the function
        {
            if (PyErr_Occurred())
            {
                PyErr_Print();
                fprintf(stderr, "Cannot find function\n");
            }
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else
    {
        PyErr_Print();
        fprintf(stderr, "Failed to load\n");
        return 1;
    }

    Py_Finalize(); //finalize the interpreter
    return 0;
}

raw_model LoadOBJ(memory_arena Arena, char *path)
{
  raw_model model = {};

  //parse the obj using python!
  char stringBuffer[260];

  PyObject *pName, *pModule, *pFunc;
  PyObject *pArgs, *pValue;

  Py_Initialize(); //initialize the python interpreter
  pName = PyString_FromString("load_obj");

  pModule = PyImport_Import(pName);
  Py_DECREF(pName); //decref kills it, but you gotta be sure it isn't NULL

  if (pModule != NULL) //check whether or not we actually loaded the module
  {
    pFunc = PyObject_GetAttrString(pModule, "obj"); //attempts to find the function
    if (pFunc && PyCallable_Check(pFunc)) //these conditions confirm that there is a function
    {
      pArgs = PyTuple_New(1);
      pValue = PyString_FromString(path);
      PyTuple_SetItem(pArgs, 0, pValue);
      pValue = PyObject_CallObject(pFunc, pArgs); //this calls the function and pValue is set to the return of the function
      Py_DECREF(pArgs); //gotta make sure to decref that shit
      if (pValue != NULL)
      {
        printf("Function returned succesfully\n");

        if(PyList_Check(pValue))
        {
          printf("Return value is a list\n");
          unsigned int listSize = PyList_Size(pValue);

          model.mem = Arena.push( (listSize - 1) * sizeof(float) );
          model.vertexCount = PY_FLOAT_FROM_LIST(pValue, 0);
          model.indexCount = model.vertexCount;
          model.indicesOffset = model.vertexCount *  8; //offset by amount of floats

          float *data = (float *)model.mem;
          for (unsigned int i = 1; i < listSize - 1; i++)
          {
            *data++ = PY_FLOAT_FROM_LIST(pValue, i);
          }
        }

        Py_DECREF(pValue); //kill that shit if its not null
      } else
      {
        //kill everything
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr,"Function did not return\n");
      }
    } else //could not find the function
    {
      if (PyErr_Occurred())
      {
        PyErr_Print();
        fprintf(stderr, "Could not find the function\n");
      }
    }
    Py_XDECREF(pFunc);
    Py_DECREF(pModule);
  } else
  {
    PyErr_Print();
    fprintf(stderr, "Failed to load moudle. Maybe there are runtime errors?\n");
  }

    Py_Finalize(); //finalize the interpreter
  return model;
}
