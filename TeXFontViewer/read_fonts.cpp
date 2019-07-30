#include "stdafx.h"
#include "TeXFontViewer.h"

extern HWND g_hwnd;

void myabort(wchar_t const*msg) 
{
	MessageBox( g_hwnd, msg, L"Error", MB_OK|MB_ICONERROR); throw -1;
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


eight_bits *image_raster[256];
char_raster_info char_info[256];
int num_chars;

int term_pos; // current terminal position

FILE *pk_file;
int pk_loc;
FILE *log_file;

const int pk_id = 89; //{the version of \.{PK} file described}
const int pk_xxx1 = 240; //{\&{special} commands}
const int pk_yyy = 244; //{\&{numspecial} commands}
const int pk_post = 245; //{postamble}
const int pk_no_op = 246; //{no operation}
const int pk_pre = 247; //{preamble}

int hppp, vppp;
int design_size;
int checksum;
int magnification;

int i, j; // {index pointers}
int flag_byte; // {the byte that introduces the character definition}
int end_of_packet; // {where we expect the end of the packet to be}
int width, height; // {width and height of character}
int x_off, y_off; // {x and y offsets of character}
int tfm_width; // {character tfm width}
Array<int, 0, 255> tfms; // : array [0..255] of integer ; {character tfm widths}
int dx, dy; // {escapement values}
Array<int, 0, 255> dxs, dys; // : array [0..255] of integer ; {escapement values}
Array<bool, 0, 255> status; // : array[0..255] of boolean ; {has the character been seen?}
int dyn_f; // {dynamic packing variable}
int car; // {the character we are reading}
int packet_length; // {the length of the character packet}


// 51
int repeat_count; //{how many times to repeat the next row?}
int rows_left; //{how many rows left?}
bool turn_on; //{are we black here?}
int h_bit; //{what is our horizontal position?}
int count; //{how many bits of current color left?}

// 47
eight_bits input_byte; // the byte we are currently decimating
eight_bits bit_weight; // weight of the current bit
eight_bits nybble; // the current nybble

// 33
eight_bits pk_byte()
{
	eight_bits temp;
	temp=fgetc(pk_file); pk_loc++; return temp;
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
					fwprintf(log_file,L"%d:  Special: '",(pk_loc-1)) ;
					i=0 ;
					for (j=240; j<=flag_byte; j++) i=256*i+pk_byte() ;
					for (j=1; j<=i; j++) fputc(pk_byte(),log_file) ;
					fwprintf(log_file,L"'\n") ;
				}
				break;
				case 244: fwprintf(log_file,L"%d:  Num special: %d\n",pk_loc-1,get_32()) ;
					break;
				case 245: fwprintf(log_file,L"%d:  Postamble\n",pk_loc-1) ;
					break;
				case 246: fwprintf(log_file,L"%d:  No op\n",(pk_loc-1));
					break;
				case 247: case 248: case 249: case  250: case  251: case  252: case  253: case  254: case  255:
				{wchar_t temp[256]; _swprintf(temp,L"Unexpected %d!",flag_byte); myabort(temp) ;}
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
		term_pos=len+2 ; fwprintf(log_file,L" \n") ; fwprintf(log_file,L"  ") ;
	} else term_pos=term_pos+len ;
	if (repeat_count) fwprintf(log_file,L"[%d]",value);
	else if (turn_on) fwprintf(log_file,L"%d",value);
	else fwprintf(log_file,L"(%d)",value);
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
		if (repeat_count!=0) myabort(L"Second repeat count for this row!") ;
  //@.Second repeat count...@>
		if (i==14)
			repeat_count=pk_packed_num();
		else
			repeat_count=1 ;
		send_out(true,repeat_count) ;
		return pk_packed_num() ;
	}
}


void set_image_raster_bit(bool set, int bit_offset, eight_bits *raster)
{
	int byte_index = bit_offset / 8;
	int bit_index = 7 - bit_offset % 8; // bit_offset 0 is the most significant bit in the byte
	if (set)
		raster[byte_index] |= (1 << bit_index);
	else 
		raster[byte_index] &= ~(1 << bit_index);
}

bool get_image_raster_bit(int bit_offset, eight_bits *raster)
{
	int byte_index = bit_offset / 8;
	int bit_index = 7 - bit_offset % 8; // bit_offset 0 is the most significant bit in the byte
	return (raster[byte_index] & (1 << bit_index)) != 0;
}


void copy_row(int width, int height, int row_to_copy, int n_rows_to_copy, eight_bits *raster)
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

void zoom_raster(int zoom_factor, int width, int height, eight_bits *raster, eight_bits **out_raster)
{
	int new_size = (zoom_factor*width*zoom_factor*height+7)/8;
	*out_raster = (eight_bits*)malloc(new_size);
	int src_offset = 0;
	int dest_offset = 0;
	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			int src_bit = get_image_raster_bit(src_offset++, raster);
			for (int i = 0; i < zoom_factor; i++)
				set_image_raster_bit(src_bit, dest_offset++,*out_raster);
		}
		copy_row(width*zoom_factor, height*zoom_factor, yy*zoom_factor, zoom_factor-1, *out_raster);
		dest_offset += (zoom_factor-1)*zoom_factor*width;

	}
}


void read_pk_file(wchar_t const * filename)
{
	int raster_char_index = 0;
	num_chars = 0;
	pk_file = _wfopen(filename, L"rb");
	if (!pk_file) {wchar_t buf[256]; _swprintf(buf, L"Error opening %s", filename); myabort(buf);}
	log_file = _wfopen(L"TeXFontViewer.log", L"a");
	fwprintf(log_file, L"Opening file %s\n", filename);
	// Read preamble 38>
	if (pk_byte()!=pk_pre) myabort(L"Bad PK file; pre command missing!");
	if (pk_byte()!=pk_id) myabort(L"Wrong version of PK file!");

	j=pk_byte(); fwprintf(log_file,L"'");
	for (i=1; i<=j; i++) fputc(pk_byte(),log_file);
	fwprintf(log_file,L"'\n"); design_size=get_32(); fwprintf(log_file,L"Design size = %d\n",design_size);
	checksum=get_32(); fwprintf(log_file,L"Checksum = %d\n",checksum); hppp=get_32(); vppp=get_32();
	fwprintf(log_file,L"Resolution: horizontal = %d  vertical = %d",hppp,vppp);
	magnification=(int)round(hppp*72.27/65536); fwprintf(log_file,L"  (%d dpi)\n",magnification);
	if (hppp!=vppp) printf("Warning:  aspect ratio not 1:1!\n");
	// Read preamble

	skip_specials();
	while (flag_byte!=pk_post) {
		// Unpack and write characters 40>
		fwprintf(log_file,L"%d:  Flag byte = %d",pk_loc-1,flag_byte); dyn_f=flag_byte/16;
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

		fwprintf(log_file,L"  Character = %d  Packet length = %d\n",car,packet_length);
		fwprintf(log_file,L"  Dynamic packing variable = %d\n",dyn_f);
		fwprintf(log_file,L"  TFM width = %d  dx = %d",tfm_width,dx);
		if (dy!=0) fwprintf(log_file,L"  dy = %d\n",dy);
		else fwprintf(log_file,L" \n");
		fwprintf(log_file,L"  Height = %d  Width = %d  X-offset = %d  Y-offset = %d\n",
			height,width,x_off,y_off);

		// store raster information in one long array of bytes, the byte has the value one for a black pixel
		// and 0 for a white pixel

		if (image_raster[raster_char_index]) {
			free(image_raster[raster_char_index]);
		}
		int storage_needed = ((height*width) + 7)/8; // in bytes aligned to word

		image_raster[raster_char_index] = (eight_bits*)malloc(storage_needed);
		char_info[raster_char_index].code = car;
		char_info[raster_char_index].width = width;
		char_info[raster_char_index].height = height;
		char_info[raster_char_index].horz_esc = dx/(256.0*256.0);
		char_info[raster_char_index].x_off = x_off;
		char_info[raster_char_index].y_off = y_off;
		char_info[raster_char_index].tfm_width = tfm_width/(1024.0*1024.0)*design_size/(1024.0*1024.0)*hppp/(256.0*256.0);
		char_info[raster_char_index].design_size = design_size/(1024.0*1024.0);
		char_info[raster_char_index].hppp = hppp/(256.0*256.0);

		eight_bits *cur_image_raster = image_raster[raster_char_index];
		raster_char_index++;
		num_chars++;
		int raster_offset = 0;
		// Read and translate raster description 48>
		bit_weight=0 ;
		if (dyn_f==14)
		   // Get raster by bits 49>
		{
			for (i=1; i<=height; i++) {
				fwprintf(log_file,L"  ") ;
				for (j=1; j<=width; j++) {
					if (get_bit()) {
						fwprintf(log_file,L"*"); 
						set_image_raster_bit(true, raster_offset, cur_image_raster);
					}
					else {
						fwprintf(log_file,L"."); 
						set_image_raster_bit(false, raster_offset, cur_image_raster);
					}
					raster_offset++;
				}
				fwprintf(log_file,L" \n") ;
			}
		}
			// Get raster by bits>
		else
			// Create normally packed raster 50>
		{
			term_pos=2 ;
			fwprintf(log_file,L"  ") ;
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
			fwprintf(log_file,L" \n") ;
			if (rows_left!=0||h_bit!=width)
				myabort(L"Bad PK file: More bits than required!");
			 //@.More bits than required@>



		}
			// Create normally packed raster>

		// Read and translate raster description >
		if (end_of_packet!=pk_loc) myabort(L"Bad PK file: Bad packet length!");



		// Unpack and write characters 40

		skip_specials();
	}
	j=0;
	while (!myfeof(pk_file)) {
		i=pk_byte();
		if (i!=pk_no_op) {
			wchar_t temp[256];
			_swprintf(temp,L"Bad byte at end of file: %d",i);
			myabort(temp);
		}
		fwprintf(log_file,L"%d:  No op\n",pk_loc-1); j++;
	}
	fwprintf(log_file,L"%d bytes read from packed file\n",pk_loc);
//final_end:
	fclose(pk_file);
	fclose(log_file);





}


///////////////////////////////////////////////////