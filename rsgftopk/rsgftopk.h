/*
Copyright (C) 2026 by Richard Sandberg.
*/

#pragma once
#include <array>
#include <cassert>
#include <cstdio>

#define array_size maxindex - minindex + 1
template <typename T, int minindex, int maxindex> struct Array {
  T &operator[](int x) {
    assert(x >= minindex && x <= maxindex);
    return _array[x - minindex];
  }

  T *get_c_str() {
    assert(array_size > 0);
    _array[array_size - 1] = T(0);
    return &_array[0];
  }
  static const int _minindex = minindex;
  static const int _maxindex = maxindex;
  std::array<T, array_size> _array;
};

#define banner                                                                 \
  "This is rsgftopk 1.0 (GFtoPK, Version 2.4)" //{printed when the program
                                               // starts}

typedef unsigned char ASCII_code;
typedef unsigned char text_char;
typedef unsigned char eight_bits;
typedef FILE *byte_file;

Array<ASCII_code, 0, 255> xord;
Array<text_char, 0, 255> xchr;

// Character status constants
#define virgin 0  // never heard of this character yet
#define located 1 // locators read for this character
#define sent 2    // at least one of these characters has been sent

// GF file opcodes
const int paint_0 = 0;
const int paint1 = 64;
const int boc = 67;
const int boc1 = 68;
const int eoc = 69;
const int skip0 = 70;
const int skip1 = 71;
const int new_row_0 = 74;
const int xxx1 = 239;
const int yyy = 243;
const int no_op = 244;
const int char_loc = 245;
const int char_loc0 = 246;
const int pre = 247;
const int post = 248;
const int post_post = 249;

// PK file opcodes
const int pk_id = 89;
const int pk_xxx1 = 240;
const int pk_yyy = 244;
const int pk_post = 245;
const int pk_no_op = 246;
const int pk_pre = 247;

const int gf_id_byte = 131;
const int name_length = 257;
const int max_row = 16000;

// Sentinel values for row encoding
#define end_of_row (-99999)
#define end_of_char (-99998)

// Character data
Array<int, 0, 255> tfm_width;
Array<int, 0, 255> dx, dy;
Array<int, 0, 255> status;
Array<int, 0, max_row> row;

// File operations
byte_file gf_file;
byte_file pk_file;
int pk_loc;   // where we are about to write in pk_file
int gf_loc;   // where we are in gf_file
bool pk_open; // is the packed file open?
int gf_len;   // length of gf_file

// Buffering for nybbles
int bit_weight;
int output_byte;

// Character processing state
int gf_ch;
int gf_ch_mod_256;
int pred_pk_loc;
int max_n, min_n;
int max_m, min_m;
int row_ptr;

// Packing state
int design_size;
int check_sum;
int hppp, vppp;
int h_mag;
int post_loc;

// Conversion state
bool on;
bool state;
int extra;
bool bad;

// Packing and sending state
int height, width;
int x_offset, y_offset;
int dyn_f;
bool first_on;
int flag_byte;
int comp_size;
int b_comp_size;

Array<int, 1, 13> deriv;
Array<int, 0, 8> power;

int count;
int p_bit;
bool r_on, s_on;
int r_count, s_count;
int r_i, s_i;
int max_2;
int put_ptr;
int repeat_flag;
int h_bit;
int buff;

// Temporary storage
char temp_buf[1048];
Array<char, 1, name_length + 1> cur_name;
Array<char, 1, name_length + 1> pk_name;

char temp_str[1024]; // temporary buffer for error messages

const int comm_length = 23; // {length of |preamble_comment|}
const int from_length = 6;  // {length of its |' from '| part}
Array<char, 1, comm_length + 1> comment;
const char *preamble_comment = "GFtoPK 2.4 output from ";

#define incr(s) (++s)
#define decr(s) (--s)
#define negate(s) (s = -s)

#define my_abort(ss)                                                           \
  do {                                                                         \
    printf(" %s\n", ss);                                                       \
    jump_out();                                                                \
  } while (false)

#define bad_gf(ss)                                                             \
  do {                                                                         \
    printf("! Bad GF file: %s!\n", ss);                                        \
    jump_out();                                                                \
  } while (false)

#define four_cases(s)                                                          \
  s : case s + 1:                                                              \
  case s + 2:                                                                  \
  case s + 3
#define eight_cases(s) four_cases(s) : case four_cases(s + 4)
#define sixteen_cases(s) eight_cases(s) : case eight_cases(s + 8)
#define thirty_two_cases(s) sixteen_cases(s) : case sixteen_cases(s + 16)
#define thirty_seven_cases(s)                                                  \
  thirty_two_cases(s) : case four_cases(s + 32):                               \
  case s + 36
#define sixty_four_cases(s) thirty_two_cases(s) : case thirty_two_cases(s + 32)
#define one_sixty_five_cases(s)                                                \
  sixty_four_cases(s) : case sixty_four_cases(s + 64):                         \
  case sixteen_cases(s + 128):                                                 \
  case sixteen_cases(s + 144):                                                 \
  case four_cases(s + 160):                                                    \
  case s + 164

#define put_count(s)                                                           \
  do {                                                                         \
    row[put_ptr] = s;                                                          \
    incr(put_ptr);                                                             \
    if (repeat_flag > 0) {                                                     \
      row[put_ptr] = -repeat_flag;                                             \
      repeat_flag = 0;                                                         \
      incr(put_ptr);                                                           \
    }                                                                          \
  } while (false)

#define put_in_rows(s)                                                         \
  do {                                                                         \
    if (row_ptr > max_row)                                                     \
      bad = true;                                                              \
    else {                                                                     \
      row[row_ptr] = s;                                                        \
      incr(row_ptr);                                                           \
    }                                                                          \
  } while (false)

void print(const char *str);
void print_nl();
void print_ln(const char *str);
void setup_char_arrays();
void set_initial_values();
void initialize();
void jump_out();
bool parse_and_validate_args(int argc, char *argv[]);
void open_gf_file();
void open_pk_file();
int gf_byte();
int gf_signed_quad();
void pk_byte(int a);
void pk_halfword(int a);
void pk_three_bytes(int a);
void pk_word(int a);
void pk_nyb(int a);
void find_gf_length();
void move_to_byte(int n);
void convert_gf_file();
void pack_and_send_character();
