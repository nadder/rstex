g++ -o cpf  -std=c++14 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsetex.cpp.pre rsetex.h.pre
g++ -o rsetex -O3  -std=c++14 -Wno-unused-result -Wno-dangling-else rsetex.cpp
