#ifdef _WIN32
#include <windows.h> // make gl.h compile
#include <GL/gl.h>
#elif defined(MACOSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif // _WIN32

#include "tdfont/tdfont.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#ifdef TDFONT_USE_UTF8
#include "utf8/utf8.h"
#endif

float extra_char_space = 0;
int begin_draw_string_counter = 0;
TdFont *symbol_font = 0;

TdFontRendererBackend *g_backend = nullptr;

#ifdef TDFONT_USE_UTF8
unsigned char UCS4_to_tdfnt(UCS4 c)
{
	// some conversion..
	switch (c)
	{
	case 0x20AC: return '€'; // EURO SIGN
	case 0x201A: return '‚'; // SINGLE LOW-9 QUOTATION MARK
	case 0x0192: return 'ƒ'; // LATIN SMALL LETTER F WITH HOOK
	case 0x201E: return '„'; // DOUBLE LOW-9 QUOTATION MARK
	case 0x2026: return '…'; // HORIZONTAL ELLIPSIS
	case 0x2020: return '†'; // DAGGER
	case 0x2021: return '‡'; // DOUBLE DAGGER
	case 0x02C6: return 'ˆ'; // MODIFIER LETTER CIRCUMFLEX ACCENT
	case 0x2030: return '‰'; // PER MILLE SIGN
	case 0x0160: return 'Š'; // LATIN CAPITAL LETTER S WITH CARON
	case 0x2039: return '‹'; // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	case 0x0152: return 'Œ'; // LATIN CAPITAL LIGATURE OE
	case 0x017D: return ''; // LATIN CAPITAL LETTER Z WITH CARON
	case 0x2018: return '‘'; // LEFT SINGLE QUOTATION MARK
	case 0x2019: return '’'; // RIGHT SINGLE QUOTATION MARK
	case 0x201C: return '“'; // LEFT DOUBLE QUOTATION MARK
	case 0x201D: return '”'; // RIGHT DOUBLE QUOTATION MARK
	case 0x2022: return '•'; // BULLET
	case 0x2013: return '–'; // EN DASH
	case 0x2014: return '—'; // EM DASH
	case 0x02DC: return '˜'; // SMALL TILDE
	case 0x2122: return '™'; // TRADE MARK SIGN
	case 0x0161: return 'š'; // LATIN SMALL LETTER S WITH CARON
	case 0x203A: return '›'; // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	case 0x0153: return 'œ'; // LATIN SMALL LIGATURE OE
	case 0x017E: return ''; // LATIN SMALL LETTER Z WITH CARON
	case 0x0178: return 'Ÿ'; // LATIN CAPITAL LETTER Y WITH DIAERESIS
	default:
		if (c > 255)
			return '?'; // It's a character we can't display
		return (unsigned char)c;
	};
}

#endif // TDFONT_USE_UTF8

// == GL SPECIFIC ===========================================

void TdFontRenderer::SetRendererBackend(TdFontRendererBackend *backend)
{
	g_backend = backend;
}

void TdFontRenderer::BeginDrawString(TdFont *font)
{
	begin_draw_string_counter++;
	if (begin_draw_string_counter > 1)
		return;

	g_backend->BeginDrawString(font);
}

void TdFontRenderer::EndDrawString(TdFont *font)
{
	begin_draw_string_counter--;
	assert(begin_draw_string_counter >= 0);
	if (begin_draw_string_counter > 0)
		return;

	g_backend->EndDrawString(font);
}

bool TdFontRenderer::CreateTexture(TdFont *font, int w, int h, unsigned int *data32)
{
//// FIX: does not need to have a backend specific renderer anymore. This should be a TBBitmap created in the backend instead!
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data32);
	bool success = glGetError() == GL_NO_ERROR;
	glBindTexture(GL_TEXTURE_2D, 0);

	font->texture_id = texture;
	return success;
}

void TdFontRenderer::DestroyTexture(TdFont *font)
{
	GLuint texture = font->texture_id;
	glDeleteTextures(1, &texture);
	font->texture_id = 0;
}

// == NON GL SPECIFIC =======================================

int tdfnt_hexnibble(char c)
{
	if(isdigit(c))
		return c - '0';
	return 10 + toupper(c) - 'A';
}

float TdFontRenderer::GetExtraCharSpacing()
{
	return extra_char_space;
}

void TdFontRenderer::SetExtraCharSpacing(float extra_char_spacing)
{
	::extra_char_space = extra_char_space;
}

float TdFontRenderer::GetStringWidth(TdFont *font, const char *str, int len)
{
	float w = 0;
	int i = 0;
	while (i < len && str[i])
	{
#ifdef TDFONT_USE_UTF8
		UCS4 uc = utf8::decode_next(str, &i, len);
		char c = UCS4_to_tdfnt(uc);
		TDFNT_GLYPH *glyph = font->GetGlyph(c);
#else
		char c = str[i++];
		TDFNT_GLYPH *glyph = font->GetGlyph(c);
		if (symbol_font && c == '\x1B' && str[i] != 0 && str[i + 1] != 0 && str[i + 2] != 0)
		{
			// Get glyph from symbol font
			int glyph_index = (tdfnt_hexnibble(str[i + 1]) << 4) + tdfnt_hexnibble(str[i + 2]);
			glyph = symbol_font->GetGlyph(glyph_index + 33);
			i += 3;
		}
#endif
		w += (glyph->advance + extra_char_space);
		if (c == ' ') // Snap to closest pixel on new word
			w = (float)(int)w;
	}
	return w;
}

float TdFontRenderer::GetFontHeight(TdFont *font)
{
	return (float)font->fonth;
}

void TdFontRenderer::DrawString(TdFont *font, float x, float y, unsigned int pos, const char *str, int len)
{
	BeginDrawString(font);

	if (pos & TDFNT_CENTER_X)
		x -= (int) GetStringWidth(font, str) / 2; // (int) to snap to not cause unnecessary blurriness
	else if (pos & TDFNT_RIGHT)
		x -= (int) GetStringWidth(font, str); // (int) to snap to not cause unnecessary blurriness
	if (pos & TDFNT_CENTER_Y)
		y -= (int) GetFontHeight(font) / 2; // (int) to snap to not cause unnecessary blurriness

	int i = 0;
	while (i < len && str[i])
	{
#ifdef TDFONT_USE_UTF8
		UCS4 uc = utf8::decode_next(str, &i, len);
		char c = UCS4_to_tdfnt(uc);
		TDFNT_GLYPH *glyph = font->GetGlyph(c);
#else
		char c = str[i++];
		TDFNT_GLYPH *glyph = font->GetGlyph(c);
		if (symbol_font && c == '\x1B' && str[i] != 0 && str[i + 1] != 0 && str[i + 2] != 0)
		{
			// Get glyph from symbol font
			int glyph_index = (tdfnt_hexnibble(str[i + 1]) << 4) + tdfnt_hexnibble(str[i + 2]);
			glyph = symbol_font->GetGlyph(glyph_index + 33);
			float ydiff = (font->fonth - symbol_font->fonth) / 2;
			g_backend->DrawGlyph(x, y + ydiff, glyph, symbol_font->texture_id);
			i += 3;
		}
		else
#endif
		if (glyph->w > 0 && c != ' ')
			g_backend->DrawGlyph(x, y, glyph, font->texture_id);

		x += (glyph->advance + extra_char_space);
		if (c == ' ') // Snap to closest pixel on new word
			x = (float)(int)x;
	}

	EndDrawString(font);
}

#ifndef TDFONT_USE_UTF8
void TdFontRenderer::SetSymbolFont(TdFont *font)
{
	symbol_font = font;
}
#endif

#ifdef _DEBUG

#ifndef TDFONT_USE_UTF8
void TdFontRenderer::DebugDrawPositioning(TdFont *font, float x, float y)
{
	const char *test[] = {
	"Grumpy wizards make toxic brew for the evil Queen and Jack.",
	"One morning, when Gregor Samsa woke from troubled dreams,",
	"he found himself transformed in his bed into a horrible vermin.",
	"He lay on his armour-like back, and if he lifted his head a",
	"little he could see his brown belly, slightly domed and divided",
	"by arches into stiff sections. The bedding was hardly able to",
	"cover it and seemed ready to slide off any moment.",
	"His many legs, pitifully thin compared with the size of the",
	"rest of him, waved about helplessly as he looked.",
	"",
	"GRUMPY WIZARDS MAKE TOXIC BREW FOR THE EVIL QUEEN AND JACK…",
	"",
	" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	"[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™",
	"š›œŸ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×",
	"ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ",
	"",
	"-------------—————————————————-------------",
	"Tests:",
	" • French (Français): Je peux manger du verre, ça ne me fait pas de mal.",
	" • Spanish (Español) ¡Hola! Puedo comer vidrio, no me hace daño.",
	" • »ÅÄÖ« <åäö> #%&@©®™ /|\\ [] {} () 5²×7³ ",
	" • Symbols 0(\x1B,00), 1(\x1B,01), 2(\x1B,02), 3(\x1B,03), 4(\x1B,04)",
	0 };
	int i = 0;
	while (test[i])
	{
		DrawString(font, x, y + i * GetFontHeight(font), 0, test[i]);
		i++;
	}
}
#endif // !TDFONT_USE_UTF8

void TdFontRenderer::DebugDrawFontTexture(TdFont *font, float x, float y)
{
	// Debug code to watch the whole texture with all glyphs
	TDFNT_GLYPH g;
	g.iw = font->GetGlyph(' ')->iw;
	g.ih = font->GetGlyph(' ')->ih;
	g.u = 0;
	g.v = 0;
	g.w = g.iw;
	g.h = g.ih;
	BeginDrawString(font);
	g_backend->DrawGlyph(x, y, &g, font->texture_id);
	EndDrawString(font);
}

#endif // _DEBUG
