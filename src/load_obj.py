import sys

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

def obj(fileName):
    #redirect stdout to a file
    sys.stdout = open("pylog.txt", "w")
    sys.stdout.write("stdout redirected\n")

    #read from the file
    file = open(fileName, "r")
    lines = file.readlines()
    file.close()

    #initialize variables needed for execution
    textureCoordinates = []
    normals = []

    vertexCount = 0
    indicesCount = 0
    floatBuffer = []
    indices = []

    #parse the file
    for line in lines:
        splitLine = line.split(" ")

        #the line specifies a vertex
        if splitLine[0] == 'v':
            #append the vertex
            floatBuffer.append( float(splitLine[1]) )
            floatBuffer.append( float(splitLine[2]) )
            floatBuffer.append( float(splitLine[3]) )
            vertexCount += 1

            #append padding floats
            floatBuffer.append(0.0)
            floatBuffer.append(0.0)
            floatBuffer.append(0.0)
            floatBuffer.append(0.0)
            floatBuffer.append(0.0)


        #the line specifies a texture coordinate
        elif splitLine[0] == 'vt':
            #append the texture coordinate to the textureCoordinates array
            textureCoordinates.append( vec2( float(splitLine[1]), float(splitLine[2]) ) )

        #the line specifies a vertex normal
        elif splitLine[0] == 'vn':
            #append the vertex normal to the normals array
            normals.append( vec3( float(splitLine[1]), float(splitLine[2]), float(splitLine[3]) ) )

        #the line specifies a face
        elif splitLine[0] == 'f':
            for i in range(3):
                vertex = splitLine[i + 1].split('/')
                vertexLocation = (int(vertex[0]) - 1) * 8
                normalLocation = int(vertex[2]) -1

                #test if the vertex contains a texture coordinate
                if vertex[1] != '':
                    textureCoordinateLocation = int(vertex[1]) - 1
                    floatBuffer[ vertexLocation + 3 ] = textureCoordinates[textureCoordinateLocation].x
                    floatBuffer[ vertexLocation + 4 ] = textureCoordinates[textureCoordinateLocation].y

                #write the normals
                floatBuffer[ vertexLocation + 5 ] = normals[normalLocation].x
                floatBuffer[ vertexLocation + 6 ] = normals[normalLocation].y
                floatBuffer[ vertexLocation + 7 ] = normals[normalLocation].z

                #write into the indices
                indices.append( int(vertex[0]) - 1 )
                indicesCount += 1

    #debug logs
    print("vertexCount: " + str(vertexCount) + "\n")
    print("indicesCount: " + str(indicesCount) + "\n")
    print(floatBuffer)
    print(indices)

    sys.stdout.close()
    return [vertexCount, indicesCount, floatBuffer, indices]
