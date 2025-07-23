This is the Unix version of rsMetaFont, uses GTK4 for GUI.
You need meson and GTK4 installed on your machine.
Then you can run the script "build.sh" to build the program.
If you don't have or want meson, it should be a fairly simple process
to compile it manually.

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

