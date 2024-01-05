g++ -o cpf ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsetex.cpp.pre rsetex.h.pre
g++ -o rsetex -O3 -Wno-unused-result -Wno-dangling-else rsetex.cpp
