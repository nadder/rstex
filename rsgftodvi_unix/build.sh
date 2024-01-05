g++ -o cpf ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsGFtoDVI.cpp.pre rsGFtoDVI.h.pre
g++ -o rsgftodvi -O3 -Wno-unused-result -Wno-dangling-else rsGFtoDVI.cpp
