#include "tdfont/tdfont.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
NOTE:
  -TdFont is a really old system that has been cleaned up somewhat but still contains quite ugly code.
   I wouldn't be susprised if it contained bugs.

TODO:
  -Add a leading advance! (there's shadow and/or antialiased pixels to the left of glyphs too).
   This should improve positioning, especially for small fonts or large fonts with lots of halo/shadow.
  -italic/obliqe by just skewing the trignale output on drawing!
  -Bold by shader/extra blit (would look bolder only for small fonts)
  -monospace only needs to be a painting mode. Use leading & trailing advance for that.
   enabled from the name? UIFont_mono_8.png
  -avecharwidth, maxcharwidth
  -ascent from pixels on first x column. (or approx if there's no pixels there)
*/

/*
ISO 8859-1 (ISO Latin-1)
 ASCII codes that should be in a font, starting with a space for space:
 !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ
*/

TdFont::TdFont()
{
	int i;
	for(i=0;i<=32;i++)
		gmap[i] = &glyph[0]; //space to space
	int glyph_index = 1;
	for(i=33;i<256;i++)
	{
		switch (i)
		{
		case 127:
		case 129:
		case 141:
		case 143:
		case 144:
		case 157:
		case 160:
		case 173:
			gmap[i] = &glyph[0]; //connect to space
			break;
		default:
			gmap[i] = &glyph[glyph_index++];
		};
	}
	fonth = 0;
	texture_id = 0;
	memset(&glyph, 0, sizeof(TDFNT_GLYPH) * FONT_NUM_GLYPH_SLOTS);
}

TdFont::~TdFont()
{
	TdFontRenderer::DestroyTexture(this);
}

inline unsigned char GetAlpha(unsigned int color)
{
	return (color & 0xff000000) >> 24;
}

void TdFont::FindGlyphs(int width, int height, unsigned int *data32)
{
	if (!width || !height)
		return;
	long int i,j,o,r,r2,r3,ofy1=-1,ofy2=0;
	long int x[2];

	unsigned int * buf = data32;
	for(j=1;j<height;j++)
	{
		r=0;
		for(i=0;i<width;i++)
		{
			if(GetAlpha(buf[i+j*width]) != 0)
			{
				r=1;
			}
		}
		if(r==1 && ofy1==-1)
			ofy1=j-1;
		if(r==0 && ofy1!=-1)
		{
			ofy2=j+1;
			break;
		}
	}
	ofy1++;
	ofy2--;
	fonth = ofy2 - ofy1;

	int average_shadow = 0;
	int average_shadow_glyph_count = 0;

	//Start search for the rest of the letters
	i=0;
	for(o=1;o<FONT_NUM_GLYPH_SLOTS;o++)
	{
		//Find the first edge of a letter
		r2=1;
		while(r2 && i < width)
		{
			for(j=ofy1;j<ofy2;j++)
			{
				if(GetAlpha(buf[i+j*width]) != 0)
					r2=0;
			}
			if(r2)
				i++;
		}
		x[0]=i;
		//Find the second edge of a letter
		int advance_i = 0;
		r2=1;
		while(r2 && i < width)
		{
			r3=0;
			bool has_solid = false;
			for(j=ofy1;j<ofy2;j++)
			{
				if(GetAlpha(buf[i+j*width]) != 0)
					r3=1;
				if((buf[i+j*width] & 0xff000000) == 0xff000000)
					has_solid = true;
			}
			if (has_solid)
				advance_i = i;
			if(r3==1)
				i++;
			else
				r2=0;
		}
		x[1]=i;

		assert(o < FONT_NUM_GLYPH_SLOTS);
		glyph[o].advance = advance_i-x[0]+1;
		glyph[o].w = x[1]-x[0]+1-1;
		glyph[o].h = fonth;
		glyph[o].u = x[0];
		glyph[o].v = ofy1;
		glyph[o].iw = width;
		glyph[o].ih = height;

		// Some glyphs might be made without any fully solid pixels. In that case we will use the actual width since advance will be 0.
		if (glyph[o].advance <= 0)
			glyph[o].advance = glyph[o].w;

	//	if (o > 33 && o < 95)
		{
			average_shadow += glyph[o].w - glyph[o].advance;
			average_shadow_glyph_count++;
		}
	}

#if 1 // Fix for small fonts
	// On small letters with very weak glyphs, we might misinterpret the anialiased corners as shadow.
	// The advance should not take away the pixels that are not shadow, so assume the shadow is the same
	// for all glyphs and use the average shadow (glyph width - glyph advance) on all.
	average_shadow /= average_shadow_glyph_count;
	for(o=1;o<FONT_NUM_GLYPH_SLOTS;o++)
	{
		glyph[o].advance = glyph[o].w - average_shadow + 1;
	}
#endif

	// Space is special since it's first and we don't find it.
	// Copy properties from charater 'x' and multiply width and advance by 0.8.
	*gmap[' '] = *gmap['x'];
	gmap[' ']->w = (int)(gmap[' ']->w * 0.8f);
	gmap[' ']->advance = (int)(gmap[' ']->advance * 0.8f);
	gmap[' ']->u = 0; // It's located first
}

TDFNT_GLYPH *TdFont::GetGlyph(char c)
{
	unsigned char uc = c;
	return gmap[uc];
}


//static
TdFont *TdFont::Create(int width, int height, unsigned int *data32)
{
	TdFont *font = new TdFont;
	if (font)
	{
		font->FindGlyphs(width, height, data32);
		TDFNT_IMG *img = font->RemapIntoSquare(width, height, data32);
		if (!img->data32 || !TdFontRenderer::CreateTexture(font, img->w, img->h, img->data32))
		{
			delete font;
			delete [] img->data32;
			return NULL;
		}
		delete [] img->data32;
	}
	return font;
}

TDFNT_IMG *TdFont::RemapIntoSquare(int width, int height, unsigned int *data32)
{
	static TDFNT_IMG img;
	int tw = 64;
	int th = 64;
	int x, y;
	TDFNT_GLYPH new_glyph[FONT_NUM_GLYPH_SLOTS];
retry:
	x = 0;
	y = 0;
	for (int i = 0; i < FONT_NUM_GLYPH_SLOTS; i++)
	{
		// Step further and update position
		if (x + glyph[i].w > tw)
		{
			x = 0;
			y += fonth;
		}
		if (y + fonth >= th) // > ?
		{
			// There is no more room. Increase dimensions and try again.
			// Only increase the smaller axis (this is if we wan't rectangular textures instead of squares)
			if (tw > th)
				th *= 2;
			else
				tw *= 2;
			goto retry;
		}
		// Update new glyph data
		new_glyph[i] = glyph[i];
		new_glyph[i].u = x;
		new_glyph[i].v = y;
		new_glyph[i].iw = tw;
		new_glyph[i].ih = th;
		x += glyph[i].w;
	}
	// Allocate new buffer and copy over glyph data to it
	img.data32 = new unsigned int[tw * th];
	img.w = tw;
	img.h = th;
	if (img.data32)
	{
		memset(img.data32, 0, tw * th * 4);
		for (int i = 0; i < FONT_NUM_GLYPH_SLOTS; i++)
		{
			for (int y = 0; y < fonth; y++)
			{
				memcpy(&img.data32[new_glyph[i].u + (new_glyph[i].v + y) * tw], &data32[glyph[i].u + (glyph[i].v + y) * width], new_glyph[i].w * 4);
			}
		}
	}
	//else
	//	error
	// The new glyphs are ready so replace the old ones with them.
	memcpy(glyph, new_glyph, sizeof(TDFNT_GLYPH) * FONT_NUM_GLYPH_SLOTS);
	return &img;
}
