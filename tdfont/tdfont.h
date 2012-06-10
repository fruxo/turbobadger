#ifndef TDFONT_H
#define TDFONT_H

/** Define to take utf8 data in TdFontRenderer::GetStringWidth and
	TdFontRenderer::DrawString instead of ISO 8859-1 (ISO Latin-1).

	Note: The font still does not support unicode. This is only done
	for compability with UTF-8 applications that doesn't require it. */
#define TDFONT_USE_UTF8

struct TDFNT_GLYPH
{
	int w, h;
	int u, v;
	int advance;
	int iw;
	int ih;
};

struct TDFNT_IMG {
	int w, h;
	unsigned int *data32;
};

#define FONT_NUM_GLYPH_SLOTS (256-32)

class TdFont
{
private:
	friend class TdFontRenderer;
	TDFNT_GLYPH glyph[FONT_NUM_GLYPH_SLOTS];
	TDFNT_GLYPH *gmap[256]; //connected to fontspr
	int fonth;
	unsigned int texture_id;

	TdFont();
	void FindGlyphs(int width, int height, unsigned int *data32);
	TDFNT_IMG *RemapIntoSquare(int width, int height, unsigned int *data32);
public:

	~TdFont();

	static TdFont *Create(int width, int height, unsigned int *data32);

	TDFNT_GLYPH *GetGlyph(char c);
};

// == Drawing =======================================

#define TDFNT_LEFT_TOP    0x000
#define TDFNT_CENTER_X    0x001
#define TDFNT_CENTER_Y    0x002
#define TDFNT_RIGHT       0x004

#define ALL_TO_TERMINATION 2147483647

class TdFontRendererBackend
{
public:
	virtual void BeginDrawString(TdFont *font) = 0;
	virtual void EndDrawString(TdFont *font) = 0;
	virtual void DrawGlyph(float x, float y, TDFNT_GLYPH *glyph, unsigned int texture) = 0;
};

class TdFontRenderer
{
private:
	friend class TdFont;
	static bool CreateTexture(TdFont *font, int w, int h, unsigned int *data32);
	static void DestroyTexture(TdFont *font);
public:
	static float GetExtraCharSpacing();
	static void SetExtraCharSpacing(float extra_char_spacing);

	static float GetStringWidth(TdFont *font, const char *str, int len = ALL_TO_TERMINATION);
	static float GetFontHeight(TdFont *font);
	static void DrawString(TdFont *font, float x, float y, unsigned int pos, const char *str, int len = ALL_TO_TERMINATION);

	static void SetRendererBackend(TdFontRendererBackend *backend);

	/** Call BeginDrawString before, and EndDrawString after many DrawString operations for faster code. */
	static void BeginDrawString(TdFont *font);
	static void EndDrawString(TdFont *font);

#ifndef TDFONT_USE_UTF8
	/** Set the font used for symbols. This will not take ownership of the font, and it
		must exist as long as it's set. It can be unset by passing NULL to this function.

		When a symbol font is set, it will be used to draw glyphs for escaped code values.
		The escape code is \x1B followed by a comma and a 2 character hex value.
		F.ex "Foo\x1B,00Bar" will draw "Foo", the first glyph in the symbol font, and "Bar" */
	static void SetSymbolFont(TdFont *font);
#endif // TDFONT_USE_UTF8

#ifdef _DEBUG
#ifndef TDFONT_USE_UTF8
	static void DebugDrawPositioning(TdFont *font, float x, float y);
#endif
	static void DebugDrawFontTexture(TdFont *font, float x, float y);
#endif
};

#endif // TDFONT_H
