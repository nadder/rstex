call "C:/Program Files (x86)/Microsoft Visual Studio/2017/BuildTools/VC/Auxiliary/Build/vcvarsall.bat" amd64
cl /EHsc /Fecpf /std:c++14 ..\CreatePoolFile\CreatePoolFile.cpp
cpf TEX_STRING rsGFtoDVI.cpp.pre rsGFtoDVI.h.pre
cl /EHsc rsGFtoDVI.cpp
