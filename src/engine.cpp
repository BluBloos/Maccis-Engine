#include <maccis.h>
#include <engine.h>

INTERNAL float positions[] = {
  -0.5f, -0.5f, //0
   0.5f, -0.5f, //1
   0.5f,  0.5f, //2
  -0.5f,  0.5f  //3
};

INTERNAL unsigned int indices[] = {
  0, 1, 2,
  2, 3, 0
};

void CatStrings(int sourceACount, char *sourceA, int sourceBCount,
 char *sourceB, int destCount, char *dest)
{
	for (int index = 0; index < sourceACount; index++)
	{
		*dest++ = *sourceA++;
	}
	for (int index = 0; index < sourceBCount; index++)
	{
		*dest++ = *sourceB++;
	}
	*dest = 0;
}

unsigned int GetStringLength(char *string)
{
	unsigned int count = 0;
	while (*string != 0)
	{
		string++; count++;
	}
	return count;
}

char *BuildFilePath(file_path fpath, char *fileName, char *buffer, int bufferLength)
{
	CatStrings(fpath.length, fpath.path, GetStringLength(fileName), fileName, bufferLength, buffer);
  return buffer;
}

INTERNAL unsigned int CompileShader(unsigned int type, char *shader)
{
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &shader, NULL);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    //std::cout << "Failed to comile shader!" << std::endl;
    //std::cout << message << std::endl;
    glDeleteShader(id);
    return 0;
  }

  return id;
}

INTERNAL unsigned int CreateShaders(char *vertexShader, char *fragmentShader)
{
  unsigned int program = glCreateProgram();
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

  glAttachShader(program, vs);
  glAttachShader(program, fs);

  //TODO(Noah): Assert if the shader compilation fails

  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

void Init(engine_memory memory)
{
  char stringBuffer[260];
  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int buffer; //create the storage for the generated ID of the buffer
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer); //select the buffer
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);

  unsigned int ibo; //create the storage for the generated ID of the buffer
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //select the buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
  glEnableVertexAttribArray(0);

  read_file_result f1 = memory.ReadFile(BuildFilePath(memory.maccisDirectory, "src\\shader.vert", stringBuffer, 260));
  read_file_result f2 = memory.ReadFile(BuildFilePath(memory.maccisDirectory, "src\\shader.frag", stringBuffer, 260));
  unsigned int program = CreateShaders((char *)f1.content, (char *)f2.content);
  glUseProgram(program);

  int location = glGetUniformLocation(program, "ucolor");
  Assert(location != -1);
  glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f);
}
void Update(engine_memory memory)
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  //glUniform4f(location, r / 255.0f, 0.3f, 0.8f, 1.0f);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}
