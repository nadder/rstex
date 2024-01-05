g++ -o cpf -std=c++11 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsetex.cpp.pre rsetex.h.pre
g++ -o rsetex -O3 -std=c++11 -Wno-unused-result -Wno-dangling-else rsetex.cpp
