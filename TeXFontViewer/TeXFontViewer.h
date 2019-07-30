#pragma once

#include "resource.h"
typedef unsigned char eight_bits;
struct char_raster_info
{
	int width;  // in pixels
	int height; // in pixels
	double horz_esc; // in pixels
	double tfm_width; // in pixels
	int y_off;// in pixels
	int x_off;// in pixels
	double hppp; // pixels per point
	double design_size; //points
	int code;
};
