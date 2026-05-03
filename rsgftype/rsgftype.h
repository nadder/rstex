/*
Copyright (C) 2026 by Richard Sandberg.
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

#define banner "This is rsGFtype, Version 1.0 (GFtype 3.1)" //{printed when the program starts}

typedef unsigned char ASCII_code; // @!ASCII_code=" ".."~"; {a subrange of the integers}
typedef unsigned char text_char; // {the data type of characters in text files}

Array<ASCII_code, 0, 255> xord; // Specifies conversion of input characters
Array<text_char, 0, 255> xchr; // Specifies conversion of output characters

bool wants_mnemonics; //{controls mnemonic output}
bool wants_pixels; //{controls pixel output}

int total_chars; // {the total number of characters seen so far}
Array<int, 0, 255> char_ptr; //array[0..255] of integer; {correct character location pointer}
int gf_prev_ptr; // {|char_ptr| for next character}
int character_code; // {current character number}

#define show_label(s1, s2) do {\
  printf("%d: %s%s", a, s1, s2);\
} while (false)

#define show_mnemonic(s1) do {\
  if (wants_mnemonics) {\
    print_nl(); show_label(s1, "");\
  }\
} while (false)

#define error(s1) do {\
  show_label("! ",s1);\
  print_nl();\
} while (false)

#define nl_error(s1) do {\
  print_nl();\
  show_label("! ", s1);\
  print_nl();\
} while (false)

#define start_op do {\
  a = cur_loc; o = get_byte(); p = first_par(o);\
  if (feof(gf_file)) bad_gf("the file ended prematurely");\
} while (false)

int min_m_stated, max_m_stated, min_n_stated, max_n_stated;// {bounds stated in the \.{GF} file}
int max_m_observed, max_n_observed; // {bounds on $(m',n')$ actually observed when painting}
int min_m_overall, max_m_overall, min_n_overall, max_n_overall;// {bounds observed in the entire file so far}

typedef unsigned char eight_bits; // Unsigned one-byte quantity
typedef FILE* byte_file; // Files that contain binary data
typedef FILE* text_file;  // packed file of text_char

#define white 0 // {could also be |false|}
#define black 1 // {could also be |true|}
typedef int pixel; // white..black; {could also be |boolean|}

int cur_loc; // {where we are about to look, in |gf_file|}
bool bad_char; // {has a non-ASCII character code appeared in this \\{xxx}?}

char temp_buf[1048]; // {used for constructing error messages}

const int terminal_line_length = 150; // {maximum number of characters input in a single line of input from the terminal}
const int line_length = 79; // {\\{xxx} strings will not produce lines longer than this}
const int max_row = 79; // {vertical extent of pixel image array}
const int max_col = 79; // {horizontal extent of pixel image array}

int design_size, check_sum; // {\.{TFM}-oriented parameters}
int hppp, vppp; // {magnification-oriented parameters}
int post_loc; // {location of the |post| command}
double pix_ratio; // {multiply by this to convert \.{TFM} width to scaled pixels}

int a; // {byte number of the current command}
int b, c, l, o, p, q, r; // {general purpose registers}

#define incr(s) (++s) // {increase a variable by unity}
#define decr(s) (--s) // {decrease a variable by unity}
#define negate(s) (s =-s) //{change the sign of a variable}

#define unity  0200000 // {$2^{16}$, represents 1.00000}

#define my_abort(ss) do {\
  printf(" %s\n", ss); jump_out();\
} while (false)

#define bad_gf(ss) do {\
  printf("! Bad GF file: %s!\n", ss);\
  jump_out();\
} while (false)

const int name_length = 257; //{a file name shouldn't be longer than this}
Array<char, 1, name_length + 1> cur_name, real_name_of_file, dvi_name;//{external name}

Array<ASCII_code, 0, terminal_line_length> buffer;
text_file term_in; // {the terminal, considered as an input file}
text_file term_out; // {the terminal, considered as an output file}
byte_file gf_file; // {the stuff we are \.{GF}typing}
const int gf_id_byte = 131; // {identifies the kind of \.{GF} files described here}

// Here are the opcodes that GFtype actually refers to.
const int paint_0 = 0; // {beginning of the \\{paint} commands}
const int paint1 = 64; // {move right a given number of columns, then black${}\swap{}$white}
const int boc = 67; // {beginning of a character}
const int boc1 = 68; // {abbreviated |boc|}
const int eoc = 69; // {end of a character}
const int skip0 = 70; // {skip no blank rows}
const int skip1 = 71; // {skip over blank rows}
const int new_row_0 = 74; // {move down one row and then right}
const int xxx1 = 239; // {for \&{special} strings}
const int yyy = 243; // {for \&{numspecial} numbers}
const int no_op = 244; // {no operation}
const int char_loc = 245; // {character locators in the postamble}
const int pre = 247; // {preamble}
const int post = 248; // {postamble beginning}
const int post_post = 249; // {postamble ending}
#define undefined_commands 250: case 251: case 252: case 253: case 254: case 255

int m, n; // {current state values, modified by |min_m| and |max_n|}
pixel paint_switch;

#define image image_array[m][n]
Array<Array<pixel, 0, max_row>, 0, max_col> image_array;

int max_subrow, max_subcol; // {size of current subarray of interest}


#define four_cases(s) s: case s+1: case s+2: case s+3
#define eight_cases(s) four_cases(s): case four_cases(s+4)
#define sixteen_cases(s) eight_cases(s): case eight_cases(s+8)
#define thirty_two_cases(s) sixteen_cases(s): case sixteen_cases(s+16)
#define thirty_seven_cases(s) thirty_two_cases(s): case four_cases(s+32):case s+36
#define sixty_four_cases(s) thirty_two_cases(s): case thirty_two_cases(s+32)




void print(const char *str);
void print_nl();
void print_ln(const char *str);
