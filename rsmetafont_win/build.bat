"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
cl /EHsc /Fecpf /std:c++14 ..\CreatePoolFile\CreatePoolFile.cpp
cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
msbuild -property:Configuration=Debug rsMetaFont.vcxproj
