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
 Helper functions called by cg_draw, cg_scoreboard, cg_info, etc.
**************************************************************************************************************************************/

#include "cg_local.h"

static vec4_t lastTextColor = {0, 0, 0, 1};

static screenPlacement_e cg_horizontalPlacement = PLACE_CENTER;
static screenPlacement_e cg_verticalPlacement = PLACE_CENTER;
static screenPlacement_e cg_lastHorizontalPlacement = PLACE_CENTER;
static screenPlacement_e cg_lastVerticalPlacement = PLACE_CENTER;

/*
=======================================================================================================================================
CG_SetScreenPlacement
=======================================================================================================================================
*/
void CG_SetScreenPlacement(screenPlacement_e hpos, screenPlacement_e vpos) {

	cg_lastHorizontalPlacement = cg_horizontalPlacement;
	cg_lastVerticalPlacement = cg_verticalPlacement;
	cg_horizontalPlacement = hpos;
	cg_verticalPlacement = vpos;
}

/*
=======================================================================================================================================
CG_PopScreenPlacement
=======================================================================================================================================
*/
void CG_PopScreenPlacement(void) {

	cg_horizontalPlacement = cg_lastHorizontalPlacement;
	cg_verticalPlacement = cg_lastVerticalPlacement;
}

/*
=======================================================================================================================================
CG_GetScreenHorizontalPlacement
=======================================================================================================================================
*/
screenPlacement_e CG_GetScreenHorizontalPlacement(void) {
	return cg_horizontalPlacement;
}

/*
=======================================================================================================================================
CG_GetScreenVerticalPlacement
=======================================================================================================================================
*/
screenPlacement_e CG_GetScreenVerticalPlacement(void) {
	return cg_verticalPlacement;
}

/*
=======================================================================================================================================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio.
=======================================================================================================================================
*/
void CG_AdjustFrom640(float *x, float *y, float *w, float *h) {

	if (cg_horizontalPlacement == PLACE_STRETCH || cg_stretch.integer) {
		// scale for screen sizes (not aspect correct in wide screen)
		*w *= cgs.screenXScaleStretch;
		*x *= cgs.screenXScaleStretch;
	} else {
		// scale for screen sizes
		*w *= cgs.screenXScale;
		*x *= cgs.screenXScale;

		if (cg_horizontalPlacement == PLACE_CENTER) {
			*x += cgs.screenXBias;
		} else if (cg_horizontalPlacement == PLACE_RIGHT) {
			*x += cgs.screenXBias * 2;
		}
	}

	if (cg_verticalPlacement == PLACE_STRETCH || cg_stretch.integer) {
		*h *= cgs.screenYScaleStretch;
		*y *= cgs.screenYScaleStretch;
	} else {
		*h *= cgs.screenYScale;
		*y *= cgs.screenYScale;

		if (cg_verticalPlacement == PLACE_CENTER) {
			*y += cgs.screenYBias;
		} else if (cg_verticalPlacement == PLACE_BOTTOM) {
			*y += cgs.screenYBias * 2;
		}
	}
}

/*
=======================================================================================================================================
CG_FillRect

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void CG_FillRect(float x, float y, float width, float height, const float *color) {

	trap_R_SetColor(color);
	CG_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
CG_DrawSides

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void CG_DrawSides(float x, float y, float w, float h, float size) {

	CG_AdjustFrom640(&x, &y, &w, &h);

	size *= cgs.screenXScale;

	trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
}

/*
=======================================================================================================================================
CG_DrawTopBottom

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void CG_DrawTopBottom(float x, float y, float w, float h, float size) {

	CG_AdjustFrom640(&x, &y, &w, &h);

	size *= cgs.screenYScale;

	trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
}

/*
=======================================================================================================================================
CG_DrawRect

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void CG_DrawRect(float x, float y, float width, float height, float size, const float *color) {

	trap_R_SetColor(color);
	CG_DrawTopBottom(x, y, width, height, size);
	CG_DrawSides(x, y + size, width, height - size * 2, size);
	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
CG_DrawPic

Coordinates are 640 * 480 virtual values.
=======================================================================================================================================
*/
void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader) {

	CG_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

/*
=======================================================================================================================================
CG_SetClipRegion
=======================================================================================================================================
*/
void CG_SetClipRegion(float x, float y, float w, float h) {
	vec4_t clip;

	CG_AdjustFrom640(&x, &y, &w, &h);

	clip[0] = x;
	clip[1] = y;
	clip[2] = x + w;
	clip[3] = y + h;

	trap_R_SetClipRegion(clip);
}

/*
=======================================================================================================================================
CG_ClearClipRegion
=======================================================================================================================================
*/
void CG_ClearClipRegion(void) {
	trap_R_SetClipRegion(NULL);
}

/*
=======================================================================================================================================

	TEXT DRAWING

=======================================================================================================================================
*/

/*
=======================================================================================================================================
Text_GetGlyph
=======================================================================================================================================
*/
const glyphInfo_t *Text_GetGlyph(const fontInfo_t *font, unsigned long index) {

	if (index == 0 || index >= GLYPHS_PER_FONT) {
		return &font->glyphs[(int)'.'];
	}

	return &font->glyphs[index];
}

/*
=======================================================================================================================================
Text_Width
=======================================================================================================================================
*/
float Text_Width(const char *text, const fontInfo_t *font, float scale, int limit) {
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

		glyph = Text_GetGlyph(font, Q_UTF8_CodePoint(&s));
		out += glyph->xSkip;
		count++;
	}

	return out * useScale;
}

/*
=======================================================================================================================================
Text_Height
=======================================================================================================================================
*/
float Text_Height(const char *text, const fontInfo_t *font, float scale, int limit) {
	int len, count;
	float max;
	const glyphInfo_t *glyph;
	float useScale;
	const char *s;

	if (!text) {
		return 0;
	}

	useScale = scale * font->glyphScale;
	max = 0;
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

		glyph = Text_GetGlyph(font, Q_UTF8_CodePoint(&s));

		if (max < glyph->height) {
			max = glyph->height;
		}

		count++;
	}

	return max * useScale;
}

/*
=======================================================================================================================================
Text_PaintGlyph
=======================================================================================================================================
*/
void Text_PaintGlyph(float x, float y, float w, float h, const glyphInfo_t *glyph, float *gradientColor) {

	if (gradientColor) {
		trap_R_DrawStretchPicGradient(x, y, w, h, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph, gradientColor);
	} else {
		trap_R_DrawStretchPic(x, y, w, h, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
	}
}

/*
=======================================================================================================================================
Text_Paint
=======================================================================================================================================
*/
void Text_Paint(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor) {
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

	CG_AdjustFrom640(&x, &y, &xscale, &yscale);

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

		glyph = Text_GetGlyph(font, Q_UTF8_CodePoint(&s));
		yadj = useScaleY * glyph->top;
		xadj = useScaleX * glyph->left;

		if (shadowOffsetX || shadowOffsetY) {
			colorBlack[3] = newColor[3];

			trap_R_SetColor(colorBlack);
			Text_PaintGlyph(x + xadj + shadowOffsetX, y - yadj + shadowOffsetY, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);
			trap_R_SetColor(newColor);

			colorBlack[3] = 1.0f;
		}

		Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, (gradient != 0) ? gradientColor : NULL);

		x += (glyph->xSkip * useScaleX) + adjust;
		count++;
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
Text_PaintWithCursor
=======================================================================================================================================
*/
void Text_PaintWithCursor(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor) {
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

	CG_AdjustFrom640(&x, &y, &xscale, &yscale);

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
	glyph2 = Text_GetGlyph(font, cursor);

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

		glyph = Text_GetGlyph(font, Q_UTF8_CodePoint(&s));

		if (count == cursorPos/*&& ((cg.realTime / BLINK_DIVISOR) & 1) == 0*/) { // Tobias: FIXME ?
			yadj = useScaleY * glyph2->top;
			// use horizontal width of text character (glyph)
			Text_PaintGlyph(x, y - yadj, (glyph->left + glyph->xSkip) * useScaleX, glyph2->imageHeight * useScaleY, glyph2, NULL);
		}

		yadj = useScaleY * glyph->top;
		xadj = useScaleX * glyph->left;

		if (shadowOffsetX || shadowOffsetY) {
			colorBlack[3] = newColor[3];

			trap_R_SetColor(colorBlack);
			Text_PaintGlyph(x + xadj + shadowOffsetX, y - yadj + shadowOffsetY, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);
			trap_R_SetColor(newColor);

			colorBlack[3] = 1.0f;
		}
		// make overstrike cursor invert color
		if (count == cursorPos/*&& !((cg.realTime / BLINK_DIVISOR) & 1)*/&& cursor == GLYPH_OVERSTRIKE) { // Tobias: FIXME ?
			// invert color
			vec4_t invertedColor;

			invertedColor[0] = 1.0f - newColor[0];
			invertedColor[1] = 1.0f - newColor[1];
			invertedColor[2] = 1.0f - newColor[2];
			invertedColor[3] = color[3];

			trap_R_SetColor(invertedColor);

			Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);
		} else {
			Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, (gradient != 0) ? gradientColor : NULL);
		}

		if (count == cursorPos/*&& !((cg.realTime / BLINK_DIVISOR) & 1)*/&& cursor == GLYPH_OVERSTRIKE) { // Tobias: FIXME ?
			// restore color
			trap_R_SetColor(newColor);
		}

		x += (glyph->xSkip * useScaleX) + adjust;
		count++;
	}
	// need to paint cursor at end of text
	if (cursorPos == len/*&& !((cg.realTime / BLINK_DIVISOR) & 1)*/) { // Tobias: FIXME ?
		yadj = useScaleY * glyph2->top;

		Text_PaintGlyph(x, y - yadj, glyph2->imageWidth * useScaleX, glyph2->imageHeight * useScaleY, glyph2, NULL);
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
Text_Paint_Limit
=======================================================================================================================================
*/
void Text_Paint_Limit(float *maxX, float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, float adjust, int limit) {
	int len, count;
	vec4_t newColor;
	const glyphInfo_t *glyph;
	const char *s;
	float max;
	float yadj, xadj;
	float useScaleX, useScaleY;
	float xscale, yscale;

	if (!text || !maxX) {
		return;
	}

	xscale = 1.0f;
	yscale = 1.0f;

	CG_AdjustFrom640(&x, &y, &xscale, &yscale);

	max = *maxX * xscale;
	adjust *= xscale;

	useScaleX = scale * font->glyphScale * xscale;
	useScaleY = scale * font->glyphScale * yscale;
	// prevent native resolution text from being blurred due to sub-pixel blending
	x = floor(x);
	y = floor(y);

	trap_R_SetColor(color);
	Vector4Copy(color, lastTextColor);

	len = Q_UTF8_PrintStrlen(text);

	if (limit > 0 && len > limit) {
		len = limit;
	}

	s = text;
	count = 0;

	while (s && *s && count < len) {
		if (Q_IsColorString(s)) {
			VectorCopy(g_color_table[ColorIndex(*(s + 1))], newColor);

			newColor[3] = color[3];

			trap_R_SetColor(newColor);
			Vector4Copy(newColor, lastTextColor);

			s += 2;
			continue;
		}

		glyph = Text_GetGlyph(font, Q_UTF8_CodePoint(&s));

		if (x + (glyph->xSkip * useScaleX) > max) {
			*maxX = 0;
			break;
		}

		yadj = useScaleY * glyph->top;
		xadj = useScaleX * glyph->left;

		Text_PaintGlyph(x + xadj, y - yadj, glyph->imageWidth * useScaleX, glyph->imageHeight * useScaleY, glyph, NULL);

		x += (glyph->xSkip * useScaleX) + adjust;
		*maxX = x / xscale;
		count++;
	}

	trap_R_SetColor(NULL);
}

#define MAX_WRAP_BYTES 1024
#define MAX_WRAP_LINES 1024
/*
=======================================================================================================================================
Text_Paint_AutoWrapped
=======================================================================================================================================
*/
void Text_Paint_AutoWrapped(float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *str, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor, float xmax, float ystep, int style) {
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
		width = Text_Width(s1, font, scale, 0);
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
				drawX = x - Text_Width(buf, font, scale, 0) / 2;
				break;
			case UI_RIGHT:
				// right justify at x
				drawX = x - Text_Width(buf, font, scale, 0);
				break;
			case UI_LEFT:
			default:
				// left justify at x
				drawX = x;
				break;
		}

		Text_Paint(drawX, y, font, scale, newColor, buf, adjust, 0, shadowOffset, gradient, forceColor);

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

/**************************************************************************************************************************************
 Tobias FIXME: UI STRINGS(remove this)
**************************************************************************************************************************************/

static int propMap[128][3] = {
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, PROP_SPACE_WIDTH}, // SPACE
	{11, 122, 7}, // !
	{154, 181, 14}, // "
	{55, 122, 17}, // #
	{79, 122, 18}, // $
	{101, 122, 23}, // %
	{153, 122, 18}, // &
	{9, 93, 7}, // '
	{207, 122, 8}, // (
	{230, 122, 9}, // )
	{177, 122, 18}, // *
	{30, 152, 18}, // +
	{85, 181, 7}, // ,
	{34, 93, 11}, // -
	{110, 181, 6}, // .
	{130, 152, 14}, // /
	{22, 64, 17}, // 0
	{41, 64, 12}, // 1
	{58, 64, 17}, // 2
	{78, 64, 18}, // 3
	{98, 64, 19}, // 4
	{120, 64, 18}, // 5
	{141, 64, 18}, // 6
	{204, 64, 16}, // 7
	{162, 64, 17}, // 8
	{182, 64, 18}, // 9
	{59, 181, 7}, // :
	{35, 181, 7}, // ;
	{203, 152, 14}, // <
	{56, 93, 14}, // =
	{228, 152, 14}, // >
	{177, 181, 18}, // ?
	{28, 122, 22}, // @
	{5, 4, 18}, // A
	{27, 4, 18}, // B
	{48, 4, 18}, // C
	{69, 4, 17}, // D
	{90, 4, 13}, // E
	{106, 4, 13}, // F
	{121, 4, 18}, // G
	{143, 4, 17}, // H
	{164, 4, 8}, // I
	{175, 4, 16}, // J
	{195, 4, 18}, // K
	{216, 4, 12}, // L
	{230, 4, 23}, // M
	{6, 34, 18}, // N
	{27, 34, 18}, // O
	{48, 34, 18}, // P
	{68, 34, 18}, // Q
	{90, 34, 17}, // R
	{110, 34, 18}, // S
	{130, 34, 14}, // T
	{146, 34, 18}, // U
	{166, 34, 19}, // V
	{185, 34, 29}, // W
	{215, 34, 18}, // X
	{234, 34, 18}, // Y
	{5, 64, 14}, // Z
	{60, 152, 7}, // [
	{106, 151, 13}, // '\'
	{83, 152, 7}, // ]
	{128, 122, 17}, // ^
	{4, 152, 21}, // _
	{134, 181, 5}, // '
	{5, 4, 18}, // A
	{27, 4, 18}, // B
	{48, 4, 18}, // C
	{69, 4, 17}, // D
	{90, 4, 13}, // E
	{106, 4, 13}, // F
	{121, 4, 18}, // G
	{143, 4, 17}, // H
	{164, 4, 8}, // I
	{175, 4, 16}, // J
	{195, 4, 18}, // K
	{216, 4, 12}, // L
	{230, 4, 23}, // M
	{6, 34, 18}, // N
	{27, 34, 18}, // O
	{48, 34, 18}, // P
	{68, 34, 18}, // Q
	{90, 34, 17}, // R
	{110, 34, 18}, // S
	{130, 34, 14}, // T
	{146, 34, 18}, // U
	{166, 34, 19}, // V
	{185, 34, 29}, // W
	{215, 34, 18}, // X
	{234, 34, 18}, // Y
	{5, 64, 14}, // Z
	{153, 152, 13}, // {
	{11, 181, 5}, // |
	{180, 152, 13}, // }
	{79, 93, 17}, // ~
	{0, 0, -1} // DEL
};

static int propMapB[26][3] = {
	{11, 12, 33},
	{49, 12, 31},
	{85, 12, 31},
	{120, 12, 30},
	{156, 12, 21},
	{183, 12, 21},
	{207, 12, 32},
	{13, 55, 30},
	{49, 55, 13},
	{66, 55, 29},
	{101, 55, 31},
	{135, 55, 21},
	{158, 55, 40},
	{204, 55, 32},
	{12, 97, 31},
	{48, 97, 31},
	{82, 97, 30},
	{118, 97, 30},
	{153, 97, 30},
	{185, 97, 25},
	{213, 97, 30},
	{11, 139, 32},
	{42, 139, 51},
	{93, 139, 32},
	{126, 139, 31},
	{158, 139, 25},
};

#define PROPB_GAP_WIDTH 4
#define PROPB_SPACE_WIDTH 12
#define PROPB_HEIGHT 48

/*
=======================================================================================================================================
UI_DrawBannerString2
=======================================================================================================================================
*/
static void UI_DrawBannerString2(int x, int y, const char *str, vec4_t color) {
	const char *s;
	unsigned char ch;
	float ax;
	float ay;
	float aw;
	float ah;
	float frow;
	float fcol;
	float fwidth;
	float fheight;

	// draw the colored text
	trap_R_SetColor(color);

	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenYScale + cgs.screenYBias;
	s = str;

	while (*s) {
		ch = *s & 127;

		if (ch == ' ') {
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH) * cgs.screenXScale;
		} else if (ch >= 'A' && ch <= 'Z') {
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * cgs.screenXScale;
			ah = (float)PROPB_HEIGHT * cgs.screenYScale;
			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, cgs.media.charsetPropB);
			ax += (aw + (float)PROPB_GAP_WIDTH * cgs.screenXScale);
		}

		s++;
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
UI_DrawBannerString
=======================================================================================================================================
*/
void UI_DrawBannerString(int x, int y, const char *str, int style, vec4_t color) {
	const char *s;
	int ch;
	int width;
	vec4_t drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;

	while (*s) {
		ch = *s;

		if (ch == ' ') {
			width += PROPB_SPACE_WIDTH;
		} else if (ch >= 'A' && ch <= 'Z') {
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}

		s++;
	}

	width -= PROPB_GAP_WIDTH;

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

	if (style & UI_DROPSHADOW) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2(x + 2, y + 2, str, drawcolor);
	}

	UI_DrawBannerString2(x, y, str, color);
}

/*
=======================================================================================================================================
UI_ProportionalStringWidth
=======================================================================================================================================
*/
int UI_ProportionalStringWidth(const char *str) {
	const char *s;
	int ch;
	int charWidth;
	int width;

	s = str;
	width = 0;

	while (*s) {
		ch = *s & 127;
		charWidth = propMap[ch][2];

		if (charWidth != -1) {
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}

		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

/*
=======================================================================================================================================
UI_DrawProportionalString2
=======================================================================================================================================
*/
static void UI_DrawProportionalString2(int x, int y, const char *str, vec4_t color, float sizeScale, qhandle_t charset) {
	const char *s;
	unsigned char ch;
	float ax;
	float ay;
	float aw;
	float ah;
	float frow;
	float fcol;
	float fwidth;
	float fheight;

	// draw the colored text
	trap_R_SetColor(color);

	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenYScale + cgs.screenYBias;
	s = str;

	while (*s) {
		ch = *s & 127;

		if (ch == ' ') {
			aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * sizeScale;
		} else if (propMap[ch][2] != -1) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * cgs.screenXScale * sizeScale;
			ah = (float)PROP_HEIGHT * cgs.screenYScale * sizeScale;
			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset);
		} else {
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
		s++;
	}

	trap_R_SetColor(NULL);
}

/*
=======================================================================================================================================
UI_ProportionalSizeScale
=======================================================================================================================================
*/
float UI_ProportionalSizeScale(int style) {

	if (style & UI_SMALLFONT) {
		return 0.75;
	}

	return 1.00;
}

/*
=======================================================================================================================================
UI_DrawProportionalString
=======================================================================================================================================
*/
void UI_DrawProportionalString(int x, int y, const char *str, int style, vec4_t color) {
	vec4_t drawcolor;
	int width;
	float sizeScale;

	sizeScale = UI_ProportionalSizeScale(style);

	switch (style & UI_FORMATMASK) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth(str) * sizeScale;
			x -= width / 2;
			break;
		case UI_RIGHT:
			width = UI_ProportionalStringWidth(str) * sizeScale;
			x -= width;
			break;
		case UI_LEFT:
		default:
			break;
	}

	if (style & UI_DROPSHADOW) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x + 2, y + 2, str, drawcolor, sizeScale, cgs.media.charsetProp);
	}

	if (style & UI_INVERSE) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetProp);
		return;
	}

	if (style & UI_PULSE) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin(cg.time / PULSE_DIVISOR);
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow);
		return;
	}

	UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);
}

/**************************************************************************************************************************************
 Tobias end
**************************************************************************************************************************************/

/*
=======================================================================================================================================
CG_DrawStringDirect
=======================================================================================================================================
*/
void CG_DrawStringDirect(float x, int y, const char *str, int style, const vec4_t color, float scale, int maxChars, float shadowOffset, float gradient, int cursorPos, int cursorChar, float wrapX) {
	int charh;
	const float *drawcolor;
	const fontInfo_t *font;
	int decent;

	if (!str) {
		return;
	}

	if (!color) {
		color = colorWhite;
	}

	switch (style & UI_FONTMASK) {
		case UI_TINYFONT:
			font = &cgs.media.tinyFont;
			break;
		case UI_SMALLFONT:
			font = &cgs.media.smallFont;
			break;
		case UI_DEFAULTFONT:
			font = &cgs.media.defaultFont;
			break;
		case UI_BIGFONT:
		default:
			font = &cgs.media.bigFont;
			break;
		case UI_GIANTFONT:
			font = &cgs.media.giantFont;
			break;
		case UI_TITANFONT:
			font = &cgs.media.titanFont;
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

	drawcolor = color;

	if (wrapX <= 0) {
		switch (style & UI_FORMATMASK) {
			case UI_CENTER:
				// center justify at x
				x = x - Text_Width(str, font, scale, 0) / 2;
				break;
			case UI_RIGHT:
				// right justify at x
				x = x - Text_Width(str, font, scale, 0);
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
		Text_PaintWithCursor(x, y, font, scale, drawcolor, str, cursorPos, cursorChar, 0, maxChars, shadowOffset, gradient, !!(style & UI_FORCECOLOR));
	} else if (wrapX > 0) {
		// replace 'char height' in line height with our scaled charh
		// ZTM: TODO: This text gap handling is kind of messy. Passing scale to CG_DrawStringLineHeight might make cleaner code here.
		int gap = CG_DrawStringLineHeight(style) - font->pointSize;

		Text_Paint_AutoWrapped(x, y, font, scale, drawcolor, str, 0, maxChars, shadowOffset, gradient, !!(style & UI_FORCECOLOR), wrapX, charh + gap, style);
	} else {
		Text_Paint(x, y, font, scale, drawcolor, str, 0, maxChars, shadowOffset, gradient, !!(style & UI_FORCECOLOR));
	}
}

/*
=======================================================================================================================================
CG_DrawStringAutoWrap
=======================================================================================================================================
*/
void CG_DrawStringAutoWrap(int x, int y, const char *str, int style, const vec4_t color, float shadowOffset, float gradient, float wrapX) {
	CG_DrawStringDirect(x, y, str, style, color, 0, 0, shadowOffset, gradient, -1, -1, wrapX);
}

/*
=======================================================================================================================================
CG_DrawStringExtWithCursor
=======================================================================================================================================
*/
void CG_DrawStringExtWithCursor(float x, int y, const char *str, int style, const vec4_t color, float scale, int maxChars, float shadowOffset, float gradient, int cursorPos, int cursorChar) {
	CG_DrawStringDirect(x, y, str, style, color, scale, maxChars, shadowOffset, gradient, cursorPos, cursorChar, 0);
}

/*
=======================================================================================================================================
CG_DrawStringWithCursor
=======================================================================================================================================
*/
void CG_DrawStringWithCursor(int x, int y, const char *str, int style, const vec4_t color, int cursorPos, int cursorChar) {
	CG_DrawStringExtWithCursor(x, y, str, style, color, 0, 0, 0, 0, cursorPos, cursorChar);
}

/*
=======================================================================================================================================
CG_DrawStringExt
=======================================================================================================================================
*/
void CG_DrawStringExt(int x, int y, const char *str, int style, const vec4_t color, float scale, int maxChars, float shadowOffset) {
	CG_DrawStringExtWithCursor(x, y, str, style, color, scale, maxChars, shadowOffset, 0, -1, -1);
}

/*
=======================================================================================================================================
CG_DrawString

Draws a multi-colored string with a drop shadow, optionally forcing to a fixed color.

Coordinates are at 640 by 480 virtual resolution

Gradient value is how much to darken color at bottom of text.
=======================================================================================================================================
*/
void CG_DrawString(int x, int y, const char *str, int style, const vec4_t color) {
	CG_DrawStringExtWithCursor(x, y, str, style, color, 0, 0, 0, 0, -1, -1);
}

/*
=======================================================================================================================================
CG_DrawFloatString

Same as CG_DrawString, but x is float.
=======================================================================================================================================
*/
void CG_DrawFloatString(float x, int y, const char *str, int style, const vec4_t color) {
	CG_DrawStringExtWithCursor(x, y, str, style, color, 0, 0, 0, 0, -1, -1);
}

/*
=======================================================================================================================================
CG_DrawBigString
=======================================================================================================================================
*/
void CG_DrawBigString(int x, int y, const char *s, float alpha) {
	float color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawString(x, y, s, UI_DROPSHADOW|UI_BIGFONT, color);
}

/*
=======================================================================================================================================
CG_DrawBigStringColor
=======================================================================================================================================
*/
void CG_DrawBigStringColor(int x, int y, const char *s, vec4_t color) {
	CG_DrawString(x, y, s, UI_FORCECOLOR|UI_DROPSHADOW|UI_BIGFONT, color);
}

/*
=======================================================================================================================================
CG_DrawSmallString
=======================================================================================================================================
*/
void CG_DrawSmallString(int x, int y, const char *s, float alpha) {
	float color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawString(x, y, s, UI_SMALLFONT, color);
}

/*
=======================================================================================================================================
CG_DrawSmallStringColor
=======================================================================================================================================
*/
void CG_DrawSmallStringColor(int x, int y, const char *s, vec4_t color) {
	CG_DrawString(x, y, s, UI_FORCECOLOR|UI_SMALLFONT, color);
}

/*
=======================================================================================================================================
CG_DrawStrlen

Returns draw width, skiping color escape codes.
=======================================================================================================================================
*/
float CG_DrawStrlen(const char *str, int style) {
	const fontInfo_t *font;
	int charh;

	switch (style & UI_FONTMASK) {
		case UI_TINYFONT:
			font = &cgs.media.tinyFont;
			break;
		case UI_SMALLFONT:
			font = &cgs.media.smallFont;
			break;
		case UI_DEFAULTFONT:
			font = &cgs.media.defaultFont;
			break;
		case UI_BIGFONT:
		default:
			font = &cgs.media.bigFont;
			break;
		case UI_GIANTFONT:
			font = &cgs.media.giantFont;
			break;
		case UI_TITANFONT:
			font = &cgs.media.titanFont;
			break;
	}

	charh = font->pointSize;

	return Text_Width(str, font, charh / 48.0f, 0);
}

/*
=======================================================================================================================================
CG_DrawStringLineHeight

Returns draw height of text line for drawing multiple lines of text.
=======================================================================================================================================
*/
int CG_DrawStringLineHeight(int style) {
	const fontInfo_t *font;
	int lineHeight;
	int charh;
	int gap;

	gap = 0;

	switch (style & UI_FONTMASK) {
		case UI_TINYFONT:
			font = &cgs.media.tinyFont;
			gap = 1;
			break;
		case UI_SMALLFONT:
			font = &cgs.media.smallFont;
			gap = 2;
			break;
		case UI_DEFAULTFONT:
			font = &cgs.media.defaultFont;
			gap = 2;
			break;
		case UI_BIGFONT:
		default:
			font = &cgs.media.bigFont;
			gap = 2;
			break;
		case UI_GIANTFONT:
			font = &cgs.media.giantFont;
			gap = 2;
			break;
		case UI_TITANFONT:
			font = &cgs.media.titanFont;
			gap = 6;
			break;
	}

	charh = font->pointSize;
	lineHeight = charh + gap;

	return lineHeight;
}

/*
=======================================================================================================================================

	COMMON UI FUNCTIONS

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_TileClearBox

This repeats a 64 * 64 tile graphic to fill the screen around a sized down refresh window.
=======================================================================================================================================
*/
static void CG_TileClearBox(int x, int y, int w, int h, qhandle_t hShader) {
	float s1, t1, s2, t2;

	s1 = x / 64.0;
	t1 = y / 64.0;
	s2 = (x + w) / 64.0;
	t2 = (y + h) / 64.0;
	trap_R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
}

/*
=======================================================================================================================================
CG_TileClear

Clear around a sized down screen.
=======================================================================================================================================
*/
void CG_TileClear(void) {
	int top, bottom, left, right;
	int w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if (cg.refdef.x == 0 && cg.refdef.y == 0 && cg.refdef.width == w && cg.refdef.height == h) {
		return; // full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height - 1;
	left = cg.refdef.x;
	right = left + cg.refdef.width - 1;
	// clear above view screen
	CG_TileClearBox(0, 0, w, top, cgs.media.backTileShader);
	// clear below view screen
	CG_TileClearBox(0, bottom, w, h - bottom, cgs.media.backTileShader);
	// clear left of view screen
	CG_TileClearBox(0, top, left, bottom - top + 1, cgs.media.backTileShader);
	// clear right of view screen
	CG_TileClearBox(right, top, w - right, bottom - top + 1, cgs.media.backTileShader);
}

/*
=======================================================================================================================================
CG_FadeColor
=======================================================================================================================================
*/
float *CG_FadeColor(int startMsec, int totalMsec) {
	static vec4_t color;
	int t;

	if (startMsec == 0) {
		return NULL;
	}

	t = cg.time - startMsec;

	if (t >= totalMsec) {
		return NULL;
	}
	// fade out
	if (totalMsec - t < FADE_TIME) {
		color[3] = (totalMsec - t) * 1.0 / FADE_TIME;
	} else {
		color[3] = 1.0;
	}

	color[0] = color[1] = color[2] = 1;
	return color;
}

/*
=======================================================================================================================================
CG_TeamColor
=======================================================================================================================================
*/
float *CG_TeamColor(int team) {
	static vec4_t red = {1, 0.2f, 0.2f, 1};
	static vec4_t blue = {0.2f, 0.2f, 1, 1};
	static vec4_t other = {1, 1, 1, 1};
	static vec4_t spectator = {0.7f, 0.7f, 0.7f, 1};

	switch (team) {
		case TEAM_RED:
			return red;
		case TEAM_BLUE:
			return blue;
		case TEAM_SPECTATOR:
			return spectator;
		default:
			return other;
	}
}

/*
=======================================================================================================================================
CG_GetColorForHealth
=======================================================================================================================================
*/
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor) {
	int count;
	int max;

	// calculate the total points of damage that can be sustained at the current health/armor level
	if (health <= 0) {
		VectorClear(hcolor); // black
		hcolor[3] = 1;
		return;
	}

	count = armor;
	max = health * ARMOR_PROTECTION / (1.0 - ARMOR_PROTECTION);

	if (max < count) {
		count = max;
	}

	health += count;
	// set the color based on health
	hcolor[0] = 1.0;
	hcolor[3] = 1.0;

	if (health >= 100) {
		hcolor[2] = 1.0;
	} else if (health < 66) {
		hcolor[2] = 0;
	} else {
		hcolor[2] = (health - 66) / 33.0;
	}

	if (health > 60) {
		hcolor[1] = 1.0;
	} else if (health < 30) {
		hcolor[1] = 0;
	} else {
		hcolor[1] = (health - 30) / 30.0;
	}
}

/*
=======================================================================================================================================
CG_ColorForHealth
=======================================================================================================================================
*/
void CG_ColorForHealth(vec4_t hcolor) {
	CG_GetColorForHealth(cg.snap->ps.stats[STAT_HEALTH], cg.snap->ps.stats[STAT_ARMOR], hcolor);
}

/*
=======================================================================================================================================
CG_KeysStringForBinding
=======================================================================================================================================
*/
void CG_KeysStringForBinding(const char *binding, char *string, int stringSize) {
	char name2[32];
	int keys[2];
	int i, key;

	for (i = 0, key = 0; i < 2; i++) {
		key = trap_Key_GetKey(binding, key);
		keys[i] = key;
		key++;
	}

	if (keys[0] == -1) {
		Q_strncpyz(string, "???", stringSize);
		return;
	}

	trap_Key_KeynumToStringBuf(keys[0], string, MIN(32, stringSize));
	Q_strupr(string);

	if (keys[1] != -1) {
		trap_Key_KeynumToStringBuf(keys[1], name2, 32);
		Q_strupr(name2);
		Q_strcat(string, stringSize, " or ");
		Q_strcat(string, stringSize, name2);
	}
}

/*
=======================================================================================================================================
CG_TranslateString
=======================================================================================================================================
*/
char *CG_TranslateString(const char *string) {
	static char staticbuf[2][MAX_VA_STRING];
	static int bufcount = 0;
	char *buf;

	buf = staticbuf[bufcount++ % 2];

	trap_TranslateString(string, buf);

	return buf;
}
