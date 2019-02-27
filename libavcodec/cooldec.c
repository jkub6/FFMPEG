/*
test images with quality... this is a subjective test by the TA's
test stuff with patterns...
test one nature def

find a 1024*1024 image and make sure that it is roughly one megabyte


*/













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


/*
Include statements
*/
#include <inttypes.h>
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "msrledec.h"

/*
Take in image of .cool file type and decode it to be a packet of data native to ffmpeg
*/
static int cool_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;           /* the header and image data*/
    int buf_size       = avpkt->size;           /* the total size of the file */
    AVFrame *p         = data;                  /* a pointer to where we are at in reading data */
    unsigned int hsize = 10;                    /* the size of the header info, unique to our .cool format */
    unsigned int color_depth = 8;               /* the depth or quality of color <in bits> the .cool format */
    int width, height, i, j, n, linesize, ret;  /* helper fields to keep track of where we are at in reading the file */
    uint8_t *p_data;                            /* a pointer to where we are at in reading the data */
    const uint8_t *buf0 = buf;
    GetByteContext gb;

    avctx->pix_fmt = AV_PIX_FMT_RGB8;

    if (bytestream_get_byte(&buf) != 'c' ||
        bytestream_get_byte(&buf) != 'o') {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }


    width  = bytestream_get_le32(&buf);
    height = bytestream_get_le32(&buf);


    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }



    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;

    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    buf   = buf0 + hsize;

    /* Line size in file multiple of 4 */
    n = ((avctx->width * color_depth + 31) / 8) & ~3;

    if (height > 0) {
        p_data      = p->data[0] + (avctx->height - 1) * p->linesize[0];
        linesize = -p->linesize[0];
    } else {
        p_data      = p->data[0];
        linesize = p->linesize[0];
    }



    for (i = 0; i < avctx->height; i++)
    {
        memcpy(p_data, buf, n);
        buf += n;
        p_data += linesize;
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
