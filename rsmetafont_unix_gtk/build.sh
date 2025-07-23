g++ -o cpf -std=c++11 ../CreatePoolFile/CreatePoolFile.cpp
./cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre
mv tex.pool mf.pool
if [ ! -d "build" ]; then
  meson setup build
fi
meson compile -C build
cp build/rsmf .

