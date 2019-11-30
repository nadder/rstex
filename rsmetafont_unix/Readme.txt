This is the Unix version of rsMetaFont, no GUI support yet.
1. Compile the file CreatePoolFile.cpp to get cpf.exe or similar.
2. Run the command cpf.exe TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre.
   Make sure that after this step you have the three files
   rsMetaFont.cpp, rsMetaFont.h, and tex.pool.
3. Rename tex.pool to mf.pool.
4. Compile rsMetaFont.cpp e.g. g++ -orsmf rsMetaFont.cpp.
5. Celebrate!

To create a plain base file make sure you have plain.mf nearby and run:
./rsmf plain dump
Now you can start rsmf with
./rsmf -base=plain
and start with plain preloaded.

The following environment variables can be set e.g.:
MFINPUTS=.:/home/local/mfinputs
MFBASES=.:/home/local/mfbases
MFPOOL=.
MFEDIT=emacs "+%d %s"

