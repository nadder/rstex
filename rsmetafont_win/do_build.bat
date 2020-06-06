cl /EHsc /Fecpf /std:c++14 ..\CreatePoolFile\CreatePoolFile.cpp
cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
ren tex.pool mf.pool
msbuild -property:Configuration=Debug rsMetaFont.vcxproj
