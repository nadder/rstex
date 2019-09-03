cl.exe /EHsc /Fecpf /std:c++14 ..\CreatePoolFile\CreatePoolFile.cpp'
cpf.exe TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
msbuild -property:Configuration=Debug rsMetaFont.vcxproj
