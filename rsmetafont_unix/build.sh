g++ -o cpf -std=c++14 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
g++ -o rsmf -std=c++14 -O3 -Wno-dangling-else rsMetaFont.cpp
