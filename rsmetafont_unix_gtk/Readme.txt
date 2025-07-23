This is the Unix version of rsMetaFont, uses GTK4 for GUI.
You need meson and GTK4 installed on your machine.
Then you can run the script "build.sh" to build the program.
If you don't have or want meson, it should be a fairly simple process
to compile it manually.

To create a plain base file make sure you have plain.mf nearby and run:
./rsmf plain dump

Now you can start rsmf with
./rsmf -base=plain
and start with plain preloaded.

To test that the graphics works,
at the '**' prompt type the following:
**\

*draw (0,0)--(100,0); showit;

If everything works, this should open a window
and draw a black horizontal line.

