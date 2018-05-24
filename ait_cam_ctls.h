/*
 * linux/drivers/media/video/s5p-mfc/s5p_mfc_enc.h
 *
 * Copyright (C) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _AIT_CAM_CTLS_H_
#define _AIT_CAM_CTLS_H_
// list of profile flag.
// 
//
typedef enum {
    CONSTRAINED_BASELINE_FLAG1 = 0x4202 ,
    CONSTRAINED_BASELINE_P = 0x4240,
    BASELINE_P = 0x4200 ,
    MAIN_P = 0x4D00,
    CONSTRAINED_HIGH_P_FLAG1 = 0x6430,
    CONSTRAINED_HIGH_P = 0x640C , // New in 1.5e 
    HIGH_P = 0x6400,
    SCALABLE_CONSTRAINED_BASELINE_P = 0x5304 , // New in 1.5e
    SCALABLE_BASELINE_P=0x5300,
    SCALABLE_CONSTRAINED_HIGH__P = 0x5604 , // New in 1.5e
    SCALABALE_HIGH_P = 0x5600,
    MULTIVIEW_HIGH_P = 0x7600,
    STEREO_HIGH_P = 0x8000,
    CONSTRAINED_SET0_FLAG = 0x0080,
    CONSTRAINED_SET1_FLAG = 0x0040,
    CONSTRAINED_SET2_FLAG = 0x0020,
    CONSTRAINED_SET3_FLAG = 0x0010,
    CONSTRAINED_SET4_FLAG = 0x0008,
    CONSTRAINED_SET5_FLAG = 0x0004
   
} H264Profile ;

struct aitcam_fmtpipe_map
{
    __u32 format ;
    int   pipe   ;
    int   ctxid  ;
    int   base_addr;
    int   max_width;
    int   max_height;
    int   min_width;
    int   min_height;
    int   max_pool_size;
    int   disable ;
} ;

const struct v4l2_ioctl_ops *aitcam_get_ioctl_ops(void);
int     aitcam_v4l2_ctrls_setup(struct aitcam_ctx *ctx);
void    aitcam_v4l2_ctrls_delete(struct aitcam_ctx *ctx);

void    aitbh_isp_frame_start (void *arg);
void    aitbh_h264enc_enc_start(void *arg);

int     aitcam_stop_stream(struct aitcam_ctx *ctx);
int     aitcam_v4l2_ctrl_val(struct aitcam_ctx *ctx,u32 id,s32 *val);
#if (USING_FIXED_MEM_POOL)
__u32 aitcam_get_format_by_ctx_id(struct aitcam_dev *dev, int ctx_id); 
__u32 aitcam_get_pipe_by_ctx_id(struct aitcam_dev *dev, int ctx_id) ;
__u32 aitcam_get_baseaddr_by_ctx_id(struct aitcam_dev *dev, int ctx_id); 
void aitcam_set_jpg_baseaddr(__u32 addr,__u32 max_pool); 
__u32 aitcam_set_h264_s2_baseaddr(__u32 addr  );
__u32 aitcam_get_maxpoolsize_by_ctx_id(struct aitcam_dev *dev, int ctx_id);
int aitcam_pipealloc(int ctxid,__u32 format, int img_width, int img_height);
int aitcam_pipefree(int ctxid);
int aitcam_pipereset(void);
#endif

#if AITCAM_IPC_EN
int aitcam_streamalloc(int ctxid,__u32 format, int img_width, int img_height);
int aitcam_streamfree(int ctxid);
int aitcam_streamreset(void);

#endif

#endif /* _AIT_CAM_CTLS_H_  */
