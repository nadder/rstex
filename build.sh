g++ -o cpf CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rstex.cpp.pre rstex.h.pre
g++ -o rstex -O3 -Wno-unused-result -Wno-dangling-else rstex.cpp
