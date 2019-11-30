g++ -o cpf -std=c++14 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsGFtoDVI.cpp.pre rsGFtoDVI.h.pre
g++ -o rsgftodvi -O3 -std=c++14 -Wno-unused-result -Wno-dangling-else rsGFtoDVI.cpp
