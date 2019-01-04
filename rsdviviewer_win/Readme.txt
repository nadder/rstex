A simple windows program to view DVI files.
This is a work in progress, very rudimentary at the moment.
When a font cannot be found it tries to invoke "mf -base=plain \mode=localfont; mag=X; input Y"
so make sure mf is in your path. It assumes the mode localfont has a resolution of 200ppi.
Oh, and it only supports gf files at the moment, pk files should be added "soon".
