cl.exe /EHsc /Fecpf /std:c++14 CreatePoolFile\CreatePoolFile.cpp
cpf.exe TEX_STRING rstex.cpp.pre rstex.h.pre
cl.exe /EHsc rstex.cpp
