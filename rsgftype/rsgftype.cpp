/*
Copyright (C) 2026 by Richard Sandberg.
*/
#include "rsgftype.h"
#include <cmath>
#include <cstdio>
#include <climits>
#include <cstdlib>
#include <cstring>

void print_nl()
{
    puts("");
}

void print_ln(const char *str)
{
    puts(str);
}

void print(const char *str)
{
    fputs(str, stdout);
}

void setup_char_arrays()
{
  for (int i = 0; i < 32; i++)
    xchr[i] = i;

  for (int i = 127; i < 256; i++)
    xchr[i] = i;

  xchr[32] = ' '; xchr[33] = '!'; xchr[34] = '\"'; xchr[35] = '#'; xchr[36] = '$';
  xchr[37] = '%'; xchr[38] = '&'; xchr[39] = '\''; xchr[40] = '('; xchr[41] = ')';
  xchr[42] = '*'; xchr[43] = '+'; xchr[44] = ','; xchr[45] = '-'; xchr[46] = '.';
  xchr[47] = '/'; xchr[48] = '0'; xchr[49] = '1'; xchr[50] = '2'; xchr[51] = '3';
  xchr[52] = '4'; xchr[53] = '5'; xchr[54] = '6'; xchr[55] = '7'; xchr[56] = '8';
  xchr[57] = '9'; xchr[58] = ':'; xchr[59] = ';'; xchr[60] = '<'; xchr[61] = '=';
  xchr[62] = '>'; xchr[63] = '?'; xchr[64] = '@'; xchr[65] = 'A'; xchr[66] = 'B';
  xchr[67] = 'C'; xchr[68] = 'D'; xchr[69] = 'E'; xchr[70] = 'F'; xchr[71] = 'G';
  xchr[72] = 'H'; xchr[73] = 'I'; xchr[74] = 'J'; xchr[75] = 'K'; xchr[76] = 'L';
  xchr[77] = 'M'; xchr[78] = 'N'; xchr[79] = 'O'; xchr[80] = 'P'; xchr[81] = 'Q';
  xchr[82] = 'R'; xchr[83] = 'S'; xchr[84] = 'T'; xchr[85] = 'U'; xchr[86] = 'V';
  xchr[87] = 'W'; xchr[88] = 'X'; xchr[89] = 'Y'; xchr[90] = 'Z'; xchr[91] = '[';
  xchr[92] = '\\'; xchr[93] = ']'; xchr[94] = '^'; xchr[95] = '_'; xchr[96] = '`';
  xchr[97] = 'a'; xchr[98] = 'b'; xchr[99] = 'c'; xchr[100] = 'd'; xchr[101] = 'e';
  xchr[102] = 'f'; xchr[103] = 'g'; xchr[104] = 'h'; xchr[105] = 'i'; xchr[106] = 'j';
  xchr[107] = 'k'; xchr[108] = 'l'; xchr[109] = 'm'; xchr[110] = 'n'; xchr[111] = 'o';
  xchr[112] = 'p'; xchr[113] = 'q'; xchr[114] = 'r'; xchr[115] = 's'; xchr[116] = 't';
  xchr[117] = 'u'; xchr[118] = 'v'; xchr[119] = 'w'; xchr[120] = 'x'; xchr[121] = 'y';
  xchr[122] = 'z'; xchr[123] = '{'; xchr[124] = '|'; xchr[125] = '}'; xchr[126] = '~';

  for (int i = 0; i < 256; i++) {
    xord[i] = 127;
  }
  for (int i = 128; i < 256; i++) {
    int ind = xchr[i];
    xord[ind] = i;
  }
  for (int i = 0; i < 127; i++) {
    int ind = xchr[i];
    xord[ind] = i;
  }
}

void set_initial_values()
{
    setup_char_arrays();

    wants_mnemonics = true; wants_pixels = true;

    for (int i = 0; i <= 255; i++) { char_ptr[i] = -1; } // {mark characters as not being in the file}
    total_chars = 0;

    min_m_overall = INT_MAX; max_m_overall = -INT_MAX;
    min_n_overall = INT_MAX; max_n_overall = -INT_MAX;
}

void initialize() // {this procedure gets things started properly}
{
  print_ln(banner);
  // <Set initial values>
  set_initial_values();
}

void jump_out()
{
	// goto final_end
	exit(0);
}

bool parse_and_validate_args(int argc, char *argv[])
{
  // Check if help is requested
  if (argc < 2 || strcmp(argv[1], "/?")==0 || strcmp(argv[1], "-help")==0 || strcmp(argv[1], "--help")==0) {
    printf("rsgftype [-mnemonics] [-nopixels] gffile\n");
    return false;
  }

  // Set defaults
  wants_mnemonics = false;
  wants_pixels = true;
  const char *gf_filename = nullptr;

  // Parse all arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-mnemonics") == 0) {
      wants_mnemonics = true;
    }
    else if (strcmp(argv[i], "-nopixels") == 0) {
      wants_pixels = false;
    }
    else if (argv[i][0] != '-') {
      // Assume this is the filename (first non-option argument)
      if (gf_filename == nullptr) {
        gf_filename = argv[i];
      }
    }
    else {
      printf("Unknown option: %s\n", argv[i]);
      printf("rsgftype [-mnemonics] [-nopixels] gffile\n");
      return false;
    }
  }

  // Validate that a filename was provided
  if (gf_filename == nullptr) {
    printf("Error: No GF file specified.!\n");
    printf("rsgftype [-mnemonics] [-nopixels] gffile\n");
    return false;
  }

  // Copy filename to cur_name
  strncpy(cur_name.get_c_str(), gf_filename, name_length - 1);
  cur_name[name_length - 1] = '\0';

  // Print selected options
  print("Options selected: Mnemonic output = ");
  print(wants_mnemonics ? "true" : "false");
  print("; pixel output = ");
  print(wants_pixels ? "true" : "false");
  print_ln(".");
  return true;
}

void open_gf_file() //{prepares to read packed bytes in |gf_file|}
{
    gf_file = fopen(cur_name.get_c_str(), "rb");
    if (!gf_file) {
      sprintf(temp_buf, "Can't open GF file %s!", cur_name.get_c_str());
      my_abort(temp_buf);
    }
    cur_loc = 0;
}

int get_byte() //{returns the next byte, unsigned}
{
  eight_bits b;
  int c;
  c = fgetc(gf_file);
  if (c == EOF)
    return 0;

  b = c;
  incr(cur_loc);
  return b;
}

int get_two_bytes() //{returns the next two bytes, unsigned}
{
  eight_bits a, b;
  a = fgetc(gf_file); b = fgetc(gf_file);
  cur_loc = cur_loc + 2;
  return a * 256 + b;
}

int get_three_bytes() //{returns the next three bytes, unsigned}
{
  eight_bits a, b, c;
  a = fgetc(gf_file); b = fgetc(gf_file); c = fgetc(gf_file);
  cur_loc = cur_loc + 3;
  return (a * 256 + b) * 256 + c;
}

int signed_quad() //{returns the next four bytes, signed}
{
  eight_bits a, b, c, d;
  a = fgetc(gf_file); b = fgetc(gf_file); c = fgetc(gf_file); d = fgetc(gf_file);
  cur_loc = cur_loc + 4;
  if (a < 128) return ((a * 256 + b) * 256 + c) * 256 + d;
  else return (((a - 256) * 256 + b) * 256 + c) * 256 + d;
}

int first_par(eight_bits o)
{
  int first_par_ret = 0;
  switch(o) {
  case sixty_four_cases(paint_0):
    first_par_ret = o - paint_0;
    break;
  case paint1:
  case skip1:
  case char_loc:
  case char_loc + 1:
  case xxx1:
    first_par_ret = get_byte();
    break;
  case paint1 + 1:
  case skip1 + 1:
  case xxx1 + 1:
    first_par_ret = get_two_bytes();
    break;
  case paint1 + 2:
  case skip1 + 2:
  case xxx1 + 2:
    first_par_ret = get_three_bytes();
    break;
  case xxx1 + 3:
  case yyy:
    first_par_ret = signed_quad();
    break;
  case boc: case boc1: case eoc: case skip0: case no_op: case pre: case post: case post_post:
  case undefined_commands:  first_par_ret = 0;
      break;
  case sixty_four_cases(new_row_0): case sixty_four_cases(new_row_0+64): case
    thirty_seven_cases(new_row_0+128): first_par_ret = o - new_row_0;
    break;
  }
  return first_par_ret;
}

void print_scaled(int s)
{
  // {prints a scaled number, rounded to five digits}
  int delta; // {amount of allowable inaccuracy}
  if (s<0) {
    print("-"); negate(s); // {print the sign, if negative}
  }

  printf("%d", s / unity); // {print the integer part}
  s = 10 * (s % unity) + 5;
  if (s != 5) {
    delta = 10; print(".");
    do {
      if (delta > unity) {
        s = s + 0100000 - (delta / 2); // {round the final digit}
      }
      fputc('0' + (s / unity), stdout); s = 10 * (s % unity); delta = delta * 10;
    } while (s > delta);
  }
}

bool do_char()
{
  eight_bits o; //{operation code of the current command}
  int p,q; // {parameters of the current command}
  bool aok; // {the value to return}
  // {we've already scanned the |boc|}
  aok = true;
  while (true) {
    // <Translate the next command in the \.{GF} file>
    // |goto 9999| if it was |eoc|;
    // |goto 9998| if premature termination is needed>
    start_op;
    //<Start translation of command |o| and |goto| the appropriate label to finish the job>
    if (o <= paint1 + 3) {
      // <Translate a sequence of |paint| commands, until reaching a non-|paint|>
      if (wants_mnemonics) print(" paint ");
      do {
        // <Paint the next |p| pixels>
        if (wants_mnemonics) {
          if (paint_switch == white) {
            printf("(%d)", p);
          } else {
            printf("%d", p);
          }
        }
        m += p;
        if (m > max_m_observed) {
          max_m_observed = m - 1;
        }
        if (wants_pixels) {
          // <Paint pixels |m-p| through |m-1| in row |n| of the subarray>
          if (paint_switch == black)
            if (n <= max_subrow)
            {
              l = m - p; r = m - 1;
              if (r > max_subcol) r = max_subcol;
              m = l;
              while (m <= r)
              {
                image = black; incr(m);
              }
              m = l + p;
            }
        }
        paint_switch = white + black - paint_switch;
          // {could also be |paint_switch:=not paint_switch|}
        start_op;
      } while (o <= paint1 + 3);
    }
    switch (o) {
    case four_cases(skip0):
      // <Translate a |skip| command>
      {
        sprintf(temp_buf, "skip%d %d", o - skip0, p);
        show_mnemonic(temp_buf);
        n = n + p + 1; m = 0; paint_switch = white;
        if (wants_mnemonics) printf(" (n=%d)",max_n_stated-n);
      }
      break;
    case sixty_four_cases(new_row_0): case sixty_four_cases(new_row_0+64):
    case thirty_seven_cases(new_row_0+128):
      // <Translate a |new_row| command>
      {
        sprintf(temp_buf, "newrow %d", p);
        show_mnemonic(temp_buf);
        incr(n); m = p; paint_switch = black;
        if (wants_mnemonics) printf(" (n=%d)",max_n_stated-n);
      }
      break;
    // <Cases for commands |no_op|, |pre|, |post|, |post_post|, |boc|, and |eoc|>
    case no_op: show_mnemonic("no op"); break;
    case pre: {
        error("preamble command within a character!"); goto _9998;
      }
      break;
    case post: case post_post: {
        error("postamble command within a character!");
        goto _9998;
      }
      break;
    case boc: case boc1: {
        error("boc occurred before eoc!"); goto _9998;
      }
      break;
    case eoc: {
        show_mnemonic("eoc");
        print_nl(); goto _9999;
      }
      break;

    case four_cases(xxx1):
      // <Translate an |xxx|...>
      {
        show_mnemonic("xxx '"); bad_char = false; b = 16;
        if (p < 0) nl_error("string of negative length!");
        while (p > 0)
        {
            q = get_byte();
            if (q < ' ' || q > '~') bad_char = true;
            if (wants_mnemonics) {
              fputc(xchr[q], stdout);
              if (b < line_length) incr(b);
              else  {
                print_nl(); b = 2;
              }
          }
          decr(p);
        }
        if (wants_mnemonics) print("'");
        if (bad_char) nl_error("non-ASCII character in xxx command!");
      }
      break;
    case yyy:
      {
        // <Translate a |yyy|...>
        sprintf(temp_buf, "yyy %d (", p);
        show_mnemonic(temp_buf);
        if (wants_mnemonics) {
          print_scaled(p);
          fputc(')', stdout);
        }
      }
      break;
    default:
      sprintf(temp_buf, "undefined command %d!", o);
      error(temp_buf);
      break;
    }
  }

  _9998: print_ln("!"); aok = false;
  _9999: return aok;
}

void read_postamble()
{
  int k; // {loop index}
  int p, q, m, u, v, w, c; // {general purpose registers}

  post_loc = cur_loc - 1;
  printf("Postamble starts at byte %d", post_loc);
  // Postamble starts at byte n
  if (post_loc==gf_prev_ptr) print_ln(".");
  else printf(", after special info at byte %d\n", gf_prev_ptr);
  p = signed_quad();
  if (p != gf_prev_ptr) {
    sprintf(temp_buf, "postamble pointer should be %d, not %d!", gf_prev_ptr, p);
    error(temp_buf);
  }
  // backpointer...should be p
  design_size = signed_quad(); check_sum = signed_quad();
  printf("design size = %d (", design_size);
  print_scaled(design_size / 16); print_ln("pt)");
  printf("check sum = %d\n", check_sum);
  hppp = signed_quad(); vppp = signed_quad();
  printf("hppp = %d (", hppp);
  print_scaled(hppp);
  printf(")");
  print_ln("");
  printf("vppp = %d (", vppp);
  print_scaled(vppp);
  printf(")");
  print_ln("");
  pix_ratio = (design_size / 1048576.0) * (hppp / 1048576.0);
  min_m_stated = signed_quad(); max_m_stated = signed_quad();
  min_n_stated = signed_quad(); max_n_stated = signed_quad();
  printf("min m = %d, max m = %d\n", min_m_stated, max_m_stated);
  if (min_m_stated > min_m_overall) {
    sprintf(temp_buf, "min m should be <= %d!", min_m_overall);
    error(temp_buf);
  }
  if (max_m_stated < max_m_overall) {
    sprintf(temp_buf, "max m should be >= %d!", max_m_overall);
    error(temp_buf);
  }

  printf("min n = %d, max n = %d\n", min_n_stated, max_n_stated);
  if (min_n_stated > min_n_overall) {
    sprintf(temp_buf, "min n should be <= %d!", min_n_overall);
    error(temp_buf);
  }
  if (max_n_stated < max_n_overall) {
    sprintf(temp_buf, "max n should be >= %d!", max_n_overall);
    error(temp_buf);
  }
  // <Process the character locations in the postamble>
  do {
    a = cur_loc; k = get_byte();
    if (k == char_loc || k == char_loc + 1)
    {
      c = first_par(k);
      if (k == char_loc) {
        u = signed_quad(); v = signed_quad();
      }
      else {
        u = get_byte() * unity; v = 0;
      }
      w = signed_quad(); p = signed_quad();
      printf("Character %d: dx %d (", c, u);
      print_scaled(u);
      if (v != 0) {
        printf("), dy %d (", v);
        print_scaled(v);
      }
      printf("), width %d (", w);
      w = round(w * pix_ratio);
      print_scaled(w);
      printf("), loc %d\n",p);
      if (char_ptr[c] == 0)
        error("duplicate locator for this character!");
      else if (p != char_ptr[c]) {
        sprintf(temp_buf, "character location should be %d!", char_ptr[c]);
        error(temp_buf);
      }
      char_ptr[c] = 0;
      k = no_op;
    }
  } while(k == no_op);

  // <Make sure that the end of the file is well-formed>
  if (k != post_post) {
    error("should be postpost!");
  }
  for (k = 0; k <= 255; k++) {
    if (char_ptr[k] > 0) {
      sprintf(temp_buf, "missing locator for character %c!", k);
      error(temp_buf);
    }
  }
  q = signed_quad();
  if (q != post_loc) {
    sprintf(temp_buf, "postamble pointer should be %d, not %d!", post_loc, q);
    error(temp_buf);
  }
  // <Make sure that the end of the file is well-formed>
  m = get_byte();
  if (m != gf_id_byte) {
    sprintf(temp_buf, "identification byte should be %d, not %d!", gf_id_byte, m);
    error(temp_buf);
  }

  k = cur_loc; m = 223;
  while (m == 223 && !feof(gf_file)) {
    m = get_byte();
  }
  if (!feof(gf_file)) {
    sprintf(temp_buf, "signature in byte %d should be 223!", cur_loc - 1);
    bad_gf(temp_buf);
  }
  else if (cur_loc < k + 4)
    error("not enough signature bytes at end of file!");
}

int main(int argc, char* argv[])
{
  initialize(); // {get all variables initialized}

  // Parse and validate command-line arguments
  if (!parse_and_validate_args(argc, argv)) {
    return 1;
  }

  // <Process the preamble>
  open_gf_file();
  int o = get_byte(); // {fetch the first byte}
  if (o != pre) bad_gf("First byte isn't start of preamble!");

  o = get_byte(); // {fetch the identification byte}
  if (o != gf_id_byte) {
    sprintf(temp_buf, "Identification byte should be %d, not %d!", gf_id_byte, o);
    bad_gf(temp_buf);
  }
  // identification byte should be n
  o = get_byte(); // {fetch the length of the introductory comment}
  printf("'");
  while (o > 0) {
    decr(o);
    printf("%c", xchr[get_byte()]);
  }
  printf("'\n");
  ///////////////////////////////////////////////////////////////

  // <Translate all the characters>
  do {
    gf_prev_ptr = cur_loc;
    // <Pass |no_op|, |xxx| and |yyy| commands>
    do {
      start_op;
      if (o == yyy) {
        // <Translate a |yyy|...>
        sprintf(temp_buf, "yyy %d (", p);
        show_mnemonic(temp_buf);
        if (wants_mnemonics) {
          print_scaled(p);
          fputc(')', stdout);
        }
        o = no_op;
      }
      else if (o >= xxx1 && o <= xxx1 + 3) {
        // <Translate an |xxx|...>
        show_mnemonic("xxx '"); bad_char = false; b = 16;
        if (p < 0) nl_error("string of negative length!");
        while (p > 0)
        {
          q = get_byte();
          if (q < ' ' || q > '~') bad_char = true;
          if (wants_mnemonics) {
            fputc(xchr[q], stdout);
            if (b < line_length) incr(b);
            else  {
              print_nl(); b = 2;
            }
          }
          decr(p);
        }
        if (wants_mnemonics) print("'");
        if (bad_char) nl_error("non-ASCII character in xxx command!");
        o = no_op;
      }
      else if (o == no_op) {
        show_mnemonic("no op");
      }
    } while (o == no_op);

    if (o != post)
    {
      if (o != boc)
        if (o != boc1) {
          snprintf(temp_buf, sizeof(temp_buf)-128, "byte %d is not boc (%d)", cur_loc -1, o);
          bad_gf(temp_buf);
        }
      print_nl();
      printf("%d: beginning of char ", cur_loc - 1);
      // <Pass a boc command>
      a = cur_loc - 1; incr(total_chars);
      if (o == boc)
      {
        character_code = signed_quad();
        p = signed_quad();
        c = character_code % 256;
        if (c < 0) c = c + 256;
        min_m_stated = signed_quad(); max_m_stated = signed_quad();
        min_n_stated = signed_quad(); max_n_stated = signed_quad();
      }
      else {
        character_code = get_byte(); p = -1; c = character_code;
        q = get_byte(); max_m_stated = get_byte(); min_m_stated = max_m_stated - q;
        q = get_byte(); max_n_stated = get_byte(); min_n_stated = max_n_stated - q;
      }
      printf("%d", c);
      if (character_code != c) {
        printf(" with extension %d", (character_code - c) / 256);
      }
      if (wants_mnemonics) {
        printf(": %d<=m<=%d %d<=n<=%d\n", min_m_stated,max_m_stated,
          min_n_stated,max_n_stated);
      }
      max_m_observed = -1;
      if (char_ptr[c] != p) {
        sprintf(temp_buf, "previous character pointer should be %d, not %d!", p, char_ptr[c]);
        error(temp_buf);
      }
      else if (p > 0) 
      if (wants_mnemonics) {
        printf("(previous character with the same code started at byte %d)", p);
      }
      char_ptr[c] = gf_prev_ptr;
      if (wants_mnemonics) {
        printf("(initially n=%d)", max_n_stated);
      }
      if (wants_pixels) {
        // <Clear the image>
        max_subcol = max_m_stated - min_m_stated - 1;
        if (max_subcol > max_col) max_subcol = max_col;
        max_subrow = max_n_stated - min_n_stated;
        if (max_subrow > max_row) max_subrow = max_row;
        n = 0;
        while (n <= max_subrow) {
          m = 0;
          while (m <= max_subcol) {
            image = white;
            incr(m);
          }
          incr(n);
        }
      }
      m = 0; n = 0; paint_switch = white;

      if (!do_char()) {
        bad_gf("char ended unexpectedly");
      }
      max_n_observed = n;
      if (wants_pixels) {
        // Print the image
        // <Compare the subarray boundaries with the observed boundaries>
        if (max_m_observed > max_col || max_n_observed > max_row) {
          printf("(The character is too large to be displayed in full.)\n");
        }
        if (max_subcol > max_m_observed) {
          max_subcol = max_m_observed;
        }
        if (max_subrow > max_n_observed) {
          max_subrow = max_n_observed;
        }
        if (max_subcol >= 0) { // there was at least one \\{paint} command
          // <Print asterisk patterns for rows 0 to |max_subrow|>
          printf(".<--This pixel's lower left corner is at (%d,%d) in METAFONT coordinates\n", min_m_stated, max_n_stated + 1);
          n = 0;
          while (n <= max_subrow) {
            m = 0; b = 0;
            while (m <= max_subcol) {
              if (image == white) {
                incr(b);
              } else {
                while (b > 0) {
                  printf(" ");
                  decr(b);
                }
                printf("*");
              }
              incr(m);
            }
            printf("\n");
            incr(n);
          }
        } else {
          printf("(The character is entirely blank.)");
        }
        printf(".<--This pixel's upper left corner is at (%d,%d) in METAFONT coordinates\n",
            min_m_stated, max_n_stated - max_subrow);
      }
      //<Pass an eoc command>
      max_m_observed = min_m_stated + max_m_observed + 1;
      n = max_n_stated - max_n_observed; // {now |n| is the minimum |n| observed}
      if (min_m_stated < min_m_overall) min_m_overall = min_m_stated;
      if (max_m_observed > max_m_overall) max_m_overall = max_m_observed;
      if (n<min_n_overall) min_n_overall = n;
      if (max_n_stated > max_n_overall) max_n_overall = max_n_stated;
      if (max_m_observed > max_m_stated)
        printf("The previous character should have had max m >= %d!", max_m_observed);
      if (n < min_n_stated)
        printf("The previous character should have had min n <= %d!", n);
    }
  } while (o != post);

  print_nl();
  read_postamble();
  printf("The file had %d character",total_chars);
  if (total_chars != 1) print("s");
  print(" altogether.");
  print_nl();
  return 0;
}
