#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <io.h>
#include "ReadDVI.h"
#include "ReadDVIinternal.h"

#pragma warning(disable:4390)

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

std::vector<Font> FontVector;
std::vector<std::vector<Character>> PageCharVector;
std::vector<std::vector<Rule>> PageRuleVector;

typedef unsigned char eight_bits;

enum paint_color
{
	pw_white,
	pw_black
};


HBITMAP CreateBmp(const char *filename, unsigned char *pixel_data, int nRows, int nCols)
{
	// create a bitmap file
	// assuming 24 bits per pixel

	unsigned nBytesPerPixel = 3;
	unsigned row_padding_bytes = (4 - (nCols*nBytesPerPixel) % 4)%4;
	unsigned row_size = nCols*nBytesPerPixel + row_padding_bytes;
	unsigned pixel_array_size = row_size*nRows;

	BITMAPFILEHEADER bfh = { 0 };
	bfh.bfType = 0x4D42;
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixel_array_size;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	BITMAPINFOHEADER bmih = { 0 };
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = nCols;
	bmih.biHeight = nRows;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = pixel_array_size;
	bmih.biXPelsPerMeter = 2835;
	bmih.biYPelsPerMeter = 2835;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	unsigned char *buffer = new unsigned char[bfh.bfSize];
	memcpy(buffer, &bfh, sizeof bfh);
	memcpy(buffer + sizeof bfh, &bmih, sizeof bmih);

	unsigned char *pixel_ptr = buffer + bfh.bfOffBits;
	for (size_t row = 0; row < nRows; row++) {
		for (size_t col = 0; col < nCols; col++) {
			for (size_t bb = 0; bb < 3; bb++) {
				*pixel_ptr++ = pixel_data[row*(nCols*nBytesPerPixel) + col*nBytesPerPixel + bb];
			}
		}
		// padding for each row
		for (size_t pad = 0; pad < row_padding_bytes; pad++) {
			*pixel_ptr++ = 0;
		}		
		
	}



	//HBITMAP hBitmap = CreateBitmap(nCols, nRows, 1, 24, buffer + bfh.bfOffBits);
	BITMAPINFO bminfo = {0};
	bminfo.bmiHeader = bmih;
	bminfo.bmiColors[0].rgbBlue = 0;
	bminfo.bmiColors[0].rgbGreen = 0;
	bminfo.bmiColors[0].rgbRed = 0;
	bminfo.bmiColors[0].rgbReserved = 0;
	HDC hdc = GetDC(NULL);
	HBITMAP hBitmap = CreateDIBitmap(hdc, &bmih, CBM_INIT, buffer+bfh.bfOffBits, &bminfo, DIB_RGB_COLORS);
	//FILE *fp = fopen("tempout.bmp", "wb");
	//if (fp) {
	//	fwrite(buffer, 1, bfh.bfSize, fp);
	//	fclose(fp);
	//}



	delete[] buffer;

	return hBitmap;
}


class pw_matrix
{
public:
	void init(int min_row, int max_row, int min_col, int max_col) {
		_min_row = min_row;
		_min_col = min_col;
		_max_row = max_row;
		_max_col = max_col - 1; // hack to get rid of last column never being used
		offset_row = -min_row;
		offset_col = -min_col;
		nRows = max_row - min_row + 1;
		nCols = max_col - min_col + 1;
		image_data.clear();
		image_data.resize(nRows*nCols,0);
	}

	int get(int row, int col) {
		int index = (row + offset_row)*nCols + (col + offset_col);
		return image_data[index];
	}

	void set(int row, int col, int val) {
		int index = (row+offset_row)*nCols + (col + offset_col);
		if (index < 0) {
			int k = 10;
			k++;
		}
		image_data[index] = val;
	}

	HBITMAP WriteToBmp(const std::string fname)
	{
		// create pixel buffer, 3 bytes per pixel
		int buf_size = nRows*nCols * 3;
		unsigned char *buffer = new unsigned char[buf_size];
		unsigned char *pbuf = buffer;

		for (int r = _min_row; r <= _max_row; r++) {
			for (int c = _min_col; c <= _max_col; c++) {
				int val = get(r, c);
				if (val == 0) {
					// white pixel
					*pbuf++ = 0xFF; *pbuf++ = 0xFF; *pbuf++ = 0xFF;
				}
				else {
					// black pixel
					if (val != 1) {
						int a = 10;
						a++;
					}
					*pbuf++ = 0x00; *pbuf++ = 0x00; *pbuf++ = 0x00;
				}
			}
		}

		HBITMAP hBitmap = CreateBmp(fname.c_str(), buffer, _max_row-_min_row+1, _max_col-_min_col+1);
		delete[] buffer;
		return  hBitmap;
		
	}

private:
	std::vector<int> image_data;
	int offset_row;
	int offset_col;
	int _min_row;
	int _min_col;
	int _max_row;
	int _max_col;
	int nRows;
	int nCols;
	int actual_min_row;
	int actual_max_row;
	int actual_min_col;
	int actual_max_col;
};


int index_from_charcode(const Font *f, int charcode)
{

	for (size_t i = 0; i < f->Char.size(); i++) {
		if (f->Char[i].char_code == charcode)
			return static_cast<int>(i);
	}
	return -1;
}


int dvi_length(FILE *fp)
{ 
	fseek(fp, 0, SEEK_END);
	return (int)ftell(fp);
}

void move_to_byte(FILE *fp, int *p_cur_loc, int n)
{
	fseek(fp, n, SEEK_SET);
	*p_cur_loc=n;
}


int get_byte(FILE *fp, int *p_cur_loc) //{returns the next byte, unsigned}
{
	eight_bits b;
	int c;
	c = fgetc(fp);
	if (c == EOF)
		return 0;
	
	b = c;
	incr(*p_cur_loc); 
	return b;
}

int signed_byte(FILE *fp, int *p_cur_loc) //{returns the next byte, signed}
{
	eight_bits b;
	b = fgetc(fp); incr(*p_cur_loc);
	if (b<128) return b; else return b-256;
}


int get_two_bytes(FILE *fp, int *p_cur_loc) //{returns the next two bytes, unsigned}
{
	eight_bits a, b;
	a = fgetc(fp); b = fgetc(fp);
	*p_cur_loc=*p_cur_loc+2;
	return a*256+b;
}

int signed_pair(FILE *fp, int *p_cur_loc) //{returns the next two bytes, signed}
{
	eight_bits a,b;
	a = fgetc(fp); b = fgetc(fp);
	*p_cur_loc=*p_cur_loc+2;
	if (a<128) return a*256+b;
	else return (a-256)*256+b;
}



int get_three_bytes(FILE *fp, int *p_cur_loc) //{returns the next three bytes, unsigned}
{
	eight_bits a,b,c;
	a = fgetc(fp); b = fgetc(fp); c = fgetc(fp);
	*p_cur_loc=*p_cur_loc+3;
	return (a*256+b)*256+c;
}

int signed_trio(FILE *fp, int *p_cur_loc) //{returns the next three bytes, signed}
{
	eight_bits a,b,c;
	a = fgetc(fp); b = fgetc(fp); c = fgetc(fp);
	*p_cur_loc=*p_cur_loc+3;
	if (a<128) return (a*256+b)*256+c;
	else return ((a-256)*256+b)*256+c;
}


int signed_quad(FILE *fp, int *p_cur_loc) //{returns the next four bytes, signed}
{
	eight_bits a,b,c,d;
	a = fgetc(fp); b = fgetc(fp); c = fgetc(fp); d = fgetc(fp);
	*p_cur_loc=*p_cur_loc+4;
	if (a<128) return ((a*256+b)*256+c)*256+d;
	else return (((a-256)*256+b)*256+c)*256+d;
}


bool ReadGFFile(int q)
{
	pw_matrix ci;

	Font f;
	FontChar fc_cur;
	*f.filename = 0;
	strncat(f.filename, cur_name.get_c_str(), sizeof f.filename);
	//f.filename[0] = 0;
	//strncat(f.filename, pFileName, sizeof f.filename);
	int current_char=0;
	// read the whole file into a buffer
	//std::ifstream input(pFileName, std::ios::binary);
	//if (!input) {
	//	char *p = strerror(errno);
	//	std::cout << "Error opening: " << pFileName << std::endl;
	//	return f;
	//}
	//std::vector<char> buffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
	//cur_file = gf_file;

	// first try to find out how big the image should be
	width_base[nf] = width_ptr;
	int paint_switch = pw_black;
	int m = 0;
	int n = 0;
	int g_min_m = 0;
	int cur_char = 0;
	bool done = false;
	while (!done) {
		// get next instruction
		unsigned char cur_instr = get_byte(gf_file, &gf_cur_loc);

		if (cur_instr >= 0 && cur_instr < 64) {
			// paint_0 through paint_63

			int num_pixels = (unsigned char)cur_instr;
			for (int i = 0; i < num_pixels; i++) {
				ci.set(n, m, paint_switch);
				m++;
			}

			if (paint_switch == pw_white)
				paint_switch = pw_black;
			else
				paint_switch = pw_white;

		}
		else if (cur_instr >=74 && cur_instr <= 238) {
			// new row
			n--;
			m = g_min_m + cur_instr - 74;
			paint_switch = pw_black;
		}
		else {
			switch (cur_instr) {
			case 64:
				// paint_64 with 1 byte parameter
				{
					int num_pixels = get_byte(gf_file, &gf_cur_loc);
					for (int i = 0; i < num_pixels; i++) {
						ci.set(n, m, paint_switch);
						m++;
					}
				}

				if (paint_switch == pw_white)
					paint_switch = pw_black;
				else
					paint_switch = pw_white;
				break;
			case 65:
				// paint_65 with 2 byte parameter
				{
					int num_pixels = get_byte(gf_file, &gf_cur_loc) * 256 + get_byte(gf_file, &gf_cur_loc);
					for (int i = 0; i < num_pixels; i++) {
						ci.set(n, m, paint_switch);
						m++;
					}
				}

				if (paint_switch == pw_white)
					paint_switch = pw_black;
				else
					paint_switch = pw_white;
				break;
			case 66:
				// paint_66 with 3 byte parameter
				{
					int num_pixels = get_byte(gf_file, &gf_cur_loc) * 256 * 256 + get_byte(gf_file, &gf_cur_loc) * 256 + get_byte(gf_file, &gf_cur_loc);
					for (int i = 0; i < num_pixels; i++) {
						ci.set(n, m, paint_switch);
						m++;
					}
				}


				if (paint_switch == pw_white)
					paint_switch = pw_black;
				else
					paint_switch = pw_white;
				break;

			case 67:
				// boc
			{
				int character = signed_quad(gf_file, &gf_cur_loc);

				int pointer = signed_quad(gf_file, &gf_cur_loc);;

				int min_m = signed_quad(gf_file, &gf_cur_loc);

				int max_m = signed_quad(gf_file, &gf_cur_loc);
				int min_n = signed_quad(gf_file, &gf_cur_loc);
				int max_n = signed_quad(gf_file, &gf_cur_loc);

				// initialize character array
				g_min_m = min_m;
				ci.init(min_n, max_n, min_m, max_m);
				//std::cout << "New char " << (int)character << " m: " << (int)min_m << " -> " << (int)max_m << " n: " << (int)min_n << " -> " << (int)max_n << std::endl;
				
				fc_cur.min_m = min_m;
				fc_cur.max_n = max_n;
				fc_cur.char_code = character;
				m = min_m;
				n = max_n;
				paint_switch = pw_white;

				

			}
			break;
			case 68:
				// boc1
				{
					unsigned char character = get_byte(gf_file, &gf_cur_loc);
					unsigned char del_m = get_byte(gf_file, &gf_cur_loc);
					unsigned char max_m = get_byte(gf_file, &gf_cur_loc);
					unsigned char del_n = get_byte(gf_file, &gf_cur_loc);
					unsigned char max_n = get_byte(gf_file, &gf_cur_loc);


					int min_n = max_n - del_n;
					int min_m = max_m - del_m;
					// initialize character array

					g_min_m = min_m;

					if (cur_char == 125)
					{
						int a = 10;
						a++;
					}

					ci.init(min_n, max_n, min_m, max_m);
					//std::cout << "New char " << (int)character << " m: " << (int)min_m << " -> " << (int)max_m << " n: " << (int)min_n << " -> " << (int)max_n << std::endl;

					fc_cur.min_m = min_m;
					fc_cur.max_n = max_n;
					fc_cur.char_code = character;
					m = min_m;
					n = max_n;
					paint_switch = pw_white;
				}
				break;

			case 69:
				// eoc
				{
					
					std::stringstream ss;
					ss << "Letter_" << std::setw(3) << std::setfill('0') << cur_char++ << ".bmp";
					HBITMAP hBitmap = ci.WriteToBmp(ss.str());
					fc_cur.hBitmap = hBitmap;
					f.Char.push_back(fc_cur);
					//std::cout << "End of character " << (cur_char - 1) << std::endl;
				}
				break;

			case 70:
				n--;
				m = g_min_m;
				paint_switch = pw_white;
				break;

			case 71:
				{
					int d = get_byte(gf_file, &gf_cur_loc);
					n -= d + 1;
					m = g_min_m;
					paint_switch = pw_white;
				}
				break;

			case 72:
				{
					int d = get_byte(gf_file, &gf_cur_loc) * 256 + get_byte(gf_file, &gf_cur_loc);
					n -= d + 1;
					m = g_min_m;
					paint_switch = pw_white;
				}
				break;
			case 73:
				{
					int d = get_byte(gf_file, &gf_cur_loc) * 256 * 256 + get_byte(gf_file, &gf_cur_loc) * 256 + get_byte(gf_file, &gf_cur_loc);
					n -= d + 1;
					m = g_min_m;
					paint_switch = pw_white;
				}
				break;
			case 74:
				// new_row0
				{
					n--;
					m = g_min_m;
					paint_switch = pw_black;
				}
				break;


			case 247:
				// pre
				{
					int i = get_byte(gf_file, &gf_cur_loc);
					int k = get_byte(gf_file, &gf_cur_loc);
					std::string data(k,0);
					for (int i = 0; i < k; i++) {
						data[i] = get_byte(gf_file, &gf_cur_loc);
					}
				}
				break;

			case 239:
				{
					// xxx1
					int k = get_byte(gf_file, &gf_cur_loc);
				}
				break;
			case 240:
				{
					int k = get_byte(gf_file, &gf_cur_loc) * 256 + get_byte(gf_file, &gf_cur_loc);
				}
				break;
			case 241:
				{
					int k = get_byte(gf_file, &gf_cur_loc) * 256 * 256 + get_byte(gf_file, &gf_cur_loc) * 256 + get_byte(gf_file, &gf_cur_loc);
				}
				break;
			case 242:
				{
					int k = signed_quad(gf_file, &gf_cur_loc);
				}
				break;

			case 243:
				// yyy
				break;

			case 244:
				// nop
				break;

			case 248:
				// post
				
				{
					int p = signed_quad(gf_file, &gf_cur_loc);
					int ds = signed_quad(gf_file, &gf_cur_loc);
					int cs = signed_quad(gf_file, &gf_cur_loc);
					int hppp = signed_quad(gf_file, &gf_cur_loc);
					int vppp = signed_quad(gf_file, &gf_cur_loc);
					int min_m = signed_quad(gf_file, &gf_cur_loc);
					int max_m = signed_quad(gf_file, &gf_cur_loc);
					int min_n = signed_quad(gf_file, &gf_cur_loc);
					int max_n = signed_quad(gf_file, &gf_cur_loc);
					f.hppp = hppp;
					f.vppp = vppp;
					tfm_design_size = (int)round(tfm_conv*ds);
					f.design_size = tfm_design_size;
					tfm_check_sum = cs;
					f.check_sum = tfm_check_sum;
				}
				
				break;

			case 245:
				// char_loc
				{
					int c = get_byte(gf_file, &gf_cur_loc);
					int dx = signed_quad(gf_file, &gf_cur_loc);
					int dy = signed_quad(gf_file, &gf_cur_loc);
					int w = signed_quad(gf_file, &gf_cur_loc);
					int p = signed_quad(gf_file, &gf_cur_loc);
					pixel_width[width_ptr++] = dx;
					for (size_t i = 0; i < f.Char.size(); i++) {
						if (f.Char[i].char_code % 256 == c) {
							f.Char[i].chardx = (int)round(dx/65536.0);
							f.Char[i].width = tfm_width_to_dvi(w, q);
						}
					}
				}
				break;
			case 246:
				// char_loc0
				{
					int c = get_byte(gf_file, &gf_cur_loc);
					int dm = get_byte(gf_file, &gf_cur_loc);
					int dx = dm*65536;
					int w = signed_quad(gf_file, &gf_cur_loc);
					int p = signed_quad(gf_file, &gf_cur_loc);
					pixel_width[width_ptr++] = dx;
					for (size_t i = 0; i < f.Char.size(); i++) {
						if (f.Char[i].char_code % 256 == c) {
							f.Char[i].chardx = (int)round(dx/65536.0);
							f.Char[i].width = tfm_width_to_dvi(w, q);
						}
					}
				}
				break;
			case 249:
				// post_post
				{
					int q = signed_quad(gf_file, &gf_cur_loc);
					int i = get_byte(gf_file, &gf_cur_loc);

					while (i != 0) {
						i = get_byte(gf_file, &gf_cur_loc);
					}
				}
				done = true;
				break;
			case 250:
			case 251:
			case 252:
			case 253:
			case 254:
			case 255:
				break;

			default:
				// error
				{
					int a = 10;
					a++;
					std::cout << "ERROR: Unhandled instruction\n";
					return false;
				}
				break;
			}
		}
	}
	//f.font_num = font_num[nf];
	FontVector.push_back(f);
	return true;
}


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


bool open_gf_file() 
{
	gf_file = NULL;
	if (test_access(font_file_path))
		gf_file = fopen(real_name_of_file.get_c_str(), "rb");

	if (!gf_file) {
		char temp_buf[300];
		sprintf(temp_buf, "\nGF file not found: %s", cur_name.get_c_str());
		return false;
	}
	gf_cur_loc = 0;
	return true;
}

bool open_dvi_file() 
{
	dvi_file = NULL;
	if (test_access(no_file_path)) {
		strcpy(dvi_name.get_c_str(), real_name_of_file.get_c_str());
		dvi_file = fopen(dvi_name.get_c_str(), "rb");
	}

	if (!dvi_file) {
		printf("DVI file %s not found\n", dvi_name.get_c_str());
		return false;
	}
	dvi_cur_loc = 0;
	return true;
}

void define_font(int e) //{|e| is an external font number}
{
	int f; //:0..max_fonts;
	int p; //{length of the area/directory spec}
	int n; //{length of the font name proper}
	int c,q,d,m; //{check sum, scaled size, design size, magnification}
	int r; //:0..name_length; //{index into |cur_name|}
	int j,k; //:0..name_size; //{indices into |names|}
	bool mismatch; //{do names disagree?}
	if (nf==max_fonts)
	{
		sprintf(temp_buf, "rsDVIViewer capacity exceeded (max fonts=%d)!", max_fonts);
		return;
	}
	font_num[nf]=e; f=0;
	while (font_num[f]!=e) incr(f);
	//@<Read the font parameters into position for font |nf|, and print the font name@>;
	c=signed_quad(dvi_file, &dvi_cur_loc); font_check_sum[nf]=c;
	q=signed_quad(dvi_file, &dvi_cur_loc); font_scaled_size[nf]=q;
	d=signed_quad(dvi_file, &dvi_cur_loc); font_design_size[nf]=d;
	if (q<=0 || d<=0) m=1000;
	else m=(int)round((1000.0*conv*q)/(true_conv*d));
	p=get_byte(dvi_file, &dvi_cur_loc); n=get_byte(dvi_file, &dvi_cur_loc);
	if (font_name[nf]+n+p>name_size)
	{
		sprintf(temp_buf, "rsDVIViewer capacity exceeded (name size=%d)!", name_size);
		return;
	}
	font_name[nf+1]=font_name[nf]+n+p;
	if (showing) 
		;//print(": ");
	//{when |showing| is true, the font number has already been printed}
	else {
		sprintf(temp_buf, "Font %d: ", e); 
		//print(temp_buf);
	}
	if (n+p==0) 
		;//print("null font name!");
	else for (k=font_name[nf]; k<=font_name[nf+1]-1; k++) {
		names[k]=get_byte(dvi_file, &dvi_cur_loc);
	}
	//print_font(nf);
	if (!showing) 
		if (m!=1000) {sprintf(temp_buf, " scaled %d", m); /*print(temp_buf);*/}
	//@<Read the font parameters into position for font |nf|, and print the font name@>;
	
	if ((/*(out_mode==the_works) && in_postamble)|| ((out_mode<the_works)&& */!in_postamble))
	{ 
		if (f<nf)
			;//print_ln("---this font was already defined!");
	}
	else { 
		if (f==nf) ;//print_ln("---this font wasn\'t loaded before!");
	}
	
	if (f==nf) 
		//@<Load the new font, unless there are problems@>
	{
		r=0;
		char font_name_ending[128] = {0};
		char basename[512];
		int real_res = (int)round(resolution*((double)q/d)*(conv/true_conv));
		sprintf(font_name_ending, ".%dgf", real_res);
		for (k=font_name[nf]; k<=font_name[nf+1]-1; k++)
		{ 
			incr(r);
			if (r+strlen(font_name_ending)>name_length)
			{
				sprintf(temp_buf, "DVItype capacity exceeded (max font name length=%d)!", name_length);
				return;
			}
			if (names[k]>=97 && names[k]<=122)
				cur_name[r]=names[k]-040;
			else cur_name[r]=names[k];
		}
		cur_name[r+1] = 0;
		strcpy(basename, cur_name.get_c_str());
		strcat(cur_name.get_c_str(), font_name_ending);
		//@<Move font name into the |cur_name| string@>;
		if (!open_gf_file())
		{
			char cmd_string[256];
			sprintf(cmd_string, "mf -base=plain \\mode=localfont; mag=%f; input %s", real_res/200.0, basename);
			int ret = system(cmd_string);
			if (!open_gf_file())
				return;
		}
		
		if (feof(gf_file))
			sprintf(temp_buf, "---not loaded, GF file can\'t be opened!");
		else {
			if (q<=0 || q>=01000000000)
			{
				sprintf(temp_buf, "---not loaded, bad scale (%d)!", q);
				//print(temp_buf);
			}
			else if (d<=0 || d>=01000000000)
			{
				sprintf(temp_buf, "---not loaded, bad design size (%d)!", d);
				//print(temp_buf);
			}
			else if (ReadGFFile(q)) 
				//@<Finish loading the new font info@>;
			{ 
				// do width calc here


				font_space[nf]=q / 6; //{this is a 3-unit ``thin space''}
				if (c!=0 && tfm_check_sum!=0 && c!=tfm_check_sum)
				{ 
					sprintf(temp_buf, "---beware: check sums do not agree!");
					sprintf(temp_buf, "   (%d vs. %d)", c, tfm_check_sum);
					//print_ln(temp_buf);
					//print("   ");
				}
				if (myabs(tfm_design_size-d)>2)
				{ 
					sprintf(temp_buf, "---beware: design sizes do not agree!");
					sprintf(temp_buf, "   (%d vs. %d)", d, tfm_design_size);
					//print_ln(temp_buf);
					//print("   ");
				}
				sprintf(temp_buf, "---loaded at size %d DVI units", q);
				//print(temp_buf);
				d=(int)round((100.0*conv*q)/(true_conv*d));
				if (d!=100)
				{ 
					//print_ln(" "); 
					sprintf(temp_buf, " (this font is magnified %d%%)", d);
					//print(temp_buf);
				}
				incr(nf); //{now the new font is officially present}
			}
				//@<Finish loading the new font info@>;
		}
		fclose(gf_file);

		
		//if (out_mode==errors_only) print_ln(" ");
	}
		//@<Load the new font, unless there are problems@>
	else
		//@<Check that the current font definition matches the old one@>;
	{ 
		if (font_check_sum[f]!=c)
			;//print_ln("---check sum doesn\'t match previous definition!");
		if (font_scaled_size[f]!=q)
			;//print_ln("---scaled size doesn\'t match previous definition!");
		if (font_design_size[f]!=d)
			;//print_ln("---design size doesn\'t match previous definition!");
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
		if (mismatch) 
			;//print_ln("---font name doesn\'t match previous definition!");
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
			ret_val=get_byte(dvi_file, &dvi_cur_loc);
			break;
		case set1+1: case put1+1: case fnt1+1: case xxx1+1: case fnt_def1+1: 
			ret_val=get_two_bytes(dvi_file, &dvi_cur_loc);
			break;
		case set1+2: case put1+2: case fnt1+2: case xxx1+2: case fnt_def1+2: 
			ret_val=get_three_bytes(dvi_file, &dvi_cur_loc);
			break;
		case right1: case w1: case x1: case down1: case dvi_y1: case z1: 
			ret_val=signed_byte(dvi_file, &dvi_cur_loc);
			break;
		case right1+1: case w1+1: case x1+1: case down1+1: case dvi_y1+1: case z1+1: 
			ret_val=signed_pair(dvi_file, &dvi_cur_loc);
			break;
		case right1+2: case w1+2: case x1+2: case down1+2: case dvi_y1+2: case z1+2: 
			ret_val=signed_trio(dvi_file, &dvi_cur_loc);
			break;
		case set1+3: case set_rule: case put1+3: case put_rule: case right1+3: case w1+3: case x1+3: case down1+3: case dvi_y1+3: case z1+3:
		case fnt1+3: case xxx1+3: case fnt_def1+3: 
			ret_val=signed_quad(dvi_file, &dvi_cur_loc);
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
	int k; //{loop index}
	int p, q, m; //{general purpose registers}

	post_loc=dvi_cur_loc-5;
	sprintf(temp_buf, "Postamble starts at byte %d.", post_loc);
	if (signed_quad(dvi_file, &dvi_cur_loc)!=numerator)
		sprintf(temp_buf, "numerator doesn\'t match the preamble!");
	if (signed_quad(dvi_file, &dvi_cur_loc)!=denominator)
		sprintf(temp_buf, "denominator doesn\'t match the preamble!");
	if (signed_quad(dvi_file, &dvi_cur_loc)!=mag) 
		if (new_mag==0)
			sprintf(temp_buf, "magnification doesn\'t match the preamble!");

	max_v=signed_quad(dvi_file, &dvi_cur_loc); 
	max_h=signed_quad(dvi_file, &dvi_cur_loc);
	sprintf(temp_buf, "maxv=%d, maxh=%d", max_v, max_h);
	//print(temp_buf);
	max_s=get_two_bytes(dvi_file, &dvi_cur_loc); 
	total_pages=get_two_bytes(dvi_file, &dvi_cur_loc);
	sprintf(temp_buf, ", maxstackdepth=%d, totalpages=%d", max_s, total_pages);
	//print_ln(temp_buf);

	do {
		k=get_byte(dvi_file, &dvi_cur_loc);
		if (k>=fnt_def1 && k<fnt_def1+4)
		{ 
			p=first_par(k); 
			define_font(p); 
			//print_ln(" "); 
			k=nop;
		}
	} while (!(k!=nop));
	if (k!=post_post)
	{
		sprintf(temp_buf, "byte %d is not postpost!", dvi_cur_loc-1);
		//print_ln(temp_buf);
	}
	//@<Make sure that the end of the file is well-formed@>;
	q=signed_quad(dvi_file, &dvi_cur_loc);
	if (q!=post_loc)
	{
		sprintf(temp_buf, "bad postamble pointer in byte %d!", dvi_cur_loc-4);
		//print_ln(temp_buf);
	}
	m=get_byte(dvi_file, &dvi_cur_loc);
	if (m!=id_byte) 
	{
		sprintf(temp_buf, "identification in byte %d should be %d!", dvi_cur_loc-1, id_byte);
		//print_ln(temp_buf);
	}
	k=dvi_cur_loc; m=223;
	while (m==223) m=get_byte(dvi_file, &dvi_cur_loc);
	if (!feof(dvi_file))
	{
		sprintf(temp_buf, "signature in byte %d should be 223", dvi_cur_loc-1);
		//bad_dvi(temp_buf);
		return;
	}
	else if (dvi_cur_loc<k+4)
	{
		sprintf(temp_buf, "not enough signature bytes at end of file (%d)", dvi_cur_loc-k);
		//print_ln(temp_buf);
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
			if (feof(dvi_file)) return;//bad_dvi("the file ended prematurely");
			k=get_byte(dvi_file, &dvi_cur_loc);
			p=first_par(k);
			switch(k) {
				case set_rule: case put_rule: 
					down_the_drain=signed_quad(dvi_file, &dvi_cur_loc);
					break;
				case four_cases(fnt_def1): 
					define_font(p);
					//print_ln(" ");
					break;
				case four_cases(xxx1): 
					while (p>0)
					{ 
						down_the_drain=get_byte(dvi_file, &dvi_cur_loc); decr(p);
					}
					break;
				case bop: case pre: case post:case post_post: case undefined_commands:
					{
						char temp_buf[300];
						sprintf(temp_buf, "illegal command at byte %d", dvi_cur_loc-1);
						//bad_dvi(temp_buf);
						return;
					}
					break;
				default: /*do_nothing*/ break;
			}
		} while (!(k==eop));
		//@<Skip until finding |eop|@>;
		bop_seen=false;
	}
//label9999:
	;
}


void scan_bop()
{

	char temp_buf[300];
	int k; //:0..255; //{command code}
	do { 
		if (feof(dvi_file)) {sprintf(temp_buf, "the file ended prematurely"); return;}
		k=get_byte(dvi_file, &dvi_cur_loc);
		if (k>=fnt_def1 && k<fnt_def1+4)
		{ 
			define_font(first_par(k)); k=nop;
		}
	} while (!(k!=nop));
	if (k==post) in_postamble=true;
	else { 
		if (k!=bop){ sprintf(temp_buf, "byte %d is not bop", dvi_cur_loc-1); return;}
		new_backpointer=dvi_cur_loc-1; incr(page_count);
		for (k=0; k<=9; k++) count[k]=signed_quad(dvi_file, &dvi_cur_loc);
		if (signed_quad(dvi_file, &dvi_cur_loc)!=old_backpointer)
		{
			sprintf(temp_buf, "backpointer in byte %d should be %d!", dvi_cur_loc-4, old_backpointer);
			//print_ln(temp_buf);
		}

		old_backpointer=new_backpointer;
	}


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
		//major(temp_buf);
		goto change_font;
		break;
	case four_cases(fnt1): 
		sprintf(temp_buf, "fnt%d %d", o-fnt1+1, p);
		//major(temp_buf);
		goto change_font;
		break;
	case four_cases(fnt_def1): 
		sprintf(temp_buf, "fntdef%d %d", o-fnt_def1+1, p);
		//major(temp_buf);
		define_font(p); goto done;
		break;
		//@<Cases for fonts@>@;

	case four_cases(xxx1): 
		//@<Translate an |xxx| command and |goto done|@>;
		//major("xxx \'"); 
		bad_char=false;
		if (p<0) ;//error("string of negative length!");
		for (k=1; k<=p; k++)
		{
			q=get_byte(dvi_file, &dvi_cur_loc);
			if (q<32 or q>126) bad_char=true;
			if (showing) ;//print_c(xchr[q]);
		}
		if (showing) ;//print_c('\'');
		if (bad_char) ;//error("non-ASCII character in xxx command!");
		goto done;
		//@<Translate an |xxx| command and |goto done|@>;
		break;
	case pre: 
		//error("preamble command within a page!"); 
		goto label9998;
		break;
	case post: case post_post: 
		//error("postamble command within a page!"); 
		goto label9998;
		break;
	default: 
		sprintf(temp_buf, "undefined command %d!", o);
		//error(temp_buf);
		goto done;
		break;
	}
move_down: 
	//@<Finish a command that sets |v:=v+p|, then |goto done|@>;
	if (v>0 && p>0) 
		if (v>infinity-p)
		{ 
			sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", p, infinity-v);
			//error(temp_buf);
			p=infinity-v;
		}
	if (v<0 && p<0) 
		if (-v>p+infinity)
		{
			sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", p, (-v)-infinity);
			//error(temp_buf);
			p=(-v)-infinity;
		}
	vvv=(int)pixel_round(v+p);
	if (myabs(vvv-vv)>max_drift)
		if (vvv>vv) vv=vvv-max_drift;
		else vv=vvv+max_drift;
	if (showing) 
		if (true /*out_mode>mnemonics_only*/)
		{ 
			sprintf(temp_buf, " v:=%d", v);
			//print(temp_buf);
			if (p>=0) ;//print("+");
			sprintf(temp_buf, "%d=%d, vv:=%d", p, v+p, vv);
			//print(temp_buf);
		}
	v=v+p;
	if (myabs(v)>max_v_so_far)
	{ 
		if (myabs(v)>max_v+99)
		{ 
			sprintf(temp_buf, "warning: |v|>%d!", max_v);
			//error(temp_buf);
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
		//error(temp_buf);
	}
	if (showing) 
		if (true /*out_mode>mnemonics_only*/)
		{ 
			//print(" current font is "); print_font(cur_font);
		}
	goto done;
	//@<Finish a command that changes the current font, then |goto done|@>;
label9998: 
	pure=false;
done: 
	return pure;
}

int get_character_width(int p, int cur_font)
{
	for (size_t j=0; j < FontVector[cur_font].Char.size(); j++) {
		if (FontVector[cur_font].Char[j].char_code == p) {
			return FontVector[cur_font].Char[j].width;
		}
	}

	return invalid_width;
}

int get_character_pixel_width(int p, int cur_font)
{
	for (size_t j=0; j < FontVector[cur_font].Char.size(); j++) {
		if (FontVector[cur_font].Char[j].char_code == p) {
			return FontVector[cur_font].Char[j].chardx;
		}
	}
	return invalid_width;
}

int tfm_width_to_dvi(int tfm_width, int tfm_scaled_size)
{
	int b3 = (unsigned)tfm_width & 255u;
	int b2 = ((unsigned)tfm_width >> 8u) & 255u;
	int b1 = ((unsigned)tfm_width >> 16u) & 255u;
	int b0 = ((unsigned)tfm_width >> 24u);
	int out_width = 0;
	int alpha=16;
	while (tfm_scaled_size>=040000000)
	{
		tfm_scaled_size=tfm_scaled_size / 2; alpha=alpha+alpha;
	}
	int beta=256 / alpha; alpha=alpha*z;
	//@<Replace |z| by $|z|^\prime$ and compute $\alpha,\beta$@>;

	out_width=(((((b3*tfm_scaled_size)/0400)+(b2*tfm_scaled_size))/0400)+(b1*tfm_scaled_size))/ beta;
	if (b0>0) 
		if (b0<255) 
			goto error_label;
		else out_width=out_width-alpha;

	return out_width;

error_label:
	out_width =-1;
	return out_width;

}


void AddCharacterToPage(int page, int font_index, int charactercode, int pixel_x, int pixel_y)
{
	Character character;
	character.pFontChar = 0;

	for (size_t i = 0; i < FontVector[font_index].Char.size(); i++) {
		if (FontVector[font_index].Char[i].char_code == charactercode) {
			character.pFontChar = &FontVector[font_index].Char[i];
			break;
		}
	}
	character.x = pixel_x;
	character.y = pixel_y;
	character.x += static_cast<int>(resolution); //resolution in ppi, offset one inch, seems to be standard that dvi-drivers do this, don't know why
	character.y += static_cast<int>(resolution);
	PageCharVector[page].push_back(character);

}

void AddRuleToPage(int page, int pixel_x, int pixel_y, int width, int height)
{
	Rule rule;
	rule.x = static_cast<int>(pixel_x+resolution);
	rule.y = static_cast<int>(pixel_y+resolution);
	rule.width = width;
	rule.height = height;
	PageRuleVector[page].push_back(rule);
}


bool do_page()
{
	char temp_buf[300];
	//label fin_set,fin_rule,move_right,show_state,done,9998,9999;
	eight_bits o; //{operation code of the current command}
	int p, q; //{parameters of the current command}
	int a; //{byte number of the current command}
	int hhh; //{|h|, rounded to the nearest pixel}

	PageCharVector.push_back({}); // add another empty page
	PageRuleVector.push_back({});
	cur_page++;
	cur_font=invalid_font; //{set current font undefined}
	s=0; h=0; v=0; w=0; x=0; y=0; z=0; hh=0; vv=0;
	//{initialize the state variables}
	while (true)
		//@<Translate the next command in the \.{DVI} file; |goto 9999| with |do_page=true| if it was |eop|; |goto 9998| if premature termination is needed@>;
	{ 
		a=dvi_cur_loc; showing=false;
		o=get_byte(dvi_file, &dvi_cur_loc); p=first_par(o);
		if (feof(dvi_file)) return false;
		//@<Start translation of command |o| and |goto| the appropriate label to finish the job@>;
		if (o<set_char_0+128) 
			//@<Translate a |set_char| command@>
		{ 
			if (o>32 && o<=126)
			{ 
				//out_text(p); 
				sprintf(temp_buf, "setchar%d", p);
				//minor(temp_buf);
			}
			else {
				sprintf(temp_buf, "setchar%d", p);
				//major(temp_buf);
			}

			AddCharacterToPage(cur_page, cur_font, p, hh, vv);


			goto fin_set;
		}
			//@<Translate a |set_char| command@>
		else switch(o) {
			case four_cases(set1): 
				sprintf(temp_buf, "set%d %d", o-set1+1, p);
				//major(temp_buf); 
				goto fin_set;
				break;
			case four_cases(put1):
				sprintf(temp_buf, "put%d %d", o-put1+1, p);
				//major(temp_buf); 
				goto fin_set;
				break;
			case set_rule:
				//major("setrule"); 
				goto fin_rule;
				break;
			case put_rule: 
				//major("putrule"); 
				goto fin_rule;
				break;
			//@<Cases for commands |nop|, |bop|, \dots, |pop|@>@;

			case nop: 
				//minor("nop"); 
				goto done;
				break;
			case bop: 
				//error("bop occurred before eop!"); 
				goto label9998;
				break;
			case eop: 
				//major("eop");
				if (s!=0) {
					sprintf(temp_buf, "stack not empty at end of page (level %d)!", s);
					//error(temp_buf);
				}
				//print_ln(" "); 
				return true;
				break;
			case push: 
				//major("push");
				if (s==max_s_so_far)
				{ 
					max_s_so_far=s+1;
					if (s==max_s) 
						;//error("deeper than claimed in postamble!");
					if (s==stack_size)
					{ 
						sprintf(temp_buf, "DVItype capacity exceeded (stack size=%d)", stack_size);
						//error(temp_buf); 
						goto label9998;
					}
				}
				hstack[s]=h; vstack[s]=v; wstack[s]=w;
				xstack[s]=x; ystack[s]=y; zstack[s]=z;
				hhstack[s]=hh; vvstack[s]=vv; incr(s); ss=s-1; goto show_state;
				break;
			case pop: 
				//major("pop");
				if (s==0) ;//error("(illegal at level zero)!");
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

		// find character in cur_font
		q = get_character_width(p, cur_font);
		//if (p<font_bc[cur_font] || p>font_ec[cur_font]) q=invalid_width;
		//else q=char_width(cur_font)(p);
		if (q==invalid_width)
		{ 
			sprintf(temp_buf, "character %d invalid in font ", p);
			//error(temp_buf);
			//print_font(cur_font);
			if (cur_font!=invalid_font)
				;//print("!"); //{the invalid font has `\.!' in its name}
		}
		if (o>=put1) goto done;
		if (q==invalid_width) q=0;
		else hh = hh + get_character_pixel_width(p, cur_font);//hh=hh+char_pixel_width(cur_font)(p);
		goto move_right;
		//@<Finish a command that either sets or puts a character, then |goto move_right| or |done|@>;
	fin_rule: 
		//@<Finish a command that either sets or puts a rule, then |goto move_right| or |done|@>;
		q=signed_quad(dvi_file, &dvi_cur_loc);
		if (true)
		{ 
			sprintf(temp_buf, " height %d, width %d", p, q);
			//print(temp_buf);
			if (true /*out_mode>mnemonics_only*/)
				if (p<=0 || q<=0) ;//print(" (invisible)");
				else {
					sprintf(temp_buf, " (%dx%d pixels)",  rule_pixels(p), rule_pixels(q)); 
					//print(temp_buf);
				}
		}

		AddRuleToPage(cur_page, hh, vv, rule_pixels(q), rule_pixels(p));
		if (o==put_rule) goto done;
		if (true) 
			if (true/*out_mode>mnemonics_only*/) 
				;//print_ln(" ");
		hh=hh+rule_pixels(q); goto move_right;
		//@<Finish a command that either sets or puts a rule, then |goto move_right| or |done|@>;
	move_right: 
		//@<Finish a command that sets |h:=h+q|, then |goto done|@>;
		if (h>0 && q>0) 
			if (h>infinity-q)
			{ 
				sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", q, infinity-h);
				//error(temp_buf);
				q=infinity-h;
			}
		if (h<0 && q<0) 
			if (-h>q+infinity)
			{ 
				sprintf(temp_buf, "arithmetic overflow! parameter changed from %d to %d", q, (-h)-infinity);
				//error(temp_buf);
				q=(-h)-infinity;
			}
		hhh=(int)pixel_round(h+q);
		if (myabs(hhh-hh)>max_drift)
		if (hhh>hh) hh=hhh-max_drift;
		else hh=hhh+max_drift;
		if (showing) 
			if (true /*out_mode>mnemonics_only*/)
			{
				sprintf(temp_buf, " h:=%d", h);
				//print(temp_buf);
				if (q>=0) ;//print("+");
				sprintf(temp_buf, "%d=%d, hh:=%d", q, h+q, hh);
				//print(temp_buf);
			}
		h=h+q;
		if (myabs(h)>max_h_so_far)
		{ 
			if (myabs(h)>max_h+99)
			{ 
				sprintf(temp_buf, "warning: |h|>%d!", max_h);
				//error(temp_buf);
				max_h=myabs(h);
			}
			max_h_so_far=myabs(h);
		}
		goto done;
		//@<Finish a command that sets |h:=h+q|, then |goto done|@>;
	show_state: 
		//@<Show the values of |ss|, |h|, |v|, |w|, |x|, |y|, |z|, |hh|, and |vv|; then |goto done|@>;
		if (showing) 
			if (true /*out_mode>mnemonics_only*/)
			{ 
				//print_ln(" ");
				sprintf(temp_buf, "level %d:(h=%d,v=%d,w=%d,x=%d,y=%d,z=%d,hh=%d,vv=%d)", ss, h, v, w, x, y, z, hh, vv);
				//print(temp_buf);
			}
		goto done;
		//@<Show the values of |ss|, |h|, |v|, |w|, |x|, |y|, |z|, |hh|, and |vv|; then |goto done|@>;
	done: 
		if (showing) ;//print_ln(" ");
	}
		//@<Translate the next command in the \.{DVI} file; |goto 9999| with |do_page=true| if it was |eop|; |goto 9998| if premature termination is needed@>;
label9998:
	//print_ln("!"); 
	return false;
//label9999: 
	;
}



bool ReadDVIFile(char const *pFileName)
{

	int m,n;
	int k;
	int q;
	//Font f = ReadGFFile("cmr10.600gf");

	set_paths();

	// delete old font bitmaps
	for (auto& f : FontVector)
	{
		for (auto& c : f.Char)
			DeleteObject(c.hBitmap);
	}

	PageCharVector.clear();
	PageRuleVector.clear();
	FontVector.clear();
// init
	nf=0;width_ptr=0;
	font_name[0]=1;font_space[max_fonts]=0;font_bc[max_fonts]=1;
	font_ec[max_fonts]=0;
	max_pages=1000000;
	cur_page = -1;
	//resolution=300.0;
	new_mag = 0;
	//start_vals=0;start_there[0]=false;
	in_postamble=false;
	//strcpy(default_directory.get_c_str(), default_directory_name);
	//text_ptr=0;
	max_v=2147483548;max_h=2147483548;max_s=stack_size+1;
	max_v_so_far=0;max_h_so_far=0;max_s_so_far=0;page_count=0;
	old_backpointer=-1;started=false;

	if (strlen(pFileName) > name_length)
		return false;
	strcpy(cur_name.get_c_str(), pFileName);

	if (!open_dvi_file())
		return false;

	//cur_file = dvi_file;

	int p = get_byte(dvi_file, &dvi_cur_loc);
	if (p!=pre)
	{
		sprintf(temp_buf, "First byte isn\'t start of preamble!");
	}
	p=get_byte(dvi_file, &dvi_cur_loc); //{fetch the identification byte}
	if (p!=id_byte)
	{
		sprintf(temp_buf, "identification in byte 1 should be %d!", id_byte); 
	}
	numerator=signed_quad(dvi_file, &dvi_cur_loc); denominator=signed_quad(dvi_file, &dvi_cur_loc);
	if (numerator<=0) {sprintf(temp_buf, "numerator is %d", numerator);goto bad_file;}
	if (denominator<=0) {sprintf(temp_buf, "denominator is %d", denominator);goto bad_file;}
	sprintf(temp_buf, "numerator/denominator=%d/%d", numerator, denominator);

	tfm_conv=(float)((25400000.0/numerator)*(denominator/473628672.0)/16.0);
	conv=(float)((numerator/254000.0)*(resolution/denominator));
	mag=signed_quad(dvi_file, &dvi_cur_loc);
	if (new_mag>0) mag=new_mag;
	else if (mag<=0) {sprintf(temp_buf, "magnification is %d", mag);goto bad_file;}
	true_conv=conv; conv=(float)(true_conv*(mag/1000.0));
	sprintf(temp_buf, "magnification=%d; %16.8f pixels per DVI unit", mag, conv);
	//print_ln('magnification=',mag:1,'; ',conv:16:8,' pixels per DVI unit')

	p=get_byte(dvi_file, &dvi_cur_loc); //{fetch the length of the introductory comment}
	while (p>0)
	{ 
		decr(p);
		get_byte(dvi_file, &dvi_cur_loc);
		//print_c(xchr[get_byte()]);
	}
	//print_ln("\'");
	after_pre=dvi_cur_loc;

	n = dvi_length(dvi_file);
	if (n<53) {sprintf(temp_buf, "only %d bytes long", n);goto bad_file;}
	m=n-4;
	do {
		if (m==0) {goto bad_file;}
		move_to_byte(dvi_file, &dvi_cur_loc, m); k=get_byte(dvi_file, &dvi_cur_loc); decr(m);
	} while (!(k!=223));

	if (k!=id_byte) {sprintf(temp_buf, "ID byte is %d", k);goto bad_file;}
	move_to_byte(dvi_file, &dvi_cur_loc, m-3); q=signed_quad(dvi_file, &dvi_cur_loc);
	if (q<0 || q>m-33) {sprintf(temp_buf, "post pointer %d at byte %d", q,m-3);goto bad_file;}
	move_to_byte(dvi_file, &dvi_cur_loc, q); k=get_byte(dvi_file, &dvi_cur_loc);
	if (k!=post) {sprintf(temp_buf, "byte %d is not post", q);goto bad_file;}
	post_loc=q; first_backpointer=signed_quad(dvi_file, &dvi_cur_loc);
	//@<Find the postamble, working back from the end@>;
	in_postamble=true; 
	read_postamble(); 
	in_postamble=false;

	q=post_loc; p=first_backpointer; start_loc=-1;
	if (p<0) in_postamble=true;
	else  { 
		do {
			//{now |q| points to a |post| or |bop| command; |p>=0| is prev pointer}
			if (p>q-46)
				{sprintf(temp_buf, "page link %d after byte %d", p,q);goto bad_file;}
				
			q=p; move_to_byte(dvi_file, &dvi_cur_loc, q); k=get_byte(dvi_file, &dvi_cur_loc);
			if (k==bop) incr(page_count);
			else {sprintf(temp_buf, "byte %d is not bop", q);goto bad_file;}
			for (k=0; k<= 9; k++) count[k]=signed_quad(dvi_file, &dvi_cur_loc);
			p=signed_quad(dvi_file, &dvi_cur_loc);
			if (start_match())
			{ 
				start_loc=q; old_backpointer=p;
			}
		} while (!(p<0));
		if (start_loc<0) 
		{
			sprintf(temp_buf, "starting page number could not be found!");
			goto bad_file;
		}
		if (old_backpointer<0) start_loc=after_pre; //{we want to check everything}
		move_to_byte(dvi_file, &dvi_cur_loc, start_loc);
	}
	if (page_count!=total_pages)
		
	{
		sprintf(temp_buf, "there are really %d pages, not %d", page_count,total_pages); 
		//print_ln(temp_buf);/*print_ln('there are really ',page_count:1,' pages, not ',total_pages:1,'!');*/
	}

	skip_pages(false);
	if (!in_postamble) 
		//@<Translate up to |max_pages| pages@>;
	{ 
		while (max_pages>0)
		{ 
			decr(max_pages);
			//print_ln(" "); 
			sprintf(temp_buf, "%d: beginning of page ", dvi_cur_loc-45);
			//print(temp_buf);
			//print(cur_loc-45:1,': beginning of page ');
			for (k=0; k<=start_vals; k++)
			{ 
				sprintf(temp_buf, "%d", count[k]);
				//print(count[k]:1);
				//print(temp_buf);
				if (k<start_vals) ;//print(".");
				else ;//print_ln(" ");
			}
			if (!do_page()) goto bad_file;
			scan_bop();
			if (in_postamble) goto done;
		}
	done:	;
	}



	fclose(dvi_file);
	return true;

bad_file:
	fclose(dvi_file);
	return false;
}
