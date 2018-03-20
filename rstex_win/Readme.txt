This is a modified version of rstex for Windows systems. Similar to Knuth's changes for SPARC.
The following extra features are added
* Increased memory, hash size, string pool etc.
* -format=<format file> command line option for ``preloading'' format.
* Path searching using default locations and environment variables:
  TEXINPUTS, TEXFONTS, TEXFORMATS, and TEXPOOL. e.g. TEXFONTS=.;c:\MyFonts
* When e is pressed in response to an error message, brings up editor using default or
  environment variable TEXEDIT, e.g. TEXEDIT="emacs +%d %s"
