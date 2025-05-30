/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Video Optimized Remoting Virtual Channel Extension for X11
 *
 * Copyright 2017 David Fort <contact@hardening-consulting.com>
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

#include "../core/update.h"

#include <winpr/assert.h>

#include <freerdp/client/geometry.h>
#include <freerdp/client/video.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/gdi/video.h>
#include <freerdp/gdi/region.h>

void gdi_video_geometry_init(rdpGdi* gdi, GeometryClientContext* geom)
{
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(geom);

	gdi->geometry = geom;

	if (gdi->video)
	{
		VideoClientContext* video = gdi->video;

		WINPR_ASSERT(video);
		WINPR_ASSERT(video->setGeometry);
		video->setGeometry(video, gdi->geometry);
	}
}

void gdi_video_geometry_uninit(rdpGdi* gdi, GeometryClientContext* geom)
{
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(geom);
	WINPR_UNUSED(gdi);
	WINPR_UNUSED(geom);
}

static VideoSurface* gdiVideoCreateSurface(WINPR_ATTR_UNUSED VideoClientContext* video, UINT32 x,
                                           UINT32 y, UINT32 width, UINT32 height)
{
	return VideoClient_CreateCommonContext(sizeof(VideoSurface), x, y, width, height);
}

static BOOL gdiVideoShowSurface(VideoClientContext* video, const VideoSurface* surface,
                                UINT32 destinationWidth, UINT32 destinationHeight)
{
	BOOL rc = FALSE;
	rdpGdi* gdi = NULL;
	rdpUpdate* update = NULL;

	WINPR_ASSERT(video);
	WINPR_ASSERT(surface);

	gdi = (rdpGdi*)video->custom;
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(gdi->context);

	update = gdi->context->update;
	WINPR_ASSERT(update);

	if (!update_begin_paint(update))
		goto fail;

	if ((gdi->width < 0) || (gdi->height < 0))
		goto fail;
	else
	{
		const UINT32 nXSrc = surface->x;
		const UINT32 nYSrc = surface->y;
		const UINT32 nXDst = nXSrc;
		const UINT32 nYDst = nYSrc;
		const UINT32 width = (destinationWidth + surface->x < (UINT32)gdi->width)
		                         ? destinationWidth
		                         : (UINT32)gdi->width - surface->x;
		const UINT32 height = (destinationHeight + surface->y < (UINT32)gdi->height)
		                          ? destinationHeight
		                          : (UINT32)gdi->height - surface->y;

		WINPR_ASSERT(gdi->primary_buffer);
		WINPR_ASSERT(gdi->primary);
		WINPR_ASSERT(gdi->primary->hdc);

		if (!freerdp_image_scale(gdi->primary_buffer, gdi->primary->hdc->format, gdi->stride, nXDst,
		                         nYDst, width, height, surface->data, surface->format,
		                         surface->scanline, 0, 0, surface->w, surface->h))
			goto fail;

		if ((nXDst > INT32_MAX) || (nYDst > INT32_MAX) || (width > INT32_MAX) ||
		    (height > INT32_MAX))
			goto fail;

		gdi_InvalidateRegion(gdi->primary->hdc, (INT32)nXDst, (INT32)nYDst, (INT32)width,
		                     (INT32)height);
	}

	rc = TRUE;
fail:

	if (!update_end_paint(update))
		return FALSE;

	return rc;
}

static BOOL gdiVideoDeleteSurface(VideoClientContext* video, VideoSurface* surface)
{
	WINPR_UNUSED(video);
	VideoClient_DestroyCommonContext(surface);
	return TRUE;
}

void gdi_video_control_init(rdpGdi* gdi, VideoClientContext* video)
{
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(video);

	gdi->video = video;
	video->custom = gdi;
	video->createSurface = gdiVideoCreateSurface;
	video->showSurface = gdiVideoShowSurface;
	video->deleteSurface = gdiVideoDeleteSurface;
	video->setGeometry(video, gdi->geometry);
}

void gdi_video_control_uninit(rdpGdi* gdi, WINPR_ATTR_UNUSED VideoClientContext* video)
{
	WINPR_ASSERT(gdi);
	gdi->video = NULL;
}

void gdi_video_data_init(WINPR_ATTR_UNUSED rdpGdi* gdi, WINPR_ATTR_UNUSED VideoClientContext* video)
{
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(gdi->context);
}

void gdi_video_data_uninit(WINPR_ATTR_UNUSED rdpGdi* gdi,
                           WINPR_ATTR_UNUSED VideoClientContext* context)
{
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(gdi->context);
}

VideoSurface* VideoClient_CreateCommonContext(size_t size, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
	VideoSurface* ret = NULL;

	WINPR_ASSERT(size >= sizeof(VideoSurface));

	ret = calloc(1, size);
	if (!ret)
		return NULL;

	ret->format = PIXEL_FORMAT_BGRX32;
	ret->x = x;
	ret->y = y;
	ret->w = w;
	ret->h = h;
	ret->alignedWidth = ret->w + 32 - ret->w % 16;
	ret->alignedHeight = ret->h + 32 - ret->h % 16;

	ret->scanline = ret->alignedWidth * FreeRDPGetBytesPerPixel(ret->format);
	ret->data = winpr_aligned_malloc(1ull * ret->scanline * ret->alignedHeight, 64);
	if (!ret->data)
		goto fail;
	return ret;
fail:
	VideoClient_DestroyCommonContext(ret);
	return NULL;
}

void VideoClient_DestroyCommonContext(VideoSurface* surface)
{
	if (!surface)
		return;
	winpr_aligned_free(surface->data);
	free(surface);
}
