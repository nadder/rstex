cl.exe /EHsc /Fecpf /std:c++14 ..\CreatePoolFile\CreatePoolFile.cpp
cpf.exe TEX_STRING rsGFtoDVI.cpp.pre rsGFtoDVI.h.pre
cl.exe /EHsc rsGFtoDVI.cpp
