The early versions of TeX were run on operating systems which allowed for an
easy way to take a memory dump of the running program and create a new executable
from this memory dump. This could considerably reduce the initialization time
of some programs.
TeX could take advantage of this by using the following approximate installation procedure.
$ initex plain \dump
$ virtex &plain
$ ^C or Save command

Assuming the new executable was called tex one could then run
$ tex
This is TeX ... (preloaded format=plain...
**

The plain format was now embedded inside the executable.

This saved alot of startup time in the old days, nowadays it's doubtful if this saves much
time at all. But I thought it would be interesting to see if it could be done anyway.
There are basically two ways to go about doing this on modern operating systems. One is to go the
binary route. You take a memory dump using a debugger or similar tool and then try to recreate
the executable directly from this memory dump. This is very difficult to do and of course extremely
system dependent but in principle it should be possible. Another way and the one I'm using here, is
to enter the data at the source level. We modify the program so that it dumps out all its data
after it has loaded the format file. Then we enter this data as initialization values into the
source file. After that we can recompile and the data is now preloaded.

The procedure is as follows.
1. Ensure you have rstex.cpp, rstex.h, diff_win.txt, and plain.fmt in the same directory.
2. Make sure you're compiling with NO_INIT defined.
3. Apply the patch, patch -i -c rstex.cpp diff_win.txt
4. Recompile rstex.cpp. If this does not work you may have to enter the new code
   manually, refer to store_tex_data.txt
5. Run rstex. And enter
   **&plain \end
6. Run patch -i -c -R rstex.cpp diff_win.txt
   This removes the code added previously which is no longer needed.
7. Run ./texdata.sh <rstex.h >rstex.h.2
   This will run a sed script which inserts the dumped data into rstex.h.2.
   This step is quite fragile and may not work perfectly if the source has changed significantly.
   One known issue is that it does not work with multiple definitions on the same line so stick
   to one definition per line.
8. Run rm -f rstex.h; mv rstex.h.2 rstex.h
   Replaces the original rstex.h with the new one containing the data.
9. Recompile and run and you should see
   This is... (preloaded format=plain...
   **
10. Enjoy running rstex preloaded, just like in the old days!
