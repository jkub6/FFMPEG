/*
 * BMP image format decoder
 * Copyright (c) 2005 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* here is just a little test comment to prove the clone repo worked -campbell */
/* here is another test comment -Jake*/
#include <inttypes.h>

#include "avcodec.h"
#include "bytestream.h"
#include "cool.h"
#include "internal.h"
#include "msrledec.h"

static int cool_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data;
    unsigned int fsize, hsize;
    int width, height;
    unsigned int depth;
    unsigned int ihsize;
    int i, j, n, linesize, ret;
    uint32_t rgb[3] = {0};
    uint32_t alpha = 0;
    uint8_t *ptr;
    int dsize;
    const uint8_t *buf0 = buf;
    GetByteContext gb;

    hsize = 10;

    if (bytestream_get_byte(&buf) != 'c' ||
        bytestream_get_byte(&buf) != 'o') {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }


        width  = bytestream_get_le32(&buf);
        height = bytestream_get_le32(&buf);


    depth = 8;

    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }

    avctx->pix_fmt = AV_PIX_FMT_RGB8;

    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;

    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    buf   = buf0 + hsize;
    dsize = buf_size - hsize;

    /* Line size in file multiple of 4 */
    n = ((avctx->width * depth + 31) / 8) & ~3;

    if (n * avctx->height > dsize) {
        n = (avctx->width * depth + 7) / 8;
        if (n * avctx->height > dsize) {
            av_log(avctx, AV_LOG_ERROR, "not enough data (%d < %d)\n",
                   dsize, n * avctx->height);
            return AVERROR_INVALIDDATA;
        }
        av_log(avctx, AV_LOG_ERROR, "data size too small, assuming missing line alignment\n");
    }

    if (height > 0) {
        ptr      = p->data[0] + (avctx->height - 1) * p->linesize[0];
        linesize = -p->linesize[0];
    } else {
        ptr      = p->data[0];
        linesize = p->linesize[0];
    }



            for (i = 0; i < avctx->height; i++)
            {
                memcpy(ptr, buf, n);
                buf += n;
                ptr += linesize;
            }


    *got_frame = 1;

    return buf_size;
}

AVCodec ff_cool_decoder = {
    .name           = "cool",
    .long_name      = NULL_IF_CONFIG_SMALL("long cool"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_COOL,
    .decode         = cool_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
