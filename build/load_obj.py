import sys

class vec3:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

class vec2:
    def __init__(self, x, y):
        self.x = x
        self.y = y

def obj(fileName):
    sys.stdout = open("plog.txt", "w")
    sys.stdout.write("stdout redirected" + "\n")

    file = open(fileName, "r")
    lines = file.readlines()
    file.close()

    vertices = []
    textureCoords = []
    normals = []
    floatBuffer = []
    indices = []
    vertexCount = 0

    floatBuffer.append(0)

    for line in lines:
        splt = line.split(" ")
        if splt[0] == 'v':
            vertices.append(vec3(float(splt[1]), float(splt[2]), float(splt[3])))
        elif splt[0] == 'vt':
            textureCoords.append(vec2(float(splt[1]), float(splt[2])))
        elif splt[0] == 'vn':
            normals.append(vec3(float(splt[1]), float(splt[2]), float(splt[3])))
        elif splt[0] == 'f':
            for i in range(3):
                vertex = splt[i + 1].split("/")
                indices.append(vertexCount)
                vertexCount += 1

                #append position of vertex 1
                floatBuffer.append(vertices[int(vertex[0]) - 1].x)
                floatBuffer.append(vertices[int(vertex[0]) - 1].y)
                floatBuffer.append(vertices[int(vertex[0]) - 1].z)

                #append texture coordinates of vertex 1
                if vertex[1] == '':
                    floatBuffer.append(0)
                    floatBuffer.append(0)
                else:
                    floatBuffer.append(textureCoords[int(vertex[1]) - 1].x)
                    floatBuffer.append(textureCoords[int(vertex[1]) - 1].y)

                #append normals of vertex 1
                floatBuffer.append(normals[int(vertex[2]) - 1].x)
                floatBuffer.append(normals[int(vertex[2]) - 1].y)
                floatBuffer.append(normals[int(vertex[2]) - 1].z)

    for index in indices:
        floatBuffer.append(index)

    floatBuffer[0] = vertexCount

    print(floatBuffer)
    sys.stdout.close()
    return floatBuffer
