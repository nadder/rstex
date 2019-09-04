#pragma once
#include <vector>



struct FontChar
{
	int min_m;
	int max_n;
	int chardx; // in pixels
	int char_code;
	int width; // in dvi units
	HBITMAP hBitmap;
};

struct Font
{
	std::vector<FontChar> Char;
	int hppp;
	int vppp;
	int check_sum;
	int design_size;
	char filename[256];
	int num_chars;
	bool loaded;
};


struct Character
{
	FontChar *pFontChar;
	int x; // screen coordinate of this character
	int y;
};

struct Rule
{
	int x; // screen coordinate of this rule
	int y;
	int width; // width in pixels
	int height; // height in pixels
};

extern std::vector<Font> FontVector;
extern std::vector<std::vector<Character>> PageCharVector;
extern std::vector<std::vector<Rule>> PageRuleVector;



bool ReadDVIFile(char const *pFileName);
int index_from_charcode(const Font *f, int charcode);
