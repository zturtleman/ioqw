/*
=======================================================================================================================================
ioquake3 png decoder and encoder
Copyright (C) 2007, 2008 Joerg Dietrich
Copyright (C) 2011 Zack Middleton

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

#include "tr_common.h"
#include "../qcommon/puff.h"

// we could limit the png size to a lower value here
#ifndef INT_MAX
#define INT_MAX 0x1fffffff
#endif

/*
=======================================================================================================================================

	PNG LOADING

=======================================================================================================================================
*/

// Quake 3 image format: RGBA
#define Q3IMAGE_BYTESPERPIXEL (4)
// PNG specifications

// The first 8 Bytes of every PNG-File are a fixed signature to identify the file as a PNG
#define PNG_Signature "\x89\x50\x4E\x47\xD\xA\x1A\xA"
#define PNG_Signature_Size (8)
// After the signature diverse chunks follow. A chunk consists of a header and if Length is bigger than 0 a body and a CRC of the body follow
struct PNG_ChunkHeader {
	uint32_t Length;
	uint32_t Type;
};

#define PNG_ChunkHeader_Size (8)
typedef uint32_t PNG_ChunkCRC;
#define PNG_ChunkCRC_Size (4)
// We use the following ChunkTypes. All others are ignored
#define MAKE_CHUNKTYPE(a, b, c, d) (((a) << 24)|((b) << 16)|((c) << 8)|((d)))

#define PNG_ChunkType_IHDR MAKE_CHUNKTYPE('I', 'H', 'D', 'R')
#define PNG_ChunkType_PLTE MAKE_CHUNKTYPE('P', 'L', 'T', 'E')
#define PNG_ChunkType_IDAT MAKE_CHUNKTYPE('I', 'D', 'A', 'T')
#define PNG_ChunkType_IEND MAKE_CHUNKTYPE('I', 'E', 'N', 'D')
#define PNG_ChunkType_tRNS MAKE_CHUNKTYPE('t', 'R', 'N', 'S')
// Per specification the first chunk after the signature SHALL be IHDR
struct PNG_Chunk_IHDR {
	uint32_t Width;
	uint32_t Height;
	uint8_t BitDepth;
	uint8_t ColourType;
	uint8_t CompressionMethod;
	uint8_t FilterMethod;
	uint8_t InterlaceMethod;
};

#define PNG_Chunk_IHDR_Size (13)
// ColourTypes
#define PNG_ColourType_Grey (0)
#define PNG_ColourType_True (2)
#define PNG_ColourType_Indexed (3)
#define PNG_ColourType_GreyAlpha (4)
#define PNG_ColourType_TrueAlpha (6)
// number of colour components

// Grey      : 1 grey
// True      : 1 R, 1 G, 1 B
// Indexed   : 1 index
// GreyAlpha : 1 grey, 1 alpha
// TrueAlpha : 1 R, 1 G, 1 B, 1 alpha
#define PNG_NumColourComponents_Grey (1)
#define PNG_NumColourComponents_True (3)
#define PNG_NumColourComponents_Indexed (1)
#define PNG_NumColourComponents_GreyAlpha (2)
#define PNG_NumColourComponents_TrueAlpha (4)
// For the different ColourTypes different BitDepths are specified
#define PNG_BitDepth_1 (1)
#define PNG_BitDepth_2 (2)
#define PNG_BitDepth_4 (4)
#define PNG_BitDepth_8 (8)
#define PNG_BitDepth_16 (16)
// Only one valid CompressionMethod is standardized
#define PNG_CompressionMethod_0 (0)
// Only one valid FilterMethod is currently standardized
#define PNG_FilterMethod_0 (0)
// This FilterMethod defines 5 FilterTypes
#define PNG_FilterType_None (0)
#define PNG_FilterType_Sub (1)
#define PNG_FilterType_Up (2)
#define PNG_FilterType_Average (3)
#define PNG_FilterType_Paeth (4)
// Two InterlaceMethods are standardized:
// 0 - NonInterlaced
// 1 - Interlaced
#define PNG_InterlaceMethod_NonInterlaced (0)
#define PNG_InterlaceMethod_Interlaced (1)
// The Adam7 interlace method uses 7 passes
#define PNG_Adam7_NumPasses (7)
// The compressed data starts with a header ...
struct PNG_ZlibHeader {
	uint8_t CompressionMethod;
	uint8_t Flags;
};

#define PNG_ZlibHeader_Size (2)
// ... and is followed by a check value
#define PNG_ZlibCheckValue_Size (4)
// Some support functions for buffered files follow

// buffered file representation
struct BufferedFile {
	byte *Buffer;
	int Length;
	byte *Ptr;
	int BytesLeft;
};

/*
=======================================================================================================================================
ReadBufferedFile

Read a file into a buffer.
=======================================================================================================================================
*/
static struct BufferedFile *ReadBufferedFile(const char *name) {
	struct BufferedFile *BF;
	union {
		byte *b;
		void *v;
	} buffer;

	// input verification
	if (!name) {
		return (NULL);
	}
	// allocate control struct
	BF = ri.Malloc(sizeof(struct BufferedFile));

	if (!BF) {
		return (NULL);
	}
	// initialize the structs components
	BF->Length = 0;
	BF->Buffer = NULL;
	BF->Ptr = NULL;
	BF->BytesLeft = 0;
	// read the file
	BF->Length = ri.FS_ReadFile((char *)name, &buffer.v);
	BF->Buffer = buffer.b;
	// did we get it? Is it big enough?
	if (!(BF->Buffer && (BF->Length > 0))) {
		ri.Free(BF);
		return (NULL);
	}
	// set the pointers and counters
	BF->Ptr = BF->Buffer;
	BF->BytesLeft = BF->Length;

	return (BF);
}

/*
=======================================================================================================================================
CloseBufferedFile

Close a buffered file.
=======================================================================================================================================
*/
static void CloseBufferedFile(struct BufferedFile *BF) {

	if (BF) {
		if (BF->Buffer) {
			ri.FS_FreeFile(BF->Buffer);
		}

		ri.Free(BF);
	}
}

/*
=======================================================================================================================================
BufferedFileRead

Get a pointer to the requested bytes.
=======================================================================================================================================
*/
static void *BufferedFileRead(struct BufferedFile *BF, unsigned Length) {
	void *RetVal;

	// input verification
	if (!(BF && Length)) {
		return (NULL);
	}
	// not enough bytes left
	if (Length > BF->BytesLeft) {
		return (NULL);
	}
	// the pointer to the requested data
	RetVal = BF->Ptr;
	// raise the pointer and counter
	BF->Ptr += Length;
	BF->BytesLeft -= Length;

	return (RetVal);
}

/*
=======================================================================================================================================
BufferedFileRewind

Rewind the buffer.
=======================================================================================================================================
*/
static qboolean BufferedFileRewind(struct BufferedFile *BF, unsigned Offset) {
	unsigned BytesRead;

	// input verification
	if (!BF) {
		return (qfalse);
	}
	// special trick to rewind to the beginning of the buffer
	if (Offset == (unsigned)-1) {
		BF->Ptr = BF->Buffer;
		BF->BytesLeft = BF->Length;
		return (qtrue);
	}
	// how many bytes do we have already read?
	BytesRead = BF->Ptr - BF->Buffer;
	// we can only rewind to the beginning of the BufferedFile
	if (Offset > BytesRead) {
		return (qfalse);
	}
	// lower the pointer and counter
	BF->Ptr -= Offset;
	BF->BytesLeft += Offset;

	return (qtrue);
}

/*
=======================================================================================================================================
BufferedFileSkip

Skip some bytes.
=======================================================================================================================================
*/
static qboolean BufferedFileSkip(struct BufferedFile *BF, unsigned Offset) {

	// input verification
	if (!BF) {
		return (qfalse);
	}
	// we can only skip to the end of the BufferedFile
	if (Offset > BF->BytesLeft) {
		return (qfalse);
	}
	// lower the pointer and counter
	BF->Ptr += Offset;
	BF->BytesLeft -= Offset;

	return (qtrue);
}

/*
=======================================================================================================================================
FindChunk

Find a chunk.
=======================================================================================================================================
*/
static qboolean FindChunk(struct BufferedFile *BF, uint32_t ChunkType) {
	struct PNG_ChunkHeader *CH;
	uint32_t Length;
	uint32_t Type;

	// input verification
	if (!BF) {
		return (qfalse);
	}
	// cycle trough the chunks
	while (qtrue) {
		// read the chunk-header
		CH = BufferedFileRead(BF, PNG_ChunkHeader_Size);

		if (!CH) {
			return (qfalse);
		}
		// do not swap the original types, they might be needed later
		Length = BigLong(CH->Length);
		Type = BigLong(CH->Type);
		// we found it!
		if (Type == ChunkType) {
			// rewind to the start of the chunk
			BufferedFileRewind(BF, PNG_ChunkHeader_Size);
			break;
		} else {
			// skip the rest of the chunk
			if (Length) {
				if (!BufferedFileSkip(BF, Length + PNG_ChunkCRC_Size)) {
					return (qfalse);
				}
			}
		}
	}

	return (qtrue);
}

/*
=======================================================================================================================================
DecompressIDATs

Decompress all IDATs.
=======================================================================================================================================
*/
static uint32_t DecompressIDATs(struct BufferedFile *BF, uint8_t **Buffer) {
	uint8_t *DecompressedData;
	uint32_t DecompressedDataLength;
	uint8_t *CompressedData;
	uint8_t *CompressedDataPtr;
	uint32_t CompressedDataLength;
	struct PNG_ChunkHeader *CH;
	uint32_t Length;
	uint32_t Type;
	int BytesToRewind;
	int32_t puffResult;
	uint8_t *puffDest;
	uint32_t puffDestLen;
	uint8_t *puffSrc;
	uint32_t puffSrcLen;

	// input verification
	if (!(BF && Buffer)) {
		return (-1);
	}
	// some zeroing
	DecompressedData = NULL;
	*Buffer = DecompressedData;

	CompressedData = NULL;
	CompressedDataLength = 0;

	BytesToRewind = 0;
	// find the first IDAT chunk
	if (!FindChunk(BF, PNG_ChunkType_IDAT)) {
		return (-1);
	}
	// count the size of the uncompressed data
	while (qtrue) {
		// read chunk header
		CH = BufferedFileRead(BF, PNG_ChunkHeader_Size);

		if (!CH) {
			// rewind to the start of this adventure and return unsuccessful
			BufferedFileRewind(BF, BytesToRewind);
			return (-1);
		}
		// length and Type of chunk
		Length = BigLong(CH->Length);
		Type = BigLong(CH->Type);
		// we have reached the end of the IDAT chunks
		if (!(Type == PNG_ChunkType_IDAT)) {
			BufferedFileRewind(BF, PNG_ChunkHeader_Size);
			break;
		}
		// add chunk header to count
		BytesToRewind += PNG_ChunkHeader_Size;
		// skip to next chunk
		if (Length) {
			if (!BufferedFileSkip(BF, Length + PNG_ChunkCRC_Size)) {
				BufferedFileRewind(BF, BytesToRewind);
				return (-1);
			}

			BytesToRewind += Length + PNG_ChunkCRC_Size;
			CompressedDataLength += Length;
		}
	}

	BufferedFileRewind(BF, BytesToRewind);

	CompressedData = ri.Malloc(CompressedDataLength);

	if (!CompressedData) {
		return (-1);
	}

	CompressedDataPtr = CompressedData;
	// collect the compressed Data
	while (qtrue) {
		// read chunk header
		CH = BufferedFileRead(BF, PNG_ChunkHeader_Size);

		if (!CH) {
			ri.Free(CompressedData);
			return (-1);
		}
		// Length and Type of chunk
		Length = BigLong(CH->Length);
		Type = BigLong(CH->Type);
		// we have reached the end of the IDAT chunks
		if (!(Type == PNG_ChunkType_IDAT)) {
			BufferedFileRewind(BF, PNG_ChunkHeader_Size);
			break;
		}
		// copy the Data
		if (Length) {
			uint8_t *OrigCompressedData;

			OrigCompressedData = BufferedFileRead(BF, Length);

			if (!OrigCompressedData) {
				ri.Free(CompressedData);
				return (-1);
			}

			if (!BufferedFileSkip(BF, PNG_ChunkCRC_Size)) {
				ri.Free(CompressedData);
				return (-1);
			}

			memcpy(CompressedDataPtr, OrigCompressedData, Length);

			CompressedDataPtr += Length;
		}
	}
	// let puff() calculate the decompressed data length
	puffDest = NULL;
	puffDestLen = 0;
	// the zlib header and checkvalue don't belong to the compressed data
	puffSrc = CompressedData + PNG_ZlibHeader_Size;
	puffSrcLen = CompressedDataLength - PNG_ZlibHeader_Size - PNG_ZlibCheckValue_Size;
	// first puff() to calculate the size of the uncompressed data
	puffResult = puff(puffDest, &puffDestLen, puffSrc, &puffSrcLen);

	if (!((puffResult == 0) && (puffDestLen > 0))) {
		ri.Free(CompressedData);
		return (-1);
	}
	// allocate the buffer for the uncompressed data
	DecompressedData = ri.Malloc(puffDestLen);

	if (!DecompressedData) {
		ri.Free(CompressedData);
		return (-1);
	}
	// set the input again in case something was changed by the last puff()
	puffDest = DecompressedData;
	puffSrc = CompressedData + PNG_ZlibHeader_Size;
	puffSrcLen = CompressedDataLength - PNG_ZlibHeader_Size - PNG_ZlibCheckValue_Size;
	// decompression puff()
	puffResult = puff(puffDest, &puffDestLen, puffSrc, &puffSrcLen);
	// the compressed data is not needed anymore
	ri.Free(CompressedData);
	// check if the last puff() was successful
	if (!((puffResult == 0) && (puffDestLen > 0))) {
		ri.Free(DecompressedData);
		return (-1);
	}
	// set the output of this function
	DecompressedDataLength = puffDestLen;
	*Buffer = DecompressedData;

	return (DecompressedDataLength);
}

/*
=======================================================================================================================================
PredictPaeth

The Paeth predictor.
=======================================================================================================================================
*/
static uint8_t PredictPaeth(uint8_t a, uint8_t b, uint8_t c) {
	// a == Left
	// b == Up
	// c == UpLeft
	uint8_t Pr;
	int p;
	int pa, pb, pc;

	p = ((int)a) + ((int)b) - ((int)c);
	pa = abs(p - ((int)a));
	pb = abs(p - ((int)b));
	pc = abs(p - ((int)c));

	if ((pa <= pb) && (pa <= pc)) {
		Pr = a;
	} else if (pb <= pc) {
		Pr = b;
	} else {
		Pr = c;
	}

	return (Pr);
}

/*
=======================================================================================================================================
UnfilterImage

Reverse the filters.
=======================================================================================================================================
*/
static qboolean UnfilterImage(uint8_t *DecompressedData, uint32_t ImageHeight, uint32_t BytesPerScanline, uint32_t BytesPerPixel) {
	uint8_t *DecompPtr;
	uint8_t FilterType;
	uint8_t *PixelLeft, *PixelUp, *PixelUpLeft;
	uint32_t w, h, p;
	// some zeros for the filters
	uint8_t Zeros[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	// input verification
	if (!(DecompressedData && BytesPerPixel)) {
		return (qfalse);
	}
	// imageHeight and BytesPerScanline can be zero in small interlaced images
	if ((!ImageHeight) || (!BytesPerScanline)) {
		return (qtrue);
	}
	// set the pointer to the start of the decompressed Data
	DecompPtr = DecompressedData;
	// un-filtering is done in place

	// go trough all scanlines
	for (h = 0; h < ImageHeight; h++) {
		// every scanline starts with a FilterType byte
		FilterType = *DecompPtr;
		DecompPtr++;
		// left pixel of the first byte in a scanline is zero
		PixelLeft = Zeros;
		// set PixelUp to previous line only if we are on the second line or above

		// plus one byte for the FilterType
		if (h > 0) {
			PixelUp = DecompPtr - (BytesPerScanline + 1);
		} else {
			PixelUp = Zeros;
		}
		// the pixel left to the first pixel of the previous scanline is zero too
		PixelUpLeft = Zeros;
		// cycle trough all pixels of the scanline
		for (w = 0; w < (BytesPerScanline / BytesPerPixel); w++) {
			// cycle trough the bytes of the pixel
			for (p = 0; p < BytesPerPixel; p++) {
				switch (FilterType) {
					case PNG_FilterType_None:
					{
						// the byte is unfiltered
						break;
					}

					case PNG_FilterType_Sub:
					{
						DecompPtr[p] += PixelLeft[p];
						break;
					}

					case PNG_FilterType_Up:
					{
						DecompPtr[p] += PixelUp[p];
						break;
					}

					case PNG_FilterType_Average:
					{
						DecompPtr[p] += ((uint8_t)((((uint16_t)PixelLeft[p]) + ((uint16_t)PixelUp[p])) / 2));
						break;
					}

					case PNG_FilterType_Paeth:
					{
						DecompPtr[p] += PredictPaeth(PixelLeft[p], PixelUp[p], PixelUpLeft[p]);
						break;
					}

					default:
					{
						return (qfalse);
					}
				}
			}

			PixelLeft = DecompPtr;
			// we only have an upleft pixel if we are on the second line or above
			if (h > 0) {
				PixelUpLeft = DecompPtr - (BytesPerScanline + 1);
			}
			// skip to the next pixel
			DecompPtr += BytesPerPixel;
			// we only have a previous line if we are on the second line and above
			if (h > 0) {
				PixelUp = DecompPtr - (BytesPerScanline + 1);
			}
		}
	}

	return (qtrue);
}

/*
=======================================================================================================================================
ConvertPixel

Convert a raw input pixel to Quake 3 RGA format.
=======================================================================================================================================
*/
static qboolean ConvertPixel(struct PNG_Chunk_IHDR *IHDR, byte *OutPtr, uint8_t *DecompPtr, qboolean HasTransparentColour, uint8_t *TransparentColour, uint8_t *OutPal) {

	// input verification
	if (!(IHDR && OutPtr && DecompPtr && TransparentColour && OutPal)) {
		return (qfalse);
	}

	switch (IHDR->ColourType) {
		case PNG_ColourType_Grey:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_1:
				case PNG_BitDepth_2:
				case PNG_BitDepth_4:
				{
					uint8_t Step;
					uint8_t GreyValue;

					Step = 0xFF / ((1 << IHDR->BitDepth) - 1);
					GreyValue = DecompPtr[0] * Step;

					OutPtr[0] = GreyValue;
					OutPtr[1] = GreyValue;
					OutPtr[2] = GreyValue;
					OutPtr[3] = 0xFF;
					// grey supports full transparency for one specified colour
					if (HasTransparentColour) {
						if (TransparentColour[1] == DecompPtr[0]) {
							OutPtr[3] = 0x00;
						}
					}

					break;
				}

				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[0];
					OutPtr[2] = DecompPtr[0];
					OutPtr[3] = 0xFF;
					// grey supports full transparency for one specified colour
					if (HasTransparentColour) {
						if (IHDR->BitDepth == PNG_BitDepth_8) {
							if (TransparentColour[1] == DecompPtr[0]) {
								OutPtr[3] = 0x00;
							}
						} else {
							if ((TransparentColour[0] == DecompPtr[0]) && (TransparentColour[1] == DecompPtr[1])) {
								OutPtr[3] = 0x00;
							}
						}
					}

					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_True:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				{
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[1];
					OutPtr[2] = DecompPtr[2];
					OutPtr[3] = 0xFF;
					// true supports full transparency for one specified colour
					if (HasTransparentColour) {
						if ((TransparentColour[1] == DecompPtr[0]) && (TransparentColour[3] == DecompPtr[1]) && (TransparentColour[5] == DecompPtr[2])) {
							OutPtr[3] = 0x00;
						}
					}

					break;
				}

				case PNG_BitDepth_16:
				{
					// we use only the upper byte
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[2];
					OutPtr[2] = DecompPtr[4];
					OutPtr[3] = 0xFF;
					// true supports full transparency for one specified colour
					if (HasTransparentColour) {
						if ((TransparentColour[0] == DecompPtr[0]) && (TransparentColour[1] == DecompPtr[1]) && (TransparentColour[2] == DecompPtr[2]) && (TransparentColour[3] == DecompPtr[3]) && (TransparentColour[4] == DecompPtr[4]) && (TransparentColour[5] == DecompPtr[5])) {
							OutPtr[3] = 0x00;
						}
					}

					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_Indexed:
		{
			OutPtr[0] = OutPal[DecompPtr[0] * Q3IMAGE_BYTESPERPIXEL + 0];
			OutPtr[1] = OutPal[DecompPtr[0] * Q3IMAGE_BYTESPERPIXEL + 1];
			OutPtr[2] = OutPal[DecompPtr[0] * Q3IMAGE_BYTESPERPIXEL + 2];
			OutPtr[3] = OutPal[DecompPtr[0] * Q3IMAGE_BYTESPERPIXEL + 3];
			break;
		}

		case PNG_ColourType_GreyAlpha:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				{
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[0];
					OutPtr[2] = DecompPtr[0];
					OutPtr[3] = DecompPtr[1];

					break;
				}

				case PNG_BitDepth_16:
				{
					// we use only the upper byte
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[0];
					OutPtr[2] = DecompPtr[0];
					OutPtr[3] = DecompPtr[2];

					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_TrueAlpha:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				{
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[1];
					OutPtr[2] = DecompPtr[2];
					OutPtr[3] = DecompPtr[3];
					break;
				}

				case PNG_BitDepth_16:
				{
					// we use only the upper byte
					OutPtr[0] = DecompPtr[0];
					OutPtr[1] = DecompPtr[2];
					OutPtr[2] = DecompPtr[4];
					OutPtr[3] = DecompPtr[6];
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		default:
		{
			return (qfalse);
		}
	}

	return (qtrue);
}

/*
=======================================================================================================================================
DecodeImageNonInterlaced

Decode a non-interlaced image.
=======================================================================================================================================
*/
static qboolean DecodeImageNonInterlaced(struct PNG_Chunk_IHDR *IHDR, byte *OutBuffer, uint8_t *DecompressedData, uint32_t DecompressedDataLength, qboolean HasTransparentColour, uint8_t *TransparentColour, uint8_t *OutPal) {
	uint32_t IHDR_Width;
	uint32_t IHDR_Height;
	uint32_t BytesPerScanline, BytesPerPixel, PixelsPerByte;
	uint32_t w, h, p;
	byte *OutPtr;
	uint8_t *DecompPtr;

	// input verification
	if (!(IHDR && OutBuffer && DecompressedData && DecompressedDataLength && TransparentColour && OutPal)) {
		return (qfalse);
	}
	// byte swapping
	IHDR_Width = BigLong(IHDR->Width);
	IHDR_Height = BigLong(IHDR->Height);
	// information for un-filtering
	switch (IHDR->ColourType) {
		case PNG_ColourType_Grey:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_1:
				case PNG_BitDepth_2:
				case PNG_BitDepth_4:
				{
					BytesPerPixel = 1;
					PixelsPerByte = 8 / IHDR->BitDepth;
					break;
				}

				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_Grey;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_True:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_True;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_Indexed:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_1:
				case PNG_BitDepth_2:
				case PNG_BitDepth_4:
				{
					BytesPerPixel = 1;
					PixelsPerByte = 8 / IHDR->BitDepth;
					break;
				}

				case PNG_BitDepth_8:
				{
					BytesPerPixel = PNG_NumColourComponents_Indexed;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_GreyAlpha:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_GreyAlpha;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_TrueAlpha:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_TrueAlpha;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		default:
		{
			return (qfalse);
		}
	}
	// calculate the size of one scanline
	BytesPerScanline = (IHDR_Width * BytesPerPixel + (PixelsPerByte - 1)) / PixelsPerByte;
	// check if we have enough data for the whole image
	if (!(DecompressedDataLength == ((BytesPerScanline + 1) * IHDR_Height))) {
		return (qfalse);
	}
	// unfilter the image
	if (!UnfilterImage(DecompressedData, IHDR_Height, BytesPerScanline, BytesPerPixel)) {
		return (qfalse);
	}
	// set the working pointers to the beginning of the buffers
	OutPtr = OutBuffer;
	DecompPtr = DecompressedData;
	// create the output image
	for (h = 0; h < IHDR_Height; h++) {
		// count the pixels on the scanline for those multipixel bytes
		uint32_t CurrPixel;

		// skip FilterType
		DecompPtr++;
		// reset the pixel count
		CurrPixel = 0;

		for (w = 0; w < (BytesPerScanline / BytesPerPixel); w++) {
			if (PixelsPerByte > 1) {
				uint8_t Mask;
				uint32_t Shift;
				uint8_t SinglePixel;

				for (p = 0; p < PixelsPerByte; p++) {
					if (CurrPixel < IHDR_Width) {
						Mask = (1 << IHDR->BitDepth) - 1;
						Shift = (PixelsPerByte - 1 - p) * IHDR->BitDepth;

						SinglePixel = ((DecompPtr[0] & (Mask << Shift)) >> Shift);

						if (!ConvertPixel(IHDR, OutPtr, &SinglePixel, HasTransparentColour, TransparentColour, OutPal)) {
							return (qfalse);
						}

						OutPtr += Q3IMAGE_BYTESPERPIXEL;
						CurrPixel++;
					}
				}
			} else {
				if (!ConvertPixel(IHDR, OutPtr, DecompPtr, HasTransparentColour, TransparentColour, OutPal)) {
					return (qfalse);
				}

				OutPtr += Q3IMAGE_BYTESPERPIXEL;
			}

			DecompPtr += BytesPerPixel;
		}
	}

	return (qtrue);
}

/*
=======================================================================================================================================
DecodeImageInterlaced

Decode an interlaced image.
=======================================================================================================================================
*/
static qboolean DecodeImageInterlaced(struct PNG_Chunk_IHDR *IHDR, byte *OutBuffer, uint8_t *DecompressedData, uint32_t DecompressedDataLength, qboolean HasTransparentColour, uint8_t *TransparentColour, uint8_t *OutPal) {
	uint32_t IHDR_Width;
	uint32_t IHDR_Height;
	uint32_t BytesPerScanline[PNG_Adam7_NumPasses], BytesPerPixel, PixelsPerByte;
	uint32_t PassWidth[PNG_Adam7_NumPasses], PassHeight[PNG_Adam7_NumPasses];
	uint32_t WSkip[PNG_Adam7_NumPasses], WOffset[PNG_Adam7_NumPasses], HSkip[PNG_Adam7_NumPasses], HOffset[PNG_Adam7_NumPasses];
	uint32_t w, h, p, a;
	byte *OutPtr;
	uint8_t *DecompPtr;
	uint32_t TargetLength;

	// input verification
	if (!(IHDR && OutBuffer && DecompressedData && DecompressedDataLength && TransparentColour && OutPal)) {
		return (qfalse);
	}
	// byte swapping
	IHDR_Width = BigLong(IHDR->Width);
	IHDR_Height = BigLong(IHDR->Height);
	// skip and Offset for the passes
	WSkip[0] = 8;
	WOffset[0] = 0;
	HSkip[0] = 8;
	HOffset[0] = 0;

	WSkip[1] = 8;
	WOffset[1] = 4;
	HSkip[1] = 8;
	HOffset[1] = 0;

	WSkip[2] = 4;
	WOffset[2] = 0;
	HSkip[2] = 8;
	HOffset[2] = 4;

	WSkip[3] = 4;
	WOffset[3] = 2;
	HSkip[3] = 4;
	HOffset[3] = 0;

	WSkip[4] = 2;
	WOffset[4] = 0;
	HSkip[4] = 4;
	HOffset[4] = 2;

	WSkip[5] = 2;
	WOffset[5] = 1;
	HSkip[5] = 2;
	HOffset[5] = 0;

	WSkip[6] = 1;
	WOffset[6] = 0;
	HSkip[6] = 2;
	HOffset[6] = 1;
	// calculate the sizes of the passes
	PassWidth[0] = (IHDR_Width + 7) / 8;
	PassHeight[0] = (IHDR_Height + 7) / 8;

	PassWidth[1] = (IHDR_Width + 3) / 8;
	PassHeight[1] = (IHDR_Height + 7) / 8;

	PassWidth[2] = (IHDR_Width + 3) / 4;
	PassHeight[2] = (IHDR_Height + 3) / 8;

	PassWidth[3] = (IHDR_Width + 1) / 4;
	PassHeight[3] = (IHDR_Height + 3) / 4;

	PassWidth[4] = (IHDR_Width + 1) / 2;
	PassHeight[4] = (IHDR_Height + 1) / 4;

	PassWidth[5] = (IHDR_Width + 0) / 2;
	PassHeight[5] = (IHDR_Height + 1) / 2;

	PassWidth[6] = (IHDR_Width + 0) / 1;
	PassHeight[6] = (IHDR_Height + 0) / 2;
	// information for un-filtering
	switch (IHDR->ColourType) {
		case PNG_ColourType_Grey:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_1:
				case PNG_BitDepth_2:
				case PNG_BitDepth_4:
				{
					BytesPerPixel = 1;
					PixelsPerByte = 8 / IHDR->BitDepth;
					break;
				}

				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_Grey;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_True:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_True;
					PixelsPerByte = 1;

					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_Indexed:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_1:
				case PNG_BitDepth_2:
				case PNG_BitDepth_4:
				{
					BytesPerPixel = 1;
					PixelsPerByte = 8 / IHDR->BitDepth;
					break;
				}

				case PNG_BitDepth_8:
				{
					BytesPerPixel = PNG_NumColourComponents_Indexed;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_GreyAlpha:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_GreyAlpha;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		case PNG_ColourType_TrueAlpha:
		{
			switch (IHDR->BitDepth) {
				case PNG_BitDepth_8:
				case PNG_BitDepth_16:
				{
					BytesPerPixel = (IHDR->BitDepth / 8) * PNG_NumColourComponents_TrueAlpha;
					PixelsPerByte = 1;
					break;
				}

				default:
				{
					return (qfalse);
				}
			}

			break;
		}

		default:
		{
			return (qfalse);
		}
	}
	// calculate the size of the scanlines per pass
	for (a = 0; a < PNG_Adam7_NumPasses; a++) {
		BytesPerScanline[a] = (PassWidth[a] * BytesPerPixel + (PixelsPerByte - 1)) / PixelsPerByte;
	}
	// calculate the size of all passes
	TargetLength = 0;

	for (a = 0; a < PNG_Adam7_NumPasses; a++) {
		TargetLength += ((BytesPerScanline[a] + (BytesPerScanline[a] ? 1 : 0)) * PassHeight[a]);
	}
	// check if we have enough data for the whole image
	if (!(DecompressedDataLength == TargetLength)) {
		return (qfalse);
	}
	// unfilter the image
	DecompPtr = DecompressedData;

	for (a = 0; a < PNG_Adam7_NumPasses; a++) {
		if (!UnfilterImage(DecompPtr, PassHeight[a], BytesPerScanline[a], BytesPerPixel)) {
			return (qfalse);
		}

		DecompPtr += ((BytesPerScanline[a] + (BytesPerScanline[a] ? 1 : 0)) * PassHeight[a]);
	}
	// set the working pointers to the beginning of the buffers
	DecompPtr = DecompressedData;
	// create the output image
	for (a = 0; a < PNG_Adam7_NumPasses; a++) {
		for (h = 0; h < PassHeight[a]; h++) {
			// count the pixels on the scanline for those multipixel bytes
			uint32_t CurrPixel;

			// skip FilterType, but only when the pass has a width bigger than zero
			if (BytesPerScanline[a]) {
				DecompPtr++;
			}
			// reset the pixel count
			CurrPixel = 0;

			for (w = 0; w < (BytesPerScanline[a] / BytesPerPixel); w++) {
				if (PixelsPerByte > 1) {
					uint8_t Mask;
					uint32_t Shift;
					uint8_t SinglePixel;

					for (p = 0; p < PixelsPerByte; p++) {
						if (CurrPixel < PassWidth[a]) {
							Mask = (1 << IHDR->BitDepth) - 1;
							Shift = (PixelsPerByte - 1 - p) * IHDR->BitDepth;
							SinglePixel = ((DecompPtr[0] & (Mask << Shift)) >> Shift);
							OutPtr = OutBuffer + (((((h * HSkip[a]) + HOffset[a]) * IHDR_Width) + ((CurrPixel * WSkip[a]) + WOffset[a])) * Q3IMAGE_BYTESPERPIXEL);

							if (!ConvertPixel(IHDR, OutPtr, &SinglePixel, HasTransparentColour, TransparentColour, OutPal)) {
								return (qfalse);
							}

							CurrPixel++;
						}
					}
				} else {
					OutPtr = OutBuffer + (((((h * HSkip[a]) + HOffset[a]) * IHDR_Width) + ((w * WSkip[a]) + WOffset[a])) * Q3IMAGE_BYTESPERPIXEL);

					if (!ConvertPixel(IHDR, OutPtr, DecompPtr, HasTransparentColour, TransparentColour, OutPal)) {
						return (qfalse);
					}
				}

				DecompPtr += BytesPerPixel;
			}
		}
	}

	return (qtrue);
}

/*
=======================================================================================================================================
R_LoadPNG

The PNG loader.
=======================================================================================================================================
*/
void R_LoadPNG(const char *name, byte **pic, int *width, int *height) {
	struct BufferedFile *ThePNG;
	byte *OutBuffer;
	uint8_t *Signature;
	struct PNG_ChunkHeader *CH;
	uint32_t ChunkHeaderLength;
	uint32_t ChunkHeaderType;
	struct PNG_Chunk_IHDR *IHDR;
	uint32_t IHDR_Width;
	uint32_t IHDR_Height;
	PNG_ChunkCRC *CRC;
	uint8_t *InPal;
	uint8_t *DecompressedData;
	uint32_t DecompressedDataLength;
	uint32_t i;
	// palette with 256 RGBA entries
	uint8_t OutPal[1024];
	// transparent colour from the tRNS chunk
	qboolean HasTransparentColour = qfalse;
	uint8_t TransparentColour[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	// input verification
	if (!(name && pic)) {
		return;
	}
	// zero out return values
	*pic = NULL;

	if (width) {
		*width = 0;
	}

	if (height) {
		*height = 0;
	}
	// read the file
	ThePNG = ReadBufferedFile(name);

	if (!ThePNG) {
		return;
	}
	// read the siganture of the file
	Signature = BufferedFileRead(ThePNG, PNG_Signature_Size);

	if (!Signature) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// is it a PNG?
	if (memcmp(Signature, PNG_Signature, PNG_Signature_Size)) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// read the first chunk-header
	CH = BufferedFileRead(ThePNG, PNG_ChunkHeader_Size);

	if (!CH) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// PNG multi-byte types are in Big Endian
	ChunkHeaderLength = BigLong(CH->Length);
	ChunkHeaderType = BigLong(CH->Type);
	// check if the first chunk is an IHDR
	if (!((ChunkHeaderType == PNG_ChunkType_IHDR) && (ChunkHeaderLength == PNG_Chunk_IHDR_Size))) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// read the IHDR
	IHDR = BufferedFileRead(ThePNG, PNG_Chunk_IHDR_Size);

	if (!IHDR) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// read the CRC for IHDR
	CRC = BufferedFileRead(ThePNG, PNG_ChunkCRC_Size);

	if (!CRC) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// here we could check the CRC if we wanted to

	// multi-byte type swapping
	IHDR_Width = BigLong(IHDR->Width);
	IHDR_Height = BigLong(IHDR->Height);
	// check if Width and Height are valid
	if (!((IHDR_Width > 0) && (IHDR_Height > 0)) || IHDR_Width > INT_MAX / Q3IMAGE_BYTESPERPIXEL / IHDR_Height) {
		CloseBufferedFile(ThePNG);
		ri.Printf(PRINT_WARNING, "%s: invalid image size\n", name);
		return;
	}
	// do we need to check if the dimensions of the image are valid for Quake3?

	// check if CompressionMethod and FilterMethod are valid
	if (!((IHDR->CompressionMethod == PNG_CompressionMethod_0) && (IHDR->FilterMethod == PNG_FilterMethod_0))) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// check if InterlaceMethod is valid
	if (!((IHDR->InterlaceMethod == PNG_InterlaceMethod_NonInterlaced) || (IHDR->InterlaceMethod == PNG_InterlaceMethod_Interlaced))) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// read palette for an indexed image
	if (IHDR->ColourType == PNG_ColourType_Indexed) {
		// we need the palette first
		if (!FindChunk(ThePNG, PNG_ChunkType_PLTE)) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// read the chunk-header
		CH = BufferedFileRead(ThePNG, PNG_ChunkHeader_Size);

		if (!CH) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// PNG multi-byte types are in Big Endian
		ChunkHeaderLength = BigLong(CH->Length);
		ChunkHeaderType = BigLong(CH->Type);
		// check if the chunk is a PLTE
		if (!(ChunkHeaderType == PNG_ChunkType_PLTE)) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// check if Length is divisible by 3
		if (ChunkHeaderLength % 3) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// read the raw palette data
		InPal = BufferedFileRead(ThePNG, ChunkHeaderLength);

		if (!InPal) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// read the CRC for the palette
		CRC = BufferedFileRead(ThePNG, PNG_ChunkCRC_Size);

		if (!CRC) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// set some default values
		for (i = 0; i < 256; i++) {
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 0] = 0x00;
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 1] = 0x00;
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 2] = 0x00;
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 3] = 0xFF;
		}
		// convert to the Quake3 RGBA-format
		for (i = 0; i < (ChunkHeaderLength / 3); i++) {
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 0] = InPal[i * 3 + 0];
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 1] = InPal[i * 3 + 1];
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 2] = InPal[i * 3 + 2];
			OutPal[i * Q3IMAGE_BYTESPERPIXEL + 3] = 0xFF;
		}
	}
	// transparency information is sometimes stored in a tRNS chunk

	// let's see if there is a tRNS chunk
	if (FindChunk(ThePNG, PNG_ChunkType_tRNS)) {
		uint8_t *Trans;

		// read the chunk-header
		CH = BufferedFileRead(ThePNG, PNG_ChunkHeader_Size);

		if (!CH) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// PNG multi-byte types are in Big Endian
		ChunkHeaderLength = BigLong(CH->Length);
		ChunkHeaderType = BigLong(CH->Type);
		// check if the chunk is a tRNS
		if (!(ChunkHeaderType == PNG_ChunkType_tRNS)) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// read the transparency information
		Trans = BufferedFileRead(ThePNG, ChunkHeaderLength);

		if (!Trans) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// read the CRC
		CRC = BufferedFileRead(ThePNG, PNG_ChunkCRC_Size);

		if (!CRC) {
			CloseBufferedFile(ThePNG);
			return;
		}
		// only for Grey, True and Indexed ColourType should tRNS exist
		switch (IHDR->ColourType) {
			case PNG_ColourType_Grey:
			{
				if (ChunkHeaderLength != 2) {
					CloseBufferedFile(ThePNG);
					return;
				}

				HasTransparentColour = qtrue;
				// grey can have one colour which is completely transparent
				// this colour is always stored in 16 bits
				TransparentColour[0] = Trans[0];
				TransparentColour[1] = Trans[1];
				break;
			}

			case PNG_ColourType_True:
			{
				if (ChunkHeaderLength != 6) {
					CloseBufferedFile(ThePNG);
					return;
				}

				HasTransparentColour = qtrue;
				// true can have one colour which is completely transparent
				// this colour is always stored in 16 bits
				TransparentColour[0] = Trans[0];
				TransparentColour[1] = Trans[1];
				TransparentColour[2] = Trans[2];
				TransparentColour[3] = Trans[3];
				TransparentColour[4] = Trans[4];
				TransparentColour[5] = Trans[5];
				break;
			}

			case PNG_ColourType_Indexed:
			{
				// maximum of 256 one byte transparency entries
				if (ChunkHeaderLength > 256) {
					CloseBufferedFile(ThePNG);
					return;
				}

				HasTransparentColour = qtrue;
				// alpha values for palette entries
				for (i = 0; i < ChunkHeaderLength; i++) {
					OutPal[i * Q3IMAGE_BYTESPERPIXEL + 3] = Trans[i];
				}

				break;
			}
			// all other ColourTypes should not have tRNS chunks
			default:
			{
				CloseBufferedFile(ThePNG);
				return;
			}
		}
	}
	// rewind to the start of the file
	if (!BufferedFileRewind(ThePNG, -1)) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// skip the signature
	if (!BufferedFileSkip(ThePNG, PNG_Signature_Size)) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// decompress all IDAT chunks
	DecompressedDataLength = DecompressIDATs(ThePNG, &DecompressedData);

	if (!(DecompressedDataLength && DecompressedData)) {
		CloseBufferedFile(ThePNG);
		return;
	}
	// allocate output buffer
	OutBuffer = ri.Malloc(IHDR_Width * IHDR_Height * Q3IMAGE_BYTESPERPIXEL);

	if (!OutBuffer) {
		ri.Free(DecompressedData);
		CloseBufferedFile(ThePNG);
		return;
	}
	// interlaced and Non-interlaced images need to be handled differently
	switch (IHDR->InterlaceMethod) {
		case PNG_InterlaceMethod_NonInterlaced:
		{
			if (!DecodeImageNonInterlaced(IHDR, OutBuffer, DecompressedData, DecompressedDataLength, HasTransparentColour, TransparentColour, OutPal)) {
				ri.Free(OutBuffer);
				ri.Free(DecompressedData);
				CloseBufferedFile(ThePNG);
				return;
			}

			break;
		}

		case PNG_InterlaceMethod_Interlaced:
		{
			if (!DecodeImageInterlaced(IHDR, OutBuffer, DecompressedData, DecompressedDataLength, HasTransparentColour, TransparentColour, OutPal)) {
				ri.Free(OutBuffer);
				ri.Free(DecompressedData);
				CloseBufferedFile(ThePNG);
				return;
			}

			break;
		}

		default:
		{
			ri.Free(OutBuffer);
			ri.Free(DecompressedData);
			CloseBufferedFile(ThePNG);

			return;
		}
	}
	// update the pointer to the image data
	*pic = OutBuffer;
	// fill width and height
	if (width) {
		*width = IHDR_Width;
	}

	if (height) {
		*height = IHDR_Height;
	}
	// decompressedData is not needed anymore
	ri.Free(DecompressedData);
	// we have all data, so close the file
	CloseBufferedFile(ThePNG);
}
