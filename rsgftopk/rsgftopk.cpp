/*
Copyright (C) 2026 by Richard Sandberg.
*/
#include "rsgftopk.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void print_nl() { puts(""); }

void print_ln(const char *str) { puts(str); }

void print(const char *str) { fputs(str, stdout); }

void setup_char_arrays() {
  for (int i = 0; i < 256; i++)
    xchr[i] = i;

  xchr[32] = ' ';
  xchr[33] = '!';
  xchr[34] = '\"';
  xchr[35] = '#';
  xchr[36] = '$';
  xchr[37] = '%';
  xchr[38] = '&';
  xchr[39] = '\'';
  xchr[40] = '(';
  xchr[41] = ')';
  xchr[42] = '*';
  xchr[43] = '+';
  xchr[44] = ',';
  xchr[45] = '-';
  xchr[46] = '.';
  xchr[47] = '/';
  xchr[48] = '0';
  xchr[49] = '1';
  xchr[50] = '2';
  xchr[51] = '3';
  xchr[52] = '4';
  xchr[53] = '5';
  xchr[54] = '6';
  xchr[55] = '7';
  xchr[56] = '8';
  xchr[57] = '9';
  xchr[58] = ':';
  xchr[59] = ';';
  xchr[60] = '<';
  xchr[61] = '=';
  xchr[62] = '>';
  xchr[63] = '?';
  xchr[64] = '@';
  xchr[65] = 'A';
  xchr[66] = 'B';
  xchr[67] = 'C';
  xchr[68] = 'D';
  xchr[69] = 'E';
  xchr[70] = 'F';
  xchr[71] = 'G';
  xchr[72] = 'H';
  xchr[73] = 'I';
  xchr[74] = 'J';
  xchr[75] = 'K';
  xchr[76] = 'L';
  xchr[77] = 'M';
  xchr[78] = 'N';
  xchr[79] = 'O';
  xchr[80] = 'P';
  xchr[81] = 'Q';
  xchr[82] = 'R';
  xchr[83] = 'S';
  xchr[84] = 'T';
  xchr[85] = 'U';
  xchr[86] = 'V';
  xchr[87] = 'W';
  xchr[88] = 'X';
  xchr[89] = 'Y';
  xchr[90] = 'Z';
  xchr[91] = '[';
  xchr[92] = '\\';
  xchr[93] = ']';
  xchr[94] = '^';
  xchr[95] = '_';
  xchr[96] = '`';
  xchr[97] = 'a';
  xchr[98] = 'b';
  xchr[99] = 'c';
  xchr[100] = 'd';
  xchr[101] = 'e';
  xchr[102] = 'f';
  xchr[103] = 'g';
  xchr[104] = 'h';
  xchr[105] = 'i';
  xchr[106] = 'j';
  xchr[107] = 'k';
  xchr[108] = 'l';
  xchr[109] = 'm';
  xchr[110] = 'n';
  xchr[111] = 'o';
  xchr[112] = 'p';
  xchr[113] = 'q';
  xchr[114] = 'r';
  xchr[115] = 's';
  xchr[116] = 't';
  xchr[117] = 'u';
  xchr[118] = 'v';
  xchr[119] = 'w';
  xchr[120] = 'x';
  xchr[121] = 'y';
  xchr[122] = 'z';
  xchr[123] = '{';
  xchr[124] = '|';
  xchr[125] = '}';
  xchr[126] = '~';

  for (int i = 0; i < 256; i++) {
    xord[i] = 127;
  }
  for (int i = 0; i < 256; i++) {
    int ind = xchr[i];
    xord[ind] = i;
  }
}

void set_initial_values() {
  setup_char_arrays();

  for (int i = 0; i <= 255; i++) {
    tfm_width[i] = 0;
    dx[i] = 0;
    dy[i] = 0;
    status[i] = virgin;
  }

  assert(strlen(preamble_comment) == comm_length);
  assert(comment._array.size() == comm_length + 1);
  strcpy(comment.get_c_str(), preamble_comment);

  pk_open = false;
  pk_loc = 0;
  gf_loc = 0;

  bit_weight = 16;
  output_byte = 0;

  // Initialize power array
  power[0] = 1;
  for (int i = 1; i <= 8; i++) {
    power[i] = power[i - 1] + power[i - 1];
  }
}

void initialize() {
  print_ln(banner);
  set_initial_values();
}

void jump_out() { exit(0); }

bool parse_and_validate_args(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "/?") == 0 || strcmp(argv[1], "-help") == 0 ||
      strcmp(argv[1], "--help") == 0) {
    printf("rsgftopk [-v] gffile pkfile\n");
    return false;
  }

  const char *gf_filename = nullptr;
  const char *pk_filename = nullptr;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      if (gf_filename == nullptr) {
        gf_filename = argv[i];
      } else if (pk_filename == nullptr) {
        pk_filename = argv[i];
      }
    }
  }

  if (gf_filename == nullptr || pk_filename == nullptr) {
    printf("Error: Both GF and PK file names required!\n");
    printf("rsgftopk [-v] gffile pkfile\n");
    return false;
  }

  strncpy(cur_name.get_c_str(), gf_filename, name_length - 1);
  cur_name[name_length - 1] = '\0';

  strncpy(pk_name.get_c_str(), pk_filename, name_length - 1);
  pk_name[name_length - 1] = '\0';

  return true;
}

void open_gf_file() {
  gf_file = fopen(cur_name.get_c_str(), "rb");
  if (!gf_file) {
    sprintf(temp_buf, "Can't open GF file %s!", cur_name.get_c_str());
    my_abort(temp_buf);
  }
  gf_loc = 0;
}

void open_pk_file() {
  pk_file = fopen(pk_name.get_c_str(), "wb");
  if (!pk_file) {
    sprintf(temp_buf, "Can't open PK file %s for writing!",
            pk_name.get_c_str());
    my_abort(temp_buf);
  }
  pk_loc = 0;
  pk_open = true;
}

void find_gf_length() {
  fseek(gf_file, 0, SEEK_END);
  gf_len = ftell(gf_file);
  fseek(gf_file, 0, SEEK_SET);
}

void move_to_byte(int n) {
  fseek(gf_file, n, SEEK_SET);
  gf_loc = n;
}

int gf_byte() {
  eight_bits b;
  int c = fgetc(gf_file);
  if (c == EOF)
    bad_gf("Unexpected end of file!");

  b = c;
  incr(gf_loc);
  return b;
}

int gf_signed_quad() {
  eight_bits a, b, c, d;
  a = fgetc(gf_file);
  b = fgetc(gf_file);
  c = fgetc(gf_file);
  d = fgetc(gf_file);
  gf_loc = gf_loc + 4;
  if (a < 128)
    return ((a * 256 + b) * 256 + c) * 256 + d;
  else
    return (((a - 256) * 256 + b) * 256 + c) * 256 + d;
}

void pk_byte(int a) {
  if (pk_open) {
    if (a < 0)
      a = a + 256;
    fputc(a, pk_file);
    incr(pk_loc);
  }
}

void pk_halfword(int a) {
  if (a < 0)
    a = a + 65536;
  fputc(a / 256, pk_file);
  fputc(a % 256, pk_file);
  pk_loc = pk_loc + 2;
}

void pk_three_bytes(int a) {
  fputc(a / 65536 % 256, pk_file);
  fputc(a / 256 % 256, pk_file);
  fputc(a % 256, pk_file);
  pk_loc = pk_loc + 3;
}

void pk_word(int a) {
  int b;
  if (pk_open) {
    if (a < 0) {
      a = a + 0x40000000;
      a = a + 0x40000000;
      b = 128 + a / 16777216;
    } else {
      b = a / 16777216;
    }
    fputc(b, pk_file);
    fputc(a / 65536 % 256, pk_file);
    fputc(a / 256 % 256, pk_file);
    fputc(a % 256, pk_file);
    pk_loc = pk_loc + 4;
  }
}

void pk_nyb(int a) {
  if (bit_weight == 16) {
    output_byte = a * 16;
    bit_weight = 1;
  } else {
    pk_byte(output_byte + a);
    bit_weight = 16;
  }
}

void pack_and_send_character() {
  int i, j, k; // {general indices}
  // <Locals to pack_and_send_character>
  int extra;       // {little buffer for count values}
  int put_ptr;     // {next location to fill in |row|}
  int repeat_flag; // {how many times the current row is repeated}
  int h_bit;       // {horizontal bit count for each row}
  int buff;        // {our count accumulator}

  // <Scan for bounding box>
  i = 2;
  decr(row_ptr);
  while (row[i] == end_of_row)
    incr(i);
  if (row[i] != end_of_char) {
    max_n = max_n - i + 2;
    while (row[row_ptr - 2] == end_of_row) {
      decr(row_ptr);
      row[row_ptr] = end_of_char;
    }
    min_n = max_n + 1;
    extra = max_m - min_m + 1;
    max_m = 0;
    j = i;
    while (row[j] != end_of_char) {
      decr(min_n);
      if (row[j] != end_of_row) {
        k = row[j];
        if (k < extra)
          extra = k;
        incr(j);
        while (row[j] != end_of_row) {
          k = k + row[j];
          incr(j);
        }
        if (max_m < k)
          max_m = k;
      }
      incr(j);
    }
    min_m = min_m + extra;
    max_m = min_m + max_m - 1 - extra;
    height = max_n - min_n + 1;
    width = max_m - min_m + 1;
    x_offset = -min_m;
    y_offset = max_n;

    // printf("W %d H %d X %d Y %d\n",width,height,x_offset,y_offset); // debug
    // print
  } else {
    height = 0;
    width = 0;
    x_offset = 0;
    y_offset = 0;
    // printf("Empty raster.\n"); // debug print
  }

  // <Convert row-list to glyph-list>

  put_ptr = 0;
  row_ptr = 2;
  repeat_flag = 0;
  state = true;
  buff = 0;
  while (row[row_ptr] == end_of_row)
    incr(row_ptr);
  while (row[row_ptr] != end_of_char) {
    // <Skip over repeated rows>
    i = row_ptr;
    if (row[i] != end_of_row && (row[i] != extra || row[i + 1] != width)) {
      j = i + 1;
      while (row[j - 1] != end_of_row)
        incr(j);
      while (row[i] == row[j]) {
        if (row[i] == end_of_row) {
          incr(repeat_flag);
          row_ptr = i + 1;
        }
        incr(i);
        incr(j);
      }
    }

    // <Reformat count list>
    if (row[row_ptr] != end_of_row)
      row[row_ptr] = row[row_ptr] - extra;
    h_bit = 0;
    while (row[row_ptr] != end_of_row) {
      h_bit = h_bit + row[row_ptr];
      if (state) {
        buff = buff + row[row_ptr];
        state = false;
      } else if (row[row_ptr] > 0) {
        put_count(buff);
        buff = row[row_ptr];
      } else
        state = true;
      incr(row_ptr);
    }
    if (h_bit < width)
      if (state)
        buff = buff + width - h_bit;
      else {
        put_count(buff);
        buff = width - h_bit;
        state = true;
      }
    else
      state = false;
    incr(row_ptr);
  }
  if (buff > 0)
    put_count(buff);
  put_count(end_of_char);

  //<Calculate |dyn_f| and packed size and write character>
  for (i = 1; i <= 13; i++)
    deriv[i] = 0;
  i = 0;
  first_on = row[i] == 0;
  if (first_on)
    incr(i);
  comp_size = 0;
  while (row[i] != end_of_char)
  // <Process count for best |dyn_f| value>
  {
    j = row[i];
    if (j == -1)
      incr(comp_size);
    else {
      if (j < 0) {
        incr(comp_size);
        j = -j;
      }
      if (j < 209)
        comp_size = comp_size + 2;
      else {
        k = j - 193;
        while (k >= 16) {
          k = k / 16;
          comp_size = comp_size + 2;
        }
        incr(comp_size);
      }
      if (j < 14)
        decr(deriv[j]);
      else if (j < 209)
        incr(deriv[(223 - j) / 15]);
      else {
        k = 16;
        while (k * 16 < j + 3)
          k = k * 16;
        if (j - k <= 192)
          deriv[(207 - j + k) / 15] = deriv[(207 - j + k) / 15] + 2;
      }
    }
    incr(i);
  }

  b_comp_size = comp_size;
  dyn_f = 0;
  for (i = 1; i <= 13; i++) {
    comp_size = comp_size + deriv[i];
    if (comp_size <= b_comp_size) {
      b_comp_size = comp_size;
      dyn_f = i;
    }
  }
  comp_size = (b_comp_size + 1) / 2;
  if (comp_size > (height * width + 7) / 8 || height * width == 0) {
    comp_size = (height * width + 7) / 8;
    dyn_f = 14;
  }
  // printf("Best packing is dyn_f of %d with length %d\n",dyn_f,comp_size); //
  // debug print

  // <Write character preamble>
  flag_byte = dyn_f * 16;
  if (first_on)
    flag_byte = flag_byte + 8;
  if (gf_ch != gf_ch_mod_256 || tfm_width[gf_ch_mod_256] > 16777215 ||
      tfm_width[gf_ch_mod_256] < 0 || dy[gf_ch_mod_256] != 0 ||
      dx[gf_ch_mod_256] < 0 || dx[gf_ch_mod_256] % 65536 != 0 ||
      comp_size > 196594 || width > 65535 || height > 65535 ||
      x_offset > 32767 || y_offset > 32767 || x_offset < -32768 ||
      y_offset < -32768)
  // <Write long character preamble>
  {
    flag_byte = flag_byte + 7;
    pk_byte(flag_byte);
    comp_size = comp_size + 28;
    pk_word(comp_size);
    pk_word(gf_ch);
    pred_pk_loc = pk_loc + comp_size;
    pk_word(tfm_width[gf_ch_mod_256]);
    pk_word(dx[gf_ch_mod_256]);
    pk_word(dy[gf_ch_mod_256]);
    pk_word(width);
    pk_word(height);
    pk_word(x_offset);
    pk_word(y_offset);
  } else if (dx[gf_ch] > 16777215 || width > 255 || height > 255 ||
             x_offset > 127 || y_offset > 127 || x_offset < -128 ||
             y_offset < -128 || comp_size > 1015)
  // <Write two-byte short character preamble>
  {
    comp_size = comp_size + 13;
    flag_byte = flag_byte + comp_size / 65536 + 4;
    pk_byte(flag_byte);
    pk_halfword(comp_size % 65536);
    pk_byte(gf_ch);
    pred_pk_loc = pk_loc + comp_size;
    pk_three_bytes(tfm_width[gf_ch_mod_256]);
    pk_halfword(dx[gf_ch_mod_256] / 65536);
    pk_halfword(width);
    pk_halfword(height);
    pk_halfword(x_offset);
    pk_halfword(y_offset);
  } else
  // <Write one-byte short character preamble>
  {
    comp_size = comp_size + 8;
    flag_byte = flag_byte + comp_size / 256;
    pk_byte(flag_byte);
    pk_byte(comp_size % 256);
    pk_byte(gf_ch);
    pred_pk_loc = pk_loc + comp_size;
    pk_three_bytes(tfm_width[gf_ch_mod_256]);
    pk_byte(dx[gf_ch_mod_256] / 65536);
    pk_byte(width);
    pk_byte(height);
    pk_byte(x_offset);
    pk_byte(y_offset);
  }

  if (dyn_f != 14)
  // <Send compressed format>
  {
    bit_weight = 16;
    max_2 = 208 - 15 * dyn_f;
    i = 0;
    if (row[i] == 0)
      incr(i);
    while (row[i] != end_of_char) {
      j = row[i];
      if (j == -1)
        pk_nyb(15);
      else {
        if (j < 0) {
          pk_nyb(14);
          j = -j;
        }
        if (j <= dyn_f)
          pk_nyb(j);
        else if (j <= max_2) {
          j = j - dyn_f - 1;
          pk_nyb(j / 16 + dyn_f + 1);
          pk_nyb(j % 16);
        } else {
          j = j - max_2 + 15;
          k = 16;
          while (k <= j) {
            k = k * 16;
            pk_nyb(0);
          }
          while (k > 1) {
            k = k / 16;
            pk_nyb(j / k);
            j = j % k;
          }
        }
      }
      incr(i);
    }
    if (bit_weight != 16)
      pk_byte(output_byte);
  } else if (height > 0)
  // <Send bit map>
  {
    buff = 0;
    p_bit = 8;
    i = 1;
    h_bit = width;
    on = false;
    state = false;
    count = row[0];
    repeat_flag = 0;
    while (row[i] != end_of_char || state || count > 0) {
      if (state) {
        count = r_count;
        i = r_i;
        on = r_on;
        decr(repeat_flag);
      } else {
        r_count = count;
        r_i = i;
        r_on = on;
      }
      // <Send one row by bits>
      do {
        if (count == 0) {
          if (row[i] < 0) {
            if (!state)
              repeat_flag = -row[i];
            incr(i);
          }
          count = row[i];
          incr(i);
          on = !on;
        }
        if (count >= p_bit && p_bit < h_bit) {
          // { we end a byte, we don't end the row }
          if (on)
            buff = buff + power[p_bit] - 1;
          pk_byte(buff);
          buff = 0;
          h_bit = h_bit - p_bit;
          count = count - p_bit;
          p_bit = 8;
        } else if (count < p_bit && count < h_bit) {
          // { we end neither the row nor the byte }
          if (on)
            buff = buff + power[p_bit] - power[p_bit - count];
          p_bit = p_bit - count;
          h_bit = h_bit - count;
          count = 0;
        } else {
          // { we end a row and maybe a byte }
          if (on)
            buff = buff + power[p_bit] - power[p_bit - h_bit];
          count = count - h_bit;
          p_bit = p_bit - h_bit;
          h_bit = width;
          if (p_bit == 0) {
            pk_byte(buff);
            buff = 0;
            p_bit = 8;
          }
        }
      } while (h_bit != width);

      if (state && repeat_flag == 0) {
        count = s_count;
        i = s_i;
        on = s_on;
        state = false;
      } else if (!state && repeat_flag > 0) {
        s_count = count;
        s_i = i;
        s_on = on;
        state = true;
      }
    }
    if (p_bit != 8)
      pk_byte(buff);
  }
}

void convert_gf_file() {
  int i, j, k; // {general purpose indices}
  int gf_com;  // {current gf command}

  // <Locals to |convert_gf_file|>
  bool on;    // {indicates whether we are white or black}
  bool state; // {a state variable---is the next count the same race as the one
              // in the |extra| buffer?}
  int extra;  // {where we pool our counts}
  bool bad;   // {did we run out of space?}
  int hppp, vppp; // {horizontal and vertical pixels per point}
  int q;          // {quad temporary}
  int post_loc;   // {where the postamble was}

  open_gf_file();
  if (gf_byte() != pre)
    bad_gf("First byte is not preamble");

  if (gf_byte() != gf_id_byte)
    bad_gf("Identification byte is incorrect");

  // <Find and interpret postamble>
  find_gf_length();
  if (gf_len < 8) {
    sprintf(temp_str, "only %d bytes long", gf_len);
    bad_gf(temp_str);
  }

  post_loc = gf_len - 4;
  do {
    if (post_loc == 0)
      bad_gf("all 223's");
    move_to_byte(post_loc);
    k = gf_byte();
    decr(post_loc);
  } while (k == 223);
  if (k != gf_id_byte) {
    sprintf(temp_str, "ID byte is %d", k);
    bad_gf(temp_str);
  }

  if (post_loc < 5) {
    sprintf(temp_str, "post location is %d", post_loc);
    bad_gf(temp_str);
  }
  move_to_byte(post_loc - 3);
  q = gf_signed_quad();
  if (q < 0 || q > post_loc - 3) {
    sprintf(temp_str, "post pointer is %d", q);
    bad_gf(temp_str);
  }

  move_to_byte(q);
  k = gf_byte();
  if (k != post) {
    sprintf(temp_str, "byte at %d is not post", q);
    bad_gf(temp_str);
  }

  i = gf_signed_quad(); // {skip over junk}
  design_size = gf_signed_quad();
  check_sum = gf_signed_quad();
  hppp = gf_signed_quad();
  h_mag = round(hppp * 72.27 / 65536);
  vppp = gf_signed_quad();
  if (hppp != vppp)
    print_ln("Odd aspect ratio!");
  i = gf_signed_quad();
  i = gf_signed_quad(); // {skip over junk}
  i = gf_signed_quad();
  i = gf_signed_quad();
  do {
    gf_com = gf_byte();
    switch (gf_com) {
    case char_loc:
    case char_loc0: {
      gf_ch = gf_byte();
      if (status[gf_ch] != virgin)
        bad_gf("Locator for this character already found.");
      //@.Locator...already found@>
      if (gf_com == char_loc) {
        dx[gf_ch] = gf_signed_quad();
        dy[gf_ch] = gf_signed_quad();
      } else {
        dx[gf_ch] = gf_byte() * 65536;
        dy[gf_ch] = 0;
      }
      tfm_width[gf_ch] = gf_signed_quad();
      i = gf_signed_quad();
      status[gf_ch] = located;
    } break;
    // <Specials and |no_op| cases>
    case four_cases(xxx1): {
      pk_byte(gf_com - xxx1 + pk_xxx1);
      i = 0;
      for (j = 0; j <= gf_com - xxx1; j++) {
        k = gf_byte();
        pk_byte(k);
        i = i * 256 + k;
      }
      for (j = 1; j <= i; j++)
        pk_byte(gf_byte());
    } break;
    case yyy: {
      pk_byte(pk_yyy);
      pk_word(gf_signed_quad());
    } break;
    case no_op:
      break;
    case post_post:;
      break;
    default: {
      sprintf(temp_str, "Unexpected %d in postamble", gf_com);
      bad_gf(temp_str);
    } break;
      // @.Unexpected command@>
    }
  } while (gf_com != post_post);

  move_to_byte(2);
  open_pk_file();
  // <Write preamble>
  pk_byte(pk_pre);
  pk_byte(pk_id);
  i = gf_byte(); // {get length of introductory comment}
  do {
    if (i == 0)
      j = '.';
    else
      j = gf_byte();
    decr(i); // {some people think it's wise to avoid |goto| statements}
  } while (j == ' '); // {remove leading blanks}
  incr(i); // {this many bytes to copy}
  if (i == 0)
    k = comm_length - from_length;
  else
    k = i + comm_length;
  if (k > 255)
    pk_byte(255);
  else
    pk_byte(k);
  for (k = 1; k <= comm_length; k++)
    if (i > 0 || k <= comm_length - from_length)
      pk_byte(xord[comment[k]]);
  print("'");
  for (k = 1; k <= i; k++) {
    if (k > 1)
      j = gf_byte();
    fputc(xchr[j], stdout);
    if (k < 256 - comm_length)
      pk_byte(j);
  }
  print_ln("'");
  pk_word(design_size);
  pk_word(check_sum);
  pk_word(hppp);
  pk_word(vppp);

  do {
    gf_com = gf_byte();
    switch (gf_com) {
    case boc:
    case boc1: {
      // <Interpret character>
      if (gf_com == boc) {
        gf_ch = gf_signed_quad();
        i = gf_signed_quad(); // {dispose of back pointer}
        min_m = gf_signed_quad();
        max_m = gf_signed_quad();
        min_n = gf_signed_quad();
        max_n = gf_signed_quad();
      } else {
        gf_ch = gf_byte();
        i = gf_byte();
        max_m = gf_byte();
        min_m = max_m - i;
        i = gf_byte();
        max_n = gf_byte();
        min_n = max_n - i;
      }
      // printf("Character %d\n",gf_ch); // debug print
      if (gf_ch >= 0)
        gf_ch_mod_256 = gf_ch % 256;
      else
        gf_ch_mod_256 = 255 - ((-(1 + gf_ch)) % 256);
      if (status[gf_ch_mod_256] == virgin) {
        sprintf(temp_str, "no character locator for character %d", gf_ch);
        bad_gf(temp_str);
      }
      // <Convert character to packed form>
      bad = false;
      row_ptr = 2;
      on = false;
      extra = 0;
      state = true;
      do {
        gf_com = gf_byte();
        switch (gf_com) {
          // <Cases for |paint| commands>
        case paint_0: {
          state = not state;
          on = not on;
        } break;
        case sixty_four_cases(paint_0 + 1):
        case paint1 + 1:
        case paint1 + 2: {
          if (gf_com < paint1)
            i = gf_com - paint_0;
          else {
            i = 0;
            for (j = 0; j <= gf_com - paint1; j++)
              i = i * 256 + gf_byte();
          }
          if (state) {
            extra = extra + i;
            state = false;
          } else {
            put_in_rows(extra);
            extra = i;
          }
          on = !on;
        } break;
        case four_cases(skip0): {
          i = 0;
          for (j = 1; j <= gf_com - skip0; j++)
            i = i * 256 + gf_byte();
          if (on == state)
            put_in_rows(extra);
          for (j = 0; j <= i; j++)
            put_in_rows(end_of_row);
          on = false;
          extra = 0;
          state = true;
        } break;
        case one_sixty_five_cases(new_row_0): {
          if (on == state)
            put_in_rows(extra);
          put_in_rows(end_of_row);
          on = true;
          extra = gf_com - new_row_0;
          state = false;
        } break;
        // <Specials and |no_op| cases>
        case four_cases(xxx1): {
          pk_byte(gf_com - xxx1 + pk_xxx1);
          i = 0;
          for (j = 0; j <= gf_com - xxx1; j++) {
            k = gf_byte();
            pk_byte(k);
            i = i * 256 + k;
          }
          for (j = 1; j <= i; j++)
            pk_byte(gf_byte());
        } break;
        case yyy: {
          pk_byte(pk_yyy);
          pk_word(gf_signed_quad());
        } break;
        case no_op:
          break;

        case eoc: {
          if (on == state)
            put_in_rows(extra);
          if (row_ptr > 2 && row[row_ptr - 1] != end_of_row)
            put_in_rows(end_of_row);
          put_in_rows(end_of_char);
          if (bad)
            my_abort("Ran out of internal memory for row counts!");
          pack_and_send_character();
          status[gf_ch_mod_256] = sent;
          if (pk_loc != pred_pk_loc)
            my_abort("Internal error while writing character!");
        } break;
        default: {
          sprintf(temp_str, "Unexpected %d command in character definition",
                  gf_com);
          bad_gf(temp_str);
        } break;
        }
      } while (gf_com != eoc);
    } break;

    // <Specials and |no_op| cases>
    case four_cases(xxx1): {
      pk_byte(gf_com - xxx1 + pk_xxx1);
      i = 0;
      for (j = 0; j <= gf_com - xxx1; j++) {
        k = gf_byte();
        pk_byte(k);
        i = i * 256 + k;
      }
      for (j = 1; j <= i; j++)
        pk_byte(gf_byte());
    } break;
    case yyy: {
      pk_byte(pk_yyy);
      pk_word(gf_signed_quad());
    } break;
    case no_op:
      break;
    case post:; // {we will actually do the work for this one later}
      break;
    default: {
      sprintf(temp_str, "Unexpected %d command between characters", gf_com);
      bad_gf(temp_str);
    } break;
    }
  } while (gf_com != post);
  // <Write postamble>
  pk_byte(pk_post);
  while (pk_loc % 4 != 0)
    pk_byte(pk_no_op);
}

int main(int argc, char *argv[]) {
  initialize();

  if (!parse_and_validate_args(argc, argv)) {
    return 1;
  }

  convert_gf_file();

  if (gf_file)
    fclose(gf_file);
  if (pk_file && pk_file != stdout)
    fclose(pk_file);

  // <Check for unrasterized locators>
  for (int i = 0; i <= 255; i++)
    if (status[i] == located)
      printf("Character %d missing raster information!\n", i);

  printf("%d bytes packed to %d bytes.\n", gf_len, pk_loc);
  return 0;
}
