g++ -o cpf -std=c++11 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
mv tex.pool mf.pool
g++ -o rsmf -std=c++11 -O3 -Wno-unused-result -Wno-dangling-else rsMetaFont.cpp
