/*
Adaptation of the FFMPEG bmp codec to deffine a cool codec.
The cool codec has the "magic number" of "co" It deffines
width and height in the file header,
followed by pixel data in RGB8 format (3 - red, 3 - green, 2 - blue).

Last modified 2/26/2019: Jacob Larkin and Campbell McGavin
*/

/*
 * Copyright (c) 2006, 2007 Michel Bardiaux
 * Copyright (c) 2009 Daniel Verkamp <daniel at drv.nu>
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

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"


/*
  Initialize information to successfully encode an image to the .cool format
*/
static av_cold int cool_encode_init(AVCodecContext *avctx)
{
   if (avctx->pix_fmt == AV_PIX_FMT_RGB8)
        avctx->bits_per_coded_sample = 8;
    else
    {
        av_log(avctx, AV_LOG_INFO, "unsupported pixel format\n");
        return AVERROR(EINVAL);
    }
    return 0;
}

/*
  Take data from internal form and putting it into a packet to be written to a file
*/
static int cool_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{

    const AVFrame * const p_pict = pict;                                /* a pointer to the AVFrame pict*/
    int n_bytes_image, n_bytes_per_row, n_bytes, i, hsize, ret;         /* helper fields to keep track of where we are at in reading the file */
    uint32_t palette256[256];                                           /* a palette of maximum available colors*/
    int pad_bytes_per_row = 0;                                          /* the amount of padding per row */
    int bit_count = avctx->bits_per_coded_sample;                       /* in our case this will always be 8 bits per pixel */
    uint8_t *p_data, *buf;                                              /* pointers to the position in the data and the buffer */


    /*  "I am not sure what this does." - Peter Jensen 02/26/2019 @ 15:12:37 */
    #if FF_API_CODED_FRAME
      FF_DISABLE_DEPRECATION_WARNINGS
      avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
      avctx->coded_frame->key_frame = 1;
      FF_ENABLE_DEPRECATION_WARNINGS
    #endif

    /* verify that the color quality is 8 bits per pixel */
    av_assert1(bit_count == 8);
    avpriv_set_systematic_pal2(palette256, avctx->pix_fmt);

    /*
    compute the number of total bytes, number of padding bytes and total image
    bytes to be used in the buffer per row
    */
    n_bytes_per_row = ((int64_t)avctx->width * (int64_t)bit_count + 7LL) >> 3LL;
    pad_bytes_per_row = (4 - n_bytes_per_row) & 3;
    n_bytes_image = avctx->height * (n_bytes_per_row + pad_bytes_per_row);

    /* our header has a constant size of 10, so we don't need to adjust hsize */
    hsize = 10;

    /* compute the total number of bytes, being the bytes from image && header */
    n_bytes = n_bytes_image + hsize;

    /* allocate packet memory and assure no errors */
    if ((ret = ff_alloc_packet2(avctx, pkt, n_bytes, 0)) < 0)
        return ret;

    /* set buffer to beginning of file header*/
    buf = pkt->data;

    /* create unique flag for the .cool image file type... "co" */
    bytestream_put_byte(&buf, 'c');
    bytestream_put_byte(&buf, 'o');

    /* write the width and height of the image to the header info */
    bytestream_put_le32(&buf, avctx->width);
    bytestream_put_le32(&buf, avctx->height);

    /* write the image data from bottom to top and then copy it to memory */
    p_data = p_pict->data[0] + (avctx->height - 1) * p_pict->linesize[0];
    buf = pkt->data + hsize;
    for(i = 0; i < avctx->height; i++)
    {
        memcpy(buf, p_data, n_bytes_per_row);
        buf += n_bytes_per_row;
        memset(buf, 0, pad_bytes_per_row);
        buf += pad_bytes_per_row;
        p_data -= p_pict->linesize[0]; // ... and go back
    }

    /* set the packet flags if initially set or flag key is set */
    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

/* The struct in charge of setting up all properties for the encoding of a .cool file */
AVCodec ff_cool_encoder = {
    .name           = "cool",
    .long_name      = NULL_IF_CONFIG_SMALL("long cool"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_COOL,
    .init           = cool_encode_init,
    .encode2        = cool_encode_frame,
    .pix_fmts       = (const enum AVPixelFormat[]){AV_PIX_FMT_RGB8, AV_PIX_FMT_NONE},
};
