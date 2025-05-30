/* FreeRDP: A Remote Desktop Protocol Client
 * Optimized sign operations.
 * vi:ts=4 sw=4:
 *
 * (c) Copyright 2012 Hewlett-Packard Development Company, L.P.
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <freerdp/config.h>

#include <freerdp/types.h>
#include <freerdp/primitives.h>
#include <winpr/sysinfo.h>

#include "prim_sign.h"

#include "prim_internal.h"
#include "prim_avxsse.h"

#if defined(SSE_AVX_INTRINSICS_ENABLED)
#include <emmintrin.h>
#include <tmmintrin.h>

static primitives_t* generic = NULL;

/* ------------------------------------------------------------------------- */
static pstatus_t ssse3_sign_16s(const INT16* WINPR_RESTRICT pSrc, INT16* WINPR_RESTRICT pDst,
                                UINT32 ulen)
{
	size_t len = ulen;
	const INT16* sptr = pSrc;
	INT16* dptr = pDst;
	size_t count = 0;

	if (len < 16)
	{
		return generic->sign_16s(pSrc, pDst, ulen);
	}

	/* Check for 16-byte alignment (eventually). */
	if ((ULONG_PTR)pDst & 0x01)
	{
		return generic->sign_16s(pSrc, pDst, ulen);
	}

	/* Seek 16-byte alignment. */
	while ((ULONG_PTR)dptr & 0x0f)
	{
		INT16 src = *sptr++;
		*dptr++ = WINPR_ASSERTING_INT_CAST(int16_t, (src < 0) ? (-1) : ((src > 0) ? 1 : 0));

		if (--len == 0)
			return PRIMITIVES_SUCCESS;
	}

	/* Do 32-short chunks using 8 XMM registers. */
	count = len >> 5;  /* / 32  */
	len -= count << 5; /* * 32 */

	if ((ULONG_PTR)sptr & 0x0f)
	{
		/* Unaligned */
		while (count--)
		{
			__m128i xmm0;
			__m128i xmm1;
			__m128i xmm2;
			__m128i xmm3;
			__m128i xmm4;
			__m128i xmm5;
			__m128i xmm6;
			__m128i xmm7;
			xmm0 = _mm_set1_epi16(0x0001U);
			xmm1 = _mm_set1_epi16(0x0001U);
			xmm2 = _mm_set1_epi16(0x0001U);
			xmm3 = _mm_set1_epi16(0x0001U);
			xmm4 = LOAD_SI128(sptr);
			sptr += 8;
			xmm5 = LOAD_SI128(sptr);
			sptr += 8;
			xmm6 = LOAD_SI128(sptr);
			sptr += 8;
			xmm7 = LOAD_SI128(sptr);
			sptr += 8;
			xmm0 = _mm_sign_epi16(xmm0, xmm4);
			xmm1 = _mm_sign_epi16(xmm1, xmm5);
			xmm2 = _mm_sign_epi16(xmm2, xmm6);
			xmm3 = _mm_sign_epi16(xmm3, xmm7);
			STORE_SI128(dptr, xmm0);
			dptr += 8;
			STORE_SI128(dptr, xmm1);
			dptr += 8;
			STORE_SI128(dptr, xmm2);
			dptr += 8;
			STORE_SI128(dptr, xmm3);
			dptr += 8;
		}
	}
	else
	{
		/* Aligned */
		while (count--)
		{
			__m128i xmm0;
			__m128i xmm1;
			__m128i xmm2;
			__m128i xmm3;
			__m128i xmm4;
			__m128i xmm5;
			__m128i xmm6;
			__m128i xmm7;
			xmm0 = _mm_set1_epi16(0x0001U);
			xmm1 = _mm_set1_epi16(0x0001U);
			xmm2 = _mm_set1_epi16(0x0001U);
			xmm3 = _mm_set1_epi16(0x0001U);
			xmm4 = LOAD_SI128(sptr);
			sptr += 8;
			xmm5 = LOAD_SI128(sptr);
			sptr += 8;
			xmm6 = LOAD_SI128(sptr);
			sptr += 8;
			xmm7 = LOAD_SI128(sptr);
			sptr += 8;
			xmm0 = _mm_sign_epi16(xmm0, xmm4);
			xmm1 = _mm_sign_epi16(xmm1, xmm5);
			xmm2 = _mm_sign_epi16(xmm2, xmm6);
			xmm3 = _mm_sign_epi16(xmm3, xmm7);
			STORE_SI128(dptr, xmm0);
			dptr += 8;
			STORE_SI128(dptr, xmm1);
			dptr += 8;
			STORE_SI128(dptr, xmm2);
			dptr += 8;
			STORE_SI128(dptr, xmm3);
			dptr += 8;
		}
	}

	/* Do 8-short chunks using two XMM registers. */
	count = len >> 3;
	len -= count << 3;

	while (count--)
	{
		__m128i xmm0 = _mm_set1_epi16(0x0001U);
		__m128i xmm1 = LOAD_SI128(sptr);
		sptr += 8;
		xmm0 = _mm_sign_epi16(xmm0, xmm1);
		STORE_SI128(dptr, xmm0);
		dptr += 8;
	}

	/* Do leftovers. */
	while (len--)
	{
		INT16 src = *sptr++;
		*dptr++ = WINPR_ASSERTING_INT_CAST(int16_t, (src < 0) ? -1 : ((src > 0) ? 1 : 0));
	}

	return PRIMITIVES_SUCCESS;
}

#endif /* SSE_AVX_INTRINSICS_ENABLED */

/* ------------------------------------------------------------------------- */
void primitives_init_sign_ssse3_int(primitives_t* WINPR_RESTRICT prims)
{
#if defined(SSE_AVX_INTRINSICS_ENABLED)
	generic = primitives_get_generic();

	/* Pick tuned versions if possible. */
	/* I didn't spot an IPP version of this. */

	WLog_VRB(PRIM_TAG, "SSE3/SSSE3 optimizations");
	prims->sign_16s = ssse3_sign_16s;

#else
	WLog_VRB(PRIM_TAG, "undefined WITH_SIMD or SSSE3/SSE3 intrinsics not available");
	WINPR_UNUSED(prims);
#endif
}
