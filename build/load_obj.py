import sys

class vertex:
    def __init__(self, position, textureCoord, normal):
        self.position = position
        self.textureCoord = textureCoord
        self.normal = normal
    def compare(self, v):
        if self.position.compare(v.position) and self.textureCoord.compare(v.textureCoord) and self.normal.compare(v.normal):
            return True
        return False

class vec3:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    def compare(self, v):
        if self.x == v.x and self.y == v.y and self.z == v.z:
          return True
        return False

class vec2:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def compare(self, v):
        if self.x == v.x and self.y == v.y and self.z == v.z:
          return True
        return False

def queryVertices(vertices, vertex):
  pass

def obj(fileName):
    sys.stdout = open("plog.txt", "w")
    sys.stdout.write("stdout redirected" + "\n")

    file = open(fileName, "r")
    lines = file.readlines()
    file.close()

    vertices = []
    textureCoords = []
    normals = []

    vertexObjects = []

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

                tempTextureCoord = vec2(0, 0)
                if vertex[1] != '':
                    tempTextureCoord[0] = (textureCoords[int(vertex[1]) - 1].x)
                    tempTextureCoord[1] = (textureCoords[int(vertex[1]) - 1].y)

                vertexObject = vertex( vertices[int(vertex[0]) - 1], tempTextureCoord,  normals[int(vertex[2]) - 1])
                queryResult = queryVertices(vertexObjects, vertexObject)
                if queryResult.good:
                    #below appends a vertex to the floatBuffer
                    #append position of vertex 1
                    floatBuffer.append(vertices[int(vertex[0]) - 1].x)
                    floatBuffer.append(vertices[int(vertex[0]) - 1].y)
                    floatBuffer.append(vertices[int(vertex[0]) - 1].z)

                    #append texture coordinates of vertex 1
                    floatBuffer.append(tempTextureCoord.x)
                    floatBuffer.append(tempTextureCoord.y)

                    #append normals of vertex 1
                    floatBuffer.append(normals[int(vertex[2]) - 1].x)
                    floatBuffer.append(normals[int(vertex[2]) - 1].y)
                    floatBuffer.append(normals[int(vertex[2]) - 1].z)

                indices.append(queryResult) #the query result will tell us the index of the requested vertex
                vertexCount += 1 #add one to the vertexCount because we added more vertices

    for index in indices:
        floatBuffer.append(index)

    floatBuffer[0] = vertexCount

    print(floatBuffer)
    sys.stdout.close()
    return [floatBuffer, indices]
