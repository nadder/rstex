/*

Copyright (C) 2018 by Richard Sandberg.

This is the Windows specific version of rsdvitype.
No Unix version yet, should be easy to add though.

*/

#include <cstring>
#include <cstdlib>
#include <io.h>
#include "rsDviType.h"


///////////////////////////////////////////////////////////////////////////
// System specific addition for paths on windows
void set_paths()
{
	char const *envpath;

	if ((envpath = getenv("TEXFONTS")) != NULL)
		font_path = envpath;
	else font_path = default_font_path;
}

void pack_real_name_of_file(char const**cpp)
{
	char const*p;
	char *real_name;

	real_name = &real_name_of_file[1];
	if ((p = *cpp) != NULL) {
		while ((*p != ';') && (*p != 0)) {
			*real_name++ = *p++;
			if (real_name == &real_name_of_file[name_length-1])
				break;
		}
		if (*p == 0) *cpp = NULL;
		else *cpp = p + 1;
		*real_name++ = '\\';
	}
	/* now append curname to realname... */
	p = cur_name.get_c_str();
	
	while (*p != 0) {
		if (real_name >= &real_name_of_file[name_length-1]) {
			fprintf(stderr, "! Full file name is too long\n");
			break;
		}
		*real_name++ = *p++;
	}
	*real_name = 0;
}

bool test_access(int filepath)
{
	bool ok;
	char const *cur_path_place;

	switch (filepath) {
	case no_file_path: cur_path_place = NULL; break;
	case font_file_path: cur_path_place = font_path; break;
	default:
		fprintf(stderr, "! This should not happen, test_access\n");
		exit(1);
		break;
	}

	if (cur_name[1] == '\\' || (isalpha(cur_name[1]) && cur_name[2] == ':'))
		cur_path_place = NULL;
	do {
		pack_real_name_of_file(&cur_path_place);
		if (_access(real_name_of_file.get_c_str(), 0) == 0)
			ok = true;
		else
			ok = false;
	} while (!ok && cur_path_place != NULL);

	return ok;
}

///////////////////////////////////////////////////////////////////////////


int myabs(int x)
{
	// overflow check
	if (x == INT_MIN && -(INT_MIN + 1) == INT_MAX) {
		printf("Overflow myabs.\n");
		exit(1);
	}
	///////////////////////

	return x >= 0 ? x : -x;
}



void input_ln() //{inputs a line from the terminal}
{
	int k; //:0..terminal_line_length;
	//update_terminal(); //reset(term_in);

	k = 0;
	int c;
	while ((c=fgetc(stdin)) != '\n' && k < terminal_line_length) {

		buffer[k] = xord[(unsigned char)c];
		incr(k);
	}
	buffer[k]=32;
}


void jump_out()
{
	// goto final_end
	exit(0);
}

void initialize() //{this procedure gets things started properly}
{
	int i; //{loop index for initializations}
	
	set_paths(); //{read environment, to find TEXFONTS, if there}
	print_ln(banner);


	//@<Set initial values@>
	for (i=0; i<=31; i++) xchr[i]='?';
	xchr[32]=' ';xchr[33]='!';
	xchr[34]='\"';xchr[35]='#';xchr[36]='$';xchr[37]='%';xchr[38]='&';
	xchr[39]='\'';xchr[40]='(';xchr[41]=')';xchr[42]='*';xchr[43]='+';
	xchr[44]=',';xchr[45]='-';xchr[46]='.';xchr[47]='/';xchr[48]='0';
	xchr[49]='1';xchr[50]='2';xchr[51]='3';xchr[52]='4';xchr[53]='5';
	xchr[54]='6';xchr[55]='7';xchr[56]='8';xchr[57]='9';xchr[58]=':';
	xchr[59]=';';xchr[60]='<';xchr[61]='=';xchr[62]='>';xchr[63]='?';
	xchr[64]='@';xchr[65]='A';xchr[66]='B';xchr[67]='C';xchr[68]='D';
	xchr[69]='E';xchr[70]='F';xchr[71]='G';xchr[72]='H';xchr[73]='I';
	xchr[74]='J';xchr[75]='K';xchr[76]='L';xchr[77]='M';xchr[78]='N';
	xchr[79]='O';xchr[80]='P';xchr[81]='Q';xchr[82]='R';xchr[83]='S';
	xchr[84]='T';xchr[85]='U';xchr[86]='V';xchr[87]='W';xchr[88]='X';
	xchr[89]='Y';xchr[90]='Z';xchr[91]='[';xchr[92]='\\';xchr[93]=']';
	xchr[94]='^';xchr[95]='_';xchr[96]='`';xchr[97]='a';xchr[98]='b';
	xchr[99]='c';xchr[100]='d';xchr[101]='e';xchr[102]='f';
	xchr[103]='g';xchr[104]='h';xchr[105]='i';xchr[106]='j';
	xchr[107]='k';xchr[108]='l';xchr[109]='m';xchr[110]='n';
	xchr[111]='o';xchr[112]='p';xchr[113]='q';xchr[114]='r';
	xchr[115]='s';xchr[116]='t';xchr[117]='u';xchr[118]='v';
	xchr[119]='w';xchr[120]='x';xchr[121]='y';xchr[122]='z';
	xchr[123]='{';xchr[124]='|';xchr[125]='}';xchr[126]='~';
	for (i=127; i<=255; i++) xchr[i]='?';
	for (i=0; i<=127; i++) xord[i]=32;
	for (i=32; i<=126; i++) xord[xchr[i]]=i;
	nf=0;width_ptr=0;
	font_name[0]=1;font_space[max_fonts]=0;font_bc[max_fonts]=1;
	font_ec[max_fonts]=0;
	out_mode=the_works;max_pages=1000000;
	resolution=300.0;
	new_mag = 0;
	start_vals=0;start_there[0]=false;
	in_postamble=false;
	//strcpy(default_directory.get_c_str(), default_directory_name);
	text_ptr=0;
	max_v=2147483548;max_h=2147483548;max_s=stack_size+1;
	max_v_so_far=0;max_h_so_far=0;max_s_so_far=0;page_count=0;
	old_backpointer=-1;started=false;

	//@<Set initial values@>
}

int get_integer()
{
	int x; //{accumulates the value}
	bool negative; //{should the value be negated?}
	if (buffer[buf_ptr]== 45/*"-"*/)
	{ 
		negative=true; incr(buf_ptr);
	}
	else negative=false;
	x=0;
	while (buffer[buf_ptr]>= 48 && buffer[buf_ptr]<=57)
	{ 
		x=10*x+buffer[buf_ptr]-48; incr(buf_ptr);
	}
	if (negative) return -x; else return x;
}

void print_selected_options()
{
	char temp_buf[300];
	//@<Print all the selected options@>;
	printf("Options selected:\n");
	print("  Starting page = ");
	for (k=0; k<=start_vals; k++)
	{ 
		if (start_there[k]) {sprintf(temp_buf, "%d", start_count[k]);print(temp_buf);}
		else print("*");
		if (k<start_vals) print(".");
		else print_ln(" ");
	}
	sprintf(temp_buf, "  Maximum number of pages = %d", max_pages);
	print_ln(temp_buf);
	sprintf(temp_buf, "  Output level = %d", out_mode);
	print(temp_buf);
	switch(out_mode) {
		case errors_only: 
			print_ln(" (showing bops, fonts, and error messages only)");
			break;
		case terse: 
			print_ln(" (terse)");
			break;
		case mnemonics_only: 
			print_ln(" (mnemonics)");
			break;
		case verbose: 
			print_ln(" (verbose)");
			break;
		case the_works: 
			if (random_reading) print_ln(" (the works)");
			else { 
				out_mode=verbose;
				print_ln(" (the works: same as level 3 in this DVItype)");
			}
			break;
	}
	sprintf(temp_buf, "  Resolution = %12.8f pixels per inch", resolution);
	print_ln(temp_buf);
	if (new_mag>0) {sprintf(temp_buf, "  New magnification factor = %8.3f", (double)new_mag/1000); print_ln(temp_buf);}
	//@<Print all the selected options@>;
}


void dialog()
{
	char temp_buf[300];
	int k; //{loop variable}
	//rewrite(term_out); {prepare the terminal for output}
	//print_ln(banner);

	// get dvi file name
	printf("DVI file name: ");
	fgets(cur_name.get_c_str(), name_length, stdin);
	size_t len = strlen(cur_name.get_c_str());
	if (cur_name[len] == '\n') cur_name[len] = 0;


	//@<Determine the desired |out_mode|@>;
label1: 
	printf("\nOutput level (default=4, ? for help): ");
	out_mode=the_works; input_ln();
	if (buffer[0]!= 32)
		if (buffer[0]>= 48 && buffer[0]<=52) out_mode=buffer[0]-48;
	else { 
			printf("Type 4 for complete listing,");
			printf(" 0 for errors and fonts only,");
			printf(" 1 or 2 or 3 for something in between.\n");
			goto label1;
	}
	//@<Determine the desired |out_mode|@>;

	//@<Determine the desired |start_count| values@>;
label2: 
	printf("Starting page (default=*): ");
	start_vals=0; start_there[0]=false;
	input_ln(); buf_ptr=0; k=0;
	if (buffer[0]!= 32)
		do { 
			if (buffer[buf_ptr]==42) // "*" then
			{ 
				start_there[k]=false; incr(buf_ptr);
			}
			else { 
				start_there[k]=true; start_count[k]=get_integer();
			}
			if (k<9 && buffer[buf_ptr]==46 /*"."*/)
			{ 
				incr(k); incr(buf_ptr);
			}
			else if (buffer[buf_ptr]==32) start_vals=k;
			else { 
				printf("Type, e.g., 1.*.-5 to specify the ");
				printf("first page with \\count0=1, \\count2=-5.\n");
				goto label2;
			}
		} while (!(start_vals==k));
	//@<Determine the desired |start_count| values@>;

	//@<Determine the desired |max_pages|@>;
	label3: 
	printf("Maximum number of pages (default=1000000): ");
	max_pages=1000000; input_ln(); buf_ptr=0;
	if (buffer[0]!= 32)
	{ 
		max_pages=get_integer();
		if (max_pages<=0)
		{ 
			printf("Please type a positive number.\n");
			goto label3;
		}
	}
	//@<Determine the desired |max_pages|@>;

	//@<Determine the desired |resolution|@>;
label4: 
	printf("Assumed device resolution");
	printf(" in pixels per inch (default=300/1): ");
	resolution=300.0; input_ln(); buf_ptr=0;
	if (buffer[0]!=32)
	{ 
		k=get_integer();
		if (k>0 && buffer[buf_ptr]==47 /*"/"*/&&
			buffer[buf_ptr+1] > 48 /*"0"*/ && buffer[buf_ptr+1] <= 57 /*"9"*/)
		{ 
			incr(buf_ptr); resolution=(float)((double)k/get_integer());
		}
		else { 
			printf("Type a ratio of positive integers;");
			printf(" (1 pixel per mm would be 254/10).\n");
			goto label4;
		}
	}
	//@<Determine the desired |resolution|@>;

	//@<Determine the desired |new_mag|@>;
label5: 
	printf("New magnification (default=0 to keep the old one): ");
	new_mag=0; input_ln(); buf_ptr=0;
	if (buffer[0]!=32)
		if (buffer[0]>=48 && buffer[0]<= 57) new_mag=get_integer();
		else { 
			printf("Type a positive integer to override ");
			printf("the magnification in the DVI file.\n");
			goto label5;
		}
	//@<Determine the desired |new_mag|@>;

	print_selected_options();


}

void open_dvi_file() //{prepares to write packed bytes in |dvi_file|}
{
	dvi_file = NULL;
	if (test_access(no_file_path)) {
		strcpy(dvi_name.get_c_str(), real_name_of_file.get_c_str());
		dvi_file = fopen(dvi_name.get_c_str(), "rb");
	}

	if (!dvi_file) {
		printf("DVI file %s not found\n", dvi_name.get_c_str());
		jump_out();
	}
}

void open_tfm_file() //{prepares to read packed bytes in |tfm_file|}
{
	tfm_file = NULL;
	if (test_access(font_file_path))
		tfm_file = fopen(real_name_of_file.get_c_str(), "rb");

	if (!tfm_file) {
		char temp_buf[300];
		sprintf(temp_buf, "\nTFM file not found: %s", cur_name.get_c_str());
		print_ln(temp_buf);
		jump_out();
	}
}

int get_byte() //{returns the next byte, unsigned}
{
	eight_bits b;
	int c;
	c = fgetc(dvi_file);
	if (c == EOF)
		return 0;
	
	b = c;
	incr(cur_loc); 
	return b;
}

int signed_byte() //{returns the next byte, signed}
{
	eight_bits b;
	b = fgetc(dvi_file); incr(cur_loc);
	if (b<128) return b; else return b-256;
}


int get_two_bytes() //{returns the next two bytes, unsigned}
{
	eight_bits a, b;
	a = fgetc(dvi_file); b = fgetc(dvi_file);
	cur_loc=cur_loc+2;
	return a*256+b;
}

int signed_pair() //{returns the next two bytes, signed}
{
	eight_bits a,b;
	a = fgetc(dvi_file); b = fgetc(dvi_file);
	cur_loc=cur_loc+2;
	if (a<128) return a*256+b;
	else return (a-256)*256+b;
}



int get_three_bytes() //{returns the next three bytes, unsigned}
{
	eight_bits a,b,c;
	a = fgetc(dvi_file); b = fgetc(dvi_file); c = fgetc(dvi_file);
	cur_loc=cur_loc+3;
	return (a*256+b)*256+c;
}

int signed_trio() //{returns the next three bytes, signed}
{
	eight_bits a,b,c;
	a = fgetc(dvi_file); b = fgetc(dvi_file); c = fgetc(dvi_file);
	cur_loc=cur_loc+3;
	if (a<128) return (a*256+b)*256+c;
	else return ((a-256)*256+b)*256+c;
}


int signed_quad() //{returns the next four bytes, signed}
{
	eight_bits a,b,c,d;
	a = fgetc(dvi_file); b = fgetc(dvi_file); c = fgetc(dvi_file); d = fgetc(dvi_file);
	cur_loc=cur_loc+4;
	if (a<128) return ((a*256+b)*256+c)*256+d;
	else return (((a-256)*256+b)*256+c)*256+d;
}


void read_tfm_word()
{
	b0 = fgetc(tfm_file);
	b1 = fgetc(tfm_file);
	b2 = fgetc(tfm_file);
	b3 = fgetc(tfm_file);
}




int dvi_length()
{ 
	fseek(dvi_file, 0, SEEK_END);
	return (int)ftell(dvi_file);
}

void move_to_byte(int n)
{
	fseek(dvi_file, n, SEEK_SET);
	cur_loc=n;
}

bool in_TFM(int z)//{input \.{TFM} data or return |false|}
{
	//label 9997, //{go here when the format is bad}
	//9998,  {go here when the information cannot be loaded}
	//9999;  {go here to exit}
	int k; //{index for loops}
	int lh; //{length of the header data, in four-byte words}
	int nw; //{number of words in the width table}
	int wp; //:0..max_widths; //{new value of |width_ptr| after successful input}
	int alpha,beta; //{quantities used in the scaling computation}

	//@<Read past the header data; |goto 9997| if there is a problem@>;
	read_tfm_word(); lh=b2*256+b3;
	read_tfm_word(); font_bc[nf]=b0*256+b1; font_ec[nf]=b2*256+b3;
	if (font_ec[nf]<font_bc[nf]) font_bc[nf]=font_ec[nf]+1;
	if (width_ptr+font_ec[nf]-font_bc[nf]+1>max_widths)
	{ 
		print_ln("---not loaded, DVItype needs larger width table");

		goto label9998;
	}
	wp=width_ptr+font_ec[nf]-font_bc[nf]+1;
	read_tfm_word(); nw=b0*256+b1;
	if (nw==0 || nw>256) 
		goto label9997;
	for (k=1; k<=3+lh; k++)
	{ 
		if (feof(tfm_file)) 
			goto label9997;
		read_tfm_word();
		if (k==4)
			if (b0<128) tfm_check_sum=((b0*256+b1)*256+b2)*256+b3;
			else tfm_check_sum=(((b0-256)*256+b1)*256+b2)*256+b3;
		else if (k==5)
			if (b0<128)
				tfm_design_size=(int)round(tfm_conv*(((b0*256+b1)*256+b2)*256+b3));
			else 
				goto label9997;
	}
	//@<Read past the header data; |goto 9997| if there is a problem@>;

	//@<Store character-width indices at the end of the |width| table@>;
	if (wp>0) 
		for (k=width_ptr; k<=wp-1; k++)
		{ 
			read_tfm_word();
			if (b0>nw) 
				goto label9997;
			width[k]=b0;
		}
	//@<Store character-width indices at the end of the |width| table@>;

	//@<Read and convert the width values, setting up the |in_width| table@>;

	//@<Replace |z| by $|z|^\prim0e$ and compute $\alpha,\beta$@>;
	alpha=16;
	while (z>=040000000)
	{
		z=z / 2; alpha=alpha+alpha;
	}
	beta=256 / alpha; alpha=alpha*z;
	//@<Replace |z| by $|z|^\prime$ and compute $\alpha,\beta$@>;
	for (k=0; k<=nw-1; k++)
	{ 
		read_tfm_word();
		in_width[k]=(((((b3*z)/0400)+(b2*z))/0400)+(b1*z))/ beta;
		if (b0>0) 
			if (b0<255) 
				goto label9997;
			else in_width[k]=in_width[k]-alpha;
	}
	//@<Read and convert the width values, setting up the |in_width| table@>;

	//@<Move the widths from |in_width| to |width|, and append |pixel_width| values@>;
	if (in_width[0]!=0) 
		goto label9997; //{the first width should be zero}
	width_base[nf]=width_ptr-font_bc[nf];
	if (wp>0) 
		for (k=width_ptr; k<=wp-1; k++)
			if (width[k]==0)
			{ 
				width[k]=invalid_width; pixel_width[k]=0;
			}
			else { 
				width[k]=in_width[width[k]];
				pixel_width[k]=(int)pixel_round(width[k]);
			}
	//@<Move the widths from |in_width| to |width|, and append |pixel_width| values@>;
	width_ptr=wp; return true;
label9997: 
	print_ln("---not loaded, TFM file is bad");

label9998: 
	return false;
//label9999: 
}

void print_font(int f) //{|f| is an internal font number}
{
	int k; //:0..name_size; {index into |names|}
	if (f==invalid_font) print("UNDEFINED!");
	else { 
		for (k=font_name[f]; k<=font_name[f+1]-1; k++)
			print_c(xchr[names[k]]);
	}
}

void define_font(int e) //{|e| is an external font number}
{
	char temp_buf[300];
	int f; //:0..max_fonts;
	int p; //{length of the area/directory spec}
	int n; //{length of the font name proper}
	int c,q,d,m; //{check sum, scaled size, design size, magnification}
	int r; //:0..name_length; //{index into |cur_name|}
	int j,k; //:0..name_size; //{indices into |names|}
	bool mismatch; //{do names disagree?}
	if (nf==max_fonts)
	{
		sprintf(temp_buf, "DVItype capacity exceeded (max fonts=%d)!", max_fonts);
		abort(temp_buf);
	}
	font_num[nf]=e; f=0;
	while (font_num[f]!=e) incr(f);
	//@<Read the font parameters into position for font |nf|, and print the font name@>;
	c=signed_quad(); font_check_sum[nf]=c;
	q=signed_quad(); font_scaled_size[nf]=q;
	d=signed_quad(); font_design_size[nf]=d;
	if (q<=0 || d<=0) m=1000;
	else m=(int)round((1000.0*conv*q)/(true_conv*d));
	p=get_byte(); n=get_byte();
	if (font_name[nf]+n+p>name_size)
	{
		sprintf(temp_buf, "DVItype capacity exceeded (name size=%d)!", name_size);
		abort(temp_buf);
	}
	font_name[nf+1]=font_name[nf]+n+p;
	if (showing) print(": ");
	//{when |showing| is true, the font number has already been printed}
	else {sprintf(temp_buf, "Font %d: ", e); print(temp_buf);}
	if (n+p==0) print("null font name!");
	else for (k=font_name[nf]; k<=font_name[nf+1]-1; k++) names[k]=get_byte();
	print_font(nf);
	if (!showing) 
		if (m!=1000) {sprintf(temp_buf, " scaled %d", m); print(temp_buf);}
	//@<Read the font parameters into position for font |nf|, and print the font name@>;

	if (((out_mode==the_works) && in_postamble)|| ((out_mode<the_works)&& !in_postamble))
	{ 
		if (f<nf) print_ln("---this font was already defined!");
	}
	else { 
		if (f==nf) print_ln("---this font wasn\'t loaded before!");
	}
	if (f==nf) 
		//@<Load the new font, unless there are problems@>
	{ 
		//@<Move font name into the |cur_name| string@>;
		//for (k=1; k<=name_length; k++) cur_name[k]=' ';
		//if (p==0)
		//{ 
			//for (k=1 to default_directory_name_length do
			//cur_name[k]:=default_directory[k];
			//r:=default_directory_name_length;
		//}
		//else 
		r=0;
		for (k=font_name[nf]; k<=font_name[nf+1]-1; k++)
		{ 
			incr(r);
			if (r+5>name_length)
			{
				sprintf(temp_buf, "DVItype capacity exceeded (max font name length=%d)!", name_length);
				abort(temp_buf);
			}
			if (names[k]>=97 && names[k]<=122)
				cur_name[r]=xchr[names[k]-040];
			else cur_name[r]=xchr[names[k]];
		}
		cur_name[r+1]='.'; cur_name[r+2]='t'; cur_name[r+3]='f'; cur_name[r+4]='m';
		cur_name[r+5] = 0;
		//@<Move font name into the |cur_name| string@>;
		open_tfm_file();
		if (feof(tfm_file))
			print("---not loaded, TFM file can\'t be opened!");
		else {
			if (q<=0 || q>=01000000000)
			{
				sprintf(temp_buf, "---not loaded, bad scale (%d)!", q);
				print(temp_buf);
			}
			else if (d<=0 || d>=01000000000)
			{
				sprintf(temp_buf, "---not loaded, bad design size (%d)!", d);
				print(temp_buf);
			}
			else if (in_TFM(q)) 
				//@<Finish loading the new font info@>;
			{ 
				font_space[nf]=q / 6; //{this is a 3-unit ``thin space''}
				if (c!=0 && tfm_check_sum!=0 && c!=tfm_check_sum)
				{ 
					print_ln("---beware: check sums do not agree!");
					sprintf(temp_buf, "   (%d vs. %d)", c, tfm_check_sum);
					print_ln(temp_buf);
					print("   ");
				}
				if (myabs(tfm_design_size-d)>2)
				{ 
					print_ln("---beware: design sizes do not agree!");
					sprintf(temp_buf, "   (%d vs. %d)", d, tfm_design_size);
					print_ln(temp_buf);
					print("   ");
				}
				sprintf(temp_buf, "---loaded at size %d DVI units", q);
				print(temp_buf);
				d=(int)round((100.0*conv*q)/(true_conv*d));
				if (d!=100)
				{ 
					print_ln(" "); 
					sprintf(temp_buf, " (this font is magnified %d%%)", d);
					print(temp_buf);
				}
				incr(nf); //{now the new font is officially present}
			}
				//@<Finish loading the new font info@>;
		}
		fclose(tfm_file);
		if (out_mode==errors_only) print_ln(" ");
	}
		//@<Load the new font, unless there are problems@>
	else
		//@<Check that the current font definition matches the old one@>;
	{ 
		if (font_check_sum[f]!=c)
			print_ln("---check sum doesn\'t match previous definition!");
		if (font_scaled_size[f]!=q)
			print_ln("---scaled size doesn\'t match previous definition!");
		if (font_design_size[f]!=d)
			print_ln("---design size doesn\'t match previous definition!");
		j=font_name[f]; k=font_name[nf];
		if (font_name[f+1]-j!=font_name[nf+1]-k) mismatch=true;
		else { 
			mismatch=false;
			while (j<font_name[f+1])
			{ 
				if (names[j]!=names[k]) mismatch=true;
				incr(j); incr(k);
			}
		}
		if (mismatch) print_ln("---font name doesn\'t match previous definition!");
	}
		//@<Check that the current font definition matches the old one@>;
}

int first_par(eight_bits o)
{
	int ret_val = 0;
	switch(o) {
		case sixty_four_cases(set_char_0): case sixty_four_cases(set_char_0+64):
			ret_val=o-set_char_0;
			break;
		case set1: case put1: case fnt1: case xxx1: case fnt_def1: 
			ret_val=get_byte();
			break;
		case set1+1: case put1+1: case fnt1+1: case xxx1+1: case fnt_def1+1: 
			ret_val=get_two_bytes();
			break;
		case set1+2: case put1+2: case fnt1+2: case xxx1+2: case fnt_def1+2: 
			ret_val=get_three_bytes();
			break;
		case right1: case w1: case x1: case down1: case dvi_y1: case z1: 
			ret_val=signed_byte();
			break;
		case right1+1: case w1+1: case x1+1: case down1+1: case dvi_y1+1: case z1+1: 
			ret_val=signed_pair();
			break;
		case right1+2: case w1+2: case x1+2: case down1+2: case dvi_y1+2: case z1+2: 
			ret_val=signed_trio();
			break;
		case set1+3: case set_rule: case put1+3: case put_rule: case right1+3: case w1+3: case x1+3: case down1+3: case dvi_y1+3: case z1+3:
		case fnt1+3: case xxx1+3: case fnt_def1+3: 
			ret_val=signed_quad();
			break;
		case nop: case bop: case eop: case push: case pop: case pre: case post: case post_post: case undefined_commands: 
			ret_val=0;
			break;
		case w0: 
			ret_val=w;
			break;
		case x0: 
			ret_val=x;
			break;
		case dvi_y0: 
			ret_val=y;
			break;
		case z0: 
			ret_val=z;
			break;
		case sixty_four_cases(fnt_num_0): 
			ret_val=o-fnt_num_0;
			break;
	}
	return ret_val;
}


void read_postamble()
{
	char temp_buf[300];
	int k; //{loop index}
	int p, q, m; //{general purpose registers}
	showing=false; post_loc=cur_loc-5;
	sprintf(temp_buf, "Postamble starts at byte %d.", post_loc);
	print_ln(temp_buf);
	if (signed_quad()!=numerator)
		print_ln("numerator doesn\'t match the preamble!");
	if (signed_quad()!=denominator)
		print_ln("denominator doesn\'t match the preamble!");
	if (signed_quad()!=mag) 
		if (new_mag==0)
			print_ln("magnification doesn\'t match the preamble!");
	max_v=signed_quad(); max_h=signed_quad();
	sprintf(temp_buf, "maxv=%d, maxh=%d", max_v, max_h);
	print(temp_buf);
	max_s=get_two_bytes(); total_pages=get_two_bytes();
	sprintf(temp_buf, ", maxstackdepth=%d, totalpages=%d", max_s, total_pages);
	print_ln(temp_buf);
	if (out_mode<the_works)
		//@<Compare the \\{lust} parameters with the accumulated facts@>;
	{ 
		if (max_v+99<max_v_so_far)
		{
			sprintf(temp_buf, "warning: observed maxv was %d", max_v_so_far);
			print_ln(temp_buf);
		}
		if (max_h+99<max_h_so_far) 
		{
			sprintf(temp_buf, "warning: observed maxh was %d", max_h_so_far);
			print_ln(temp_buf);
		}
		if (max_s<max_s_so_far) 
		{
			sprintf(temp_buf, "warning: observed maxstackdepth was %d", max_s_so_far);
			print_ln(temp_buf);
		}
		if (page_count!=total_pages)
		{
			sprintf(temp_buf, "there are really %d pages, not %d!", page_count, total_pages);
			print_ln(temp_buf);
		}
	}
		//@<Compare the \\{lust} parameters with the accumulated facts@>;


	//@<Process the font definitions of the postamble@>;
	do {
		k=get_byte();
		if (k>=fnt_def1 && k<fnt_def1+4)
		{ 
			p=first_par(k); define_font(p); print_ln(" "); k=nop;
		}
	} while (!(k!=nop));
	if (k!=post_post)
	{
		sprintf(temp_buf, "byte %d is not postpost!", cur_loc-1);
		print_ln(temp_buf);
	}
	//@<Process the font definitions of the postamble@>;
	
	//@<Make sure that the end of the file is well-formed@>;
	q=signed_quad();
	if (q!=post_loc)
	{
		sprintf(temp_buf, "bad postamble pointer in byte %d!", cur_loc-4);
		print_ln(temp_buf);
	}
	m=get_byte();
	if (m!=id_byte) 
	{
		sprintf(temp_buf, "identification in byte %d should be %d!", cur_loc-1, id_byte);
		print_ln(temp_buf);
	}
	k=cur_loc; m=223;
	while (m==223) m=get_byte();
	if (!feof(dvi_file))
	{
		sprintf(temp_buf, "signature in byte %d should be 223", cur_loc-1);
		bad_dvi(temp_buf);
	}
	else if (cur_loc<k+4)
	{
		sprintf(temp_buf, "not enough signature bytes at end of file (%d)", cur_loc-k);
		print_ln(temp_buf);
	}


	//@<Make sure that the end of the file is well-formed@>;

}

bool start_match()
{
	int k; //:0..9;  {loop index}
	bool match; //{does everything match so far?}
	match=true;
	for (k=0; k<=start_vals; k++)
		if (start_there[k]&& start_count[k]!=count[k]) match=false;
	return match;
}

void skip_pages(bool bop_seen)
{

	//label 9999; {end of this subroutine}
	int p; //{a parameter}
	int k; //:0..255; //{command code}
	int down_the_drain; //{garbage}
	showing=false;
	while (true)
	{ 
		if (!bop_seen)
		{ 
			scan_bop();
			if (in_postamble) return;
			if (!started) 
				if (start_match())
				{ 
					started=true; 
					return;
				}
		}
		//@<Skip until finding |eop|@>;
		do { 
			if (feof(dvi_file)) bad_dvi("the file ended prematurely");
			k=get_byte();
			p=first_par(k);
			switch(k) {
				case set_rule: case put_rule: 
					down_the_drain=signed_quad();
					break;
				case four_cases(fnt_def1): 
					define_font(p);
					print_ln(" ");
					break;
				case four_cases(xxx1): 
					while (p>0)
					{ 
						down_the_drain=get_byte(); decr(p);
					}
					break;
				case bop: case pre: case post:case post_post: case undefined_commands:
					{
						char temp_buf[300];
						sprintf(temp_buf, "illegal command at byte %d", cur_loc-1);
						bad_dvi(temp_buf);
					}
					break;
				default: do_nothing break;
			}
		} while (!(k==eop));
		//@<Skip until finding |eop|@>;
		bop_seen=false;
	}
//label9999:
	;
}

void flush_text()
{
	int k; //:0..line_length; {index into |text_buf|}
	if (text_ptr>0)
	{
		if (out_mode>errors_only)
		{ 
			print("[");
			for (k=1; k<=text_ptr; k++) print_c(xchr[text_buf[k]]);
			print_ln("]");
		}
		text_ptr=0;
	}
}


void out_text(ASCII_code c)
{
	if (text_ptr==line_length-2) flush_text();
	incr(text_ptr); text_buf[text_ptr]=c;
}

int rule_pixels(int x)
  //{computes $\lceil|conv|\cdot x\rceil$}
{
	int n;
	n=(int)trunc(conv*x);
	if (n<conv*x) return n+1; else return n;
}

bool special_cases(eight_bits o, int p, int a)
{
	char temp_buf[300];
	//label change_font,move_down,done,9998;
	int q; //{parameter of the current command}
	int k; //{loop index}
	bool bad_char; //{has a non-ASCII character code appeared in this \\{xxx}?}
	bool pure; //{is the command error-free?}
	int vvv; //{|v|, rounded to the nearest pixel}
	pure=true;
	switch(o) {
		//@<Cases for vertical motion@>@;
	case four_cases(down1):
		sprintf(temp_buf, "down%d", o-down1+1);
		out_vmove(temp_buf);
		break;
	case dvi_y0: case four_cases(dvi_y1):
		y=p; 
		sprintf(temp_buf, "y%d", o-dvi_y0);
		out_vmove(temp_buf);
		break;
	case z0: case four_cases(z1):
		z=p; 
		sprintf(temp_buf, "z%d", o-z0);
		out_vmove(temp_buf);
		break;
		//@<Cases for vertical motion@>@;

		//@<Cases for fonts@>@;
	case sixty_four_cases(fnt_num_0):
		sprintf(temp_buf, "fntnum%d", p);
		major(temp_buf);
		goto change_font;
		break;
	case four_cases(fnt1): 
		sprintf(temp_buf, "fnt%d %d", o-fnt1+1, p);
		major(temp_buf);
		goto change_font;
		break;
	case four_cases(fnt_def1): 
		sprintf(temp_buf, "fntdef%d %d", o-fnt_def1+1, p);
		major(temp_buf);
		define_font(p); goto done;
		break;
		//@<Cases for fonts@>@;

	case four_cases(xxx1): 
		//@<Translate an |xxx| command and |goto done|@>;
		major("xxx \'"); bad_char=false;
		if (p<0) error("string of negative length!");
		for (k=1; k<=p; k++)
		{
			q=get_byte();
			if (q<32 or q>126) bad_char=true;
			if (showing) print_c(xchr[q]);
		}
		if (showing) print_c('\'');
		if (bad_char) error("non-ASCII character in xxx command!");
		goto done;
		//@<Translate an |xxx| command and |goto done|@>;
		break;
	case pre: 
		error("preamble command within a page!"); goto label9998;
		break;
	case post: case post_post: 
		error("postamble command within a page!"); goto label9998;
		break;
	default: 
		sprintf(temp_buf, "undefined command %d!", o);
		error(temp_buf);
		goto done;
		break;
	}
move_down: 
	//@<Finish a command that sets |v:=v+p|, then |goto done|@>;
	if (v>0 && p>0) 
		if (v>infinity-p)
		{ 
			sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", p, infinity-v);
			error(temp_buf);
			p=infinity-v;
		}
	if (v<0 && p<0) 
		if (-v>p+infinity)
		{
			sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", p, (-v)-infinity);
			error(temp_buf);
			p=(-v)-infinity;
		}
	vvv=(int)pixel_round(v+p);
	if (myabs(vvv-vv)>max_drift)
		if (vvv>vv) vv=vvv-max_drift;
		else vv=vvv+max_drift;
	if (showing) 
		if (out_mode>mnemonics_only)
		{ 
			sprintf(temp_buf, " v:=%d", v);
			print(temp_buf);
			if (p>=0) print("+");
			sprintf(temp_buf, "%d=%d, vv:=%d", p, v+p, vv);
			print(temp_buf);
		}
	v=v+p;
	if (myabs(v)>max_v_so_far)
	{ 
		if (myabs(v)>max_v+99)
		{ 
			sprintf(temp_buf, "warning: |v|>%d!", max_v);
			error(temp_buf);
			max_v=myabs(v);
		}
		max_v_so_far=myabs(v);
	}
	goto done;
	//@<Finish a command that sets |v:=v+p|, then |goto done|@>;
change_font: 
	//@<Finish a command that changes the current font, then |goto done|@>;
	font_num[nf]=p; cur_font=0;
	while (font_num[cur_font]!=p) incr(cur_font);
	if (cur_font==nf)
	{ 
		cur_font=invalid_font;
		sprintf(temp_buf, "invalid font selection: font %d was never defined!", p);
		error(temp_buf);
	}
	if (showing) 
		if (out_mode>mnemonics_only)
		{ 
			print(" current font is "); print_font(cur_font);
		}
	goto done;
	//@<Finish a command that changes the current font, then |goto done|@>;
label9998: 
	pure=false;
done: 
	return pure;
}


bool do_page()
{
	char temp_buf[300];
	//label fin_set,fin_rule,move_right,show_state,done,9998,9999;
	eight_bits o; //{operation code of the current command}
	int p, q; //{parameters of the current command}
	int a; //{byte number of the current command}
	int hhh; //{|h|, rounded to the nearest pixel}
	cur_font=invalid_font; //{set current font undefined}
	s=0; h=0; v=0; w=0; x=0; y=0; z=0; hh=0; vv=0;
	//{initialize the state variables}
	while (true)
		//@<Translate the next command in the \.{DVI} file; |goto 9999| with |do_page=true| if it was |eop|; |goto 9998| if premature termination is needed@>;
	{ 
		a=cur_loc; showing=false;
		o=get_byte(); p=first_par(o);
		if (feof(dvi_file)) bad_dvi("the file ended prematurely");
		//@<Start translation of command |o| and |goto| the appropriate label to finish the job@>;
		if (o<set_char_0+128) 
			//@<Translate a |set_char| command@>
		{ 
			if (o>32 && o<=126)
			{ 
				out_text(p); 
				sprintf(temp_buf, "setchar%d", p);
				minor(temp_buf);
			}
			else {sprintf(temp_buf, "setchar%d", p);major(temp_buf);}
			goto fin_set;
		}
			//@<Translate a |set_char| command@>
		else switch(o) {
			case four_cases(set1): 
				sprintf(temp_buf, "set%d %d", o-set1+1, p);
				major(temp_buf); goto fin_set;
				break;
			case four_cases(put1):
				sprintf(temp_buf, "put%d %d", o-put1+1, p);
				major(temp_buf); goto fin_set;
				break;
			case set_rule:
				major("setrule"); goto fin_rule;
				break;
			case put_rule: 
				major("putrule"); goto fin_rule;
				break;
			//@<Cases for commands |nop|, |bop|, \dots, |pop|@>@;

			case nop: 
				minor("nop"); goto done;
				break;
			case bop: 
				error("bop occurred before eop!"); goto label9998;
				break;
			case eop: 
				major("eop");
				if (s!=0) {
					sprintf(temp_buf, "stack not empty at end of page (level %d)!", s);
					error(temp_buf);
				}
				print_ln(" "); return true;
				break;
			case push: 
				major("push");
				if (s==max_s_so_far)
				{ 
					max_s_so_far=s+1;
					if (s==max_s) error("deeper than claimed in postamble!");
					if (s==stack_size)
					{ 
						sprintf(temp_buf, "DVItype capacity exceeded (stack size=%d)", stack_size);
						error(temp_buf); goto label9998;
					}
				}
				hstack[s]=h; vstack[s]=v; wstack[s]=w;
				xstack[s]=x; ystack[s]=y; zstack[s]=z;
				hhstack[s]=hh; vvstack[s]=vv; incr(s); ss=s-1; goto show_state;
				break;
			case pop: 
				major("pop");
				if (s==0) error("(illegal at level zero)!");
				else {
					decr(s); hh=hhstack[s]; vv=vvstack[s];
					h=hstack[s]; v=vstack[s]; w=wstack[s];
					x=xstack[s]; y=ystack[s]; z=zstack[s];
				}
				ss=s; goto show_state;
				break;


			//@<Cases for commands |nop|, |bop|, \dots, |pop|@>@;

			//@<Cases for horizontal motion@>@;
			case four_cases(right1):
				sprintf(temp_buf, "right%d", o-right1+1);
				out_space(temp_buf);
				break;
			case w0: case four_cases(w1):
				w=p; 
				sprintf(temp_buf, "w%d", o-w0);
				out_space(temp_buf);
				break;
			case x0: case four_cases(x1):
				x=p;
				sprintf(temp_buf, "x%d", o-x0);
				out_space(temp_buf);
				break;
			//@<Cases for horizontal motion@>@;

			default: if (special_cases(o,p,a)) goto done; else goto label9998;
		}
		//@<Start translation of command |o| and |goto| the appropriate label to finish the job@>;
	fin_set: 
		//@<Finish a command that either sets or puts a character, then |goto move_right| or |done|@>;
		if (p<0) p=255-((-1-p) % 256);
		else if (p>=256) p=p % 256; //{width computation for oriental fonts}
		//@^oriental characters@>@^Chinese characters@>@^Japanese characters@>

		if (p<font_bc[cur_font] || p>font_ec[cur_font]) q=invalid_width;
		else q=char_width(cur_font)(p);
		if (q==invalid_width)
		{ 
			sprintf(temp_buf, "character %d invalid in font ", p);
			error(temp_buf);
			print_font(cur_font);
			if (cur_font!=invalid_font)
				print("!"); //{the invalid font has `\.!' in its name}
		}
		if (o>=put1) goto done;
		if (q==invalid_width) q=0;
		else hh=hh+char_pixel_width(cur_font)(p);
		goto move_right;
		//@<Finish a command that either sets or puts a character, then |goto move_right| or |done|@>;
	fin_rule: 
		//@<Finish a command that either sets or puts a rule, then |goto move_right| or |done|@>;
		q=signed_quad();
		if (showing)
		{ 
			sprintf(temp_buf, " height %d, width %d", p, q);
			print(temp_buf);
			if (out_mode>mnemonics_only)
				if (p<=0 || q<=0) print(" (invisible)");
				else {
					sprintf(temp_buf, " (%dx%d pixels)", rule_pixels(p), rule_pixels(q)); 
					print(temp_buf);
				}
		}
		if (o==put_rule) goto done;
		if (showing) 
			if (out_mode>mnemonics_only) 
				print_ln(" ");
		hh=hh+rule_pixels(q); goto move_right;
		//@<Finish a command that either sets or puts a rule, then |goto move_right| or |done|@>;
	move_right: 
		//@<Finish a command that sets |h:=h+q|, then |goto done|@>;
		if (h>0 && q>0) 
			if (h>infinity-q)
			{ 
				sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", q, infinity-h);
				error(temp_buf);
				q=infinity-h;
			}
		if (h<0 && q<0) 
			if (-h>q+infinity)
			{ 
				sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", q, (-h)-infinity);
				error(temp_buf);
				q=(-h)-infinity;
			}
		hhh=(int)pixel_round(h+q);
		if (myabs(hhh-hh)>max_drift)
		if (hhh>hh) hh=hhh-max_drift;
		else hh=hhh+max_drift;
		if (showing) 
			if (out_mode>mnemonics_only)
			{
				sprintf(temp_buf, " h:=%d", h);
				print(temp_buf);
				if (q>=0) print("+");
				sprintf(temp_buf, "%d=%d, hh:=%d", q, h+q, hh);
				print(temp_buf);
			}
		h=h+q;
		if (myabs(h)>max_h_so_far)
		{ 
			if (myabs(h)>max_h+99)
			{ 
				sprintf(temp_buf, "warning: |h|>%d!", max_h);
				error(temp_buf);
				max_h=myabs(h);
			}
			max_h_so_far=myabs(h);
		}
		goto done;
		//@<Finish a command that sets |h:=h+q|, then |goto done|@>;
	show_state: 
		//@<Show the values of |ss|, |h|, |v|, |w|, |x|, |y|, |z|, |hh|, and |vv|; then |goto done|@>;
		if (showing) 
			if (out_mode>mnemonics_only)
			{ 
				print_ln(" ");
				sprintf(temp_buf, "level %d:(h=%d,v=%d,w=%d,x=%d,y=%d,z=%d,hh=%d,vv=%d)", ss, h, v, w, x, y, z, hh, vv);
				print(temp_buf);
			}
		goto done;
		//@<Show the values of |ss|, |h|, |v|, |w|, |x|, |y|, |z|, |hh|, and |vv|; then |goto done|@>;
	done: 
		if (showing) print_ln(" ");
	}
		//@<Translate the next command in the \.{DVI} file; |goto 9999| with |do_page=true| if it was |eop|; |goto 9998| if premature termination is needed@>;
label9998:
	print_ln("!"); 
	return false;
//label9999: 
	;
}

void scan_bop()
{

	char temp_buf[300];
	int k; //:0..255; //{command code}
	do { 
		if (feof(dvi_file)) bad_dvi("the file ended prematurely");
		k=get_byte();
		if (k>=fnt_def1 && k<fnt_def1+4)
		{ 
			define_font(first_par(k)); k=nop;
		}
	} while (!(k!=nop));
	if (k==post) in_postamble=true;
	else { 
		if (k!=bop){ sprintf(temp_buf, "byte %d is not bop", cur_loc-1); bad_dvi(temp_buf);}
		new_backpointer=cur_loc-1; incr(page_count);
		for (k=0; k<=9; k++) count[k]=signed_quad();
		if (signed_quad()!=old_backpointer)
		{
			sprintf(temp_buf, "backpointer in byte %d should be %d!", cur_loc-4, old_backpointer);
			print_ln(temp_buf);
		}

		old_backpointer=new_backpointer;
	}


}

void get_command_line_options(int argc, char *argv[])
{
	while (--argc != 0) {
		if (strncmp(argv[argc], "-output-level=", strlen("-output-level=")) == 0) {
			size_t len = strlen("-output-level=");
			if (strlen(argv[argc]) > len) {
				int digit = xord[argv[argc][len]];
				if (digit >= 48 && digit <= 57)
					out_mode=buffer[0]-48;
			}
		}
		else if (strncmp(argv[argc], "-start-count=",strlen("-start-count=")) == 0) {
			char *pbuf = &argv[argc][strlen("-start-count=")];
			char *pendbuf = argv[argc] + strlen(argv[argc]);
			// put argument in buffer
			buf_ptr = 0;
			while (pbuf != pendbuf && buf_ptr < terminal_line_length) {
				buffer[buf_ptr++] = xord[*pbuf++];
			}
			buffer[buf_ptr] = 32;

			start_vals=0; start_there[0]=false;
			buf_ptr=0; k=0;
			if (buffer[0]!= 32)
				do { 
					if (buffer[buf_ptr]==42) // "*" then
					{ 
						start_there[k]=false; incr(buf_ptr);
					}
					else { 
						start_there[k]=true; start_count[k]=get_integer();
					}
					if (k<9 && buffer[buf_ptr]==46 /*"."*/)
					{ 
						incr(k); incr(buf_ptr);
					}
					else if (buffer[buf_ptr]==32) start_vals=k;
					else { 
						printf("Error, -start-count.\n");
						printf("Type, e.g., 1.*.-5 to specify the ");
						printf("first page with \\count0=1, \\count2=-5.\n");
						jump_out();
					}
				} while (!(start_vals==k));

		}
		else if (strncmp(argv[argc], "-max-pages=",strlen("-max-pages=")) == 0) {
			char *pbuf = &argv[argc][strlen("-max-pages=")];
			char *pendbuf = argv[argc] + strlen(argv[argc]);
			// put argument in buffer
			buf_ptr = 0;
			while (pbuf != pendbuf && buf_ptr < terminal_line_length) {
				buffer[buf_ptr++] = xord[*pbuf++];
			}
			buffer[buf_ptr] = 32;

			buf_ptr=0;
			if (buffer[0]!= 32)
			{ 
				max_pages=get_integer();
				if (max_pages<=0)
				{ 
					printf("Error, -max-pages.\n");
					printf("Please type a positive number.\n");
					jump_out();
				}
			}
		}
		else if (strncmp(argv[argc], "-resolution=",strlen("-resolution=")) == 0) {
			char *pbuf = &argv[argc][strlen("-resolution=")];
			char *pendbuf = argv[argc] + strlen(argv[argc]);
			// put argument in buffer
			buf_ptr = 0;
			while (pbuf != pendbuf && buf_ptr < terminal_line_length) {
				buffer[buf_ptr++] = xord[*pbuf++];
			}
			buffer[buf_ptr] = 32;

			resolution=300.0; buf_ptr=0;
			if (buffer[0]!=32)
			{ 
				k=get_integer();
				if (k>0 && buffer[buf_ptr]==47 /*"/"*/&&
					buffer[buf_ptr+1] > 48 /*"0"*/ && buffer[buf_ptr+1] <= 57 /*"9"*/)
				{ 
					incr(buf_ptr); resolution=(float)((double)k/get_integer());
				}
				else { 
					printf("Error, -resolution.\n");
					printf("Type a ratio of positive integers;");
					printf(" (1 pixel per mm would be 254/10).\n");
					jump_out();
				}
			}



		}
		else if (strncmp(argv[argc], "-mag=",strlen("-mag=")) == 0) {

			char *pbuf = &argv[argc][strlen("-mag=")];
			char *pendbuf = argv[argc] + strlen(argv[argc]);
			// put argument in buffer
			buf_ptr = 0;
			while (pbuf != pendbuf && buf_ptr < terminal_line_length) {
				buffer[buf_ptr++] = xord[*pbuf++];
			}
			buffer[buf_ptr] = 32;
			new_mag=0; input_ln(); buf_ptr=0;
			if (buffer[0]!=32)
				if (buffer[0]>=48 && buffer[0]<= 57) new_mag=get_integer();
				else { 
					printf("Error, -mag.\n");
					printf("Type a positive integer to override ");
					printf("the magnification in the DVI file.\n");
					jump_out();
				}
		}
		else { // assume filename
			cur_name[1] = 0;
			strncat(cur_name.get_c_str(), argv[argc], name_length);
		}
	}
}


int main(int argc, char *argv[])
{
	char temp_buf[300];

	initialize(); //{get all variables initialized}
	if (argc == 2) {
		if (strcmp(argv[1], "/?") == 0 || strcmp(argv[1], "-help") == 0) {
			printf("dvitype [options] dvifile\n\n");
			printf("-output-level\n-start-count\n-max-pages\n-resolution\n-mag\n");
			return 0;
		}
	}

	if (argc >= 2) {
		get_command_line_options(argc, argv);
	}

	if (argc < 2)
		dialog(); //{set up all the options}
	else
		print_selected_options();
	//@<Process the preamble@>;
	open_dvi_file();
	p=get_byte(); //{fetch the first byte}
	if (p!=pre) {sprintf(temp_buf, "First byte isn\'t start of preamble!"); bad_dvi(temp_buf);}
	p=get_byte(); //{fetch the identification byte}
	if (p!=id_byte)
		{sprintf(temp_buf, "identification in byte 1 should be %d!", id_byte); print_ln(temp_buf);/*print_ln('identification in byte 1 should be ',id_byte:1,'!');*/}

	//@<Compute the conversion factors@>;
	numerator=signed_quad(); denominator=signed_quad();
	if (numerator<=0) {sprintf(temp_buf, "numerator is %d", numerator);bad_dvi(temp_buf);}
	if (denominator<=0) {sprintf(temp_buf, "denominator is %d", denominator);bad_dvi(temp_buf);}
	sprintf(temp_buf, "numerator/denominator=%d/%d", numerator, denominator);
	print_ln(temp_buf);
	tfm_conv=(float)((25400000.0/numerator)*(denominator/473628672.0)/16.0);
	conv=(float)((numerator/254000.0)*(resolution/denominator));
	mag=signed_quad();
	if (new_mag>0) mag=new_mag;
	else if (mag<=0) {sprintf(temp_buf, "magnification is %d", mag);bad_dvi(temp_buf);}
	true_conv=conv; conv=(float)(true_conv*(mag/1000.0));
	sprintf(temp_buf, "magnification=%d; %16.8f pixels per DVI unit", mag, conv);
	//print_ln('magnification=',mag:1,'; ',conv:16:8,' pixels per DVI unit')
	print_ln(temp_buf);

	//@<Compute the conversion factors@>;
	p=get_byte(); //{fetch the length of the introductory comment}
	print("\'");
	while (p>0)
	{ 
		decr(p); print_c(xchr[get_byte()]);
	}
	print_ln("\'");
	after_pre=cur_loc;
	//@<Process the preamble@>;
	if (out_mode==the_works) //{|random_reading=true|}
	{ 
		//@<Find the postamble, working back from the end@>;
		n=dvi_length();
		if (n<53) {sprintf(temp_buf, "only %d bytes long", n);bad_dvi(temp_buf);}
		m=n-4;
		do {
			if (m==0) {bad_dvi("all 223s");}
			move_to_byte(m); k=get_byte(); decr(m);
		} while (!(k!=223));
		if (k!=id_byte) {sprintf(temp_buf, "ID byte is %d", k);bad_dvi(temp_buf);}
		move_to_byte(m-3); q=signed_quad();
		if (q<0 || q>m-33) {sprintf(temp_buf, "post pointer %d at byte %d", q,m-3);bad_dvi(temp_buf);}
		move_to_byte(q); k=get_byte();
		if (k!=post) {sprintf(temp_buf, "byte %d is not post", q);bad_dvi(temp_buf);}
		post_loc=q; first_backpointer=signed_quad();
		//@<Find the postamble, working back from the end@>;
		in_postamble=true; read_postamble(); in_postamble=false;
		//@<Count the pages and move to the starting page@>;
		q=post_loc; p=first_backpointer; start_loc=-1;
		if (p<0) in_postamble=true;
		else  { 
			do {
				//{now |q| points to a |post| or |bop| command; |p>=0| is prev pointer}
				if (p>q-46)
					{sprintf(temp_buf, "page link %d after byte %d", p,q);bad_dvi(temp_buf);}
				
				q=p; move_to_byte(q); k=get_byte();
				if (k==bop) incr(page_count);
				else {sprintf(temp_buf, "byte %d is not bop", q);bad_dvi(temp_buf);}
				for (k=0; k<= 9; k++) count[k]=signed_quad();
				p=signed_quad();
				if (start_match())
				{ 
					start_loc=q; old_backpointer=p;
				}
			} while (!(p<0));
			if (start_loc<0) abort("starting page number could not be found!");
			if (old_backpointer<0) start_loc=after_pre; //{we want to check everything}
			move_to_byte(start_loc);
		}
		if (page_count!=total_pages)
			{sprintf(temp_buf, "there are really %d pages, not %d", page_count,total_pages); print_ln(temp_buf);/*print_ln('there are really ',page_count:1,' pages, not ',total_pages:1,'!');*/}
		//@<Count the pages and move to the starting page@>;
	}
	skip_pages(false);
	if (!in_postamble) 
		//@<Translate up to |max_pages| pages@>;
	{ 
		while (max_pages>0)
		{ 
			decr(max_pages);
			print_ln(" "); 
			sprintf(temp_buf, "%d: beginning of page ", cur_loc-45);
			print(temp_buf);
			//print(cur_loc-45:1,': beginning of page ');
			for (k=0; k<=start_vals; k++)
			{ 
				sprintf(temp_buf, "%d", count[k]);
				//print(count[k]:1);
				print(temp_buf);
				if (k<start_vals) print(".");
				else print_ln(" ");
			}
			if (!do_page()) bad_dvi("page ended unexpectedly");
			scan_bop();
			if (in_postamble) goto done;
		}
	done:	;
	}
		//@<Translate up to |max_pages| pages@>;
	if (out_mode<the_works)
	{ 
		if (!in_postamble) skip_pages(true);
		if (signed_quad()!=old_backpointer)
		{ sprintf(temp_buf, "backpointer in byte %d should be %d!", cur_loc-4, old_backpointer); print_ln(temp_buf);/*print_ln('backpointer in byte ',cur_loc-4:1,' should be ',old_backpointer:1,'!');*/}
	  read_postamble();
	}

	fclose(dvi_file);
}
