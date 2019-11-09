g++ -o cpf -std=c++14 CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
g++ -o rsmf -g -Wno-dangling-else rsMetaFont.cpp
