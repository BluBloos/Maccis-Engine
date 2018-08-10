#include <python/Python.h>
#define PY_FLOAT_FROM_LIST(list, index) (float)PyFloat_AsDouble(PyList_GetItem(list, index))
#define PY_INT_FROM_LIST(list, index) (unsigned int)PyInt_AsLong(PyList_GetItem(list, index))

void SaveBitmap(char *fileName, platform_write_file *WriteFile, loaded_bitmap bitmap, memory_arena *arena)
{
	bitmap_header header = {};
	header.FileType = 0x4D42; /* File type, always 4D42 ("BM") */
	header.FileSize = sizeof(bitmap_header) + bitmap.width * bitmap.height * sizeof(unsigned int); /* Size of the file in bytes */
	header.BitmapOffset = sizeof(bitmap_header); /* Starting position of image data in bytes */
	header.size = 40; /* Size of this header in bytes */
	header.Width = bitmap.width; /* Image width in pixels */
	header.Height = bitmap.height; /* Image height in pixels */
	header.Planes = 1; /* Number of color planes, must be set to 1 */
	header.BitsPerPixel = 32; /* Number of bits per pixel */
  header.biCompression = BI_RGB; //specfies type of compression to be used

	void *data = arena->push(sizeof(bitmap_header) + bitmap.width * bitmap.height * sizeof(unsigned int));
	char *scan = (char *)data;
	memcpy(scan, &header, sizeof(bitmap_header));
	scan += sizeof(bitmap_header);
	memcpy(scan, bitmap.pixelPointer, bitmap.width * bitmap.height * sizeof(unsigned int));
	WriteFile(fileName, sizeof(bitmap_header) + bitmap.width * bitmap.height * sizeof(unsigned int), data);
}

inline loaded_bitmap MakeEmptyBitmap(memory_arena *arena, unsigned int width, unsigned int height)
{
	loaded_bitmap bitmap;
	bitmap.scale = 1;
	bitmap.width = width;
	bitmap.height = height;
	bitmap.pixelPointer = (unsigned int *)arena->push(width * height * sizeof(unsigned int));
	return bitmap;
}

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

raw_model LoadOBJ(memory_arena *Arena, char *maccisDirectory, char *objFilePath)
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
	MaccisCatStringsUnchecked(maccisDirectory, "io", stringBuffer); //build src directory string!

	//TODO(Noah): Remove this windows call
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
