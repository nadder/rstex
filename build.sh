g++ -o cpf -std=c++11 CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rstex.cpp.pre rstex.h.pre
g++ -o rstex -O3 -std=c++11 -Wno-unused-result -Wno-dangling-else rstex.cpp
