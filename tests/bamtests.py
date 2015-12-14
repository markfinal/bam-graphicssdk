from testconfigurations import TestSetup, visualc64, mingw32, gcc64, clang64

def configure_repository():
    configs = {}
    configs["Direct3DTriangle"] = TestSetup(win={"Native":[visualc64],"VSSolution":[visualc64],"MakeFile":[visualc64]})
    configs["OpenGLTriangle"] = TestSetup(win={"Native":[visualc64,mingw32],"VSSolution":[visualc64],"MakeFile":[visualc64,mingw32]})
    configs["RenderTextureAndProcessor"] = TestSetup(win={"Native":[visualc64,mingw32],"VSSolution":[visualc64],"MakeFile":[visualc64,mingw32]})
    return configs
