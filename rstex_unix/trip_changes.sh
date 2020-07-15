#!/bin/bash
# after running cpf on the .pre files you can run this script to make changes suitable for tex trip test
# see https://ctan.org/tex-archive/systems/knuth/dist/texfor instruction on how to run the test
sed -i 's/mem_min = [0-9]\+;/mem_min = 1;/' rstex.h
sed -i 's/mem_bot = [0-9]\+;/mem_bot = 1;/' rstex.h
sed -i 's/mem_top = [0-9]\+;/mem_top = 3000;/' rstex.h
sed -i 's/mem_max = [0-9]\+;/mem_max = 3000;/' rstex.h
sed -i 's/error_line = [0-9]\+;/error_line = 64;/' rstex.h
sed -i 's/half_error_line = [0-9]\+;/half_error_line = 32;/' rstex.h
sed -i 's/max_print_line = [0-9]\+;/max_print_line = 72;/' rstex.h
sed -i 's/stack_size = [0-9]\+;/stack_size = 200;/' rstex.h
sed -i 's/font_max = [0-9]\+;/font_max = 75;/' rstex.h
sed -i 's/font_mem_size = [0-9]\+;/font_mem_size = 20000;/' rstex.h
sed -i 's/param_size = [0-9]\+;/param_size = 60;/' rstex.h
sed -i 's/nest_size = [0-9]\+;/nest_size = 40;/' rstex.h
sed -i 's/max_strings = [0-9]\+;/max_strings = 3000;/' rstex.h
sed -i 's/string_vacancies = [0-9]\+;/string_vacancies = 8000;/' rstex.h
sed -i 's/pool_size = [0-9]\+;/pool_size = 32000;/' rstex.h
sed -i 's/save_size = [0-9]\+;/save_size = 600;/' rstex.h
sed -i 's/dvi_buf_size = [0-9]\+;/dvi_buf_size = 800;/' rstex.h
