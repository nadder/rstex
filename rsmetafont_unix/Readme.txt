This is the Unix version of rsMetaFont.
1. Compile the file CreatePoolFile.cpp to get cpf or similar.
2. Run the command cpf TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre.
   Make sure that after this step you have the three files:
   rsMetaFont.cpp, rsMetaFont.h, and tex.pool.
3. Rename tex.pool to mf.pool.
4. Compile rsMetaFont.cpp by e.g. g++ -orsmf rsMetaFont.cpp.
5. Celebrate!

To create a plain base file make sure you have plain.mf nearby and run:
./rsmf plain dump
or even
./rsmf plain input modes dump
to include the printer modes.

Now you can start rsmf with
./rsmf -base=plain
and start with plain preloaded.

Another option is to run
./rsmf plain input modes dump
as before, but now compile the program with the defines NO_INIT and NO_DEBUG set.
This will create a leaner version of Metafont unable to create base files.
Now you can run this version with the command
./rsmf '&plain'
Traditionally this was the way Metafont was used to save some precious memory.


The following environment variables can be set e.g.:
MFINPUTS=.:/home/local/mfinputs
MFBASES=.:/home/local/mfbases
MFPOOL=.
MFEDIT="emacs +%d %s"

