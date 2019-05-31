import os
def getAllDirAndFile(path):
    fileList = os.listdir(path)
    for fileName in fileList:
        fileAbs = os.path.join(path, fileName)
        if os.path.isdir(fileAbs):
            getAllDirAndFile(fileAbs)
        else:
            if fileName.endswith(".cxx"):
                f = open(fileAbs)
                contents = f.read()
                f.close()
                index = contents.find("#include", 0)       
                if index != -1:       
                    content = contents[:index] + "#include \"pch.h\"\r\n" + contents[index:]
                    file = open( fileAbs, "w" )
                    file.write( content )
                    file.close()
getAllDirAndFile("scintilla")