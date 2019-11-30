cl /EHsc /Fecpf /std:c++14 CreatePoolFile\CreatePoolFile.cpp
cpf TEX_STRING rstex.cpp.pre rstex.h.pre
cl /EHsc rstex.cpp
