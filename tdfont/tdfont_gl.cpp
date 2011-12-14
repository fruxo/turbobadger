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

float extra_char_space = 0;
int begin_draw_string_counter = 0;
TdFont *symbol_font = 0;

TdFontRendererBackend *g_backend = nullptr;

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
	while(len > 0 && *str)
	{
		TDFNT_GLYPH *glyph = font->GetGlyph(*str);
		if (symbol_font && *str == '\x1B' && str[1] != 0 && str[2] != 0 && str[3] != 0)
		{
			// Get glyph from symbol font
			int glyph_index = (tdfnt_hexnibble(str[2]) << 4) + tdfnt_hexnibble(str[3]);
			glyph = symbol_font->GetGlyph(glyph_index + 33);
			str += 3;
		}
		w += (glyph->advance + extra_char_space);
		if (*str == ' ') // Snap to closest pixel on new word
			w = (int)w;
		str++;
		len--;
	}
	return w;
}

float TdFontRenderer::GetFontHeight(TdFont *font)
{
	return font->fonth;
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

	while(len > 0 && *str)
	{
		TDFNT_GLYPH *glyph = font->GetGlyph(*str);

		if (symbol_font && *str == '\x1B' && str[1] != 0 && str[2] != 0 && str[3] != 0)
		{
			// Get glyph from symbol font
			int glyph_index = (tdfnt_hexnibble(str[2]) << 4) + tdfnt_hexnibble(str[3]);
			glyph = symbol_font->GetGlyph(glyph_index + 33);
			float ydiff = (font->fonth - symbol_font->fonth) / 2;
			g_backend->DrawGlyph(x, y + ydiff, glyph, symbol_font->texture_id);
			str += 3;
		}
		else if (glyph->w > 0 && *str != ' ')
			g_backend->DrawGlyph(x, y, glyph, font->texture_id);

		x += (glyph->advance + extra_char_space);
		if (*str == ' ') // Snap to closest pixel on new word
			x = (int)x;
		str++;
		len--;
	}

	EndDrawString(font);
}

void TdFontRenderer::SetSymbolFont(TdFont *font)
{
	symbol_font = font;
}

#ifdef _DEBUG

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
