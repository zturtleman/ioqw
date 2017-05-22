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
// tr_font.c
// 
//
// The font system uses FreeType 2.x to render TrueType fonts for use within the game.
// As of this writing ( Nov, 2000 ) Team Arena uses these fonts for all of the ui and 
// about 90% of the cgame presentation. A few areas of the CGAME were left uses the old 
// fonts since the code is shared with standard Q3A.
//
// Q3A 1.25+ and Team Arena were shipped with the font rendering code disabled, there was
// a patent held by Apple at the time which FreeType MIGHT infringe on.
// This removed any potential patent issues and it kept us (id Software) from having to 
// distribute an actual TrueTrype font which is 1. expensive to do and 2. seems to require
// an act of god to accomplish.
//
// What we did was pre-render the fonts using FreeType ( which is why we leave the FreeType
// credit in the credits ) and then saved off the glyph data and then hand touched up the 
// font bitmaps so they scale a bit better in GL.
//
// In the UI Scripting code, a scale of 1.0 is equal to a 48 point font. In Team Arena, we
// use three or four scales, most of them exactly equaling the specific rendered size. We 
// rendered three sizes in Team Arena, 12, 16, and 20.
//
// To generate new font data you need to go through the following steps.
// 1. delete the fontImage_x_xx.tga files and fontImage_xx.dat files from the fonts path.
// 2. in a ui script, specificy a font, smallFont, and bigFont keyword with font name and 
//    point size. the original TrueType fonts must exist in fonts at this point.
// 3. run the game, you should see things normally.
// 4. Exit the game and there will be three dat files and at least three tga files. The 
//    tga's are in 256x256 pages so if it takes three images to render a 24 point font you 
//    will end up with fontImage_0_24.tga through fontImage_2_24.tga
// 5. In future runs of the game, the system looks for these images and data files when a
//    specific point sized font is rendered and loads them for use. 
// 6. Because of the original beta nature of the FreeType code you will probably want to hand
//    touch the font bitmaps.
// 
// Currently a define in the project turns on or off the FreeType code which is currently 
// defined out. To pre-render new fonts you need enable the define ( BUILD_FREETYPE ) and 
// uncheck the exclude from build check box in the FreeType2 area of the Renderer project. 


#include "tr_common.h"
#include "../qcommon/qcommon.h"

#ifdef BUILD_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H
#include FT_SYSTEM_H
#include FT_IMAGE_H
#include FT_OUTLINE_H
#include FT_STROKER_H

// FT_PIXEL_MODE_BGRA exists in freetype 2.5+, but not 2.4.x.
// Currently it's only used for passing data from between functions in this file,
// so define it if needed.
#if (FREETYPE_MAJOR == 2 && FREETYPE_MINOR < 5) || FREETYPE_MAJOR < 2
#define FT_PIXEL_MODE_BGRA FT_PIXEL_MODE_MAX
#endif

#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

FT_Library ftLibrary = NULL;  
#endif

#define MAX_FONTS 12
static int registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

#ifdef BUILD_FREETYPE
// A horizontal pixel span generated by the FreeType renderer.
typedef struct
{
	int x, y, width, coverage;
} Span;

typedef struct
{
	int numSpans, maxSpans;
	Span *spans;
} SpanList;

void SpanList_Init( SpanList *list ) {
	list->spans = NULL;
	list->numSpans = 0;
	list->maxSpans = 0;
}

void SpanList_Free( SpanList *list ) {
	if ( list->spans ) {
		free( list->spans );
	}
	list->spans = NULL;
	list->numSpans = 0;
	list->maxSpans = 0;
}

// Expand list as needed like a C++ std::vector
void SpanList_Add( SpanList *list, int x, int y, int width, int coverage ) {
	// First time, allocate initial list size
	if ( !list->spans ) {
		list->maxSpans = 1024;
		list->spans = (Span *)malloc( list->maxSpans * sizeof ( Span ) );

		if ( !list->spans ) {
			ri.Error( ERR_DROP, "Font rendering out of memory" );
		}
	}

	// Ran out of space, expand the list
	if ( list->numSpans >= list->maxSpans ) {
		list->maxSpans *= 2;
		list->spans = (Span *)realloc( list->spans, list->maxSpans * sizeof ( Span ) );

		if ( !list->spans ) {
			ri.Error( ERR_DROP, "Font rendering out of memory" );
		}
	}

	list->spans[list->numSpans].x = x;
	list->spans[list->numSpans].y = y;
	list->spans[list->numSpans].width = width;
	list->spans[list->numSpans].coverage = coverage;
	list->numSpans++;
}


// Each time the renderer calls us back we just push another span entry on
// our list.
void
RasterCallback(const int y,
				const int count,
				const FT_Span * const spans,
				void * const user)
{
	SpanList *list = (SpanList *)user;
	int i;

	for (i = 0; i < count; ++i) {
		SpanList_Add(list, spans[i].x, y, spans[i].len, spans[i].coverage);
	}
}


// Set up the raster parameters and render the outline.
void
RenderSpans(FT_Library library,
			FT_Outline * const outline,
			SpanList *list)
{
	FT_Raster_Params params;

	memset(&params, 0, sizeof(params));

	params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
	params.gray_spans = RasterCallback;
	params.user = list;

	FT_Outline_Render(library, outline, &params);
}

/*
==================
R_RenderOutlineBitmap

Render an outline glyph with a optional border around it.

This function and the "span" rendering code is based on freetype2 example2.cpp
by Erik Möller, which was released into the public domain.
==================
*/
qboolean R_RenderOutlineBitmap( FT_GlyphSlot glyph, FT_Bitmap *bit2, float borderWidth ) {
	// Spans for glyph face and border
	SpanList glyphSpans, borderSpans;
	// Iterators
	Span *s;
	int i, w;
	unsigned char *dst;
	// Span bounds
	int xmin, xmax, ymin, ymax;
	// Image metrics
	int imgWidth, imgHeight;
	// RGB colors
	unsigned char glyphColor[3] = { 255, 255, 255 };
	unsigned char borderColor[3] = { 0, 0, 0 };

	// Initialize the span lists
	SpanList_Init( &glyphSpans );
	SpanList_Init( &borderSpans );

	// Render the basic glyph to a span list.
	RenderSpans(ftLibrary, &glyph->outline, &glyphSpans);

	if ( glyphSpans.numSpans == 0 ) {
		// This happens with space character and maybe others that are invisible.
		return qfalse;
	}

	// Next we need the spans for the border.
	if ( borderWidth > 0 ) {
		FT_Glyph glyphTemp;

		if (FT_Get_Glyph(glyph, &glyphTemp) == 0)
		{
			// This needs to be an outline glyph to work.
			if (glyphTemp->format == FT_GLYPH_FORMAT_OUTLINE)
			{
				FT_Stroker stroker;

				// Set up a stroker.
				FT_Stroker_New(ftLibrary, &stroker);
				FT_Stroker_Set(stroker,
					(int)(borderWidth * 64),
					FT_STROKER_LINECAP_ROUND,
					FT_STROKER_LINEJOIN_ROUND,
					0);

				FT_Glyph_StrokeBorder(&glyphTemp, stroker, 0, 1);

				// Render the border spans to the span list
				FT_Outline *o = &((FT_OutlineGlyph)(glyphTemp))->outline;
				RenderSpans(ftLibrary, o, &borderSpans);

				// Clean up afterwards.
				FT_Stroker_Done(stroker);
			}

			FT_Done_Glyph(glyphTemp);
		}
	}

	// Now we need to put it all together.

	// Figure out what the bounding rect is for both the span lists.
	xmin = glyphSpans.spans[0].x;
	xmax = glyphSpans.spans[0].x;
	ymin = glyphSpans.spans[0].y;
	ymax = glyphSpans.spans[0].y;

	for (i = 0, s = glyphSpans.spans; i < glyphSpans.numSpans; ++i, ++s)
	{
		xmin = MIN(xmin, s->x);
		xmax = MAX(xmax, s->x);

		ymin = MIN(ymin, s->y);
		ymax = MAX(ymax, s->y);

		xmin = MIN(xmin, s->x + s->width - 1);
		xmax = MAX(xmax, s->x + s->width - 1);
	}
	for (i = 0, s = borderSpans.spans; i < borderSpans.numSpans; ++i, ++s)
	{
		xmin = MIN(xmin, s->x);
		xmax = MAX(xmax, s->x);

		ymin = MIN(ymin, s->y);
		ymax = MAX(ymax, s->y);

		xmin = MIN(xmin, s->x + s->width - 1);
		xmax = MAX(xmax, s->x + s->width - 1);
	}

	// Get some metrics of our image.
	imgWidth = xmax - xmin + 1;
	imgHeight = ymax - ymin + 1;

	// Allocate data for our image and clear it out to transparent.
	bit2->width      = imgWidth;
	bit2->rows       = imgHeight;
	bit2->pitch      = imgWidth*4;
	bit2->pixel_mode = FT_PIXEL_MODE_BGRA;
	bit2->buffer     = ri.Malloc(bit2->pitch*bit2->rows);

	Com_Memset( bit2->buffer, 0, bit2->pitch*bit2->rows );

	// Loop over the border spans and just draw them into the image.
	for (i = 0, s = borderSpans.spans; i < borderSpans.numSpans; ++i, ++s)
	{
		dst = &bit2->buffer[(int)((imgHeight - 1 - (s->y - ymin)) * bit2->pitch + (s->x - xmin/* + w*/) * 4)];

		for (w = 0; w < s->width; ++w, dst += 4)
		{
			// Flip border color RGB to BGR
			dst[0] = borderColor[2];
			dst[1] = borderColor[1];
			dst[2] = borderColor[0];
			dst[3] = s->coverage;
		}
	}

	// Then loop over the regular glyph spans and blend them into the image.
	for (i = 0, s = glyphSpans.spans; i < glyphSpans.numSpans; ++i, ++s)
	{
		dst = &bit2->buffer[(int)((imgHeight - 1 - (s->y - ymin)) * bit2->pitch + (s->x - xmin/* + w*/) * 4)];

		for (w = 0; w < s->width; ++w, dst += 4)
		{
			// Flip glyph color RGB to BGR
			dst[0] = (int)(dst[0] + ((glyphColor[2] - dst[0]) * s->coverage) / 255.0f);
			dst[1] = (int)(dst[1] + ((glyphColor[1] - dst[1]) * s->coverage) / 255.0f);
			dst[2] = (int)(dst[2] + ((glyphColor[0] - dst[2]) * s->coverage) / 255.0f);
			dst[3] = MIN(255, dst[3] + s->coverage);
		}
	}

	SpanList_Free( &glyphSpans );
	SpanList_Free( &borderSpans );

	return qtrue;
}


void R_GetGlyphInfo(FT_GlyphSlot glyph, int *left, int *right, int *width, int *top, int *bottom, int *height, int *pitch) {
	*left  = _FLOOR( glyph->metrics.horiBearingX );
	*right = _CEIL( glyph->metrics.horiBearingX + glyph->metrics.width );
	*width = _TRUNC(*right - *left);

	*top    = _CEIL( glyph->metrics.horiBearingY );
	*bottom = _FLOOR( glyph->metrics.horiBearingY - glyph->metrics.height );
	*height = _TRUNC( *top - *bottom );
	*pitch  = ( qtrue ? (*width+3) & -4 : (*width+7) >> 3 );
}


FT_Bitmap *R_RenderGlyph(FT_GlyphSlot glyph, glyphInfo_t* glyphOut, float borderWidth) {
	FT_Bitmap  *bit2;
	int left, right, width, top, bottom, height, pitch, size;
	int borderSizeX, borderSizeY;

	R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

	if ( glyph->format == ft_glyph_format_outline ) {
		size   = pitch*height; 

		bit2 = ri.Malloc(sizeof(FT_Bitmap));
		bit2->buffer = NULL;

		// ZTM: FIXME: Why is R_RenderOutlineBitmap with borderWidth == 0 different than FT_Outline_Get_Bitmap?
		if ( borderWidth != 0 && R_RenderOutlineBitmap( glyph, bit2, borderWidth ) ) {
			// check how much the border increased the size of the glyph
			borderSizeX = bit2->width - width;
			borderSizeY = bit2->rows - height;

			glyphOut->height = bit2->rows;
			glyphOut->pitch = bit2->pitch;

			glyphOut->top = _TRUNC(glyph->metrics.horiBearingY) + borderSizeY / 2;// + 1;
			glyphOut->left = _TRUNC(glyph->metrics.horiBearingX) - borderSizeX / 2;// + 1;
			glyphOut->xSkip = _TRUNC(glyph->metrics.horiAdvance) + borderSizeX / 2;// + 1;

			return bit2;
		}

		// If not able to load the glyph by rendering spans, fallback to original Q3 method.
		// This fixes 'space' (character 32) not having xSkip.

		bit2->width      = width;
		bit2->rows       = height;
		bit2->pitch      = pitch;
		bit2->pixel_mode = FT_PIXEL_MODE_GRAY;
		//bit2->pixel_mode = FT_PIXEL_MODE_MONO;
		bit2->buffer     = ri.Malloc(pitch*height);
		bit2->num_grays = 256;

		Com_Memset( bit2->buffer, 0, size );

		FT_Outline_Translate( &glyph->outline, -left, -bottom );

		FT_Outline_Get_Bitmap( ftLibrary, &glyph->outline, bit2 );

		glyphOut->height = height;
		glyphOut->pitch = pitch;

		glyphOut->top = _TRUNC(glyph->metrics.horiBearingY);// + 1;
		glyphOut->left = _TRUNC(glyph->metrics.horiBearingX);// + 1;

		glyphOut->xSkip = _TRUNC(glyph->metrics.horiAdvance);// + 1;

		return bit2;
	} else {
		ri.Printf(PRINT_ALL, "Non-outline fonts are not supported\n");
	}
	return NULL;
}

static void WriteTGA (char *filename, byte *data, int width, int height) {
	byte			*buffer;
	int				i, c;
	int             row;
	unsigned char  *flip;
	unsigned char  *src, *dst;

	buffer = ri.Malloc(width*height*4 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width&255;
	buffer[13] = width>>8;
	buffer[14] = height&255;
	buffer[15] = height>>8;
	buffer[16] = 32;	// pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for (i=18 ; i<c ; i+=4)
	{
		buffer[i] = data[i-18+2];		// blue
		buffer[i+1] = data[i-18+1];		// green
		buffer[i+2] = data[i-18+0];		// red
		buffer[i+3] = data[i-18+3];		// alpha
	}

	// flip upside down
	flip = (unsigned char *)ri.Malloc(width*4);
	for(row = 0; row < height/2; row++)
	{
		src = buffer + 18 + row * 4 * width;
		dst = buffer + 18 + (height - row - 1) * 4 * width;

		Com_Memcpy(flip, src, width*4);
		Com_Memcpy(src, dst, width*4);
		Com_Memcpy(dst, flip, width*4);
	}
	ri.Free(flip);

	ri.FS_WriteFile(filename, buffer, c);

	//f = fopen (filename, "wb");
	//fwrite (buffer, 1, c, f);
	//fclose (f);

	ri.Free (buffer);
}

// returns NULL if glyph does not fit in image
static glyphInfo_t *RE_ConstructGlyphInfo(int imageSize, unsigned char *imageOut, int *xOut, int *yOut, int *rowHeight, FT_Face face, unsigned long c, float borderWidth, qboolean forceAutoHint) {
	int i, loadFlags;
	static glyphInfo_t glyph;
	unsigned char *src, *dst;
	FT_Bitmap *bitmap = NULL;

	Com_Memset(&glyph, 0, sizeof(glyphInfo_t));
	// make sure everything is here
	if (face != NULL) {
		loadFlags = FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP;

		if ( forceAutoHint ) {
			loadFlags |= FT_LOAD_FORCE_AUTOHINT;
		}

		FT_Load_Glyph( face, FT_Get_Char_Index( face, c ), loadFlags );
		bitmap = R_RenderGlyph(face->glyph, &glyph, borderWidth);
		if (!bitmap) {
			return &glyph;
		}

		// we need to make sure we fit
		if (*xOut + bitmap->width + 1 >= imageSize-1) {
			*xOut = 0;
			*yOut += *rowHeight + 1;
			*rowHeight = 0;
		}

		if (*yOut + bitmap->rows + 1 >= imageSize-1) {
			ri.Free(bitmap->buffer);
			ri.Free(bitmap);
			return NULL;
		}

		// reserve vertical space in image
		if (bitmap->rows > *rowHeight) {
			*rowHeight = bitmap->rows;
		}

		// convert glyph image into a 32 bit RGBA image
		src = bitmap->buffer;
		dst = imageOut + (*yOut * imageSize * 4) + *xOut * 4;

		if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
			for (i = 0; i < bitmap->rows; i++) {
				int j;
				unsigned char *_src = src;
				unsigned char *_dst = dst;
				unsigned char mask = 0x80;
				unsigned char val = *_src;
				for (j = 0; j < bitmap->pitch; j++) {
					if (mask == 0x80) {
						val = *_src++;
					}
					if (val & mask) {
						_dst[0] = 0xff;
						_dst[1] = 0xff;
						_dst[2] = 0xff;
						_dst[3] = 0xff;
					}
					mask >>= 1;

					if ( mask == 0 ) {
						mask = 0x80;
					}
					_dst += 4;
				}

				src += bitmap->pitch;
				dst += imageSize * 4;
			}
		} else if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
			int j;
			for (i = 0; i < bitmap->rows; i++) {
				for ( j = 0; j < bitmap->pitch; j++ ) {
					dst[j*4] = 255;
					dst[j*4+1] = 255;
					dst[j*4+2] = 255;
					dst[j*4+3] = src[j];
				}
				src += bitmap->pitch;
				dst += imageSize * 4;
			}
		} else {
			// FT_PIXEL_MODE_BGRA
			int j;
			// swap BGRA src to RGBA dst
			for (i = 0; i < bitmap->rows; i++) {
				for ( j = 0; j < bitmap->pitch; j += 4 ) {
					dst[j+0] = src[j+2];	// red
					dst[j+1] = src[j+1];	// green
					dst[j+2] = src[j+0];	// blue
					dst[j+3] = src[j+3];	// alpha
				}
				src += bitmap->pitch;
				dst += imageSize * 4;
			}
		}

		// store the pixel values in texture coords until the image height is known
		glyph.s = *xOut;
		glyph.t = *yOut;
		glyph.imageWidth = bitmap->width;
		glyph.imageHeight = bitmap->rows;

		*xOut += bitmap->width + 1;

		ri.Free(bitmap->buffer);
		ri.Free(bitmap);
	}

	return &glyph;
}
#endif

static int fdOffset;
static byte	*fdFile;

int readInt( void ) {
	int i = fdFile[fdOffset]+(fdFile[fdOffset+1]<<8)+(fdFile[fdOffset+2]<<16)+(fdFile[fdOffset+3]<<24);
	fdOffset += 4;
	return i;
}

typedef union {
	byte	fred[4];
	float	ffred;
} poor;

float readFloat( void ) {
	poor	me;
#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset+3];
	me.fred[1] = fdFile[fdOffset+2];
	me.fred[2] = fdFile[fdOffset+1];
	me.fred[3] = fdFile[fdOffset+0];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset+0];
	me.fred[1] = fdFile[fdOffset+1];
	me.fred[2] = fdFile[fdOffset+2];
	me.fred[3] = fdFile[fdOffset+3];
#endif
	fdOffset += 4;
	return me.ffred;
}

#ifdef BUILD_FREETYPE
// Q3A's gfx/2d/bigchars some additional symbols, by default these glyphs would just be default missing glyph anyway
unsigned long R_RemapGlyphCharacter( FT_Face face, int charIndex ) {
	switch ( charIndex ) {
		// thick box drawing characters
		// - top
		case 1:
			return 0x2554; // box drawings double down and right
		case 2:
			return 0x2550; // box drawings double horizontal
		case 3:
			return 0x2557; // box drawings double down and left
		// - middle
		case 4:
			return 0x2551; // box drawings double vertical
		case 5:
			return ' '; // blank
		case 6:
			return 0x2551; // box drawings double vertical
		// - bottom
		case 7:
			return 0x255A; // box drawings double up and right
		case 8:
			return 0x2550; // box drawings double horizontal
		case 9:
			return 0x255D; // box drawings double up and left

		//
		// cursors
		//
		case 10:
			return 0xFF3F; // full width low line (underline)
		case 11:
			return 0x2588; // full block
		case 13:
			// Many fonts have up/down pointing triangle but matching left/right triangle
			// are in slot for pointer. So if triangle is missing fallback to pointer.
			if ( FT_Get_Char_Index( face, 0x25B6 ) != 0 ) {
				return 0x25B6; // black right-pointing triangle
			}
			return 0x25BA; // black right-pointing pointer

		//
		// misc
		//
		case 16:
			return 0x301A; // left white square bracket
		case 17:
			return 0x301B; // right white square bracket

		// thin box drawing characters
		// - top
		case 18:
			return 0x250C; // box drawings light down and right
		case 19:
			return 0x2500; // box drawings light horizontal
		case 20:
			return 0x2510; // box drawings light down and left
		// - middle
		case 21:
			return 0x2502; // box drawings light vertical
		case 22:
			return ' '; // blank
		case 23:
			return 0x2502; // box drawings light vertical
		// - bottom
		case 24:
			return 0x2514; // box drawings light up and right
		case 25:
			return 0x2500; // box drawings light horizontal
		case 26:
			return 0x2518; // box drawings light up and left

		//
		// misc
		//
		case 27:
			return 0xFFE3; // full width macron (overline)

		// horzontal bar
		case 29:
			return 0x2576; // box drawings light right
		case 30:
			return 0x2500; // box drawings light horizontal
		case 31:
			return 0x2574; // box drawings light left

		case 127:
			return 0x2B05; // leftward black arrow

		// old slider bar characters
		case 128:
			return 0x255E; // box drawings vertical single and right double
		case 129:
			return 0x2550; // box drawings double horizontal
		case 130:
			return 0x2561; // box drawings vertical single and left double

		// 131 is a half width block but unicode doesn't seem to have one that doesn't include space on left or right side
		// 132 is a transparent square

		case 134:
			return 0x25BC; // black down-pointing triangle
		case 135:
			return 0x25B2; // black up-pointing triangle
		case 136:
			// Many fonts have up/down pointing triangle but matching left/right triangle
			// are in slot for pointer. So if triangle is missing fallback to pointer.
			if ( FT_Get_Char_Index( face, 0x25C0 ) != 0 ) {
				return 0x25C0; // black left-pointing triangle
			}
			return 0x25C4; // black left-pointing pointer

		case 139: // same as index 11
			return 0x2588; // full block

		case 141: // same as index 13
			// Many fonts have up/down pointing triangle but matching left/right triangle
			// are in slot for pointer. So if triangle is missing fallback to pointer.
			if ( FT_Get_Char_Index( face, 0x25B6 ) != 0 ) {
				return 0x25B6; // black right-pointing triangle
			}
			return 0x25BA; // black right-pointing pointer

		default:
			break;
	}

	return charIndex;
}

/*
===============
R_LoadScalableFont
===============
*/
qboolean R_LoadScalableFont( const char *fontName, int pointSize, float borderWidth, qboolean forceAutoHint, fontInfo_t *font ) {
	FT_Face		face;
	int			j, k, xOut, yOut, lastStart, imageNumber;
	int			scaledSize, rowHeight;
	unsigned char *out;
	glyphInfo_t *glyph;
	image_t		*image;
	qhandle_t	h;
	float		max;
	int			imageSize;
	float		dpi;
	float		glyphScale;
	void		*faceData;
	int			i, len;
	char		imageName[MAX_QPATH];
	char		datName[MAX_QPATH];
	char		strippedName[MAX_QPATH];
	float		screenScale;
	int			saveHeight;

	if (ftLibrary == NULL) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType not initialized.\n");
		return qfalse;
	}

	COM_StripExtension( fontName, strippedName, sizeof ( strippedName ) );

	len = ri.FS_ReadFile(fontName, &faceData);
	if (len <= 0) {
		ri.Printf(PRINT_DEVELOPER, "RE_RegisterFont: Unable to read font file '%s'\n", fontName);
		return qfalse;
	}

	// allocate on the stack first in case we fail
	if (FT_New_Memory_Face( ftLibrary, faceData, len, 0, &face )) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType, unable to allocate new face.\n");
		return qfalse;
	}

	FT_Select_Charmap( face, ft_encoding_unicode );

	// point sizes are for a virtual 640x480 screen
	if ( glConfig.vidWidth * 480 > glConfig.vidHeight * 640 ) {
		screenScale = (glConfig.vidHeight / 480.0f);
	} else {
		screenScale = (glConfig.vidWidth / 640.0f);
	}

	// scale dpi based on screen resolution
	dpi = 72.0f * screenScale;

	if (FT_Set_Char_Size( face, pointSize << 6, pointSize << 6, dpi, dpi)) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType, unable to set face char size.\n");
		return qfalse;
	}

	//*font = &registeredFonts[registeredFontCount++];

	// scale image size based on screen height, use the next higher power of two
	for (imageSize = 256; imageSize < 256.0f * dpi / 72.0f; imageSize<<=1);

	// do not exceed maxTextureSize
	if (imageSize > glConfig.maxTextureSize) {
		imageSize = glConfig.maxTextureSize;
	}

	// make a 256x256 image buffer, once it is full, register it, clean it and keep going 
	// until all glyphs are rendered

	out = ri.Malloc(imageSize*imageSize*4);
	if (out == NULL) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: ri.Malloc failure during output image creation.\n");
		return qfalse;
	}
	Com_Memset(out, 0, imageSize*imageSize*4);

	rowHeight = 0;
	xOut = 0;
	yOut = 0;
	i = GLYPH_START;
	lastStart = i;
	imageNumber = 0;

	while ( i <= GLYPH_END + 1 ) {

		if ( i == GLYPH_END + 1 ) {
			// upload/save current image buffer
			glyph = NULL;
		} else {
			glyph = RE_ConstructGlyphInfo(imageSize, out, &xOut, &yOut, &rowHeight, face, R_RemapGlyphCharacter( face, i ), borderWidth, forceAutoHint);
		}

		if (!glyph)  {
			// ran out of room
			// we need to create an image from the bitmap, set all the handles in the glyphs to this point
			// 

			// scale alpha
			scaledSize = imageSize*imageSize*4;
			max = 0;
			for ( k = 0; k < scaledSize; k += 4 ) {
				if (max < out[k+3]) {
					max = out[k+3];
				}
			}

			if (max > 0) {
				max = 255/max;
			}

			for ( k = 0; k < scaledSize; k += 4 ) {
				out[k+3] = ((float)out[k+3] * max);
			}

			// make sure last row doesn't get cut off when reducing image height
			yOut += rowHeight + 1;

			// check if image height can be reduced to a lower power of two
			for ( saveHeight = 1; saveHeight < yOut; saveHeight <<= 1 );

			if ( saveHeight > imageSize ) {
				saveHeight = imageSize;
			}

			Com_sprintf(imageName, sizeof(imageName), "%s_%i_%i.tga", strippedName, imageNumber++, pointSize);
			if(r_saveFontData->integer && !ri.FS_FileExists(imageName)) {
				WriteTGA(imageName, out, imageSize, saveHeight);
			}

			image = R_CreateImage(imageName, out, imageSize, saveHeight, IMGTYPE_COLORALPHA, IMGFLAG_CLAMPTOEDGE|IMGFLAG_MIPMAP, 0 );
			h = RE_RegisterShaderFromImage(imageName, LIGHTMAP_2D, image, qfalse);
			for (j = lastStart; j < i; j++) {
				font->glyphs[j].glyph = h;
				COM_StripExtension(imageName, font->glyphs[j].shaderName, sizeof(font->glyphs[j].shaderName));

				// fill in the texture coords now that image height is known
				font->glyphs[j].s = (float)font->glyphs[j].s / imageSize;
				font->glyphs[j].t = (float)font->glyphs[j].t / saveHeight;
				font->glyphs[j].s2 = font->glyphs[j].s + (float)font->glyphs[j].imageWidth / imageSize;
				font->glyphs[j].t2 = font->glyphs[j].t + (float)font->glyphs[j].imageHeight / saveHeight;
			}
			lastStart = i;
			Com_Memset(out, 0, imageSize*imageSize*4);
			xOut = 0;
			yOut = 0;
			rowHeight = 0;
			if ( i == GLYPH_END + 1 )
				i++;
		} else {
			Com_Memcpy(&font->glyphs[i], glyph, sizeof(glyphInfo_t));
			i++;
		}
	}

	// change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )
	glyphScale = 72.0f / dpi;

	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

	font->glyphScale = glyphScale;
	font->pointSize = pointSize;
	font->flags = FONTFLAG_CURSORS;

	if ( borderWidth > 0 ) {
		font->flags |= FONTFLAG_BORDER;
	}

	Com_sprintf(datName, sizeof(datName), "%s_%i.dat", strippedName, pointSize);
	Q_strncpyz(font->name, datName, sizeof(font->name));
	Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));

	if(r_saveFontData->integer && !ri.FS_FileExists(datName)) {
#if defined Q3_BIG_ENDIAN
		Com_Printf( S_COLOR_YELLOW "WARNING: Cannot write font data on big endian systems\n" );
#else
		// ZTM: FIXME: need to swap for big endian systems
		ri.FS_WriteFile(datName, font, sizeof(fontInfo_t));
#endif
	}

	ri.Free(out);

	ri.FS_FreeFile(faceData);
	return qtrue;
}
#endif

/*
==================
R_GetFont

Get already registered font or load a scalable font or a pre-rendered legacy font.
==================
*/
static qboolean R_GetFont(const char *name, int pointSize, float borderWidth, qboolean forceAutoHint, fontInfo_t *font) {
	int			i;
	char		strippedName[MAX_QPATH];
	char		datName[MAX_QPATH];
#ifdef BUILD_FREETYPE
	char		altName[MAX_QPATH];
	char		*scaleableFontExts[] = { "ttf", "otf", NULL };
	const char	*ext;
#endif

	COM_StripExtension( name, strippedName, sizeof ( strippedName ) );
	Com_sprintf( datName, sizeof ( datName ), "%s_%i.dat", strippedName, pointSize );

	for (i = 0; i < registeredFontCount; i++) {
		if (Q_stricmp(datName, registeredFont[i].name) == 0) {
			Com_Memcpy(font, &registeredFont[i], sizeof(fontInfo_t));
			return qtrue;
		}
	}

	if (registeredFontCount >= MAX_FONTS) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: Too many fonts registered already.\n");
		return qfalse;
	}

#ifdef BUILD_FREETYPE
	ext = COM_GetExtension( name );

	// if there is an extension, check if it's a supported format
	if ( *ext ) {
		for ( i = 0; scaleableFontExts[i] != NULL; i++ ) {
			if ( Q_stricmp( ext, scaleableFontExts[i] ) != 0 ) {
				continue;
			}

			if ( R_LoadScalableFont( name, pointSize, borderWidth, forceAutoHint, font ) ) {
				return qtrue;
			}
			break;
		}
	}

	// fallback to all formats, but don't retry the original extension
	for ( i = 0; scaleableFontExts[i] != NULL; i++ ) {
		if ( *ext && Q_stricmp( ext, scaleableFontExts[i] ) == 0 ) {
			continue;
		}

		Com_sprintf( altName, sizeof (altName), "%s.%s", strippedName, scaleableFontExts[i] );

		if ( R_LoadScalableFont( altName, pointSize, borderWidth, forceAutoHint, font ) ) {
			return qtrue;
		}
	}
#endif
	return qfalse;
}

/*
===============
RE_RegisterFont
===============
*/
void RE_RegisterFont(const char *fontName, int pointSize, float borderWidth, qboolean forceAutoHint, fontInfo_t *font) {
	char		strippedName[MAX_QPATH];

	if ( borderWidth < r_fontBorderWidth->value ) {
		borderWidth = r_fontBorderWidth->value;
	}
	if ( r_fontForceAutoHint->integer ) {
		forceAutoHint = qtrue;
	}

	if (!fontName) {
		ri.Printf(PRINT_ALL, "RE_RegisterFont: called with empty name\n");
		return;
	}

	if (pointSize <= 0) {
		pointSize = 12;
	}

	R_IssuePendingRenderCommands();

	if ( R_GetFont( fontName, pointSize, borderWidth, forceAutoHint, font ) ) {
		return;
	}

	COM_StripExtension( fontName, strippedName, sizeof ( strippedName ) );

	// If there is no extension, assume this is loading one of the legacy fonts
	if( !Q_stricmpn( strippedName, fontName, strlen( fontName ) ) ) {
		if ( R_GetFont( "fonts/fontImage", pointSize, borderWidth, forceAutoHint, font ) ){
			return;
		}
	}

#ifdef BUILD_FREETYPE
	ri.Printf( PRINT_DEVELOPER, "RE_RegisterFont: Failed to register font %s.\n", fontName );
#else
	ri.Printf( PRINT_DEVELOPER, "RE_RegisterFont: Failed to register font %s (Note: FreeType code is not available).\n", fontName );
#endif
}


void R_InitFreeType(void) {
#ifdef BUILD_FREETYPE
	if (FT_Init_FreeType( &ftLibrary )) {
		ri.Printf(PRINT_WARNING, "R_InitFreeType: Unable to initialize FreeType.\n");
	}
#endif

	registeredFontCount = 0;
}


void R_DoneFreeType(void) {
#ifdef BUILD_FREETYPE
	if (ftLibrary) {
		FT_Done_FreeType( ftLibrary );
		ftLibrary = NULL;
	}
#endif

	registeredFontCount = 0;
}

