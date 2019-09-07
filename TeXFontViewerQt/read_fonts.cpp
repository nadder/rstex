/*

Copyright (C) 2018 by Richard Sandberg.

*/

#include <vector>
#include <array>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <mainwindow.h>
#include <cmath>
#include <cassert>
#include "read_fonts.h"


#define incr(s) (s)++

#define UNUSED(x) (void)x

static FILE *pk_file;
static FILE *gf_file;
static FILE *pxl_file;
static FILE *log_file;

void myabort(char const*msg)
{

    MainWindow * win = static_cast<MainWindow*>(QApplication::activeWindow());
    QMessageBox::critical(win, "Error", msg);
    if (log_file) {fclose(log_file); log_file = nullptr;}
    if (gf_file) {fclose(gf_file); gf_file = nullptr;}
    if (pk_file) {fclose(pk_file); pk_file = nullptr;}
    throw -1;
}


bool myfeof(FILE *fp) {
	int c=fgetc(fp);
	if (c!=EOF)
		ungetc(c,fp);
	return c==EOF;
}

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


//////////////////////////////////// pk file ////////////////////////////////////////


std::vector<std::vector<eight_bits>> image_raster(256);
char_raster_info char_info[256];

sfont_info font_info;

int num_chars;

static int term_pos; // current terminal position


static int pk_loc;


const int pk_id = 89; //{the version of \.{PK} file described}
//const int pk_xxx1 = 240; //{\&{special} commands}
//const int pk_yyy = 244; //{\&{numspecial} commands}
const int pk_post = 245; //{postamble}
const int pk_no_op = 246; //{no operation}
const int pk_pre = 247; //{preamble}

static int hppp, vppp;
static int design_size;
static int checksum;
static int magnification;

static int i, j; // {index pointers}
static int flag_byte; // {the byte that introduces the character definition}
static int end_of_packet; // {where we expect the end of the packet to be}
static int width, height; // {width and height of character}
static int x_off, y_off; // {x and y offsets of character}
static int tfm_width; // {character tfm width}
//static Array<int, 0, 255> tfms; // : array [0..255] of integer ; {character tfm widths}
static int dx, dy; // {escapement values}
//static Array<int, 0, 255> dxs, dys; // : array [0..255] of integer ; {escapement values}
static Array<bool, 0, 255> status; // : array[0..255] of boolean ; {has the character been seen?}
static int dyn_f; // {dynamic packing variable}
static int car; // {the character we are reading}
static int packet_length; // {the length of the character packet}


// 51
static int repeat_count; //{how many times to repeat the next row?}
static int rows_left; //{how many rows left?}
static bool turn_on; //{are we black here?}
static int h_bit; //{what is our horizontal position?}
static int count; //{how many bits of current color left?}

// 47
static eight_bits input_byte; // the byte we are currently decimating
static eight_bits bit_weight; // weight of the current bit
//static eight_bits nybble; // the current nybble

// 33
eight_bits pk_byte()
{
	eight_bits temp;
    temp=static_cast<unsigned char>(fgetc(pk_file)); pk_loc++; return temp;
}
// 36
int get_16()
{
	int a;
	a=pk_byte(); return a*256+pk_byte();
}

int get_32()
{
	int a;
	a=get_16();
	if (a>32767) a-=65536;
	return a*65536+get_16();
}

// 45
int get_nyb()
{
	eight_bits temp;
	if (bit_weight==0) {
		input_byte=pk_byte(); bit_weight=16;
	}
	temp=input_byte/bit_weight; input_byte=input_byte-temp*bit_weight;
	bit_weight=bit_weight/16; return temp;
}

bool get_bit()
{
	bool temp;
	bit_weight=bit_weight/2;
	if (bit_weight==0) {
		input_byte=pk_byte(); bit_weight=128;
	}
	temp=input_byte>=bit_weight;
	if (temp) input_byte=input_byte-bit_weight;
	return temp;
}


void skip_specials()
{
	int i,j/*, k*/; // : integer ;

	do {
		flag_byte=pk_byte() ;
		if (flag_byte>=240)
			switch (flag_byte) {
				case 240: case 241: case 242: case 243:
				{
                    fprintf(log_file,"%d:  Special: '",(pk_loc-1)) ;
					i=0 ;
					for (j=240; j<=flag_byte; j++) i=256*i+pk_byte() ;
					for (j=1; j<=i; j++) fputc(pk_byte(),log_file) ;
                    fprintf(log_file,"'\n") ;
				}
				break;
                case 244: fprintf(log_file,"%d:  Num special: %d\n",pk_loc-1,get_32()) ;
					break;
                case 245: fprintf(log_file,"%d:  Postamble\n",pk_loc-1) ;
					break;
                case 246: fprintf(log_file,"%d:  No op\n",(pk_loc-1));
					break;
				case 247: case 248: case 249: case  250: case  251: case  252: case  253: case  254: case  255:
                {char temp[256]; sprintf(temp,"Unexpected %d!",flag_byte); myabort(temp) ;}
				break;
	//@.Unexpected bbb@>
			}
	} while (!((flag_byte<240||flag_byte==pk_post)));
}

void send_out(bool repeat_count,int value)
{
	int i,len;
	i=10 ; len=1 ;
	while (value>=i) { len++ ; i=i*10 ; }
	if (repeat_count||!turn_on) len=len+2 ;
	if (term_pos+len>78) {
        term_pos=len+2 ; fprintf(log_file," \n") ; fprintf(log_file,"  ") ;
	} else term_pos=term_pos+len ;
    if (repeat_count) fprintf(log_file,"[%d]",value);
    else if (turn_on) fprintf(log_file,"%d",value);
    else fprintf(log_file,"(%d)",value);
}

int pk_packed_num()
{
	int i,j/*, k*/;

	i=get_nyb() ;
	if (i==0) {
		do { j=get_nyb() ; i++ ; } while (j==0) ;
		while (i>0) { j=j*16+get_nyb() ; i-- ; }
		return j-15+(13-dyn_f)*16+dyn_f ;
	} else if (i<=dyn_f)
		return i;
	else if (i<14)
		return (i-dyn_f-1)*16+get_nyb()+dyn_f+1;
	else {
        if (repeat_count!=0) myabort("Second repeat count for this row!") ;
  //@.Second repeat count...@>
		if (i==14)
			repeat_count=pk_packed_num();
		else
			repeat_count=1 ;
		send_out(true,repeat_count) ;
		return pk_packed_num() ;
	}
}

void set_image_raster_bit(bool set, unsigned bit_offset, std::vector<eight_bits>& raster)
{
    unsigned byte_index = bit_offset / 8;
    unsigned bit_index = 7 - bit_offset % 8; // bit_offset 0 is the most significant bit in the byte
	if (set)
		raster[byte_index] |= (1 << bit_index);
	else 
		raster[byte_index] &= ~(1 << bit_index);
}

bool get_image_raster_bit(unsigned bit_offset, std::vector<eight_bits>& raster)
{
    unsigned byte_index = bit_offset / 8;
    unsigned bit_index = 7 - bit_offset % 8; // bit_offset 0 is the most significant bit in the byte
	return (raster[byte_index] & (1 << bit_index)) != 0;
}


void copy_row(int width, int /*height*/, int row_to_copy, int n_rows_to_copy, std::vector<eight_bits>&raster)
{
	int src_offset;
	int dst_offset;

	dst_offset = row_to_copy*width + width;
	for (int i = 0; i < n_rows_to_copy; i++) {
		src_offset = row_to_copy*width;
		for (int k = 0; k < width; k++) {
			bool bitval = get_image_raster_bit(src_offset++, raster);
			set_image_raster_bit(bitval, dst_offset++, raster);
		}
	}
}

void zoom_raster(int zoom_factor, int width, int height, std::vector<eight_bits>& raster, std::vector<eight_bits>& out_raster)
{
    size_t new_size = size_t((zoom_factor*width*zoom_factor*height+7)/8);
	out_raster.resize(new_size);
	int src_offset = 0;
	int dest_offset = 0;
	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			int src_bit = get_image_raster_bit(src_offset++, raster);
			for (int i = 0; i < zoom_factor; i++)
				set_image_raster_bit(src_bit, dest_offset++,out_raster);
		}
		copy_row(width*zoom_factor, height*zoom_factor, yy*zoom_factor, zoom_factor-1, out_raster);
		dest_offset += (zoom_factor-1)*zoom_factor*width;

	}
}


void read_pk_file(char const * filename)
{
	int raster_char_index = 0;
	num_chars = 0;
    pk_file = fopen(filename, "rb");
    if (!pk_file) {char buf[256]; sprintf(buf, "Error opening %s", filename); myabort(buf);}
    log_file = fopen("TeXFontViewer.log", "a");
    if (!log_file) {
        // failed to open log file, try to change to home dir and try once more
        QDir::setCurrent(QDir::homePath());
        log_file = fopen("TeXFontViewer.log", "a");
    }
    fprintf(log_file, "Opening file %s\n", filename);
	// Read preamble 38>
    if (pk_byte()!=pk_pre) myabort("Bad PK file; pre command missing!");
    if (pk_byte()!=pk_id) myabort("Wrong version of PK file!");

    j=pk_byte(); fprintf(log_file,"'");
	for (i=1; i<=j; i++) fputc(pk_byte(),log_file);
    fprintf(log_file,"'\n"); design_size=get_32(); fprintf(log_file,"Design size = %d\n",design_size);
    checksum=get_32(); fprintf(log_file,"Checksum = %d\n",checksum); hppp=get_32(); vppp=get_32();
    fprintf(log_file,"Resolution: horizontal = %d  vertical = %d",hppp,vppp);
    magnification=int(round(hppp*72.27/65536)); fprintf(log_file,"  (%d dpi)\n",magnification);
	if (hppp!=vppp) printf("Warning:  aspect ratio not 1:1!\n");
	// Read preamble

    font_info.design_size = design_size/(1024.0*1024.0);
    font_info.hppp = hppp/(256.0*256.0);
    font_info.vppp = vppp/(256.0*256.0);



	skip_specials();
	while (flag_byte!=pk_post) {
		// Unpack and write characters 40>
        fprintf(log_file,"%d:  Flag byte = %d",pk_loc-1,flag_byte); dyn_f=flag_byte/16;
		flag_byte=flag_byte%16; turn_on=flag_byte>=8;
		if (turn_on) flag_byte=flag_byte-8;
		if (flag_byte==7)
			// Read long character preamble 42>
		{
			packet_length=get_32() ; car=get_32() ;
			end_of_packet=packet_length+pk_loc ;
			packet_length=packet_length+9 ;
			tfm_width=get_32() ;
			dx=get_32() ; dy=get_32() ;
			width=get_32() ;
			height=get_32() ;
			x_off=get_32() ; y_off=get_32() ;
		}
		// Read long character preamble 42
		else if (flag_byte>3)
			// Read extended short character preamble 43>
		{
			packet_length=(flag_byte-4)*65536+get_16() ;
			car=pk_byte();
			end_of_packet=packet_length+pk_loc ;
			packet_length=packet_length+4 ;
			i=pk_byte();
			tfm_width=i*65536+get_16() ;
			dx=get_16()*65536 ;
			dy=0 ;
			width=get_16();
			height=get_16();
			x_off=get_16(); y_off=get_16();
			if (x_off>32767) x_off=x_off-65536 ;
			if (y_off>32767) y_off=y_off-65536 ;
		}
			// Read extended short character preamble 43>
		else
			// Read short character preamble 44>
		{
			packet_length=flag_byte*256+pk_byte() ;
			car=pk_byte() ;
			end_of_packet=packet_length+pk_loc ;
			packet_length=packet_length+3 ;
			i=pk_byte() ;
			tfm_width=i*65536+get_16() ;
			dx=pk_byte()*65536 ;
			dy=0 ;
			width=pk_byte() ;
			height=pk_byte() ;
			x_off=pk_byte() ; y_off=pk_byte() ;
			if (x_off>127) x_off=x_off-256 ;
			if (y_off>127) y_off=y_off-256 ;
		}
			// Read short character preamble 44

        fprintf(log_file,"  Character = %d  Packet length = %d\n",car,packet_length);
        fprintf(log_file,"  Dynamic packing variable = %d\n",dyn_f);
        fprintf(log_file,"  TFM width = %d  dx = %d",tfm_width,dx);
        if (dy!=0) fprintf(log_file,"  dy = %d\n",dy);
        else fprintf(log_file," \n");
        fprintf(log_file,"  Height = %d  Width = %d  X-offset = %d  Y-offset = %d\n",
			height,width,x_off,y_off);

		// store raster information in one long array of bytes, the byte has the value one for a black pixel
		// and 0 for a white pixel

		//if (image_raster[raster_char_index]) {
		//	free(image_raster[raster_char_index]);
		//}
		int storage_needed = ((height*width) + 7)/8; // in bytes aligned to word

		//image_raster[raster_char_index] = (eight_bits*)malloc(storage_needed);
		image_raster[raster_char_index].resize(storage_needed);
		char_info[raster_char_index].code = car;
		char_info[raster_char_index].width = width;
		char_info[raster_char_index].height = height;
		char_info[raster_char_index].horz_esc = dx/(256.0*256.0);
		char_info[raster_char_index].x_off = x_off;
		char_info[raster_char_index].y_off = y_off;
		char_info[raster_char_index].tfm_width = tfm_width/(1024.0*1024.0)*design_size/(1024.0*1024.0)*hppp/(256.0*256.0);

		std::vector<eight_bits>& cur_image_raster = image_raster[raster_char_index];
		raster_char_index++;
		num_chars++;
		int raster_offset = 0;
		// Read and translate raster description 48>
		bit_weight=0 ;
		if (dyn_f==14)
		   // Get raster by bits 49>
		{
			for (i=1; i<=height; i++) {
                fprintf(log_file,"  ") ;
				for (j=1; j<=width; j++) {
					if (get_bit()) {
                        fprintf(log_file,"*");
						set_image_raster_bit(true, raster_offset, cur_image_raster);
					}
					else {
                        fprintf(log_file,".");
						set_image_raster_bit(false, raster_offset, cur_image_raster);
					}
					raster_offset++;
				}
                fprintf(log_file," \n") ;
			}
		}
			// Get raster by bits>
		else
			// Create normally packed raster 50>
		{
			term_pos=2 ;
            fprintf(log_file,"  ") ;
			rows_left=height ;
			h_bit=width ;
			repeat_count=0 ;
			while (rows_left>0) {
				count=pk_packed_num() ;
				send_out(false,count) ;

				if (count >= h_bit)
				{
					// finish row, repeat the row, then write run length
					for (int oo = 0; oo < h_bit; oo++) {
						set_image_raster_bit(turn_on, raster_offset++, cur_image_raster);
					}

					if (repeat_count > 0) {
						copy_row(width, height, height - rows_left, repeat_count, cur_image_raster);
						raster_offset += width*repeat_count;
					}

					// write the rest
					for (int oo = 0; oo < count - h_bit; oo++) {
						set_image_raster_bit(turn_on, raster_offset++, cur_image_raster);
					}
				}
				else {
					for (int oo = 0; oo < count; oo++) {
						set_image_raster_bit(turn_on, raster_offset++, cur_image_raster);
					}
				}

				if (count>=h_bit) {
					rows_left=rows_left-repeat_count-1 ;
					repeat_count=0 ;
					count=count-h_bit ;
					h_bit=width ;
					rows_left=rows_left-count/width ;
					count=count%width ;
				}
				h_bit=h_bit-count ;
				turn_on=!turn_on ;
			}
            fprintf(log_file," \n") ;
			if (rows_left!=0||h_bit!=width)
                myabort("Bad PK file: More bits than required!");
			 //@.More bits than required@>



		}
			// Create normally packed raster>

		// Read and translate raster description >
        if (end_of_packet!=pk_loc) myabort("Bad PK file: Bad packet length!");



		// Unpack and write characters 40

		skip_specials();
	}
	j=0;
	while (!myfeof(pk_file)) {
		i=pk_byte();
		if (i!=pk_no_op) {
            char temp[256];
            sprintf(temp,"Bad byte at end of file: %d",i);
			myabort(temp);
		}
        fprintf(log_file,"%d:  No op\n",pk_loc-1); j++;
	}
    fprintf(log_file,"%d bytes read from packed file\n",pk_loc);
//final_end:
	fclose(pk_file);
	fclose(log_file);
    pk_file = nullptr;
    log_file = nullptr;




}




///////////////////////////////////////////////////




//////////////////////////////////// gf file ////////////////////////////////////////

#define three_cases(s) s: case s+1: case s+2
#define four_cases(s) s: case s+1: case s+2: case s+3
#define eight_cases(s) four_cases(s): case four_cases(s+4)
#define sixteen_cases(s) eight_cases(s): case eight_cases(s+8)
#define thirty_two_cases(s) sixteen_cases(s): case sixteen_cases(s+16)
#define thirty_seven_cases(s) thirty_two_cases(s): case four_cases(s+32): case s+36
#define sixty_four_cases(s) thirty_two_cases(s): case thirty_two_cases(s+32)


const int paint_0=0; // beginning of the \\{paint} commands}
const int paint1=64; // move right a given number of columns, then black${}\swap{}$white}
const int boc=67; // beginning of a character}
const int boc1=68; // abbreviated |boc|}
const int eoc=69; // end of a character}
const int skip0=70; // skip no blank rows}
const int skip1=71; // skip over blank rows}
const int new_row_0=74; // move down one row and then right}
//const int max_new_row=238; // move down one row and then right}
const int xxx1=239; // for \&{special} strings}
const int yyy=243; // for \&{numspecial} numbers}
const int no_op=244; // no operation}
const int char_loc=245; // character locators in the postamble}
const int char_loc0=246; // character locators in the postamble}
const int pre=247; // preamble}
const int post=248; // postamble beginning}
const int post_post=249; // postamble ending}
#define undefined_commands 250:251:252:253:254:255
const int gf_id_byte = 131;


enum paint_color
{
	pw_white,
	pw_black
};



static int gf_loc;
static int gf_len;

void find_gf_length()
{
	fseek(gf_file, 0L, SEEK_END);
	gf_len = ftell(gf_file);
}

void move_to_byte(int n)
{
	fseek(gf_file, n, SEEK_SET);
	gf_loc=n;
}


int gf_byte() //{returns the next byte, unsigned}
{
	eight_bits b = 0;
    if (myfeof(gf_file)) myabort("Unexpected end of file!");
	else b = fgetc(gf_file);
	gf_loc++;
	return b;
}
	


int gf_signed_quad() //{returns the next four bytes, signed}
{
	eight_bits a,b,c,d;
	int ret;
	a = fgetc(gf_file); b = fgetc(gf_file); c = fgetc(gf_file); d = fgetc(gf_file);
	if (a<128) ret =  ((a*256+b)*256+c)*256+d;
	else ret = (((a-256)*256+b)*256+c)*256+d;
	gf_loc += 4;
	return ret;
}

unsigned int read_big_endian_32bit(FILE *fp)
{
	unsigned char c[4];
	for (int i = 0; i < 4; i++) {
		int cc = fgetc(fp);
		if (cc == EOF) {
			return 0;
		}
		c[i] = (unsigned char) cc;
	}
	return c[3] + c[2]*256u + c[1]*256u*256u + c[0]*256u*256u*256u;
}


void ReadPXLFile(char const *filename)
{
	int raster_char_index = 0;
	num_chars = 0;

    pxl_file = fopen(filename, "rb");
    if (!pxl_file) {char buf[256]; sprintf(buf, "Error opening %s", filename); myabort(buf);}
    log_file = fopen("TeXFontViewer.log", "a");
    if (!log_file) {
        // failed to open log file, try to change to home dir and try once more
        QDir::setCurrent(QDir::homePath());
        log_file = fopen("TeXFontViewer.log", "a");
    }
    fprintf(log_file, "Opening file %s\n", filename);

	unsigned design_size, checksum;
	unsigned magnification;
	unsigned dir_pointer;
	unsigned int sig1, sig2;
	sig1 = read_big_endian_32bit(pxl_file);
	if (sig1 != 1001) {
        char buf[256]; sprintf(buf, "Not pxl file, sig1 not 1001"); myabort(buf);
	}

	fseek(pxl_file, -20, SEEK_END);

	checksum = read_big_endian_32bit(pxl_file);
	UNUSED(checksum);
	magnification = read_big_endian_32bit(pxl_file);
	design_size = read_big_endian_32bit(pxl_file);
	dir_pointer = read_big_endian_32bit(pxl_file);

    font_info.design_size = design_size/(1024.0*1024.0);
    font_info.hppp = font_info.vppp = 200.0/72.27*magnification/1000.0;

	sig2 = read_big_endian_32bit(pxl_file);
	if (sig2 != 1001) {
        char buf[256]; sprintf(buf, "Not pxl file, sig2 not 1001"); myabort(buf);
	}

	long file_size = ftell(pxl_file);
	file_size /= 4;
	if (dir_pointer != file_size - 512 - 5) {
        char buf[256]; sprintf(buf, "The directory pointer should be %ld, not %u", file_size - 512 - 5, dir_pointer); myabort(buf);
	}

	// move to start of directory
	fseek(pxl_file, dir_pointer*4, SEEK_SET);

	unsigned int cc_info[4];
	for (int cc = 0; cc < 128; cc++) {
		for (int i = 0; i < 4; i++) {
			cc_info[i] = read_big_endian_32bit(pxl_file);
		}
		if (cc_info[0] != 0 || cc_info[1] != 0 || cc_info[2] != 0 || cc_info[3] != 0) {
			unsigned w = cc_info[0] >> 16;
			unsigned h = cc_info[0] & 0xFFFFu;
			short int xoff = (short int)(cc_info[1] >> 16);
			short int yoff = (short int)(cc_info[1] & 0xFFFFu);
			unsigned rasterpointer = cc_info[2];
            int storage_needed = ((h*w) + 7)/8; // in bytes

			image_raster[raster_char_index].resize(storage_needed);
			char_info[raster_char_index].code = cc;
			char_info[raster_char_index].width = w;
			char_info[raster_char_index].height = h;
			char_info[raster_char_index].horz_esc = 0; // what else could we put here?
			char_info[raster_char_index].x_off = xoff;
			char_info[raster_char_index].y_off = yoff;
			char_info[raster_char_index].tfm_width = cc_info[3]/(1024.0*1024.0) * (design_size/(1024.0*1024.0)) * 200.0/72.27*magnification/1000.0;

			long cur_pos = ftell(pxl_file);
            fseek(pxl_file, (int)rasterpointer*4, SEEK_SET);
            unsigned words_per_row = (w+31)/32;
            for (unsigned row = 0; row < h; row++) {
                for (unsigned row_word = 0; row_word < words_per_row; row_word++) {
					unsigned int cur_word = read_big_endian_32bit(pxl_file);

					for (unsigned bit = 31; bit < 32; bit--) {
                        unsigned cur_bit = (cur_word >> bit) & 1u;
                        unsigned col = row_word*32+31-bit;
						if (col >= w)
							break;
						set_image_raster_bit(cur_bit, row*w+col, image_raster[raster_char_index]);
						//if (cur_bit) fprintf(fpout, "*"); 
						//else fprintf(fpout, ".");
					}
				}
				//fprintf(fpout, "\n");
			}

			raster_char_index++;
			num_chars++;


			//fprintf(fpout, "\n\n\n");
			fseek(pxl_file, cur_pos, SEEK_SET); // move back where we were before printing the raster
		}
	}

    fclose(pxl_file);
    pxl_file = NULL;
    fclose(log_file);
    log_file = NULL;




}




void ReadGFFile(char const *filename)
{
	int raster_char_index = 0;
	int k;
	int post_loc;
	int q;
	int design_size;
	int check_sum=0;
	int hppp;
	int vppp;
	int cur_char_tfm_width;
	int cur_char_dx=0;
	int cur_char_dy=0;
	int m = 0, n = 0; // cur col, row - 0 - based
	Array<int, 0, 8> power;
	const int virgin = 0;
	eight_bits gf_com;
    int gf_ch=0;
    int max_n=0, min_n=0, max_m=0, min_m=0;
    int g_max_n=0, g_min_n=0, g_max_m=0, g_min_m=0;
	int cur_color = 0; // 0 = white
    int n_cols=0, n_rows=0;
    const int invalid_num = 100000000;
    int max_m_seen = -invalid_num; // 0 ..
    int min_n_seen = invalid_num; // 0 ..
    int min_m_seen = invalid_num;
    int max_n_seen = -invalid_num;
	int p;
	int save_pos;
	int save_com;

	UNUSED(check_sum);
	UNUSED(cur_char_dy);
	UNUSED(gf_ch);
	num_chars = 0;


	for (i = 0; i < 256; i++) status[i] = virgin;

	power[0] = 1;
	for (i = 1; i <= 8; i++) power[i] = power[i-1] + power[i-1];

    gf_file = fopen(filename, "rb");
    if (!gf_file) {char buf[256]; sprintf(buf, "Error opening %s", filename); myabort(buf);}
    log_file = fopen("TeXFontViewer.log", "a");
    if (!log_file) {
        // failed to open log file, try to change to home dir and try once more
        QDir::setCurrent(QDir::homePath());
        log_file = fopen("TeXFontViewer.log", "a");
    }
    fprintf(log_file, "Opening file %s\n", filename);

    if (gf_byte() != pre) myabort("First byte is not preamble");
    if (gf_byte() != gf_id_byte) myabort("Identification byte is incorrect");

	find_gf_length(); post_loc = gf_len - 4;
	do {
        if (post_loc == 0) myabort("all 223's");
		move_to_byte(post_loc); k = gf_byte(); post_loc--;
	} while (k == 223);

    if (k != gf_id_byte) {char buf[256]; sprintf(buf, "ID byte is %d", k); myabort(buf);}

	move_to_byte(post_loc - 3); q = gf_signed_quad();

    if (q < 0 || q > post_loc - 3) {char buf[256]; sprintf(buf, "post pointer is %d", q); myabort(buf);}
	move_to_byte(q); k = gf_byte();
    if (k != post) {char buf[256]; sprintf(buf, "byte at %d is not post", q); myabort(buf);}
	i = gf_signed_quad(); // skip over junk

	design_size = gf_signed_quad(); check_sum = gf_signed_quad();
	hppp = gf_signed_quad();
	vppp = gf_signed_quad();

    font_info.design_size = design_size/(1024.0*1024.0);
    font_info.hppp = hppp/(256.0*256.0);
    font_info.vppp = vppp/(256.0*256.0);

	g_min_m = gf_signed_quad(); g_max_m = gf_signed_quad(); 
	g_min_n = gf_signed_quad(); g_max_n = gf_signed_quad();
	// allocate space large enough to hold raster of any character
	// array indexed by [row][col], one byte per pixel
	n_rows = g_max_n - g_min_n + 1;
	n_cols = g_max_m - g_min_m + 1;

    std::vector<std::vector<int>> raster_buffer;

	do { 
		gf_com = gf_byte();
		switch (gf_com) {
			case char_loc: case char_loc0:
				gf_ch = gf_byte();
				if (gf_com == char_loc) {
					cur_char_dx = gf_signed_quad(); cur_char_dy = gf_signed_quad();
				}
				else {
					cur_char_dx = gf_byte() * 65536; cur_char_dy = 0;
				}
				cur_char_tfm_width = gf_signed_quad(); p = gf_signed_quad();


				// test with dingbat.600gf
				save_pos = ftell(gf_file);
				save_com = gf_com;
				move_to_byte(p);
				// go find linked list of characters
				do {
					gf_com = gf_byte();
					switch (gf_com) {

                        case sixty_four_cases(paint_0): {
                            int cur_col = m;
							for (int cc = 0; cc < gf_com; cc++) {
                                raster_buffer[max_n - n][cur_col++ - min_m] = cur_color;
							}
                            if (cur_color == 1 && cur_col > m) { // something was painted
                                if (n < min_n_seen) min_n_seen = n;
                                if (n > max_n_seen) max_n_seen = n;
                                if (m < min_m_seen) min_m_seen = m;
                                if (cur_col - 1 > max_m_seen) max_m_seen = cur_col - 1;
                            }
                            m = cur_col;

							cur_color = !cur_color;
                            Q_ASSERT(m >= min_m);
                            Q_ASSERT(n >= min_n);
                            Q_ASSERT(m <= max_m);
                            Q_ASSERT(n <= max_n);
							break;
                        }

						case three_cases(paint1): {
                                int cur_col = m;
								i = 0;
								for (j=0; j <= gf_com - paint1; j++) {
									k = gf_byte(); i = i*256 + k;
								}
								for (int cc = 0; cc < i; cc++)
                                    raster_buffer[max_n - n][cur_col++ - min_m] = cur_color;

                                if (cur_color == 1 && cur_col > m) { // something was painted
                                    if (n < min_n_seen) min_n_seen = n;
                                    if (n > max_n_seen) max_n_seen = n;
                                    if (m < min_m_seen) min_m_seen = m;
                                    if (cur_col - 1 > max_m_seen) max_m_seen = cur_col - 1;
                                }
                                m = cur_col;
								cur_color = !cur_color;

							}
                            Q_ASSERT(m >= min_m);
                            Q_ASSERT(n >= min_n);
                            Q_ASSERT(m <= max_m);
                            Q_ASSERT(n <= max_n);
							break;

						case four_cases(xxx1): {
							i = 0;
							for (j=0; j <= gf_com - xxx1; j++) {
								k = gf_byte(); i = i*256 + k;
							}
							for (j=1; j <= i; j++) gf_byte();
							}
							break;


						case four_cases(skip0):
							i = 0;
							for (j=0; j <= gf_com - skip1; j++) {
								k = gf_byte(); i = i*256 + k;
							}
							n -= i+1;
                            m = min_m;
							cur_color = 0;
                            Q_ASSERT(m >= min_m);
                            Q_ASSERT(n >= min_n);
                            Q_ASSERT(m <= max_m);
                            Q_ASSERT(n <= max_n);
							break;

						case sixty_four_cases(new_row_0):
						case sixty_four_cases(new_row_0+64):
						case thirty_seven_cases(new_row_0+128):
							i = gf_com - new_row_0;
							n--;
                            m = min_m + gf_com - new_row_0;
                            Q_ASSERT(m >= min_m);
                            Q_ASSERT(n >= min_n);
                            Q_ASSERT(m <= max_m);
                            Q_ASSERT(n <= max_n);
							cur_color = 1; // black
							break;

						case yyy:
							gf_signed_quad();
							break;

						case no_op:
							break;

						case boc:
						case boc1: {
							if (gf_com == boc) {
								char_info[raster_char_index].code = gf_signed_quad();
								p = gf_signed_quad();
								min_m = gf_signed_quad(); max_m = gf_signed_quad(); min_n = gf_signed_quad(); max_n = gf_signed_quad();
							}
							else {
								char_info[raster_char_index].code = gf_byte();
								p = -1;
								int del_m = gf_byte();
								max_m = gf_byte();
								int del_n = gf_byte();
								max_n = gf_byte();
								min_m = max_m - del_m;
								min_n = max_n - del_n;
							}

                            max_m_seen = -100000000; // 0 ..
                            min_n_seen = 100000000; // 0 ..
                            min_m_seen = 100000000;
                            max_n_seen = -100000000;

                            m = min_m;
                            n = max_n;
							cur_color = 0; // white

                            n_rows = max_n - min_n + 1;
                            n_cols = max_m - min_m + 1;

                            raster_buffer.resize(n_rows);
                            for (int o = 0; o < n_rows; o++) raster_buffer[o].resize(n_cols);

							// set all pixels to white
							for (int rr = min_n; rr <= max_n; rr++) {
								for (int cc = min_m; cc <= max_m; cc++) {
									raster_buffer[rr - min_n][cc - min_m] = 0;
								}
							}
							break;
						}

						case eoc: {
                            int w,h;
                            if (max_m_seen == -invalid_num ||
                                min_n_seen == invalid_num ||
                                min_m_seen == invalid_num ||
                                max_n_seen == -invalid_num) {
                                w = h = 0;
                            }
                            else {
                                w = max_m_seen - min_m_seen + 1;
                                h = max_n_seen - min_n_seen + 1;
                            }

							int storage_needed = ((h*w) + 7)/8; // in bytes aligned to word

							image_raster[raster_char_index].resize(storage_needed);
							char_info[raster_char_index].width = w;
							char_info[raster_char_index].height = h;
							char_info[raster_char_index].horz_esc = cur_char_dx/(256.0*256.0);
							char_info[raster_char_index].x_off = -min_m;
							char_info[raster_char_index].y_off = max_n;
							char_info[raster_char_index].tfm_width = cur_char_tfm_width/(1024.0*1024.0)*design_size/(1024.0*1024.0)*hppp/(256.0*256.0);

                            if ( !(w == 0 && h == 0))
                                for (int rr = max_n_seen; rr >= min_n_seen; rr--) {
                                    for (int cc = min_m_seen; cc <= max_m_seen; cc++) {
                                        set_image_raster_bit(raster_buffer[max_n - rr][cc-min_m], (max_n_seen - rr)*w + cc-min_m_seen, image_raster[raster_char_index]);
                                    }
                                }
							raster_char_index++;
							num_chars++;

							if (p == -1)
								goto done_char;
							else move_to_byte(p);
							break;
						}
					}
				} while (true);
				done_char:
				fseek(gf_file, save_pos, SEEK_SET); // go back to postamble
				gf_com = save_com;
				break;

			case four_cases(xxx1): {
				i = 0;
				for (j=0; j <= gf_com - xxx1; j++) {
					k = gf_byte(); i = i*256 + k;
				}
				for (j=1; j <= i; j++) gf_byte();
			}
				break;

			case yyy:
				gf_signed_quad();
				break;

			case no_op:
				break;


			case post_post:
				;
				break;

			default:
				{
                    char buf[256]; sprintf(buf, "Unexpected %d in postamble", gf_com); myabort(buf);
				}

				break;

		}
	} while (gf_com != post_post);

    fclose(gf_file);
    fclose(log_file);
    gf_file = nullptr;
    log_file = nullptr;
}



