"C:/Program Files (x86)/Microsoft Visual Studio/2017/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" amd64
cl /EHsc /Fecpf /std:c++17 ..\CreatePoolFile\CreatePoolFile.cpp
cpf TEX_STRING rsetex.cpp.pre rsetex.h.pre
cl /EHsc /std:c++17 rsetex.cpp

