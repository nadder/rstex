call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat" -host_arch=amd64 -arch=amd64
cl /EHsc /Fecpf /std:c++14 ..\CreatePoolFile\CreatePoolFile.cpp
cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
msbuild -property:Configuration=Debug rsMetaFont.vcxproj
