/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * RLE Compressed Bitmap Stream
 *
 * Copyright 2011 Jay Sorg <jay.sorg@gmail.com>
 * Copyright 2016 Armin Novak <armin.novak@thincast.com>
 * Copyright 2016 Thincast Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <winpr/assert.h>
#include <winpr/cast.h>
#include <winpr/wtypes.h>

/* do not compile the file directly */

/**
 * Write a foreground/background image to a destination buffer.
 */
static inline BYTE* WRITEFGBGIMAGE(BYTE* WINPR_RESTRICT pbDest,
                                   const BYTE* WINPR_RESTRICT pbDestEnd, UINT32 rowDelta,
                                   BYTE bitmask, PIXEL fgPel, UINT32 cBits)
{
	PIXEL xorPixel = 0;
	BYTE mask = 0x01;

	if (cBits > 8)
	{
		WLog_ERR(TAG, "cBits %d > 8", cBits);
		return NULL;
	}

	if (!ENSURE_CAPACITY(pbDest, pbDestEnd, cBits))
		return NULL;

	UNROLL(cBits, {
		PIXEL data = 0;
		DESTREADPIXEL(xorPixel, pbDest - rowDelta);

		if (bitmask & mask)
			data = xorPixel ^ fgPel;
		else
			data = xorPixel;

		DESTWRITEPIXEL(pbDest, data);
		mask = WINPR_ASSERTING_INT_CAST(BYTE, (mask << 1) & 0xFF);
	});
	return pbDest;
}

/**
 * Write a foreground/background image to a destination buffer
 * for the first line of compressed data.
 */
static inline BYTE* WRITEFIRSTLINEFGBGIMAGE(BYTE* WINPR_RESTRICT pbDest,
                                            const BYTE* WINPR_RESTRICT pbDestEnd, BYTE bitmask,
                                            PIXEL fgPel, UINT32 cBits)
{
	BYTE mask = 0x01;

	if (cBits > 8)
	{
		WLog_ERR(TAG, "cBits %d > 8", cBits);
		return NULL;
	}

	if (!ENSURE_CAPACITY(pbDest, pbDestEnd, cBits))
		return NULL;

	UNROLL(cBits, {
		PIXEL data;

		if (bitmask & mask)
			data = fgPel;
		else
			data = BLACK_PIXEL;

		DESTWRITEPIXEL(pbDest, data);
		mask = WINPR_ASSERTING_INT_CAST(BYTE, (mask << 1) & 0xFF);
	});
	return pbDest;
}

/**
 * Decompress an RLE compressed bitmap.
 */
static inline BOOL RLEDECOMPRESS(const BYTE* WINPR_RESTRICT pbSrcBuffer, UINT32 cbSrcBuffer,
                                 BYTE* WINPR_RESTRICT pbDestBuffer, UINT32 rowDelta, UINT32 width,
                                 UINT32 height)
{
	const BYTE* pbSrc = pbSrcBuffer;
	BYTE* pbDest = pbDestBuffer;
	PIXEL temp = 0;
	PIXEL fgPel = WHITE_PIXEL;
	BOOL fInsertFgPel = FALSE;
	BOOL fFirstLine = TRUE;
	BYTE bitmask = 0;
	PIXEL pixelA = 0;
	PIXEL pixelB = 0;
	UINT32 runLength = 0;
	UINT32 code = 0;
	UINT32 advance = 0;
	RLEEXTRA

	if ((rowDelta == 0) || (rowDelta < width))
	{
		WLog_ERR(TAG, "Invalid arguments: rowDelta=%" PRIu32 " == 0 || < width=%" PRIu32, rowDelta,
		         width);
		return FALSE;
	}

	if (!pbSrcBuffer || !pbDestBuffer)
	{
		WLog_ERR(TAG, "Invalid arguments: pbSrcBuffer=%p, pbDestBuffer=%p", pbSrcBuffer,
		         pbDestBuffer);
		return FALSE;
	}

	const BYTE* pbEnd = &pbSrcBuffer[cbSrcBuffer];
	const BYTE* pbDestEnd = &pbDestBuffer[1ULL * rowDelta * height];

	while (pbSrc < pbEnd)
	{
		/* Watch out for the end of the first scanline. */
		if (fFirstLine)
		{
			if ((UINT32)(pbDest - pbDestBuffer) >= rowDelta)
			{
				fFirstLine = FALSE;
				fInsertFgPel = FALSE;
			}
		}

		/*
		   Extract the compression order code ID from the compression
		   order header.
		*/
		code = ExtractCodeId(*pbSrc);

#if defined(WITH_DEBUG_CODECS)
		WLog_VRB(TAG, "pbSrc=%p code=%s, rem=%" PRIuz, pbSrc, rle_code_str(code), pbEnd - pbSrc);
#endif

		/* Handle Background Run Orders. */
		if ((code == REGULAR_BG_RUN) || (code == MEGA_MEGA_BG_RUN))
		{
			runLength = ExtractRunLength(code, pbSrc, pbEnd, &advance);
			if (advance == 0)
				return FALSE;
			pbSrc = pbSrc + advance;

			if (fFirstLine)
			{
				if (fInsertFgPel)
				{
					if (!ENSURE_CAPACITY(pbDest, pbDestEnd, 1))
						return FALSE;

					DESTWRITEPIXEL(pbDest, fgPel);
					runLength = runLength - 1;
				}

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, runLength))
					return FALSE;

				UNROLL(runLength, { DESTWRITEPIXEL(pbDest, BLACK_PIXEL); });
			}
			else
			{
				if (fInsertFgPel)
				{
					DESTREADPIXEL(temp, pbDest - rowDelta);

					if (!ENSURE_CAPACITY(pbDest, pbDestEnd, 1))
						return FALSE;

					DESTWRITEPIXEL(pbDest, temp ^ fgPel);
					runLength--;
				}

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, runLength))
					return FALSE;

				UNROLL(runLength, {
					DESTREADPIXEL(temp, pbDest - rowDelta);
					DESTWRITEPIXEL(pbDest, temp);
				});
			}

			/* A follow-on background run order will need a foreground pel inserted. */
			fInsertFgPel = TRUE;
			continue;
		}

		/* For any of the other run-types a follow-on background run
		    order does not need a foreground pel inserted. */
		fInsertFgPel = FALSE;

		switch (code)
		{
			/* Handle Foreground Run Orders. */
			case REGULAR_FG_RUN:
			case MEGA_MEGA_FG_RUN:
			case LITE_SET_FG_FG_RUN:
			case MEGA_MEGA_SET_FG_RUN:
				runLength = ExtractRunLength(code, pbSrc, pbEnd, &advance);
				if (advance == 0)
					return FALSE;
				pbSrc = pbSrc + advance;

				if (code == LITE_SET_FG_FG_RUN || code == MEGA_MEGA_SET_FG_RUN)
				{
					if (!buffer_within_range(pbSrc, PIXEL_SIZE, pbEnd))
						return FALSE;
					SRCREADPIXEL(fgPel, pbSrc);
				}

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, runLength))
					return FALSE;

				if (fFirstLine)
				{
					UNROLL(runLength, { DESTWRITEPIXEL(pbDest, fgPel); });
				}
				else
				{
					UNROLL(runLength, {
						DESTREADPIXEL(temp, pbDest - rowDelta);
						DESTWRITEPIXEL(pbDest, temp ^ fgPel);
					});
				}

				break;

			/* Handle Dithered Run Orders. */
			case LITE_DITHERED_RUN:
			case MEGA_MEGA_DITHERED_RUN:
				runLength = ExtractRunLength(code, pbSrc, pbEnd, &advance);
				if (advance == 0)
					return FALSE;
				pbSrc = pbSrc + advance;
				if (!buffer_within_range(pbSrc, PIXEL_SIZE, pbEnd))
					return FALSE;
				SRCREADPIXEL(pixelA, pbSrc);
				if (!buffer_within_range(pbSrc, PIXEL_SIZE, pbEnd))
					return FALSE;
				SRCREADPIXEL(pixelB, pbSrc);

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, runLength * 2))
					return FALSE;

				UNROLL(runLength, {
					DESTWRITEPIXEL(pbDest, pixelA);
					DESTWRITEPIXEL(pbDest, pixelB);
				});
				break;

			/* Handle Color Run Orders. */
			case REGULAR_COLOR_RUN:
			case MEGA_MEGA_COLOR_RUN:
				runLength = ExtractRunLength(code, pbSrc, pbEnd, &advance);
				if (advance == 0)
					return FALSE;
				pbSrc = pbSrc + advance;
				if (!buffer_within_range(pbSrc, PIXEL_SIZE, pbEnd))
					return FALSE;
				SRCREADPIXEL(pixelA, pbSrc);

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, runLength))
					return FALSE;

				UNROLL(runLength, { DESTWRITEPIXEL(pbDest, pixelA); });
				break;

			/* Handle Foreground/Background Image Orders. */
			case REGULAR_FGBG_IMAGE:
			case MEGA_MEGA_FGBG_IMAGE:
			case LITE_SET_FG_FGBG_IMAGE:
			case MEGA_MEGA_SET_FGBG_IMAGE:
				runLength = ExtractRunLength(code, pbSrc, pbEnd, &advance);
				if (advance == 0)
					return FALSE;
				pbSrc = pbSrc + advance;

				if (code == LITE_SET_FG_FGBG_IMAGE || code == MEGA_MEGA_SET_FGBG_IMAGE)
				{
					if (!buffer_within_range(pbSrc, PIXEL_SIZE, pbEnd))
						return FALSE;
					SRCREADPIXEL(fgPel, pbSrc);
				}

				if (!buffer_within_range(pbSrc, runLength / 8, pbEnd))
					return FALSE;
				if (fFirstLine)
				{
					while (runLength > 8)
					{
						bitmask = *pbSrc;
						pbSrc = pbSrc + 1;
						pbDest = WRITEFIRSTLINEFGBGIMAGE(pbDest, pbDestEnd, bitmask, fgPel, 8);

						if (!pbDest)
							return FALSE;

						runLength = runLength - 8;
					}
				}
				else
				{
					while (runLength > 8)
					{
						bitmask = *pbSrc++;

						pbDest = WRITEFGBGIMAGE(pbDest, pbDestEnd, rowDelta, bitmask, fgPel, 8);

						if (!pbDest)
							return FALSE;

						runLength = runLength - 8;
					}
				}

				if (runLength > 0)
				{
					if (!buffer_within_range(pbSrc, 1, pbEnd))
						return FALSE;
					bitmask = *pbSrc++;

					if (fFirstLine)
					{
						pbDest =
						    WRITEFIRSTLINEFGBGIMAGE(pbDest, pbDestEnd, bitmask, fgPel, runLength);
					}
					else
					{
						pbDest =
						    WRITEFGBGIMAGE(pbDest, pbDestEnd, rowDelta, bitmask, fgPel, runLength);
					}

					if (!pbDest)
						return FALSE;
				}

				break;

			/* Handle Color Image Orders. */
			case REGULAR_COLOR_IMAGE:
			case MEGA_MEGA_COLOR_IMAGE:
				runLength = ExtractRunLength(code, pbSrc, pbEnd, &advance);
				if (advance == 0)
					return FALSE;
				pbSrc = pbSrc + advance;
				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, runLength))
					return FALSE;
				if (!ENSURE_CAPACITY(pbSrc, pbEnd, runLength))
					return FALSE;

				UNROLL(runLength, {
					SRCREADPIXEL(temp, pbSrc);
					DESTWRITEPIXEL(pbDest, temp);
				});
				break;

			/* Handle Special Order 1. */
			case SPECIAL_FGBG_1:
				if (!buffer_within_range(pbSrc, 1, pbEnd))
					return FALSE;
				pbSrc = pbSrc + 1;

				if (fFirstLine)
				{
					pbDest =
					    WRITEFIRSTLINEFGBGIMAGE(pbDest, pbDestEnd, g_MaskSpecialFgBg1, fgPel, 8);
				}
				else
				{
					pbDest =
					    WRITEFGBGIMAGE(pbDest, pbDestEnd, rowDelta, g_MaskSpecialFgBg1, fgPel, 8);
				}

				if (!pbDest)
					return FALSE;

				break;

			/* Handle Special Order 2. */
			case SPECIAL_FGBG_2:
				if (!buffer_within_range(pbSrc, 1, pbEnd))
					return FALSE;
				pbSrc = pbSrc + 1;

				if (fFirstLine)
				{
					pbDest =
					    WRITEFIRSTLINEFGBGIMAGE(pbDest, pbDestEnd, g_MaskSpecialFgBg2, fgPel, 8);
				}
				else
				{
					pbDest =
					    WRITEFGBGIMAGE(pbDest, pbDestEnd, rowDelta, g_MaskSpecialFgBg2, fgPel, 8);
				}

				if (!pbDest)
					return FALSE;

				break;

			/* Handle White Order. */
			case SPECIAL_WHITE:
				if (!buffer_within_range(pbSrc, 1, pbEnd))
					return FALSE;
				pbSrc = pbSrc + 1;

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, 1))
					return FALSE;

				DESTWRITEPIXEL(pbDest, WHITE_PIXEL);
				break;

			/* Handle Black Order. */
			case SPECIAL_BLACK:
				if (!buffer_within_range(pbSrc, 1, pbEnd))
					return FALSE;
				pbSrc = pbSrc + 1;

				if (!ENSURE_CAPACITY(pbDest, pbDestEnd, 1))
					return FALSE;

				DESTWRITEPIXEL(pbDest, BLACK_PIXEL);
				break;

			default:
				WLog_ERR(TAG, "invalid code 0x%08" PRIx32 ", pbSrcBuffer=%p, pbSrc=%p, pbEnd=%p",
				         code, pbSrcBuffer, pbSrc, pbEnd);
				return FALSE;
		}
	}

	return TRUE;
}
