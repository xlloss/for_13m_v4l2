/*
 * * Alpha-Image Tech. v 0.1
 *
 * This file contains definitions of enums and structs used by the camera
 * driver.
 *
 * Copyright (C) 201x AIT Co., Ltd.
 * xx xx, <x@a-i-t.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version
 */

#ifndef AIT_CAM_RAWSTREAM_H_
#define AIT_CAM_RAWSTREAM_H_


int ait_rawstream_ioctl (struct aitcam_ctx *ctx, struct aitcam_stream_param *param);

int ait_calculate_rawdata_size (__u32 fmt, int width, int height);


#endif //AIT_CAM_RAWSTREAM_H_
