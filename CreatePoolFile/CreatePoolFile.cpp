/*
Please excuse the crudity of this program. I didn't have time to make it robust or to make it pretty.

*/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>


// local char set to ``ascii''
char local_to_ascii[256];

void init_charset()
{
	for (int i = 0; i <= 255; i++) {
		local_to_ascii[i] = i;
	}
	local_to_ascii[' '] = 32;
	local_to_ascii['!'] = 33;
	local_to_ascii['\"'] = 34;
	local_to_ascii['#'] = 35;
	local_to_ascii['$'] = 36;
	local_to_ascii['%'] = 37;
	local_to_ascii['&'] = 38;
	local_to_ascii['\''] = 39;
	local_to_ascii['('] = 40;
	local_to_ascii[')'] = 41;
	local_to_ascii['*'] = 42;
	local_to_ascii['+'] = 43;
	local_to_ascii[','] = 44;
	local_to_ascii['-'] = 45;
	local_to_ascii['.'] = 46;
	local_to_ascii['/'] = 47;
	local_to_ascii['0'] = 48;
	local_to_ascii['1'] = 49;
	local_to_ascii['2'] = 50;
	local_to_ascii['3'] = 51;
	local_to_ascii['4'] = 52;
	local_to_ascii['5'] = 53;
	local_to_ascii['6'] = 54;
	local_to_ascii['7'] = 55;
	local_to_ascii['8'] = 56;
	local_to_ascii['9'] = 57;
	local_to_ascii[':'] = 58;
	local_to_ascii[';'] = 59;
	local_to_ascii['<'] = 60;
	local_to_ascii['='] = 61;
	local_to_ascii['>'] = 62;
	local_to_ascii['?'] = 63;
	local_to_ascii['@'] = 64;
	local_to_ascii['A'] = 65;
	local_to_ascii['B'] = 66;
	local_to_ascii['C'] = 67;
	local_to_ascii['D'] = 68;
	local_to_ascii['E'] = 69;
	local_to_ascii['F'] = 70;
	local_to_ascii['G'] = 71;
	local_to_ascii['H'] = 72;
	local_to_ascii['I'] = 73;
	local_to_ascii['J'] = 74;
	local_to_ascii['K'] = 75;
	local_to_ascii['L'] = 76;
	local_to_ascii['M'] = 77;
	local_to_ascii['N'] = 78;
	local_to_ascii['O'] = 79;
	local_to_ascii['P'] = 80;
	local_to_ascii['Q'] = 81;
	local_to_ascii['R'] = 82;
	local_to_ascii['S'] = 83;
	local_to_ascii['T'] = 84;
	local_to_ascii['U'] = 85;
	local_to_ascii['V'] = 86;
	local_to_ascii['W'] = 87;
	local_to_ascii['X'] = 88;
	local_to_ascii['Y'] = 89;
	local_to_ascii['Z'] = 90;
	local_to_ascii['['] = 91;
	local_to_ascii['\\'] = 92;
	local_to_ascii[']'] = 93;
	local_to_ascii['^'] = 94;
	local_to_ascii['_'] = 95;
	local_to_ascii['`'] = 96;
	local_to_ascii['a'] = 97;
	local_to_ascii['b'] = 98;
	local_to_ascii['c'] = 99;
	local_to_ascii['d'] = 100;
	local_to_ascii['e'] = 101;
	local_to_ascii['f'] = 102;
	local_to_ascii['g'] = 103;
	local_to_ascii['h'] = 104;
	local_to_ascii['i'] = 105;
	local_to_ascii['j'] = 106;
	local_to_ascii['k'] = 107;
	local_to_ascii['l'] = 108;
	local_to_ascii['m'] = 109;
	local_to_ascii['n'] = 110;
	local_to_ascii['o'] = 111;
	local_to_ascii['p'] = 112;
	local_to_ascii['q'] = 113;
	local_to_ascii['r'] = 114;
	local_to_ascii['s'] = 115;
	local_to_ascii['t'] = 116;
	local_to_ascii['u'] = 117;
	local_to_ascii['v'] = 118;
	local_to_ascii['w'] = 119;
	local_to_ascii['x'] = 120;
	local_to_ascii['y'] = 121;
	local_to_ascii['z'] = 122;
	local_to_ascii['{'] = 123;
	local_to_ascii['|'] = 124;
	local_to_ascii['}'] = 125;
	local_to_ascii['~'] = 126;
}

std::string to_ascii(std::string s)
{
	std::string asciistring;
	for (size_t i = 0; i < s.size(); i++) {
		asciistring.append(1, local_to_ascii[s[i]]);
	}
	return asciistring;
}

int main(int argc, char **argv)
{
	// first argument should be a string e.g. TEX_STRING
	// any string of the form TEX_STRING("blaha") will then be replaced by a number representing an entry in the stringpool file
	// corresponding to this string
	// second and third argument and so on represent files which should be searched for strings of above sort.

	if (argc < 3) {
		std::cerr << "Error too few arguments. Need <string> <filename1> <filename2> ...\n";
		return 1;
	}

	int pool_check_sum = 271828;
	int check_sum_prime = 536870839;

	init_charset();


	std::string replace_text(argv[1]);
	replace_text += "(\"";
	std::vector<std::string> pool_table;

	std::ofstream pool_file("tex.pool");
	if (!pool_file) {
		std::cerr << "Error opening " << "tex.pool" << " aborting.\n";
		return 1;
	}
	// for each file
	for (int i = 2; i < argc; i++) {
		std::string filename = argv[i];
		std::string outfilename = filename + ".out";
		std::ofstream output_file(outfilename.c_str());
		if (!output_file) {
			std::cerr << "Error opening " << outfilename << " aborting.\n";
			return 1;
		}
		std::ifstream filein(filename.c_str());
		if (!filein) {
			std::cerr << "Error opening " << filename << " aborting.\n";
			return 1;
		}

		std::string sline;
		int line_count = 0;
		while (std::getline(filein, sline)) {
			line_count++;
			size_t start_fpos = 0;
			size_t end_prev_fpos = 0;


			while ((start_fpos = sline.find(replace_text, end_prev_fpos)) != std::string::npos) {
				// write everything from previous TEX_STRING up until this TEX_STRING
				for (size_t k = end_prev_fpos; k < start_fpos; k++) {
					output_file << sline[k];
				}

				// found the string, do replacement
				// find ending quote mark
				bool found_end_quote = false;

				std::string replacement;
				if (start_fpos + replace_text.size() + 1 >= sline.size()) {
					std::cerr << "Line ended before replacement text was complete." << " Line: " << line_count << " Aborting.\n";
					return 1;
				}
				for (size_t i = start_fpos + replace_text.size(); i < sline.size() - 1; i++) {
					if (sline[i] == '\"') {
						// possible end quote
						if (sline[i + 1] == '\"') {
							// double quote, store one quote mark and continue searching
							replacement += '\"';
							i++;
						}
						else {
							if (sline[i + 1] != ')') {
								std::cerr << "Format error not valid TEX_STRING(\"blaha\")" << " on line " << line_count << " aborting.\n";
								return 1;
							}
							found_end_quote = true;
							end_prev_fpos = start_fpos = i + 2;
							break;
						}
					}
					else {
						replacement += sline[i];
					}
				}

				if (!found_end_quote) {
					std::cerr << "No valid end quote found in string." << " Line: " << line_count << " Aborting.\n";
					return 1;
				}

				size_t index_of_string;

				if (replacement.size() != 1) {
					std::string replacement_ascii = to_ascii(replacement);
					auto it = std::find(pool_table.begin(), pool_table.end(), replacement_ascii);

					if (it == pool_table.end())
					{
						pool_table.push_back(replacement_ascii);
						pool_check_sum += pool_check_sum + replacement_ascii.size();
						while (pool_check_sum > check_sum_prime)
							pool_check_sum -= check_sum_prime;
						for (size_t i = 0; i < replacement_ascii.size(); i++) {
							pool_check_sum += pool_check_sum + replacement_ascii[i];
							while (pool_check_sum > check_sum_prime)
								pool_check_sum -= check_sum_prime;
						}
						index_of_string = pool_table.size() - 1 + 256;
					}
					else {
						index_of_string = std::distance(pool_table.begin(), it) + 256;
					}
				}
				else {
					index_of_string = local_to_ascii[replacement[0]];// just use the character code in case of a one character string
				}
				// Put the text in as a comment next to the number
				// If the text contains */ we're in trouble, better check that
				if (replacement.find("*/") == std::string::npos) {
					output_file << "/*" << replacement << "*/" << index_of_string;
				}
				else {
					output_file << "/*" << "This text contained comment characters, refer to unprocessed file for this one." << "*/" << index_of_string;
				}
			}

			for (size_t k = end_prev_fpos; k < sline.size(); k++) {
				output_file << sline[k];
			}
			output_file << std::endl;

		}
	}

	// write the strings to the pool file

	auto it = pool_table.begin();
	for (; it != pool_table.end(); ++it) {
		int len = it->size();
		char first_digit = local_to_ascii['0']+len / 10;
		char second_digit = local_to_ascii['0']+len % 10;
		pool_file << first_digit << second_digit << *it << '\n';
		
	}

	// finish off with the checksum
	pool_file << local_to_ascii['*'];
	char check_digits[10] = { 0 }; // 9 digits
	for (int k = 0; k < 9; k++) {
		check_digits[k] = local_to_ascii['0'] + pool_check_sum % 10;
		pool_check_sum /= 10;
	}
	for (int k = 8; k >= 0; k--) {
		pool_file << check_digits[k];
	}
	pool_file << '\n';

	// Now that we have the checksum we need to scan the outfiles to see if there's the
	// special macro @$ which will be replaced by the checksum
	// Limitation, only replaces the first @$ in the line.
	for (int i = 2; i < argc; i++) {
		std::string filename = argv[i];
		filename += ".out";
		std::string outfilename = filename + ".out";

		std::ofstream out_file(outfilename.c_str());
		if (!out_file) {
			std::cerr << "Error opening " << outfilename << " aborting.\n";
			return 1;
		}
		std::ifstream filein(filename.c_str());
		std::string sline;

		
		while (std::getline(filein, sline)) {
			size_t pos = sline.find("@$", 0);
			if (pos != std::string::npos)
			{
				// write line modified
				for (size_t k = 0; k < sline.size(); k++) {
					if (k == pos) {
						// write checksum instead
						// skip leading zeros since they're interpreted as octal in C
						int num_leading_zeros = 0;
						for (int j = 8; j >= 0; j--) {
							if (check_digits[j] == '0')
								num_leading_zeros++;
							else
								break;
						}
						if (num_leading_zeros == 9)
							out_file << '0';
						else {
							for (int j = 8 - num_leading_zeros; j >= 0; j--) {
								out_file << check_digits[j];
							}
						}
					}
					if (k < pos || k > pos + 1) {
						out_file << sline[k];
					}
				}
			}
			else {
				// write as is
				out_file << sline;
			}
			out_file << std::endl;
		}

		// clean up filenames a bit
		filein.close();
		out_file.close();

		// remove the temporary file, no need for this anymore
		remove(filename.c_str());

		// now we have filename equal to something like file.cpp.pre.out
		// remove last 8 characters, unless we can't
		
		if (filename.size() >= 9)
			filename.resize(filename.size() - 8);
		// Before we rename the new file, get rid of the old one, move it to .bak just in case
		// But before we do that we better remove the old .bak file if there was one
		remove((filename + ".bak").c_str());
		rename(filename.c_str(), (filename + ".bak").c_str());
		rename(outfilename.c_str(), filename.c_str());

	}

}

