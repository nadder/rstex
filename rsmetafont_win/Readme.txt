This is the windows version of rsMetaFont. It has support for drawing graphics in a window.

1. Compile the file CreatePoolFile.cpp to get cpf.exe or similar.
2. Run the command cpf.exe TEX_STRING rsMetaFont.cpp.pre rsMetaFont.h.pre.
   Make sure that after this step you have the three files
   rsMetaFont.cpp, rsMetaFont.h, and tex.pool.
3. Rename tex.pool to mf.pool.
4. Build the project with Visual Studio.
5. Celebrate!

To create a plain base file make sure you have plain.mf nearby and run:
rsMetaFont plain dump
Now you can start rsMetaFont with
rsMetaFont -base=plain
and start with plain preloaded.

The following environment variables can be set e.g.:
MFINPUTS=.;c:\mfinputs
MFBASES=.;c:\mfbases
MFPOOL=.
MFEDIT=emacs +%d %s
