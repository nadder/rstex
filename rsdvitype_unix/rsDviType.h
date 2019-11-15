/*

Copyright (C) 2018 by Richard Sandberg.

This is the Unix specific version of rsdvitype.

*/
#pragma once

#include <cstdio>
#include <cassert>
#include <array>

#define array_size maxindex - minindex + 1
template<typename T, int minindex, int maxindex>
struct Array
{
	T& operator[](int x) {
		assert(x >= minindex && x <= maxindex);
		return _array[x - minindex];
	}

	T* get_c_str() {
		assert(array_size > 0);
		_array[array_size - 1] = T(0);
		return &_array[0];
	}
	static const int _minindex = minindex;
	static const int _maxindex = maxindex;
	std::array<T, array_size> _array;
};

///////////////////////////////////////////////////////////////////////////
// System specific addition on Unix

#define MAX_OTH_PATH_CHARS 512
#define default_font_path "."

char const *font_path;

const int no_file_path = 0;
const int font_file_path = 3;
///////////////////////////////////////////////////////////////////////////



#define banner "This is rsDVItype, Version 1.0 for Unix" // printed when the program starts


#define random_reading true

#define print_ln(s) printf("%s\n", s)
#define print(s) printf("%s",s)
#define print_c(c) printf("%c",c)
#define incr(n)  n=n+1 //{increase a variable by unity}
#define decr(n)  n=n-1 //{decrease a variable by unity}
#define loop  while(true) //{repeat over and over until a |goto| happens}
#define do_nothing


#define four_cases(s) s: case s+1: case s+2: case s+3
#define eight_cases(s) four_cases(s): case four_cases(s+4)
#define sixteen_cases(s) eight_cases(s): case eight_cases(s+8)
#define thirty_two_cases(s) sixteen_cases(s): case sixteen_cases(s+16)
#define sixty_four_cases(s) thirty_two_cases(s): case thirty_two_cases(s+32)


// Constants
const int max_fonts=100; //{maximum number of distinct fonts per \.{DVI} file}
const int max_widths=10000; //{maximum number of different characters among all fonts}
const int line_length=79; //{bracketed lines of output will be at most this long}
const int terminal_line_length=150; //{maximum number of characters input in a single line of input from the terminal}
const int stack_size=100; //{\.{DVI} files shouldn't |push| beyond this depth}
const int name_size=1000; //{total length of all font file names}
const int name_length=257; //{a file name shouldn't be longer than this}


typedef unsigned char text_char; //{the data type of characters in text files}
const int first_text_char=0; //{ordinal number of the smallest element of |text_char|}
const int last_text_char=255; //{ordinal number of the largest element of |text_char|}

typedef FILE* text_file;

typedef unsigned char ASCII_code; //32..126


Array<ASCII_code, 0, 255> xord; // {specifies conversion of input characters}
Array<text_char, 0, 255> xchr; // {specifies conversion of output characters}
  

#define abort(s) do {\
	fprintf(stdout, " %s\n",s); jump_out();\
} while (false)

#define bad_dvi(n) do {\
	char temp[300];\
	sprintf(temp, "Bad DVI file: %s!",n);\
	abort(temp);\
} while (false)


typedef unsigned char eight_bits; // {unsigned one-byte quantity}
typedef FILE* byte_file; // {files that contain binary data}

byte_file dvi_file; //{the stuff we are \.{DVI}typing}
byte_file tfm_file; //{a font metric file}

int cur_loc; //{where we are about to look, in |dvi_file|}

Array<char, 1, name_length+1> cur_name, real_name_of_file, dvi_name;//{external name}

int h,v,w,x,y,z,hh,vv;//{current state values}

Array<int, 0, stack_size> hstack,vstack,wstack,xstack,ystack,zstack; //{pushed down values in \.{DVI} units}
Array<int, 0, stack_size> hhstack,vvstack; //{pushed down values in pixels}

int max_v; //{the value of |abs(v)| should probably not exceed this}
int max_h; //{the value of |abs(h)| should probably not exceed this}
int max_s; //{the stack depth should not exceed this}
int max_v_so_far,max_h_so_far,max_s_so_far;//{the record high levels}
int total_pages; //{the stated total number of pages}
int page_count; //{the total number of pages seen so far}
int s; //{current stack size}
int ss; //{stack size to print}
int cur_font; //{current internal font number}

bool showing; //{is the current command being translated in full?}
int old_backpointer; //{the previous |bop| command location}
int new_backpointer; //{the current |bop| command location}
bool started; //{has the starting page been found?}
int post_loc; //{byte location where the postamble begins}
int first_backpointer; //{the pointer following |post|}
int start_loc; //{byte location of the first page to process}
int after_pre; //{byte location immediately following the preamble}
int k,m,n,p,q; //{general purpose registers}

Array<int, 0, max_widths> pixel_width; //{actual character widths, in pixels}
float conv; //{converts \.{DVI} units to pixels}
float true_conv; //{converts unmagnified \.{DVI} units to pixels}
int numerator,denominator; //{stated conversion ratio}
int mag; //{magnification factor times 1000}

int out_mode; //:errors_only..the_works; //{controls the amount of output}
int max_pages; //{at most this many |bop..eop| pages will be printed}
float resolution; //{pixels per inch}
int new_mag; //{if positive, overrides the postamble's magnification}

Array<int, 0, 9> start_count; //{count values to select starting page}
Array<bool, 0, 9> start_there; //{is the |start_count| value relevant?}
int start_vals; //:0..9; //{the last count considered significant}
Array<int, 0, 9> count; //:array[0..9] of int; //{the count values on the current page}


Array<ASCII_code, 0, terminal_line_length> buffer; //:array[0..terminal_line_length] of ASCII_code;
#define term_in stdin
//text_file term_in; //{the terminal, considered as an input file}
//text_file term_out; {the terminal, considered as an output file}

int buf_ptr; //:0..terminal_line_length; {the number of characters read}

bool in_postamble; //{are we reading the postamble?}

//#define default_directory_name "TeXfonts:" //{change this to the correct name}
//const int default_directory_name_length=9+1; //{change this to the correct length}


//Array<char, 1, default_directory_name_length> default_directory; //:packed array[1..default_directory_name_length] of char;

int text_ptr; //:0..line_length; //{the number of characters in |text_buf|}
Array<ASCII_code, 1, line_length> text_buf; //:array[1..line_length] of ASCII_code; //{saved characters}

eight_bits b0,b1,b2,b3; //{four bytes input at once}


Array<int, 0, max_fonts> font_num; //:array [0..max_fonts] of integer; //{external font numbers}
Array<int, 0, max_fonts>  font_name; //:array [0..max_fonts] of 1..name_size; //{starting positions of external font names}
Array<ASCII_code, 1, name_size> names; //:array [1..name_size] of ASCII_code; //{characters of names}
Array<int, 0, max_fonts> font_check_sum; //:array [0..max_fonts] of integer; //{check sums}
Array<int, 0, max_fonts> font_scaled_size; //:array [0..max_fonts] of integer; //{scale factors}
Array<int, 0, max_fonts> font_design_size; //:array [0..max_fonts] of integer; //{design sizes}
Array<int, 0, max_fonts> font_space; //:array [0..max_fonts] of integer; //{boundary between ``small'' and ``large'' spaces}
Array<int, 0, max_fonts> font_bc; //:array [0..max_fonts] of integer; //{beginning characters in fonts}
Array<int, 0, max_fonts> font_ec; //:array [0..max_fonts] of integer; //{ending characters in fonts}
Array<int, 0, max_fonts> width_base; //:array [0..max_fonts] of integer; //{index into |width| table}
Array<int, 0, max_widths> width; //:array [0..max_widths] of integer; //{character widths, in \.{DVI} units}
int nf; //:0..max_fonts; //{the number of known fonts}
int width_ptr; //:0..max_widths; //{the number of known character widths}

Array<int, 0, 255> in_width; //:array[0..255] of integer; {\.{TFM} width data in \.{DVI} units}
int tfm_check_sum; //{check sum found in |tfm_file|}
int tfm_design_size; //{design size found in |tfm_file|, in \.{DVI} units}
float tfm_conv; //{\.{DVI} units per absolute \.{TFM} unit}

const int errors_only=0; //{value of |out_mode| when minimal printing occurs}
const int terse=1; //{value of |out_mode| for abbreviated output}
const int mnemonics_only=2; //{value of |out_mode| for medium-quantity output}
const int verbose=3; //{value of |out_mode| for detailed tracing}
const int the_works=4; //{|verbose|, plus check of postamble if |random_reading|}

enum dvi_cmd
{
	set_char_0=0, //{typeset character 0 and move right}
	set1=128, //{typeset a character and move right}
	set_rule=132, //{typeset a rule and move right}
	put1=133, //{typeset a character}
	put_rule=137, //{typeset a rule}
	nop=138, //{no operation}
	bop=139, //{beginning of page}
	eop=140, //{ending of page}
	push=141, //{save the current positions}
	pop=142, //{restore previous positions}
	right1=143, //{move right}
	w0=147, //{move right by |w|}
	w1=148, //{move right and set |w|}
	x0=152, //{move right by |x|}
	x1=153, //{move right and set |x|}
	down1=157, //{move down}
	dvi_y0=161, //{move down by |y|}
	dvi_y1=162, //{move down and set |y|}
	z0=166, //{move down by |z|}
	z1=167, //{move down and set |z|}
	fnt_num_0=171, //{set current font to 0}
	fnt1=235, //{set current font}
	xxx1=239, //{extension to \.{DVI} primitives}
	xxx4=242, //{potentially long extension to \.{DVI} primitives}
	fnt_def1=243, //{define the meaning of a font number}
	pre=247, //{preamble}
	post=248, //{postamble beginning}
	post_post=249, //{postamble ending}

};

#define undefined_commands 250: case 251: case 252: case 253: case 254: case 255

const int id_byte=2; //{identifies the kind of \.{DVI} files described here}

#define char_width_end(s) s]
#define char_width(s) width[width_base[s]+char_width_end
#define invalid_font max_fonts
const int invalid_width = 017777777777;

#define char_pixel_width(s) pixel_width[width_base[s]+char_width_end

#define pixel_round(s) round(conv*(s))

const int infinity=017777777777; // {$\infty$ (approximately)}
const int max_drift=2; //{we insist that abs|(hh-pixel_round(h))<=max_drift|}


#define show(s) do {\
	flush_text(); showing=true;\
	printf("%d: %s", a, s);\
} while (false)

#define _major(s) if (out_mode>errors_only) show(s)
#define _minor(s) do {\
	if (out_mode>terse) {\
		showing=true;\
		printf("%d: %s", a, s);\
	}\
} while (false);

#define error(s) do {\
	if (!showing)\
		show(s);\
	else {\
		printf(" %s", s);\
	}\
} while (false)

#define out_space(s) do {\
	char temp[300];\
	if (p>=font_space[cur_font] || p<=-4*font_space[cur_font])\
    {\
		out_text(32); hh=(int)pixel_round(h+p);\
	}\
	else hh=(int)(hh+pixel_round(p));\
	sprintf(temp, "%s %d", s, p);\
	_minor(temp); q=p; goto move_right;\
} while (false)


#define out_vmove(s) do {\
	char temp[300];\
	if (myabs(p)>=5*font_space[cur_font]) vv=(int)pixel_round(v+p);\
	else vv=(int)(vv+pixel_round(p));\
	sprintf(temp, "%s %d", s, p);\
	_major(temp); goto move_down;\
} while (false)



int first_par(eight_bits o);
void scan_bop();








