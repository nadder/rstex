/*
Please excuse the crudity of this program. I didn't have time to make it robust or to make it pretty.
*/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>

int main(int argc, char **argv)
{
	// first argument should be a string e.g. TEX_STRING
	// any string of the form TEX_STRING("blaha") will then be replaced by a number representing an entry in the stringpool file
	// corresponding to this string
	// second and third argument and so on represent files which should be searched for strings of above sort.

	if (argc < 3) {
		std::cerr << "Error too few arguments\n. Need <string> <filename1> <filename2> ...\n";
		return 1;
	}

	int pool_check_sum = 271828;
	int check_sum_prime = 536870839;

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
		std::ofstream output_file(outfilename);
		if (!output_file) {
			std::cerr << "Error opening " << outfilename << " aborting.\n";
			return 1;
		}
		std::ifstream filein(filename);
		if (!filein) {
			std::cerr << "Error opening " << filename << " aborting.\n";
			return 1;
		}

		std::string sline;
		int line_count = 0;
		while (std::getline(filein, sline)) {
			line_count++;
			int start_fpos = 0;
			int end_prev_fpos = 0;


			while ((start_fpos = sline.find(replace_text, end_prev_fpos)) != std::string::npos) {
				// write everything from previous TEX_STRING up until this TEX_STRING
				for (int k = end_prev_fpos; k < start_fpos; k++) {
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
					auto it = std::find(pool_table.begin(), pool_table.end(), replacement);

					if (it == pool_table.end())
					{
						pool_table.push_back(replacement);
						pool_check_sum += pool_check_sum + replacement.size();
						while (pool_check_sum > check_sum_prime)
							pool_check_sum -= check_sum_prime;
						for (size_t i = 0; i < replacement.size(); i++) {
							pool_check_sum += pool_check_sum + replacement[i];
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
					index_of_string = replacement[0];// just use the character code in case of a one character string
				}
				output_file << index_of_string;

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
		char first_digit = '0'+len / 10;
		char second_digit = '0'+len % 10;
		pool_file << first_digit << second_digit << *it << '\n';
		
	}

	// finish off with the checksum
	pool_file << '*';
	char check_digits[10] = { 0 }; // 9 digits
	for (int k = 0; k < 9; k++) {
		check_digits[k] = '0' + pool_check_sum % 10;
		pool_check_sum /= 10;
	}
	for (int k = 8; k >= 0; k--) {
		pool_file << check_digits[k];
	}
	pool_file << '\n';

	// now that we have the checksum we need to scan the outfiles to see if there's the
	// special macro @$ which will be replaced by the checksum

	for (int i = 2; i < argc; i++) {
		std::string filename = argv[i];
		filename += ".out";
		std::string outfilename = filename + ".out";

		std::ofstream out_file(outfilename);
		if (!out_file) {
			std::cerr << "Error opening " << outfilename << " aborting.\n";
			return 1;
		}
		std::ifstream filein(filename);
		std::string sline;
		int line_count = 0;
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
		int ret = remove(filename.c_str());

		// now we have filename equal to something like file.cpp.pre.out
		// remove last 8 characters, unless we can't
		
		if (filename.size() >= 9)
			filename.resize(filename.size() - 8);
		// Before we rename the new file, get rid of the old one, move it to .bak just in case
		rename(filename.c_str(), (filename + ".bak").c_str());
		rename(outfilename.c_str(), filename.c_str());

	}

}

