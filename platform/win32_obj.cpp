#include <python/Python.h>
#define PY_FLOAT_FROM_LIST(list, index) (float)PyFloat_AsDouble(PyList_GetItem(list, index))
#define PY_INT_FROM_LIST(list, index) (unsigned int)PyInt_AsLong(PyList_GetItem(list, index))

#include <windows.h>
#include <maccis.h>
#include <platform.h>
#include <maccis_strings.h>
#include <maccis_file_io.h>
#include <maccis_asset.h>
#include <asset.cpp>
#include <win32_file_io.cpp>
#include <win32.h>


//NOTE(Noah): I don't like the standard library!
#include <stdio.h>

raw_model Win32LoadOBJ(memory_arena *Arena, char *maccisDirectory, char *objFilePath)
{
  raw_model model = {};

  //parse the obj using python!
	//TODO(Noah): Remove hard coded value of 260 in this function!
  char stringBuffer[260];
	char stringBuffer2[260];

  PyObject *pName, *pModule, *pFunc;
  PyObject *pArgs, *pValue;
	PyObject *vertices, *indices;

  Py_Initialize(); //initialize the python interpreter
	printf("Initialized python interpreter.\n");
	MaccisCatStringsUnchecked(maccisDirectory, "src", stringBuffer); //build src directory string!

	wsprintf(stringBuffer2, "import sys\nsys.path.insert(0, '%s')", stringBuffer); //format the command for python

	PyRun_SimpleString(stringBuffer2); //set the sys.path
  pName = PyString_FromString("load_obj"); //load the module!

	pModule = PyImport_Import(pName);
  Py_DECREF(pName); //decref kills it, but you gotta be sure it isn't NULL

  if (pModule != NULL) //check whether or not we actually loaded the module
  {
    pFunc = PyObject_GetAttrString(pModule, "obj"); //attempts to find the function
    if (pFunc && PyCallable_Check(pFunc)) //these conditions confirm that there is a function
    {
      pArgs = PyTuple_New(1);
      pValue = PyString_FromString(objFilePath);
      PyTuple_SetItem(pArgs, 0, pValue);
      pValue = PyObject_CallObject(pFunc, pArgs); //this calls the function and pValue is set to the return of the function
      Py_DECREF(pArgs); //gotta make sure to decref that shit
      if (pValue != NULL)
      {
        printf("obj function called and returned succesfully\n");
        if(PyList_Check(pValue))
        {
					vertices = PyList_GetItem(pValue, 2);
					indices = PyList_GetItem(pValue, 3);

					vertices = PyList_GetItem(pValue, 2);
					indices = PyList_GetItem(pValue, 3);

					model.vertexCount = PY_INT_FROM_LIST(pValue, 0);
					model.indexCount = PY_INT_FROM_LIST(pValue, 1);

          model.vertices = Arena->push( model.vertexCount * 8 * sizeof(float) );
					model.indices = Arena->push( model.indexCount * sizeof(unsigned int) );

          float *vp = (float *)model.vertices;
          for (unsigned int i = 0; i < model.vertexCount * 8; i++)
          {
            *vp++ = PY_FLOAT_FROM_LIST(vertices, i);
          }

					unsigned int *ip = (unsigned int *)model.indices;
          for (unsigned int i = 0; i < model.indexCount; i++)
          {
            *ip++ = PY_INT_FROM_LIST(indices, i);
          }
        }

				//Kill everything
        Py_DECREF(pValue);
				Py_XDECREF(vertices);
				Py_XDECREF(indices);
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
		printf("Finalized the python interpreter.\n");
  return model;
}


int main(int argc, char **argv)
{
  char stringBuffer[MAX_PATH];
  char win32FilePath[MAX_PATH];
  Win32GetRelativePath(win32FilePath, MAX_PATH);

  unsigned int storageSize = MB(64);
  void *storage = VirtualAlloc(0, storageSize, MEM_COMMIT, PAGE_READWRITE);

  memory_arena arena = {};
  arena.init(storage, storageSize);

  printf("Amount of arguments: %d\n", argc);
  if (argc >= 3) //NOTE(Noah): If there actually are arguments
  {
    //preparse the obj file
    raw_model model = Win32LoadOBJ(&arena, win32FilePath, MaccisCatStringsUnchecked(win32FilePath, argv[1], stringBuffer));

    //write the raw_model to the asset file
    loaded_asset asset = {};
    PushRawModelToAsset(model, &asset, &arena);

    WriteAsset(Win32WriteFile, &arena, &asset, MaccisCatStringsUnchecked(win32FilePath, argv[2], stringBuffer));
  } else
  {
    printf("Not enough arguments, did you enter a file name?\n");
  }

  VirtualFree(storage, storageSize, MEM_RELEASE);
}
