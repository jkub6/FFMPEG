/*
Adaptation of the FFMPEG bmp codec to deffine a cool codec.
The cool codec has the "magic number" of "co" It deffines
width and height in the file header,
followed by pixel data in RGB8 format (3 - red, 3 - green, 2 - blue).

Last modified 2/26/2019: Jacob Larkin and Campbell McGavin
*/


/*
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
    const uint8_t *buf_pos_0 = buf;             /* the position of the beginning of the buffer */
    avctx->pix_fmt = AV_PIX_FMT_RGB8;           /* set the picture format to RGB8 color profile */


    /*
      Assure that the first two bytes of a .cool file are correct.
    */
    if (bytestream_get_byte(&buf) != 'c' ||
        bytestream_get_byte(&buf) != 'o') {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }


    /*
      define the width and height of the image by looking into header info from .cool
    */
    width  = bytestream_get_le32(&buf);
    height = bytestream_get_le32(&buf);


    /*
      assure that the dimensions are correctly set
    */
    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }


    /* assure that we can successfully get the buffer */
    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;

    /*  "I am not sure what this does." - Peter Jensen 02/26/2019 @ 15:12:37 */
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    /* set buffer to the start of image data */
    buf   = buf_pos_0 + hsize;

    /* compute n to be used later on */
    n = ((avctx->width * color_depth + 31) / 8) & ~3;

    /* verify that the image height is greater than 0 */
    if (height > 0) {
        p_data      = p->data[0] + (avctx->height - 1) * p->linesize[0];
        linesize = -p->linesize[0];
    } else {
        p_data      = p->data[0];
        linesize = p->linesize[0];
    }


    /* read through the entire file pixel data and copy into the buffer*/
    for (i = 0; i < avctx->height; i++)
    {
        memcpy(p_data, buf, n);
        buf += n;
        p_data += linesize;
    }
    *got_frame = 1;

    /* return the size of the buffer */
    return buf_size;
}

/* The struct in charge of setting up all properties for the decoding of a .cool file */
AVCodec ff_cool_decoder = {
    .name           = "cool",
    .long_name      = NULL_IF_CONFIG_SMALL("long cool"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_COOL,
    .decode         = cool_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
