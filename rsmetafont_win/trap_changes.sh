#!/bin/bash
# After running cpf on the .pre files you can run this script to make changes suitable for metafont trap test.
# See https://ctan.org/tex-archive/systems/knuth/dist/mf for instructions on how to run the test.
# The capsule numbers will show up wrong in the test but this also happens with texlive and MikTeX
# which is curious. Update: Someone hinted that Knuth's trap test may not have been updated in a long time.
sed -i 's/mem_max = [0-9]\+;/mem_max = 3000;/' rsMetaFont.h
sed -i 's/mem_top = [0-9]\+;/mem_top = 3000;/' rsMetaFont.h
sed -i 's/error_line = [0-9]\+;/error_line = 64;/' rsMetaFont.h
sed -i 's/half_error_line = [0-9]\+;/half_error_line = 32;/' rsMetaFont.h
sed -i 's/max_print_line = [0-9]\+;/max_print_line = 72;/' rsMetaFont.h
sed -i 's/screen_width = [0-9]\+;/screen_width = 100;/' rsMetaFont.h
sed -i 's/screen_depth = [0-9]\+;/screen_depth = 200;/' rsMetaFont.h
sed -i 's/gf_buf_size = [0-9]\+;/gf_buf_size = 8;/' rsMetaFont.h
perl -i -p0e "s/bool init_screen\(\).*?$.*?^}/bool init_screen()\n{\n\treturn true;\n}\n/gms" rsMetaFont.cpp
perl -i -p0e "s/void update_screen.*?$.*?^}/void update_screen()\n{\n\twlog_ln_s\(\"Calling UPDATESCREEN\"\);\n}\n/gms" rsMetaFont.cpp
perl -i -p0e "s/void blank_rectangle\(screen_col.*?$.*?^}/void blank_rectangle(screen_col left_col,screen_col right_col, screen_row top_row,screen_row bot_row)\n{\twlog_cr;\n\tfprintf(log_file, \"Calling BLANKRECTANGLE(%d,%d,%d,%d)\\\\n\", left_col, right_col, top_row, bot_row);\n}\n/gms" rsMetaFont.cpp
perl -i -p0e "s/void paint_row\(screen_row.*?$.*?^}/void paint_row(screen_row r, pixel_color b, trans_spec& a, screen_col n)\n{\n\tscreen_col k;\n\tfprintf(log_file, \"Calling PAINTROW(%d,%d;\", r, b);\n\tfor (k = 0; k <= n; k++)\n\t{\n\t\tfprintf(log_file, \"%d\", a[k]); if (k != n) fprintf(log_file, \",\");\n\t}\n\tfprintf(log_file, \")\\\\n\");\n}\n/gms" rsMetaFont.cpp
