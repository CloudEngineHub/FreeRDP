/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * H.264 Bitmap Compression
 *
 * Copyright 2014 Mike McDonald <Mike.McDonald@software.dell.com>
 * Copyright 2015 Vic Lee <llyzs.vic@gmail.com>
 * Copyright 2014 Armin Novak <armin.novak@gmail.com>
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
#include <freerdp/config.h>

#include <winpr/winpr.h>
#include <winpr/library.h>
#include <winpr/assert.h>
#include <winpr/cast.h>

#include <freerdp/log.h>
#include <freerdp/codec/h264.h>

#include <wels/codec_def.h>
#include <wels/codec_api.h>
#include <wels/codec_ver.h>

#include "h264.h"

typedef void (*pWelsGetCodecVersionEx)(OpenH264Version* pVersion);

typedef long (*pWelsCreateDecoder)(ISVCDecoder** ppDecoder);
typedef void (*pWelsDestroyDecoder)(ISVCDecoder* pDecoder);

typedef int (*pWelsCreateSVCEncoder)(ISVCEncoder** ppEncoder);
typedef void (*pWelsDestroySVCEncoder)(ISVCEncoder* pEncoder);

typedef struct
{
#if defined(WITH_OPENH264_LOADING)
	HMODULE lib;
	OpenH264Version version;
#endif
	pWelsGetCodecVersionEx WelsGetCodecVersionEx;
	pWelsCreateDecoder WelsCreateDecoder;
	pWelsDestroyDecoder WelsDestroyDecoder;
	pWelsCreateSVCEncoder WelsCreateSVCEncoder;
	pWelsDestroySVCEncoder WelsDestroySVCEncoder;
	ISVCDecoder* pDecoder;
	ISVCEncoder* pEncoder;
	SEncParamExt EncParamExt;
} H264_CONTEXT_OPENH264;

#if defined(WITH_OPENH264_LOADING)
static const char* openh264_library_names[] = {
#if defined(_WIN32)
	"openh264.dll"
#elif defined(__APPLE__)
	"libopenh264.dylib"
#else
	"libopenh264.so.7",     "libopenh264.so.2.5.0", "libopenh264.so.2.4.1", "libopenh264.so.2.4.0",
	"libopenh264.so.2.3.1", "libopenh264.so.2.3.0", "libopenh264.so",

#endif
};
#endif

static void openh264_trace_callback(void* ctx, int level, const char* message)
{
	H264_CONTEXT* h264 = ctx;
	if (h264)
		WLog_Print(h264->log, WLOG_TRACE, "%d - %s", level, message);
}

static int openh264_decompress(H264_CONTEXT* WINPR_RESTRICT h264,
                               const BYTE* WINPR_RESTRICT pSrcData, UINT32 SrcSize)
{
	DECODING_STATE state = dsInvalidArgument;
	SBufferInfo sBufferInfo = { 0 };
	SSysMEMBuffer* pSystemBuffer = NULL;
	H264_CONTEXT_OPENH264* sys = NULL;
	UINT32* iStride = NULL;
	BYTE** pYUVData = NULL;

	WINPR_ASSERT(h264);
	WINPR_ASSERT(pSrcData || (SrcSize == 0));

	sys = (H264_CONTEXT_OPENH264*)h264->pSystemData;
	WINPR_ASSERT(sys);

	iStride = h264->iStride;
	WINPR_ASSERT(iStride);

	pYUVData = h264->pYUVData;
	WINPR_ASSERT(pYUVData);

	if (!sys->pDecoder)
		return -2001;

	/*
	 * Decompress the image.  The RDP host only seems to send I420 format.
	 */
	pYUVData[0] = NULL;
	pYUVData[1] = NULL;
	pYUVData[2] = NULL;

	WINPR_ASSERT(sys->pDecoder);
	state = (*sys->pDecoder)
	            ->DecodeFrame2(sys->pDecoder, pSrcData, WINPR_ASSERTING_INT_CAST(int, SrcSize),
	                           pYUVData, &sBufferInfo);

	if (sBufferInfo.iBufferStatus != 1)
	{
		if (state == dsNoParamSets)
		{
			/* this happens on the first frame due to missing parameter sets */
			state = (*sys->pDecoder)->DecodeFrame2(sys->pDecoder, NULL, 0, pYUVData, &sBufferInfo);
		}
		else if (state == dsErrorFree)
		{
			/* call DecodeFrame2 again to decode without delay */
			state = (*sys->pDecoder)->DecodeFrame2(sys->pDecoder, NULL, 0, pYUVData, &sBufferInfo);
		}
		else
		{
			WLog_Print(h264->log, WLOG_WARN, "DecodeFrame2 state: 0x%04X iBufferStatus: %d", state,
			           sBufferInfo.iBufferStatus);
			return -2002;
		}
	}

	if (state != dsErrorFree)
	{
		WLog_Print(h264->log, WLOG_WARN, "DecodeFrame2 state: 0x%02X", state);
		return -2003;
	}

#if OPENH264_MAJOR >= 2
	state = (*sys->pDecoder)->FlushFrame(sys->pDecoder, pYUVData, &sBufferInfo);
	if (state != dsErrorFree)
	{
		WLog_Print(h264->log, WLOG_WARN, "FlushFrame state: 0x%02X", state);
		return -2003;
	}
#endif

	pSystemBuffer = &sBufferInfo.UsrData.sSystemBuffer;
	iStride[0] = WINPR_ASSERTING_INT_CAST(uint32_t, pSystemBuffer->iStride[0]);
	iStride[1] = WINPR_ASSERTING_INT_CAST(uint32_t, pSystemBuffer->iStride[1]);
	iStride[2] = WINPR_ASSERTING_INT_CAST(uint32_t, pSystemBuffer->iStride[1]);

	if (sBufferInfo.iBufferStatus != 1)
	{
		WLog_Print(h264->log, WLOG_WARN, "DecodeFrame2 iBufferStatus: %d",
		           sBufferInfo.iBufferStatus);
		return 0;
	}

	if (state != dsErrorFree)
	{
		WLog_Print(h264->log, WLOG_WARN, "DecodeFrame2 state: 0x%02X", state);
		return -2003;
	}

	if (pSystemBuffer->iFormat != videoFormatI420)
		return -2004;

	if (!pYUVData[0] || !pYUVData[1] || !pYUVData[2])
		return -2005;

	return 1;
}

static int openh264_compress(H264_CONTEXT* WINPR_RESTRICT h264,
                             const BYTE** WINPR_RESTRICT pYUVData,
                             const UINT32* WINPR_RESTRICT iStride, BYTE** WINPR_RESTRICT ppDstData,
                             UINT32* WINPR_RESTRICT pDstSize)
{
	int status = 0;
	SFrameBSInfo info = { 0 };
	SSourcePicture pic = { 0 };

	H264_CONTEXT_OPENH264* sys = NULL;

	WINPR_ASSERT(h264);
	WINPR_ASSERT(pYUVData);
	WINPR_ASSERT(iStride);
	WINPR_ASSERT(ppDstData);
	WINPR_ASSERT(pDstSize);

	sys = &((H264_CONTEXT_OPENH264*)h264->pSystemData)[0];
	WINPR_ASSERT(sys);

	if (!sys->pEncoder)
		return -1;

	if (!pYUVData[0] || !pYUVData[1] || !pYUVData[2])
		return -1;

	if ((h264->width > INT_MAX) || (h264->height > INT_MAX))
		return -1;

	if ((h264->FrameRate > INT_MAX) || (h264->NumberOfThreads > INT_MAX) ||
	    (h264->BitRate > INT_MAX) || (h264->QP > INT_MAX))
		return -1;

	WINPR_ASSERT(sys->pEncoder);
	if ((sys->EncParamExt.iPicWidth != (int)h264->width) ||
	    (sys->EncParamExt.iPicHeight != (int)h264->height))
	{
		WINPR_ASSERT((*sys->pEncoder)->GetDefaultParams);
		status = (*sys->pEncoder)->GetDefaultParams(sys->pEncoder, &sys->EncParamExt);

		if (status < 0)
		{
			WLog_Print(h264->log, WLOG_ERROR,
			           "Failed to get OpenH264 default parameters (status=%d)", status);
			return status;
		}

		EUsageType usageType = SCREEN_CONTENT_REAL_TIME;

		switch (h264->UsageType)
		{
			case H264_CAMERA_VIDEO_NON_REAL_TIME:
				usageType = CAMERA_VIDEO_NON_REAL_TIME;
				break;
			case H264_CAMERA_VIDEO_REAL_TIME:
				usageType = CAMERA_VIDEO_REAL_TIME;
				break;
			case H264_SCREEN_CONTENT_NON_REAL_TIME:
				usageType = SCREEN_CONTENT_NON_REAL_TIME;
				break;
			case H264_SCREEN_CONTENT_REAL_TIME:
			default:
				break;
		}

		sys->EncParamExt.iUsageType = usageType;
		sys->EncParamExt.iPicWidth = WINPR_ASSERTING_INT_CAST(int, h264->width);
		sys->EncParamExt.iPicHeight = WINPR_ASSERTING_INT_CAST(int, h264->height);
		sys->EncParamExt.fMaxFrameRate = WINPR_ASSERTING_INT_CAST(short, h264->FrameRate);
		sys->EncParamExt.iMaxBitrate = UNSPECIFIED_BIT_RATE;
		sys->EncParamExt.bEnableDenoise = 0;
		sys->EncParamExt.bEnableLongTermReference = 0;
		sys->EncParamExt.iSpatialLayerNum = 1;
		sys->EncParamExt.iMultipleThreadIdc =
		    WINPR_ASSERTING_INT_CAST(unsigned short, h264->NumberOfThreads);
		sys->EncParamExt.sSpatialLayers[0].fFrameRate =
		    WINPR_ASSERTING_INT_CAST(short, h264->FrameRate);
		sys->EncParamExt.sSpatialLayers[0].iVideoWidth = sys->EncParamExt.iPicWidth;
		sys->EncParamExt.sSpatialLayers[0].iVideoHeight = sys->EncParamExt.iPicHeight;
		sys->EncParamExt.sSpatialLayers[0].iMaxSpatialBitrate = sys->EncParamExt.iMaxBitrate;

		switch (h264->RateControlMode)
		{
			case H264_RATECONTROL_VBR:
				sys->EncParamExt.iRCMode = RC_BITRATE_MODE;
				sys->EncParamExt.iTargetBitrate = (int)h264->BitRate;
				sys->EncParamExt.sSpatialLayers[0].iSpatialBitrate =
				    sys->EncParamExt.iTargetBitrate;
				sys->EncParamExt.bEnableFrameSkip = 1;
				break;

			case H264_RATECONTROL_CQP:
				sys->EncParamExt.iRCMode = RC_OFF_MODE;
				sys->EncParamExt.sSpatialLayers[0].iDLayerQp = (int)h264->QP;
				sys->EncParamExt.bEnableFrameSkip = 0;
				break;
			default:
				break;
		}

		if (sys->EncParamExt.iMultipleThreadIdc > 1)
		{
#if (OPENH264_MAJOR == 1) && (OPENH264_MINOR <= 5)
			sys->EncParamExt.sSpatialLayers[0].sSliceCfg.uiSliceMode = SM_AUTO_SLICE;
#else
			sys->EncParamExt.sSpatialLayers[0].sSliceArgument.uiSliceMode = SM_FIXEDSLCNUM_SLICE;
#endif
		}

		WINPR_ASSERT((*sys->pEncoder)->InitializeExt);
		status = (*sys->pEncoder)->InitializeExt(sys->pEncoder, &sys->EncParamExt);

		if (status < 0)
		{
			WLog_Print(h264->log, WLOG_ERROR, "Failed to initialize OpenH264 encoder (status=%d)",
			           status);
			return status;
		}

		WINPR_ASSERT((*sys->pEncoder)->GetOption);
		status =
		    (*sys->pEncoder)
		        ->GetOption(sys->pEncoder, ENCODER_OPTION_SVC_ENCODE_PARAM_EXT, &sys->EncParamExt);

		if (status < 0)
		{
			WLog_Print(h264->log, WLOG_ERROR,
			           "Failed to get initial OpenH264 encoder parameters (status=%d)", status);
			return status;
		}
	}
	else
	{
		switch (h264->RateControlMode)
		{
			case H264_RATECONTROL_VBR:
				if (sys->EncParamExt.iTargetBitrate != (int)h264->BitRate)
				{
					SBitrateInfo bitrate = { 0 };

					sys->EncParamExt.iTargetBitrate = (int)h264->BitRate;
					bitrate.iLayer = SPATIAL_LAYER_ALL;
					bitrate.iBitrate = (int)h264->BitRate;

					WINPR_ASSERT((*sys->pEncoder)->SetOption);
					status = (*sys->pEncoder)
					             ->SetOption(sys->pEncoder, ENCODER_OPTION_BITRATE, &bitrate);

					if (status < 0)
					{
						WLog_Print(h264->log, WLOG_ERROR,
						           "Failed to set encoder bitrate (status=%d)", status);
						return status;
					}
				}

				if ((uint32_t)sys->EncParamExt.fMaxFrameRate != h264->FrameRate)
				{
					sys->EncParamExt.fMaxFrameRate = WINPR_ASSERTING_INT_CAST(int, h264->FrameRate);

					WINPR_ASSERT((*sys->pEncoder)->SetOption);
					status = (*sys->pEncoder)
					             ->SetOption(sys->pEncoder, ENCODER_OPTION_FRAME_RATE,
					                         &sys->EncParamExt.fMaxFrameRate);

					if (status < 0)
					{
						WLog_Print(h264->log, WLOG_ERROR,
						           "Failed to set encoder framerate (status=%d)", status);
						return status;
					}
				}

				break;

			case H264_RATECONTROL_CQP:
				if (sys->EncParamExt.sSpatialLayers[0].iDLayerQp != (int)h264->QP)
				{
					sys->EncParamExt.sSpatialLayers[0].iDLayerQp = (int)h264->QP;

					WINPR_ASSERT((*sys->pEncoder)->SetOption);
					status = (*sys->pEncoder)
					             ->SetOption(sys->pEncoder, ENCODER_OPTION_SVC_ENCODE_PARAM_EXT,
					                         &sys->EncParamExt);

					if (status < 0)
					{
						WLog_Print(h264->log, WLOG_ERROR,
						           "Failed to set encoder parameters (status=%d)", status);
						return status;
					}
				}

				break;
			default:
				break;
		}
	}

	pic.iPicWidth = (int)h264->width;
	pic.iPicHeight = (int)h264->height;
	pic.iColorFormat = videoFormatI420;
	pic.iStride[0] = (int)iStride[0];
	pic.iStride[1] = (int)iStride[1];
	pic.iStride[2] = (int)iStride[2];
	pic.pData[0] = WINPR_CAST_CONST_PTR_AWAY(pYUVData[0], BYTE*);
	pic.pData[1] = WINPR_CAST_CONST_PTR_AWAY(pYUVData[1], BYTE*);
	pic.pData[2] = WINPR_CAST_CONST_PTR_AWAY(pYUVData[2], BYTE*);

	WINPR_ASSERT((*sys->pEncoder)->EncodeFrame);
	status = (*sys->pEncoder)->EncodeFrame(sys->pEncoder, &pic, &info);

	if (status < 0)
	{
		WLog_Print(h264->log, WLOG_ERROR, "Failed to encode frame (status=%d)", status);
		return status;
	}

	*ppDstData = info.sLayerInfo[0].pBsBuf;
	*pDstSize = 0;

	for (int i = 0; i < info.iLayerNum; i++)
	{
		for (int j = 0; j < info.sLayerInfo[i].iNalCount; j++)
		{
			const int val = info.sLayerInfo[i].pNalLengthInByte[j];
			*pDstSize += WINPR_ASSERTING_INT_CAST(uint32_t, val);
		}
	}

	return 1;
}

static void openh264_uninit(H264_CONTEXT* h264)
{
	H264_CONTEXT_OPENH264* sysContexts = NULL;

	WINPR_ASSERT(h264);

	sysContexts = (H264_CONTEXT_OPENH264*)h264->pSystemData;

	if (sysContexts)
	{
		for (UINT32 x = 0; x < h264->numSystemData; x++)
		{
			H264_CONTEXT_OPENH264* sys = &sysContexts[x];

			if (sys->pDecoder)
			{
				(*sys->pDecoder)->Uninitialize(sys->pDecoder);
				sysContexts->WelsDestroyDecoder(sys->pDecoder);
				sys->pDecoder = NULL;
			}

			if (sys->pEncoder)
			{
				(*sys->pEncoder)->Uninitialize(sys->pEncoder);
				sysContexts->WelsDestroySVCEncoder(sys->pEncoder);
				sys->pEncoder = NULL;
			}
		}

#if defined(WITH_OPENH264_LOADING)
		if (sysContexts->lib)
			FreeLibrary(sysContexts->lib);
#endif
		free(h264->pSystemData);
		h264->pSystemData = NULL;
	}
}

#if defined(WITH_OPENH264_LOADING)
static BOOL openh264_load_functionpointers(H264_CONTEXT* h264, const char* name)
{
	H264_CONTEXT_OPENH264* sysContexts;

	WINPR_ASSERT(name);

	if (!h264)
		return FALSE;

	sysContexts = h264->pSystemData;

	if (!sysContexts)
		return FALSE;

	sysContexts->lib = LoadLibraryA(name);

	if (!sysContexts->lib)
		return FALSE;

	sysContexts->WelsGetCodecVersionEx =
	    GetProcAddressAs(sysContexts->lib, "WelsGetCodecVersionEx", pWelsGetCodecVersionEx);
	sysContexts->WelsCreateDecoder =
	    GetProcAddressAs(sysContexts->lib, "WelsCreateDecoder", pWelsCreateDecoder);
	sysContexts->WelsDestroyDecoder =
	    GetProcAddressAs(sysContexts->lib, "WelsDestroyDecoder", pWelsDestroyDecoder);
	sysContexts->WelsCreateSVCEncoder =
	    GetProcAddressAs(sysContexts->lib, "WelsCreateSVCEncoder", pWelsCreateSVCEncoder);
	sysContexts->WelsDestroySVCEncoder =
	    GetProcAddressAs(sysContexts->lib, "WelsDestroySVCEncoder", pWelsDestroySVCEncoder);

	if (!sysContexts->WelsCreateDecoder || !sysContexts->WelsDestroyDecoder ||
	    !sysContexts->WelsCreateSVCEncoder || !sysContexts->WelsDestroySVCEncoder ||
	    !sysContexts->WelsGetCodecVersionEx)
	{
		FreeLibrary(sysContexts->lib);
		sysContexts->lib = NULL;
		return FALSE;
	}

	sysContexts->WelsGetCodecVersionEx(&sysContexts->version);
	WLog_Print(h264->log, WLOG_INFO, "loaded %s %d.%d.%d", name, sysContexts->version.uMajor,
	           sysContexts->version.uMinor, sysContexts->version.uRevision);

	if ((sysContexts->version.uMajor < 1) ||
	    ((sysContexts->version.uMajor == 1) && (sysContexts->version.uMinor < 6)))
	{
		WLog_Print(
		    h264->log, WLOG_ERROR,
		    "OpenH264 %s %d.%d.%d is too old, need at least version 1.6.0 for dynamic loading",
		    name, sysContexts->version.uMajor, sysContexts->version.uMinor,
		    sysContexts->version.uRevision);
		FreeLibrary(sysContexts->lib);
		sysContexts->lib = NULL;
		return FALSE;
	}

	return TRUE;
}
#endif

static BOOL openh264_init(H264_CONTEXT* h264)
{
#if defined(WITH_OPENH264_LOADING)
	BOOL success = FALSE;
#endif
	long status = 0;
	H264_CONTEXT_OPENH264* sysContexts = NULL;
	static int traceLevel = WELS_LOG_DEBUG;
#if (OPENH264_MAJOR == 1) && (OPENH264_MINOR <= 5)
	static EVideoFormatType videoFormat = videoFormatI420;
#endif
	static WelsTraceCallback traceCallback = openh264_trace_callback;

	WINPR_ASSERT(h264);

	h264->numSystemData = 1;
	sysContexts =
	    (H264_CONTEXT_OPENH264*)calloc(h264->numSystemData, sizeof(H264_CONTEXT_OPENH264));

	if (!sysContexts)
		goto EXCEPTION;

	h264->pSystemData = (void*)sysContexts;
#if defined(WITH_OPENH264_LOADING)

	for (size_t i = 0; i < ARRAYSIZE(openh264_library_names); i++)
	{
		const char* current = openh264_library_names[i];
		success = openh264_load_functionpointers(h264, current);

		if (success)
			break;
	}

	if (!success)
		goto EXCEPTION;

#else
	sysContexts->WelsGetCodecVersionEx = WelsGetCodecVersionEx;
	sysContexts->WelsCreateDecoder = WelsCreateDecoder;
	sysContexts->WelsDestroyDecoder = WelsDestroyDecoder;
	sysContexts->WelsCreateSVCEncoder = WelsCreateSVCEncoder;
	sysContexts->WelsDestroySVCEncoder = WelsDestroySVCEncoder;
#endif

	for (UINT32 x = 0; x < h264->numSystemData; x++)
	{
		SDecodingParam sDecParam = { 0 };
		H264_CONTEXT_OPENH264* sys = &sysContexts[x];

		if (h264->Compressor)
		{
			sysContexts->WelsCreateSVCEncoder(&sys->pEncoder);

			if (!sys->pEncoder)
			{
				WLog_Print(h264->log, WLOG_ERROR, "Failed to create OpenH264 encoder");
				goto EXCEPTION;
			}
		}
		else
		{
			sysContexts->WelsCreateDecoder(&sys->pDecoder);

			if (!sys->pDecoder)
			{
				WLog_Print(h264->log, WLOG_ERROR, "Failed to create OpenH264 decoder");
				goto EXCEPTION;
			}

#if (OPENH264_MAJOR == 1) && (OPENH264_MINOR <= 5)
			sDecParam.eOutputColorFormat = videoFormatI420;
#endif
			sDecParam.eEcActiveIdc = ERROR_CON_FRAME_COPY;
			sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
			status = (*sys->pDecoder)->Initialize(sys->pDecoder, &sDecParam);

			if (status != 0)
			{
				WLog_Print(h264->log, WLOG_ERROR,
				           "Failed to initialize OpenH264 decoder (status=%ld)", status);
				goto EXCEPTION;
			}

#if (OPENH264_MAJOR == 1) && (OPENH264_MINOR <= 5)
			status =
			    (*sys->pDecoder)->SetOption(sys->pDecoder, DECODER_OPTION_DATAFORMAT, &videoFormat);
#endif

			if (status != 0)
			{
				WLog_Print(h264->log, WLOG_ERROR,
				           "Failed to set data format option on OpenH264 decoder (status=%ld)",
				           status);
				goto EXCEPTION;
			}

			if (WLog_GetLogLevel(h264->log) == WLOG_TRACE)
			{
				status = (*sys->pDecoder)
				             ->SetOption(sys->pDecoder, DECODER_OPTION_TRACE_LEVEL, &traceLevel);

				if (status != 0)
				{
					WLog_Print(h264->log, WLOG_ERROR,
					           "Failed to set trace level option on OpenH264 decoder (status=%ld)",
					           status);
					goto EXCEPTION;
				}

				status = (*sys->pDecoder)
				             ->SetOption(sys->pDecoder, DECODER_OPTION_TRACE_CALLBACK_CONTEXT,
				                         (void*)&h264);

				if (status != 0)
				{
					WLog_Print(h264->log, WLOG_ERROR,
					           "Failed to set trace callback context option on OpenH264 decoder "
					           "(status=%ld)",
					           status);
					goto EXCEPTION;
				}

				status = (*sys->pDecoder)
				             ->SetOption(sys->pDecoder, DECODER_OPTION_TRACE_CALLBACK,
				                         (void*)&traceCallback);

				if (status != 0)
				{
					WLog_Print(
					    h264->log, WLOG_ERROR,
					    "Failed to set trace callback option on OpenH264 decoder (status=%ld)",
					    status);
					goto EXCEPTION;
				}
			}
		}
	}

	h264->hwAccel = FALSE; /* not supported */
	return TRUE;
EXCEPTION:
	openh264_uninit(h264);
	return FALSE;
}

const H264_CONTEXT_SUBSYSTEM g_Subsystem_OpenH264 = { "OpenH264", openh264_init, openh264_uninit,
	                                                  openh264_decompress, openh264_compress };
