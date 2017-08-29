/*
=======================================================================================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Spearmint Source Code.
If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms. You should have received a copy of these additional
terms immediately following the terms and conditions of the GNU General Public License. If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
=======================================================================================================================================
*/

/**************************************************************************************************************************************
 User interface building blocks and support functions.
**************************************************************************************************************************************/

#include "ui_local.h"
#include "../qcommon/q_unicode.h"

#define MAX_WRAP_BYTES 1024
#define MAX_WRAP_LINES 1024

#define PROPB_GAP_WIDTH 4
#define PROPB_SPACE_WIDTH 12
#define PROPB_HEIGHT 48

uiStatic_t uis;
qboolean m_entersound; // after a frame, so caching won't disrupt the sound

static vec4_t lastTextColor = {0, 0, 0, 1};

/*
=======================================================================================================================================
Com_Error
=======================================================================================================================================
*/
void QDECL Com_Error(int level, const char *error, ...) {
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	Q_vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	trap_Error(text);
}

/*
=======================================================================================================================================
Com_Printf
=======================================================================================================================================
*/
void QDECL Com_Printf(const char *msg, ...) {
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	trap_Print(text);
}

/*
=======================================================================================================================================
UI_StartMenuMusic
=======================================================================================================================================
*/
void UI_StartMenuMusic(void) {

	trap_S_StartBackgroundTrack("music/ui_music01.ogg", "music/ui_music02.ogg");

	uis.playmusic = qtrue;
}

/*
=======================================================================================================================================
UI_StopMenuMusic
=======================================================================================================================================
*/
void UI_StopMenuMusic(void) {

	if (uis.playmusic == qtrue) {
		trap_S_StopBackgroundTrack();
	}

	uis.playmusic = qfalse;
}

/*
=======================================================================================================================================
UI_PushMenu
=======================================================================================================================================
*/
void UI_PushMenu(menuframework_s *menu) {
	int i;
	menucommon_s *item;

	// avoid stacking menus invoked by hotkeys
	for (i = 0; i < uis.menusp; i++) {
		if (uis.stack[i] == menu) {
			uis.menusp = i;
			break;
		}
	}

	if (i == uis.menusp) {
		if (uis.menusp >= MAX_MENUDEPTH) {
			trap_Error("UI_PushMenu: menu stack overflow");
		}

		uis.stack[uis.menusp++] = menu;
	}

	uis.activemenu = menu;
	// default cursor position
	menu->cursor = 0;
	menu->cursor_prev = 0;

	m_entersound = qtrue;

	trap_Key_SetCatcher(KEYCATCH_UI);
	// force first available item to have focus
	for (i = 0; i < menu->nitems; i++) {
		item = (menucommon_s *)menu->items[i];

		if (!(item->flags &(QMF_GRAYED|QMF_MOUSEONLY|QMF_INACTIVE))) {
			menu->cursor_prev = -1;
			Menu_SetCursor(menu, i);
			break;
		}
	}

	uis.firstdraw = qtrue;
}

/*
=======================================================================================================================================
UI_PopMenu
=======================================================================================================================================
*/
void UI_PopMenu(void) {

	trap_S_StartLocalSound(menu_out_sound, CHAN_LOCAL_SOUND);

	uis.menusp--;

	if (uis.menusp < 0) {
		trap_Error("UI_PopMenu: menu stack underflow");
	}

	if (uis.menusp) {
		uis.activemenu = uis.stack[uis.menusp - 1];
		uis.firstdraw = qtrue;
	} else {
		UI_ForceMenuOff();
	}
}

/*
=======================================================================================================================================
UI_ForceMenuOff
=======================================================================================================================================
*/
void UI_ForceMenuOff(void) {

	uis.menusp = 0;
	uis.activemenu = NULL;

	if (!trap_Cvar_VariableValue("cl_paused")) {
		UI_StopMenuMusic();
	}

	trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
	trap_Cvar_SetValue("cl_paused", 0);
}

/*
=======================================================================================================================================
UI_LerpColor
=======================================================================================================================================
*/
void UI_LerpColor(const vec4_t a, const vec4_t b, vec4_t c, float t) {
	int i;

	// lerp and clamp each component
	for (i = 0; i < 4; i++) {
		c[i] = a[i] + t * (b[i] - a[i]);

		if (c[i] < 0) {
			c[i] = 0;
		} else if (c[i] > 1.0) {
			c[i] = 1.0;
		}
	}
}

/*
=======================================================================================================================================

	TEXT DRAWING

=======================================================================================================================================
*/

/*
=======================================================================================================================================
UI_Text_GetGlyph
=======================================================================================================================================
*/
const glyphInfo_t *UI_Text_GetGlyph(const fontInfo_t *font, unsigned long index) {

	if (index == 0 || index >= GLYPHS_PER_FONT) {
		return &font->glyphs[(int)'.'];
	}

	return &font->glyphs[index];
}

/*
=======================================================================================================================================
UI_Text_Width
=======================================================================================================================================
*/
float UI_Text_Width(const char *text, const fontInfo_t *font, float scale, int limit) {
	int count, len;
	float out;
	const glyphInfo_t *glyph;
	float useScale;
	const char *s;

	if (!text) {
		return 0;
	}

	useScale = scale * font->glyphScale;
	out = 0;

	len = Q_UTF8_PrintStrlen(text);

	if (limit > 0 && len > limit) {
		len = limit;
	}

	s = text;
	count = 0;

	while (s && *s && count < len) {
		if (Q_IsColorString(s)) {
			s += 2;
			continue;
		}

		glyph = UI_Text_GetGlyph(font, Q_UTF8_CodePoint(&s));
		out += glyph->xSkip;
		count++;
	}

	return out * useScale;
}

/*
=======================================================================================================================================
UI_Text_PaintGlyph
=======================================================================================================================================
*/
void UI_Text_PaintGlyph(float x, float y, float w, float h, const glyphInfo_t *glyph, float *gradientColor) {

	if (gradientColor) {
		trap_R_DrawStretchPicGradient(x, y, w, h, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph, gradientColor);
	} else {
		trap_R_DrawStretchPic(x, y, w, h, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
	}
}

/*
=======================================================================================================================================
UI_Text_Paint
=======================================================================================================================================
*/
void UI_Text_Paint(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor) {
	int len, count;
	vec4_t newColor;
	vec4_t gradientColor;
	const glyphInfo_t *glyph;
	const char *s;
	float yadj, xadj;
	float useScaleX, useScaleY;
	float xscale, yscale;
	float shadowOffsetX, shadowOffsetY;

	if (!text) {
		return;
	}

	xscale = 1.0f;
	yscale = 1.0f;

	UI_AdjustFrom640(&x, &y, &xscale, &yscale);

	shadowOffsetX = shadowOffset * xscale;
	shadowOffsetY = shadowOffset * yscale;

	adjust *= xscale;

	useScaleX = scale * font->glyphScale * xscale;
	useScaleY = scale * font->glyphScale * yscale;
	// prevent native resolution text from being blurred due to sub-pixel blending
	x = floor(x);
	y = floor(y);

	shadowOffsetX = floor(shadowOffsetX);
	shadowOffsetY = floor(shadowOffsetY);

	trap_R_SetColor(color);
	Vector4Copy(color, newColor);
	Vector4Copy(color, lastTextColor);

	gradientColor[0] = Com_Clamp(0, 1, newColor[0] - gradient);
	gradientColor[1] = Com_Clamp(0, 1, newColor[1] - gradient);
	gradientColor[2] = Com_Clamp(0, 1, newColor[2] - gradient);
	gradientColor[3] = color[3];

	len = Q_UTF8_PrintStrlen(text);

	if (limit > 0 && len > limit) {
		len = limit;
	}

	s = text;
	count = 0;

	while (s && *s && count < len) {
		if (Q_IsColorString(s)) {
			if (!forceColor) {
				VectorCopy(g_color_table[ColorIndex(*(s + 1))], newColor);

				newColor[3] = color[3];

				trap_R_SetColor(newColor);
				Vector4Copy(newColor, lastTextColor);

				gradientColor[0] = Com_Clamp(0, 1, newColor[0] - gradient);
				gradientColor[1] = Com_Clamp(0, 1, newColor[1] - gradient);
				gradientColor[2] = Com_Clamp(0, 1, newColor[2] - gradient);
				gradientColor[3] = color[3];
			}

			s += 2;
			continue;
		}

		glyph = UI_Text_GetGlyph(font, Q_UTF8_CodePoint(&s));

		yadj = useScaleY * glyph->top;
		xadj = useScaleX * glyph->left;

		if (shadowOffsetX || shadowOffsetY) {
			colorBlack[3] = newColor[3];

			trap_R_SetColor(colorBlack);
			UI_Text_PaintGlyph(x + xadj + shadowOffsetX, y - yadj + shadowOffsetY, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);
			trap_R_SetColor(newColor);

			colorBlack[3] = 1.0f;
		}

		UI_Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, (gradient != 0) ? gradientColor : NULL);

		x += (glyph->xSkip * useScaleX) + adjust;
		count++;
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
UI_Text_PaintWithCursor
=======================================================================================================================================
*/
void UI_Text_PaintWithCursor(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor) {
	int len, count;
	vec4_t newColor;
	vec4_t gradientColor;
	const glyphInfo_t *glyph, *glyph2;
	float yadj, xadj;
	float useScaleX, useScaleY;
	float xscale, yscale;
	float shadowOffsetX, shadowOffsetY;
	const char *s;

	if (!text) {
		return;
	}

	xscale = 1.0f;
	yscale = 1.0f;

	UI_AdjustFrom640(&x, &y, &xscale, &yscale);

	shadowOffsetX = shadowOffset * xscale;
	shadowOffsetY = shadowOffset * yscale;

	adjust *= xscale;

	useScaleX = scale * font->glyphScale * xscale;
	useScaleY = scale * font->glyphScale * yscale;
	// prevent native resolution text from being blurred due to sub-pixel blending
	x = floor(x);
	y = floor(y);

	shadowOffsetX = floor(shadowOffsetX);
	shadowOffsetY = floor(shadowOffsetY);

	trap_R_SetColor(color);
	Vector4Copy(color, newColor);
	Vector4Copy(color, lastTextColor);

	gradientColor[0] = Com_Clamp(0, 1, newColor[0] - gradient);
	gradientColor[1] = Com_Clamp(0, 1, newColor[1] - gradient);
	gradientColor[2] = Com_Clamp(0, 1, newColor[2] - gradient);
	gradientColor[3] = color[3];
	// NOTE: doesn't use Q_UTF8_PrintStrlen because this function draws color codes
	len = Q_UTF8_Strlen(text);

	if (limit > 0 && len > limit) {
		len = limit;
	}

	s = text;
	count = 0;
	glyph2 = UI_Text_GetGlyph(font, cursor);

	while (s && *s && count < len) {
		if (Q_IsColorString(s)) {
			if (!forceColor) {
				VectorCopy(g_color_table[ColorIndex(*(s + 1))], newColor);

				newColor[3] = color[3];

				trap_R_SetColor(newColor);
				Vector4Copy(newColor, lastTextColor);

				gradientColor[0] = Com_Clamp(0, 1, newColor[0] - gradient);
				gradientColor[1] = Com_Clamp(0, 1, newColor[1] - gradient);
				gradientColor[2] = Com_Clamp(0, 1, newColor[2] - gradient);
				gradientColor[3] = color[3];
			}
			// display color codes in edit fields instead of skipping them
		}

		glyph = UI_Text_GetGlyph(font, Q_UTF8_CodePoint(&s));

		if (count == cursorPos && ((uis.realtime / BLINK_DIVISOR) & 1) == 0) {
			yadj = useScaleY * glyph2->top;
			// use horizontal width of text character (glyph)
			UI_Text_PaintGlyph(x, y - yadj, (glyph->left + glyph->xSkip) * useScaleX, glyph2->imageHeight * useScaleY, glyph2, NULL);
		}

		yadj = useScaleY * glyph->top;
		xadj = useScaleX * glyph->left;

		if (shadowOffsetX || shadowOffsetY) {
			colorBlack[3] = newColor[3];

			trap_R_SetColor(colorBlack);
			UI_Text_PaintGlyph(x + xadj + shadowOffsetX, y - yadj + shadowOffsetY, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);
			trap_R_SetColor(newColor);

			colorBlack[3] = 1.0f;
		}
		// make overstrike cursor invert color
		if (count == cursorPos && !((uis.realtime / BLINK_DIVISOR) & 1)/* && cursor == GLYPH_OVERSTRIKE*/) { // Tobias: FIXME
			// invert color
			vec4_t invertedColor;

			invertedColor[0] = 1.0f - newColor[0];
			invertedColor[1] = 1.0f - newColor[1];
			invertedColor[2] = 1.0f - newColor[2];
			invertedColor[3] = color[3];

			trap_R_SetColor(invertedColor);

			UI_Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);
		} else {
			UI_Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, (gradient != 0) ? gradientColor : NULL);
		}

		if (count == cursorPos && !((uis.realtime / BLINK_DIVISOR) & 1)/* && cursor == GLYPH_OVERSTRIKE*/) { // Tobias: FIXME
			// restore color
			trap_R_SetColor(newColor);
		}

		x += (glyph->xSkip * useScaleX) + adjust;
		count++;
	}
	// need to paint cursor at end of text
	if (cursorPos == len && !((uis.realtime / BLINK_DIVISOR) & 1)) {
		yadj = useScaleY * glyph2->top;

		UI_Text_PaintGlyph(x, y - yadj, glyph2->imageWidth * useScaleX, glyph2->imageHeight * useScaleY, glyph2, NULL);
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
UI_Text_Paint_AutoWrapped
=======================================================================================================================================
*/
void UI_Text_Paint_AutoWrapped(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *str, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor, float xmax, float ystep, int style) {
	int width;
	char *s1, *s2, *s3;
	char c_bcp;
	char buf[MAX_WRAP_BYTES];
	char wrapped[MAX_WRAP_BYTES + MAX_WRAP_LINES];
	qboolean autoNewline[MAX_WRAP_LINES];
	int numLines;
	vec4_t newColor;
	const char *p, *start;
	float drawX;

	if (!str || str[0] == '\0') {
		return;
	}
	// wrap the text
	Q_strncpyz(buf, str, sizeof(buf));

	s1 = s2 = s3 = buf;

	wrapped[0] = 0;
	numLines = 0;

	while (1) {
		do {
			s3 += Q_UTF8_Width(s3);
		} while (*s3 != '\n' && *s3 != ' ' && *s3 != '\0');

		c_bcp = *s3;
		*s3 = '\0';
		width = UI_Text_Width(s1, font, scale, 0);
		*s3 = c_bcp;

		if (width > xmax) {
			if (s1 == s2) {
				// fuck, don't have a clean cut, we'll overflow
				s2 = s3;
			}

			*s2 = '\0';

			Q_strcat(wrapped, sizeof(wrapped), s1);
			Q_strcat(wrapped, sizeof(wrapped), "\n");

			if (numLines < MAX_WRAP_LINES) {
				autoNewline[numLines] = qtrue;
				numLines++;
			}

			if (c_bcp == '\0') {
				// that was the last word
				// we could start a new loop, but that wouldn't be much use
				// even if the word is too long, we would overflow it (see above) so just print it now if needed
				s2 += Q_UTF8_Width(s2);

				if (*s2 != '\0' && *s2 != '\n') { // if we are printing an overflowing line we have s2 == s3
					Q_strcat(wrapped, sizeof(wrapped), s2);
					Q_strcat(wrapped, sizeof(wrapped), "\n");

					if (numLines < MAX_WRAP_LINES) {
						autoNewline[numLines] = qtrue;
						numLines++;
					}
				}

				break;
			}

			s2 += Q_UTF8_Width(s2);
			s1 = s2;
			s3 = s2;
		} else if (c_bcp == '\n') {
			*s3 = '\0';

			Q_strcat(wrapped, sizeof(wrapped), s1);
			Q_strcat(wrapped, sizeof(wrapped), "\n");

			if (numLines < MAX_WRAP_LINES) {
				autoNewline[numLines] = qfalse;
				numLines++;
			}

			s3 += Q_UTF8_Width(s3);
			s1 = s3;
			s2 = s3;

			if (*s3 == '\0') { // we reached the end
				break;
			}
		} else {
			s2 = s3;

			if (c_bcp == '\0') { // we reached the end
				Q_strcat(wrapped, sizeof(wrapped), s1);
				Q_strcat(wrapped, sizeof(wrapped), "\n");

				if (numLines < MAX_WRAP_LINES) {
					autoNewline[numLines] = qfalse;
					numLines++;
				}

				break;
			}
		}
	}
	// draw the text
	switch (style & UI_VA_FORMATMASK) {
		case UI_VA_CENTER:
			// center justify at y
			y = y - numLines * ystep / 2.0f;
			break;
		case UI_VA_BOTTOM:
			// bottom justify at y
			y = y - numLines * ystep;
			break;
		case UI_VA_TOP:
		default:
			// top justify at y
			break;
	}

	numLines = 0;

	Vector4Copy(color, newColor);

	start = wrapped;
	p = strchr(wrapped, '\n');

	while (p && *p) {
		strncpy(buf, start, p - start + 1);
		buf[p - start] = '\0';

		switch (style & UI_FORMATMASK) {
			case UI_CENTER:
				// center justify at x
				drawX = x - UI_Text_Width(buf, font, scale, 0) / 2;
				break;
			case UI_RIGHT:
				// right justify at x
				drawX = x - UI_Text_Width(buf, font, scale, 0);
				break;
			case UI_LEFT:
			default:
				// left justify at x
				drawX = x;
				break;
		}

		UI_Text_Paint(drawX, y, font, scale, newColor, buf, adjust, 0, shadowOffset, gradient, forceColor);

		y += ystep;

		if (numLines >= MAX_WRAP_LINES || autoNewline[numLines]) {
			Vector4Copy(lastTextColor, newColor);
		} else {
			// reset color after non-wrapped lines
			Vector4Copy(color, newColor);
		}

		numLines++;
		start += p - start + 1;
		p = strchr(p + 1, '\n');
	}
}

/*
=======================================================================================================================================

	STRING FUNCTIONS

=======================================================================================================================================
*/

/*
=======================================================================================================================================

	STRING FUNCTIONS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
UI_DrawBannerString
=======================================================================================================================================
*/
void UI_DrawBannerString(int x, int y, const char *str, int style, vec4_t color) {
	float decent;
	int width;

	// find the width of the drawn text
	width = UI_Text_Width(str, &uis.titanFont, PROPB_HEIGHT / 48.0f, 0);

	switch (style & UI_FORMATMASK) {
		case UI_CENTER:
			x -= width / 2;
			break;
		case UI_RIGHT:
			x -= width;
			break;
		case UI_LEFT:
		default:
			break;
	}
	// this function expects that y is top of line, text_paint expects at baseline
	decent = -uis.titanFont.glyphs[(int)'g'].top + uis.titanFont.glyphs[(int)'g'].height;
	y = y + PROPB_HEIGHT - decent * PROPB_HEIGHT / 48.0f * uis.titanFont.glyphScale;

	UI_Text_Paint(x, y, &uis.titanFont, PROPB_HEIGHT / 48.0f, color, str, 0, 0, (style & UI_DROPSHADOW) ? 2 : 0, 0, qfalse);
}

/*
=======================================================================================================================================
UI_ProportionalStringWidth
=======================================================================================================================================
*/
int UI_ProportionalStringWidth(const char *str) {
	return UI_Text_Width(str, &uis.titanFont, PROP_HEIGHT / 48.0f, 0);
}

/*
=======================================================================================================================================
UI_ProportionalSizeScale
=======================================================================================================================================
*/
float UI_ProportionalSizeScale(int style) {

	if ((style & UI_FONTMASK) == UI_SMALLFONT) {
		return PROP_SMALL_SIZE_SCALE;
	}

	return 1.00;
}

/*
=======================================================================================================================================
UI_DrawProportionalString
=======================================================================================================================================
*/
void UI_DrawProportionalString(int x, int y, const char *str, int style, vec4_t color) {
	float decent;
	int glowY;
	vec4_t drawcolor;
	int width;
	int charh;
	float propScale;
	float scale;

	propScale = UI_ProportionalSizeScale(style);
	charh = propScale * PROP_HEIGHT;
	scale = propScale * PROP_HEIGHT / 48.0f;

	switch (style & UI_FORMATMASK) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth(str) * propScale;
			x -= width / 2;
			break;
		case UI_RIGHT:
			width = UI_ProportionalStringWidth(str) * propScale;
			x -= width;
			break;
		case UI_LEFT:
		default:
			break;
	}
	// this function expects that y is top of line, text_paint expects at baseline

	// glow font
	decent = -uis.titanFont.glyphs[(int)'g'].top + uis.titanFont.glyphs[(int)'g'].height;
	glowY = y + charh - decent * scale * uis.titanFont.glyphScale;

	if (decent != 0) {
		// make TrueType fonts line up with font1_prop bitmap font which has 4 transparent pixels above glyphs at 16 point font size
		glowY += 3.0f * propScale;
	}
	// normal font
	decent = -uis.titanFont.glyphs[(int)'g'].top + uis.titanFont.glyphs[(int)'g'].height;
	y = y + charh - decent * scale * uis.titanFont.glyphScale;

	if (decent != 0) {
		// make TrueType fonts line up with font1_prop bitmap font which has 4 transparent pixels above glyphs at 16 point font size
		y += 3.0f * propScale;
	}

	if (style & UI_INVERSE) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
		UI_Text_Paint(x, y, &uis.titanFont, scale, drawcolor, str, 0, 0, (style & UI_DROPSHADOW) ? 2 : 0, 0, qfalse);
		return;
	}

	if (style & UI_PULSE) {
		UI_Text_Paint(x, y, &uis.titanFont, scale, color, str, 0, 0, (style & UI_DROPSHADOW) ? 2 : 0, 0, qfalse);
		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin(uis.realtime / PULSE_DIVISOR);
		UI_Text_Paint(x, glowY, &uis.titanFont, scale, drawcolor, str, 0, 0, 0, 0, qfalse);
		return;
	}

	UI_Text_Paint(x, y, &uis.titanFont, scale, color, str, 0, 0, (style & UI_DROPSHADOW) ? 2 : 0, 0, qfalse);
}

/*
=======================================================================================================================================
UI_DrawProportionalString_AutoWrapped
=======================================================================================================================================
*/
void UI_DrawProportionalString_AutoWrapped(int x, int y, int xmax, int ystep, const char *str, int style, vec4_t color) {
	int width;
	char *s1, *s2, *s3;
	char c_bcp;
	char buf[1024];
	float sizeScale;

	if (!str || str[0] == '\0') {
		return;
	}

	sizeScale = UI_ProportionalSizeScale(style);

	Q_strncpyz(buf, str, sizeof(buf));

	s1 = s2 = s3 = buf;

	while (1) {
		do {
			s3++;
		} while (*s3 != ' ' && *s3 != '\0');

		c_bcp = *s3;
		*s3 = '\0';
		width = UI_ProportionalStringWidth(s1) * sizeScale;
		*s3 = c_bcp;

		if (width > xmax) {
			if (s1 == s2) {
				// fuck, don't have a clean cut, we'll overflow
				s2 = s3;
			}

			*s2 = '\0';

			UI_DrawProportionalString(x, y, s1, style, color);

			y += ystep;

			if (c_bcp == '\0') {
				// that was the last word
				// we could start a new loop, but that wouldn't be much use even if the word is too long, we would overflow it (see above)
				// so just print it now if needed
				s2++;

				if (*s2 != '\0') { // if we are printing an overflowing line we have s2 == s3
					UI_DrawProportionalString(x, y, s2, style, color);
				}

				break;
			}

			s2++;
			s1 = s2;
			s3 = s2;
		} else {
			s2 = s3;

			if (c_bcp == '\0') { // we reached the end
				UI_DrawProportionalString(x, y, s1, style, color);
				break;
			}
		}
	}
}

/*
=======================================================================================================================================
UI_DrawStringLineHeight

Returns draw height of text line for drawing multiple lines of text.
=======================================================================================================================================
*/
int UI_DrawStringLineHeight(int style) {
	const fontInfo_t *font;
	int lineHeight;
	int charh;
	int gap;

	gap = 0;

	switch (style & UI_FONTMASK) {
		case UI_TINYFONT:
			font = &uis.tinyFont;
			gap = 1;
			break;
		case UI_SMALLFONT:
			font = &uis.smallFont;
			gap = 2;
			break;
		case UI_DEFAULTFONT:
			font = &uis.defaultFont;
			gap = 2;
			break;
		case UI_BIGFONT:
		default:
			font = &uis.bigFont;
			gap = 2;
			break;
		case UI_GIANTFONT:
			font = &uis.giantFont;
			gap = 2;
			break;
		case UI_TITANFONT:
			font = &uis.titanFont;
			gap = 6;
			break;
	}

	charh = font->pointSize;
	lineHeight = charh + gap;

	return lineHeight;
}

/*
=======================================================================================================================================
UI_DrawStringDirect
=======================================================================================================================================
*/
void UI_DrawStringDirect(int x, int y, const char *str, int style, const vec4_t color, float scale, int maxChars, float shadowOffset, float gradient, int cursorPos, int cursorChar, float wrapX) {
	int charh;
	vec4_t newcolor;
	vec4_t lowlight;
	const float *drawcolor;
	const fontInfo_t *font;
	int decent;

	if (!str) {
		return;
	}

	if (!color) {
		color = colorWhite;
	}

	if ((style & UI_BLINK) && ((uis.realtime / BLINK_DIVISOR) & 1)) {
		return;
	}

	switch (style & UI_FONTMASK) {
		case UI_TINYFONT:
			font = &uis.tinyFont;
			break;
		case UI_SMALLFONT:
			font = &uis.smallFont;
			break;
		case UI_DEFAULTFONT:
			font = &uis.defaultFont;
			break;
		case UI_BIGFONT:
		default:
			font = &uis.bigFont;
			break;
		case UI_GIANTFONT:
			font = &uis.giantFont;
			break;
		case UI_TITANFONT:
			font = &uis.titanFont;
			break;
	}

	charh = font->pointSize;

	if (shadowOffset == 0 && (style & UI_DROPSHADOW)) {
		shadowOffset = 2;
	}

	if (gradient == 0 && (style & UI_GRADIENT)) {
		gradient = 0.4f;
	}

	if (scale <= 0) {
		scale = charh / 48.0f;
	} else {
		charh = 48 * scale;
	}

	if (style & UI_PULSE) {
		lowlight[0] = 0.8 * color[0];
		lowlight[1] = 0.8 * color[1];
		lowlight[2] = 0.8 * color[2];
		lowlight[3] = 0.8 * color[3];

		UI_LerpColor(color, lowlight, newcolor, 0.5 + 0.5 * sin(uis.realtime / PULSE_DIVISOR));
		drawcolor = newcolor;
	} else {
		drawcolor = color;
	}

	if (wrapX <= 0) {
		switch (style & UI_FORMATMASK) {
			case UI_CENTER:
				// center justify at x
				x = x - UI_Text_Width(str, font, scale, 0) / 2;
				break;
			case UI_RIGHT:
				// right justify at x
				x = x - UI_Text_Width(str, font, scale, 0);
				break;
			case UI_LEFT:
			default:
				// left justify at x
				break;
		}

		switch (style & UI_VA_FORMATMASK) {
			case UI_VA_CENTER:
				// center justify at y
				y = y - charh / 2;
				break;
			case UI_VA_BOTTOM:
				// bottom justify at y
				y = y - charh;
				break;
			case UI_VA_TOP:
			default:
				// top justify at y
				break;
		}
	}
	// this function expects that y is top of line, text_paint expects at baseline
	decent = -font->glyphs[(int)'g'].top + font->glyphs[(int)'g'].height;
	y = y + charh - decent * scale * font->glyphScale;

	if (decent != 0) {
		// make TrueType fonts line up with bigchars bitmap font which has 2 transparent pixels above glyphs at 16 point font size
		y += 2.0f * charh / 16.0f;
	}

	if (cursorChar >= 0) {
		UI_Text_PaintWithCursor(x, y, font, scale, drawcolor, str, cursorPos, cursorChar, 0, maxChars, shadowOffset, gradient, !!(style & UI_FORCECOLOR));
	} else if (wrapX > 0) {
		// replace 'char height' in line height with our scaled charh
		// ZTM: TODO: This text gap handling is kind of messy. Passing scale to UI_DrawStringLineHeight might make cleaner code here.
		int gap = UI_DrawStringLineHeight(style) - font->pointSize;

		UI_Text_Paint_AutoWrapped(x, y, font, scale, drawcolor, str, 0, maxChars, shadowOffset, gradient, !!(style & UI_FORCECOLOR), wrapX, charh + gap, style);
	} else {
		UI_Text_Paint(x, y, font, scale, drawcolor, str, 0, maxChars, shadowOffset, gradient, !!(style & UI_FORCECOLOR));
	}
}

/*
=======================================================================================================================================
UI_DrawStringExtWithCursor
=======================================================================================================================================
*/
void UI_DrawStringExtWithCursor(int x, int y, const char *str, int style, const vec4_t color, float scale, int maxChars, float shadowOffset, float gradient, int cursorPos, int cursorChar) {
	UI_DrawStringDirect(x, y, str, style, color, scale, maxChars, shadowOffset, gradient, cursorPos, cursorChar, 0);
}

/*
=======================================================================================================================================
UI_DrawString
=======================================================================================================================================
*/
void UI_DrawString(int x, int y, const char *str, int style, vec4_t color) {
	UI_DrawStringExtWithCursor(x, y, str, style, color, 0, 0, 0, 0, -1, -1);
}

/*
=======================================================================================================================================
UI_DrawChar
=======================================================================================================================================
*/
void UI_DrawChar(int x, int y, int ch, int style, vec4_t color) {
	char buff[2];

	buff[0] = ch;
	buff[1] = '\0';

	UI_DrawString(x, y, buff, style, color);
}

/*
=======================================================================================================================================

	COMMON UI FUNCTIONS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
UI_IsFullscreen
=======================================================================================================================================
*/
qboolean UI_IsFullscreen(void) {

	if (uis.activemenu && (trap_Key_GetCatcher() & KEYCATCH_UI)) {
		return uis.activemenu->fullscreen;
	}

	return qfalse;
}

/*
=======================================================================================================================================
UI_SetActiveMenu
=======================================================================================================================================
*/
void UI_SetActiveMenu(uiMenuCommand_t menu) {

	switch (menu) {
		case UIMENU_NONE:
			UI_ForceMenuOff();
			return;
		case UIMENU_MAIN:
			UI_MainMenu();
			return;
		case UIMENU_INGAME:
			trap_Cvar_SetValue("cl_paused", 1);
			UI_InGameMenu();
			return;
		case UIMENU_TEAM:
		case UIMENU_POSTGAME:
		default:
#ifndef NDEBUG
			Com_Printf("UI_SetActiveMenu: bad enum %d\n", menu);
#endif
			break;
	}
}

/*
=======================================================================================================================================
UI_KeyEvent
=======================================================================================================================================
*/
void UI_KeyEvent(int key, int down) {
	sfxHandle_t s;

	if (!uis.activemenu) {
		return;
	}

	if (!down) {
		return;
	}

	if (uis.activemenu->key) {
		s = uis.activemenu->key(key);
	} else {
		s = Menu_DefaultKey(uis.activemenu, key);
	}

	if ((s > 0) && (s != menu_null_sound)) {
		trap_S_StartLocalSound(s, CHAN_LOCAL_SOUND);
	}
}

/*
=======================================================================================================================================
UI_MouseEvent
=======================================================================================================================================
*/
void UI_MouseEvent(int dx, int dy) {
	int i, bias;
	menucommon_s *m;

	if (!uis.activemenu) {
		return;
	}
	// convert X bias to 640 coords
	bias = uis.bias / uis.xscale;
	// update mouse screen position
	uis.cursorx += dx;

	if (uis.cursorx < -bias) {
		uis.cursorx = -bias;
	} else if (uis.cursorx > SCREEN_WIDTH + bias) {
		uis.cursorx = SCREEN_WIDTH + bias;
	}

	uis.cursory += dy;

	if (uis.cursory < 0) {
		uis.cursory = 0;
	} else if (uis.cursory > SCREEN_HEIGHT) {
		uis.cursory = SCREEN_HEIGHT;
	}
	// region test the active menu items
	for (i = 0; i < uis.activemenu->nitems; i++) {
		m = (menucommon_s *)uis.activemenu->items[i];

		if (m->flags &(QMF_GRAYED|QMF_INACTIVE)) {
			continue;
		}

		if ((uis.cursorx < m->left) || (uis.cursorx > m->right) || (uis.cursory < m->top) || (uis.cursory > m->bottom)) {
			// cursor out of item bounds
			continue;
		}
		// set focus to item at cursor
		if (uis.activemenu->cursor != i) {
			Menu_SetCursor(uis.activemenu, i);
			((menucommon_s *)(uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

			if (!(((menucommon_s *)(uis.activemenu->items[uis.activemenu->cursor]))->flags & QMF_SILENT)) {
				trap_S_StartLocalSound(menu_move_sound, CHAN_LOCAL_SOUND);
			}
		}

		((menucommon_s *)(uis.activemenu->items[uis.activemenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
		return;
	}

	if (uis.activemenu->nitems > 0) {
		// out of any region
		((menucommon_s *)(uis.activemenu->items[uis.activemenu->cursor]))->flags &= ~QMF_HASMOUSEFOCUS;
	}
}

/*
=======================================================================================================================================
UI_Argv
=======================================================================================================================================
*/
char *UI_Argv(int arg) {
	static char buffer[MAX_STRING_CHARS];

	trap_Argv(arg, buffer, sizeof(buffer));
	return buffer;
}

/*
=======================================================================================================================================
UI_Cvar_VariableString
=======================================================================================================================================
*/
char *UI_Cvar_VariableString(const char *var_name) {
	static char buffer[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer(var_name, buffer, sizeof(buffer));
	return buffer;
}

/*
=======================================================================================================================================
UI_Cache_f
=======================================================================================================================================
*/
void UI_Cache_f(void) {

	MainMenu_Cache();
	InGame_Cache();
	ConfirmMenu_Cache();
	PlayerModel_Cache();
	PlayerSettings_Cache();
	Controls_Cache();
	Demos_Cache();
	UI_CinematicsMenu_Cache();
	Preferences_Cache();
	ServerInfo_Cache();
	SpecifyServer_Cache();
	ArenaServers_Cache();
	StartServer_Cache();
	CreateServer_Cache();
	ServerOptions_Cache();
	DriverInfo_Cache();
	GraphicsOptions_Cache();
	UI_DisplayOptionsMenu_Cache();
	UI_SoundOptionsMenu_Cache();
	UI_NetworkOptionsMenu_Cache();
	UI_SPLevelMenu_Cache();
	UI_SPSkillMenu_Cache();
	UI_SPPostgameMenu_Cache();
	TeamMain_Cache();
	UI_AddBots_Cache();
	UI_RemoveBots_Cache();
	UI_SetupMenu_Cache();
//	UI_LoadConfig_Cache();
//	UI_SaveConfigMenu_Cache();
	UI_BotSelectMenu_Cache();
	UI_ModsMenu_Cache();
}

/*
=======================================================================================================================================
UI_ConsoleCommand
=======================================================================================================================================
*/
qboolean UI_ConsoleCommand(int realTime) {
	const char *cmd;

	uis.frametime = realTime - uis.realtime;
	uis.realtime = realTime;

	cmd = UI_Argv(0);

	if (Q_stricmp(cmd, "levelselect") == 0) {
		UI_SPLevelMenu_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "postgame") == 0) {
		UI_SPPostgameMenu_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "ui_cache") == 0) {
		UI_Cache_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "ui_cinematics") == 0) {
		UI_CinematicsMenu_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "teamorders") == 0) {
		UI_BotCommandMenu_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "iamacheater") == 0) {
		UI_SPUnlock_f();
		return qtrue;
	}

	if (Q_stricmp(cmd, "iamamonkey") == 0) {
		UI_SPUnlockMedals_f();
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
UI_Shutdown
=======================================================================================================================================
*/
void UI_Shutdown(void) {
	UI_StopMenuMusic();
}

/*
=======================================================================================================================================
UI_Init
=======================================================================================================================================
*/
void UI_Init(void) {

	UI_RegisterCvars();
	UI_InitGameinfo();
	// cache redundant calulations
	trap_GetGlconfig(&uis.glconfig);
	// for 640x480 virtualized screen
	uis.xscale = uis.glconfig.vidWidth * (1.0 / 640.0);
	uis.yscale = uis.glconfig.vidHeight * (1.0 / 480.0);

	if (uis.glconfig.vidWidth * 480 > uis.glconfig.vidHeight * 640) {
		// wide screen
		uis.bias = 0.5 * (uis.glconfig.vidWidth - (uis.glconfig.vidHeight * (640.0 / 480.0)));
		uis.xscale = uis.yscale;
	} else {
		// no wide screen
		uis.bias = 0;
	}
	// initialize the menu system
	Menu_Cache();

	uis.activemenu = NULL;
	uis.menusp = 0;
}

/*
=======================================================================================================================================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio.
=======================================================================================================================================
*/
void UI_AdjustFrom640(float *x, float *y, float *w, float *h) {

	// expect valid pointers
	*x = *x * uis.xscale + uis.bias;
	*y *= uis.yscale;
	*w *= uis.xscale;
	*h *= uis.yscale;
}

/*
=======================================================================================================================================
UI_DrawNamedPic
=======================================================================================================================================
*/
void UI_DrawNamedPic(float x, float y, float width, float height, const char *picname) {
	qhandle_t hShader;

	hShader = trap_R_RegisterShaderNoMip(picname);

	UI_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

/*
=======================================================================================================================================
UI_DrawHandlePic
=======================================================================================================================================
*/
void UI_DrawHandlePic(float x, float y, float w, float h, qhandle_t hShader) {
	float s0;
	float s1;
	float t0;
	float t1;

	if (w < 0) { // flip about vertical
		w = -w;
		s0 = 1;
		s1 = 0;
	} else {
		s0 = 0;
		s1 = 1;
	}

	if (h < 0) { // flip about horizontal
		h = -h;
		t0 = 1;
		t1 = 0;
	} else {
		t0 = 0;
		t1 = 1;
	}

	UI_AdjustFrom640(&x, &y, &w, &h);
	trap_R_DrawStretchPic(x, y, w, h, s0, t0, s1, t1, hShader);
}

/*
=======================================================================================================================================
UI_FillRect

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void UI_FillRect(float x, float y, float width, float height, const float *color) {

	trap_R_SetColor(color);

	UI_AdjustFrom640(&x, &y, &width, &height);

	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, uis.whiteShader);
	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
UI_DrawRect

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void UI_DrawRect(float x, float y, float width, float height, const float *color) {

	trap_R_SetColor(color);

	UI_AdjustFrom640(&x, &y, &width, &height);

	trap_R_DrawStretchPic(x, y, width, 1, 0, 0, 0, 0, uis.whiteShader);
	trap_R_DrawStretchPic(x, y, 1, height, 0, 0, 0, 0, uis.whiteShader);
	trap_R_DrawStretchPic(x, y + height - 1, width, 1, 0, 0, 0, 0, uis.whiteShader);
	trap_R_DrawStretchPic(x + width - 1, y, 1, height, 0, 0, 0, 0, uis.whiteShader);
	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
UI_UpdateScreen
=======================================================================================================================================
*/
void UI_UpdateScreen(void) {
	trap_UpdateScreen();
}

/*
=======================================================================================================================================
UI_Refresh
=======================================================================================================================================
*/
void UI_Refresh(int realtime) {

	uis.frametime = realtime - uis.realtime;
	uis.realtime = realtime;

	if (!(trap_Key_GetCatcher() & KEYCATCH_UI)) {
		return;
	}

	UI_UpdateCvars();

	if (!trap_Cvar_VariableValue("cl_paused")) {
		if (!uis.playmusic) {
			UI_StartMenuMusic();
		}
	}

	if (uis.activemenu) {
		if (uis.activemenu->fullscreen) {
			// draw the background
			if (uis.activemenu->showlogo) {
				UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackShader);
			} else {
				UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackNoLogoShader);
			}
		}

		if (uis.activemenu->draw) {
			uis.activemenu->draw();
		} else {
			Menu_Draw(uis.activemenu);
		}

		if (uis.firstdraw) {
			UI_MouseEvent(0, 0);
			uis.firstdraw = qfalse;
		}
	}
	// draw cursor
	trap_R_SetColor(NULL);
	UI_DrawHandlePic(uis.cursorx - 16, uis.cursory - 16, 32, 32, uis.cursor);
#ifndef NDEBUG
	if (uis.debug) {
		// cursor coordinates
		UI_DrawString(0, 0, va("(%d, %d)", uis.cursorx, uis.cursory), UI_LEFT|UI_SMALLFONT, colorRed);
	}
#endif
	// delay playing the enter sound until after the menu has been drawn, to avoid delay while caching images
	if (m_entersound) {
		trap_S_StartLocalSound(menu_in_sound, CHAN_LOCAL_SOUND);
		m_entersound = qfalse;
	}
}

/*
=======================================================================================================================================
UI_CursorInRect
=======================================================================================================================================
*/
qboolean UI_CursorInRect(int x, int y, int width, int height) {

	if (uis.cursorx < x || uis.cursory < y || uis.cursorx > x + width || uis.cursory > y + height) {
		return qfalse;
	}

	return qtrue;
}
