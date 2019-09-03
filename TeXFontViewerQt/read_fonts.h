#ifndef READ_FONTS_H
#define READ_FONTS_H
#include <vector>

typedef unsigned char eight_bits;
struct char_raster_info
{
    int width;  // in pixels
    int height; // in pixels
    double horz_esc; // in pixels
    double tfm_width; // in pixels
    int y_off;// in pixels
    int x_off;// in pixels
    int code;
};

struct sfont_info
{
    double hppp; // pixels per point
    double vppp; // pixels per point
    double design_size; //points
};

extern char_raster_info char_info[256];
extern sfont_info font_info;

extern std::vector<std::vector<eight_bits>> image_raster;
extern int num_chars;

void read_pk_file(char const * filename);
void ReadGFFile(char const *filename);
void ReadPXLFile(char const *filename);
void zoom_raster(int zoom_factor, int width, int height, std::vector<eight_bits>& raster, std::vector<eight_bits>& out_raster);
bool get_image_raster_bit(unsigned bit_offset, std::vector<eight_bits>& raster);
void set_image_raster_bit(bool set, unsigned bit_offset, std::vector<eight_bits>& raster);
#endif // READ_FONTS_H
