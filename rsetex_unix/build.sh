g++ -o cpf -std=c++17 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsetex.cpp.pre rsetex.h.pre
g++ -o rsetex -O3 -std=c++17 -Wno-unused-result -Wno-dangling-else rstex.cpp
