from testconfigurations import TestSetup, visualc, visualc64, visualc32, mingw32, gcc, gcc64, gcc32, clang, clang32, clang64

def configure_repository():
    configs = {}
    configs["Direct3DTriangle"] = TestSetup(win={"Native": [visualc64, visualc32], "VSSolution": [visualc64, visualc32], "MakeFile": [visualc64, visualc32]})
    configs["OpenGLTriangle"] = TestSetup(win={"Native": [visualc64, visualc32, mingw32], "VSSolution": [visualc64, visualc32], "MakeFile": [visualc64, visualc32, mingw32]},
                                          linux={"Native": [gcc64], "MakeFile": [gcc64]}, # 64-bit only because https://askubuntu.com/questions/695604/libglu1-mesa-dev-how-to-keep-both-i386-and-amd64-packages
                                          osx={"Native": [clang64, clang32], "MakeFile": [clang64, clang32], "Xcode": [clang64, clang32]})
    configs["RenderTextureAndProcessor"] = TestSetup(win={"Native": [visualc64, visualc32, mingw32], "VSSolution": [visualc64, visualc32], "MakeFile": [visualc64, visualc32, mingw32]})
    configs["VulkanTriangle"] = TestSetup(win={"Native": [visualc64], "VSSolution": [visualc64], "MakeFile": [visualc64]},
                                          osx={"Native": [clang64], "MakeFile": [clang64], "Xcode": [clang64]})
    configs["MetalTriangle"] = TestSetup(osx={"Native": [clang64], "MakeFile": [clang64], "Xcode": [clang64]})
    #configs["WindowLibrary"] = TestSetup(win={"Native": [visualc64, visualc32, mingw32], "VSSolution": [visualc64, visualc32], "MakeFile": [visualc64, visualc32, mingw32]},
    #                                     linux={"Native": [gcc64, gcc32], "MakeFile": [gcc64, gcc32]},
    #                                     osx={"Native": [clang64, clang32], "MakeFile": [clang64, clang32], "Xcode": [clang64, clang32]})
    return configs
