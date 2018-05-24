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

#ifndef AIT_CAM_COMMON_H_
#define AIT_CAM_COMMON_H_

#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>

#include "ait_cam_v4l2.h"
#include "ait_osd.h"

#include "includes_fw.h"
#include "mmpf_vidbuf.h"
#include "mmpf_fctl.h"
#include "mmpf_h264enc.h"

#include "hdm_ctl.h"


#if (CHIP == MCR_V2)
// undef the definition in config_fw.h 
// & redefine here
//#undef  H264_LOW_BITRATE_CONTROL
//#define H264_LOW_BITRATE_CONTROL        (1) // support low bitrate control. if on, h264 dosen't support temp layers

//#undef  AITCAM_MULTI_STREAM_EN
//#define AITCAM_MULTI_STREAM_EN        (1) // support low bitrate control. if on, h264 dosen't support temp layers
#endif

//#define AITCAM_IPC_DISABLE        (0)
//#define AITCAM_IPC_ONLY           (1)
//#define AITCAM_IPC_AND_LOCAL      (2)
#define AITCAM_IPC_EN             (1)//(AITCAM_IPC_AND_LOCAL)



#define STATISTIC_EN                (0)

#define HDR_SUPPORT             (0)

#if (CUSTOMER == HBS)
#define SUPPORT_DUAL_SNR_PRV             (1)
#else
#define SUPPORT_DUAL_SNR_PRV             (0)
#endif

#define HDR_3M                  (0)
#define ISP_EN                  (1)
#define AITCAM_EVENT_QUEUE      (0) // implement event queue to report data

#define AITCAM_REALTIME_MODE    (0)

#if AITCAM_MULTI_STREAM_EN
#define AITCAM_NUM_CONTEXTS     (6) // match to cpu-b ver: 3.1.3
#else
#define AITCAM_NUM_CONTEXTS     (6)
#endif
#define AITCAM_MAX_BUFFERS      (32)
#define AITCAM_MAX_CTRLS        (64)

#define AITCAM_SCHED_RT         (1)
#if (CUSTOMER == LON)
#define AITCAM_FIX_SENSOR_FR    (1)
#else
#define AITCAM_FIX_SENSOR_FR    (0)
#endif

#define GET_TIMESTAMP_AFTER_H264ENC    (1)   

//#define AITCAM_DBG_MSG

#define SUPPORT_MAX_1080P       (1)
#define SUPPORT_MAX_360P         (0)

#if 1//(CUSTOMER == CVN)
    #define OPTIMUM_RESV_DRAM_SIZE    (1)
#else
    #define OPTIMUM_RESV_DRAM_SIZE    (0)
#endif

#define AIT_ISP_VIDEO_IQ                    (0)
#if (AIT_ISP_VIDEO_IQ == 1)
    #define GNR_ADJUST_BY_VIDEO     (0)
    #define LOW_LIGHT_BR_DEC        (1)
    #define WTF_LOW_BR              (1)
    #if (WTF_LOW_BR == 1)
        #define WTF_MAGIC_TIME      (4)     ///< seconds
        #define WTF_MAGIC_IQP_H     (32)
        #define WTF_MAGIC_SKIP_H    (4096)
        #define WTF_MAGIC_STOP_H    (4096)
    #endif
#endif //(AIT_ISP_VIDEO_IQ == 1)
#define AIT_FOV_CROP_EN                     (0)
#define AIT_OSD_EN                          (1)
#define AIT_WATERMARK_EN                          (1)
#define AIT_FDFRAME_EN                          (1)

#define ISP_SATURATION_LEVEL_NOMAL          (0)
#define ISP_SATURATION_LEVEL_GRAY           (-128)

#if (SUPPORT_MAX_1080P)
#define MAX_WIDTH_H264          (1920)
#define MAX_HEIGHT_H264         (1088)
#define MIN_WIDTH_H264          (640)  ///< dynamic incr. resol. limit
#define MIN_HEIGHT_H264         (368)   ///< dynamic incr. resol. limit
#elif (SUPPORT_MAX_360P)
#define MAX_WIDTH_H264          (640)
#define MAX_HEIGHT_H264         (368)
#define MIN_WIDTH_H264          (640)  ///< dynamic incr. resol. limit
#define MIN_HEIGHT_H264         (368)   ///< dynamic incr. resol. limit
#else
#define MAX_WIDTH_H264          (1280)
#define MAX_HEIGHT_H264         (720)
#define MIN_WIDTH_H264          (640)  ///< dynamic incr. resol. limit
#define MIN_HEIGHT_H264         (368)   ///< dynamic incr. resol. limit
#endif

#if (SUPPORT_MAX_1080P)
#define MAX_WIDTH_JPEG          (1920)
#define MAX_HEIGHT_JPEG         (1080)
#else
#define MAX_WIDTH_JPEG          (1280)
#define MAX_HEIGHT_JPEG         (720)
#endif

#if (CUSTOMER == CVN)
#define H264_SPSPPS_EVERY_FRM   (1)
#else
#define H264_SPSPPS_EVERY_FRM   (0)
#endif

#define RESV_DRAM_V1            (1)

#if (CUSTOMER == CVN) || (CUSTOMER == LGT) || (CUSTOMER == HBS)
#define USING_FIXED_MEM_POOL    (1)
#else
#define USING_FIXED_MEM_POOL    (0)
#endif

/* Reserving hardware memory used for dynamic multi-streaming */
#define RESV_SRAM_ISP           (ISP_BUFFER_SIZE+IQ_OPR_DMA_SIZE)//(32*1024)
#if (AITCAM_REALTIME_MODE)
//#define RESV_SRAM_H264          (MAX_WIDTH_H264*16*2)
#define RESV_SRAM_H264          (MAX_WIDTH_H264*16*2*3/2)//(92160)//(0)
#else
#define RESV_SRAM_H264          (0)
#endif
#define RESV_SRAM_JPEG          (MAX_WIDTH_JPEG << 5)
#if(HDR_SUPPORT == 1)
#define RESV_DRAM_HDR           ((2312*1520*2*4/3)+256)*2
#endif
#define RESV_DRAM_ISP           (0)
#define RESV_DRAM_H264          (ALIGN_PAGE((((MAX_WIDTH_H264*MAX_HEIGHT_H264)\
                                  >>(8+2))+2)<<2) + H264E_DUMMY_BUF_SIZE) //min 4MB per slice
#define RESV_DRAM_JPEG          (0)
#if AITCAM_MULTI_STREAM_EN
#define RESV_DRAM_GRA           (((MAX_WIDTH_H264*MAX_HEIGHT_H264*3)>>1)*2)
#endif

#if SUPPORT_DUAL_SNR_PRV
#define MAX_WIDTH_RAW          (640)
#define MAX_HEIGHT_RAW         (480)
#define RESV_DRAM_RAW           (((MAX_WIDTH_RAW*MAX_HEIGHT_RAW*2))*2)
#else
#define RESV_DRAM_RAW           (0)
#endif

#define RESV_SRAM_HW            ((RESV_SRAM_ISP+RESV_SRAM_H264+RESV_SRAM_JPEG+0xFF) & ~0xFF)
#if AITCAM_MULTI_STREAM_EN
    #if(HDR_SUPPORT == 1)
	#define RESV_DRAM_HW            ((RESV_DRAM_ISP+RESV_DRAM_H264+RESV_DRAM_JPEG+0xFF+RESV_DRAM_HDR+RESV_DRAM_RAW) & ~0xFF)
    //#define RESV_DRAM_HW            ((RESV_DRAM_ISP+RESV_DRAM_H264+RESV_DRAM_JPEG+RESV_DRAM_GRA+0xFF+RESV_DRAM_HDR) & ~0xFF)
    #else
#define RESV_DRAM_HW            ((RESV_DRAM_ISP+RESV_DRAM_H264+RESV_DRAM_JPEG+RESV_DRAM_GRA+0xFF+RESV_DRAM_RAW) & ~0xFF)
    #endif
#else
    #if(HDR_SUPPORT == 1)
    #define RESV_DRAM_HW            ((RESV_DRAM_ISP+RESV_DRAM_H264+RESV_DRAM_JPEG+0xFF+RESV_DRAM_HDR+RESV_DRAM_RAW) & ~0xFF)
#else
#define RESV_DRAM_HW            ((RESV_DRAM_ISP+RESV_DRAM_H264+RESV_DRAM_JPEG+0xFF+RESV_DRAM_RAW) & ~0xFF)
#endif
#endif
    /* minimum memory allocation size per ctx */
#define MIN_RESV_SRAM_CTX       (0)
#if AITCAM_MULTI_STREAM_EN
#define MIN_RESV_DRAM_CTX       (((MAX_WIDTH_H264*MAX_HEIGHT_H264*3)>>1)*2+1024*1024+230*1024)
#else
#define MIN_RESV_DRAM_CTX       (((MAX_WIDTH_H264*MAX_HEIGHT_H264*3)>>1)*3+1024*1024+230*1024)
#endif
/* Reserving hardware memory end */
/* Depends on project specification, remember to adjust this size */
//#define MIN_RESV_DRAM_CTX       (5400 * 1024)

#if AITCAM_IPC_EN
#undef  AITCAM_MULTI_STREAM_EN
#define AITCAM_MULTI_STREAM_EN    (0)
#undef  AIT_OSD_EN
#define AIT_OSD_EN                (0)
#undef  AIT_WATERMARK_EN           
#define AIT_WATERMARK_EN          (0)
#undef  AIT_FDFRAME_EN
#define AIT_FDFRAME_EN            (0)
#undef  HDR_SUPPORT
#define HDR_SUPPORT               (0)

//#undef  AIT_ISP_VIDEO_IQ
//#define AIT_ISP_VIDEO_IQ          (0)
#undef  USING_FIXED_MEM_POOL       
#define USING_FIXED_MEM_POOL      (1)
#undef  SUPPORT_DUAL_SNR_PRV       
#define SUPPORT_DUAL_SNR_PRV      (0)
//#undef  AITCAM_EVENT_QUEUE  
//#define AITCAM_EVENT_QUEUE        (0)
/*
#undef  RESV_DRAM_HW
#define RESV_DRAM_HW              (0)
#undef  RESV_SRAM_HW
#define RESV_SRAM_HW              (0)
*/
#endif


#define RUN_AT_IPC_OFF(ipc,statement) \
do {                                  \
	if(!ipc) {                          \
	  statement 								        \
  }                                   \
} while (0)

#define RUN_AT_IPC_ON(ipc,statement)  \
do {                                  \
	if( ipc) {                          \
	  statement   							        \
  }                                   \
} while (0)



enum ait_mem_type {
    MEM_SRAM = 0,
    MEM_DRAM,
    MEM_MAX
};

enum ait_mem_idx {
    AIT_MEM_IDX_ISP = AITCAM_NUM_CONTEXTS,
    AIT_MEM_IDX_H264,
    AIT_MEM_IDX_JPEG,
    #if AITCAM_MULTI_STREAM_EN
    AIT_MEM_IDX_GRA,
    #endif
    #if (HDR_SUPPORT == 1) || (SUPPORT_DUAL_SNR_PRV)
    AIT_MEM_IDX_RAW,
    #endif

    AIT_MEM_IDX_MAX
};

struct ait_mem_regions {
    MMP_ULONG   base_addr[MEM_MAX];
    MMP_ULONG   end_addr[MEM_MAX];
    MMP_ULONG   max_buf_size[MEM_MAX];
    MMP_BOOL    allocated;
};

/**
 * enum aitcam_state - for each file handle
 */
enum aitcam_state {
	AITCAM_STAT_OPEN = 0,
	AITCAM_STAT_ALLOC,
	AITCAM_STAT_RUNNING,
	AITCAM_STAT_FINISH,
	AITCAM_STAT_ERROR,
//	AITCAM_STAT_ABORT,
//	AITCAM_STAT_RES_CHANGE_INIT,
//	AITCAM_STAT_RES_CHANGE_FLUSH,
//	AITCAM_STAT_RES_CHANGE_END
};

/**
 * enum aitcam_stream_state, state of the uncompressed stream
 */
enum aitcam_stream_state {
    AITCAM_STREAM_NONE = 0,
    AITCAM_STREAM_START,
    AITCAM_STREAM_STOP
};

/**
 * enum aitcam_stream_cmd, command for the uncompressed stream
 */
enum aitcam_stream_cmd {
    AITCAM_STREAM_S_START = 0,
    AITCAM_STREAM_S_STOP,
    AITCAM_STREAM_G_STATE
};

struct aitcam_video_fmt {
	char *name;
	u32 fourcc;
};

/**
 * struct aitcam_stream_param - parameter for the uncompressed stream
 */
struct aitcam_stream_param {
    enum aitcam_stream_cmd cmd;
    void *arg;
};

/**
 * struct aitcam_rawstream_ctx - instance for the uncompressed stream
 */
struct aitcam_rawstream_ctx {
    enum aitcam_stream_state operation;
    enum aitcam_stream_state status;

    __u32   frame_size;
    __u32   frame_addr[3];
};

/**
 * struct ait_h264_enc_params - encoding parameters for h264
 */
struct ait_h264_enc_params {
//	enum v4l2_mpeg_video_h264_profile   profile;
	enum v4l2_mpeg_video_h264_entropy_mode entropy_v4l2;
	enum v4l2_mpeg_video_h264_level     level_v4l2;
//	u8 max_ref_pic;
//	u8 rc_frame_qp[3];
//	u8 rc_min_qp[3];
//	u8 rc_max_qp[3];
//	enum v4l2_mpeg_video_h264_loop_filter_mode loop_filter_mode;
//	s8 loop_filter_alpha;
//	s8 loop_filter_beta;
//	u16 cpb_size;
};

/**
 * struct aitcam_params - general encoding parameters
 */
struct aitcam_params {
	u16 width;
	u16 height;

	u16 gop_size;
	enum v4l2_mpeg_video_multi_slice_mode slice_mode;
	u16 slice_mb;
	u32 slice_byte;
	u16 intra_refresh_mb;
	int pad;
	u8 pad_luma;
	u8 pad_cb;
	u8 pad_cr;
	u16 rc_reaction_coeff;
	u16 vbv_size;

	enum v4l2_mpeg_video_header_mode seq_hdr_mode;
	enum v4l2_mpeg_mfc51_video_frame_skip_mode frame_skip_mode;
	int fixed_target_bit;

	u8 num_b_frame;
	u32 fps_increament;
	u32 fps_resolution;
	int interlace;

	union {
		struct ait_h264_enc_params h264;
	} codec;

};

#if (AIT_ISP_VIDEO_IQ == 1)
#include    "ait_iq_video.h"
#endif
/**
 * struct aitcam_dev - The struct containing driver internal parameters.
 *
 * @v4l2_dev:       v4l2_device
 * @vfd_cam:        video device for encoding
 * @plat_dev:       platform device
 * @num_opening:    couter of active instances
 * @num_streaming:  couter of instances under streaming
 * @irqlock:		lock for operations on vidbuf queues
 * @aitcam_mutex:   lock for video_device
 * @ctx:            array of driver contexts
 *
 */
#define DEV_TYPE_V4L2_RESET_VAL		((__s32)0x80000000)
struct aitcam_dev {
	struct v4l2_device	    v4l2_dev;
	struct video_device	    *vfd_cam;
	struct platform_device  *plat_dev;
	//struct v4l2_ctrl_handler    ctrl_handler;
	int                 num_opening;
	int                 num_streaming;
    #if AITCAM_MULTI_STREAM_EN
	int                 num_reqbufs;
    int                 num_streamon;
	#endif

	spinlock_t          irqlock;        /* lock when operating on video buf queues */
	struct mutex        aitcam_mutex;   /* video_device lock */

	unsigned long       ctx_work_bits;

    int                 isp_ir_refresh;
    #if (AIT_ISP_VIDEO_IQ == 1)
    int                 isp_vidiq_ctx_num;
    int                 isp_vidiq_en;
    AIT_IQ_VIDEO_HANDLE hVidIq;
    #endif
    //enum v4l2_camera_ait_orientation   orientation;
    __s32	 orientation;
    __s32  brightness;
    __s32  contrast;
    __s32  hue;
    __s32  saturation;
    __s32  gamma;
    __s32  sharpness;
    __s32  wdr;
    __s32  powerfreq;
    
#if 1 // keep night mode / light condition
    __s32  nv_mode;  // 0: off, 1: on, 2:auto
    __s32  is_night ;
    __s32  lightcond;
    __s32  nr_gain;
#endif
    struct workqueue_struct     *SensorTask;
    struct workqueue_struct     *H264EncTask;
    struct work_struct          IspFrameStWork;
    #if (AITCAM_SCHED_RT == 1)
    struct work_struct          SetupTaskWork;
    #endif

	struct aitcam_ctx           *ctx[AITCAM_NUM_CONTEXTS];

    struct ait_mem_regions      dev_mem;
	struct ait_mem_regions      mem_regions[AIT_MEM_IDX_MAX];
	MMP_ULONG                   mem_ptr[MEM_MAX];

    #if (RESV_DRAM_V1 == 1)
    MMP_ULONG                   mem_hwresv_ptr[MEM_MAX];
    MMP_ULONG                   mem_hwresv_end[MEM_MAX];
    #endif
    #if (AIT_OSD_EN == 1)
    struct {
        MMP_ULONG               nticks;
        unsigned long           target_time;

        AIT_RTC_TIME            handle;
        struct timer_list       event;
        spinlock_t              time_rdlock;
    } rtc; // for OSD RTC time control
    #endif
    //int   cpu_ipc ;
};

#define CTX_STOP_V4L2_RET_ERR   (1<<0)

#define V4L2_IPC_FLAG_RTOS_INIT (1<<0)
/**
 * struct aitcam_ctx - This struct contains the instance context
 *
 * @dev:		pointer to device handle
 * @fh:			struct v4l2_fh
 * @num:		number of the context that this structure describes
 * @inst_state:	state of the context
 * @img_width:		width of the image that is decoded or encoded
 * @img_height:		height of the image that is decoded or encoded
 * @enc_dst_buf_size:	size of the buffers for encoder output
 * @ctrls:		array of controls, used when adding controls to the
 *			    v4l2 control framework
 * @ctrl_handler:	handler for v4l2 framework
 */
struct aitcam_ctx {
	struct aitcam_dev *dev;
	struct v4l2_fh fh;

	int num;
	int img_pipe;

    MMPF_VIDBUF_QUEUE   *cam_vbq;
    MMPF_FCTL_LINK      pipe_link;
    #if AITCAM_MULTI_STREAM_EN
    MMPF_FCTL_LINK      pipe_link_1;
    #endif
    void                *codec_handle;

    enum aitcam_state   inst_state;
  union {
	int                 img_width;
	int                 bitrate ;
  } ;
  union {
	int                 img_height;
	int                 samplerate;
  };
	int                 scale_resol;
	int                 snr_src_id;
  int                 loop_recording;
  int                 realtime ;
#if 1 // DEBUG
  int                 qcnt,dqcnt ;
  MMP_ULONG64         ts;
  int                 buf_index;
#endif 
 
    __u32               output_format;
    __u32               ctx_flags ; // control flags for each ctx

	struct aitcam_params    cam_params;

	size_t enc_dst_buf_size;

	union {
        struct work_struct      EncStartWork;
        struct aitcam_rawstream_ctx RawStreamCtx;
	};

	#if (AIT_OSD_EN == 1)
	struct {
	    int                     active_num;
	    struct mutex            mlock;  /* guard the osd struct in this ctx */
	    /* cache for time in str when current osd type is RTC.              */
	    /* if sec is same with RTC one, then no need update time to str.    */
	    AIT_RTC_TIME            local_time;
	    bool                    resync_time;
	    struct {
	        enum ait_osd_type   type;
	        char                str[MAX_OSD_STR_LEN];
	        AIT_OSD_HANDLE      hdl;
	    } win[MAX_OSD_INDEX];
	} osd;
    #endif
	struct v4l2_ctrl *ctrls[AITCAM_MAX_CTRLS];
	struct v4l2_ctrl_handler ctrl_handler;
};

/**
 * struct aitcam_control - structure used to store information about MFC controls
 *			it is used to initialize the control framework.
 */
#define AITCAM_FLAG_VOLATILE    (1)
#define AITCAM_FLAG_EXE_ON_WR   (2) // need kernel v4l2 backport. 
struct aitcam_control {
	__u32			id;
	enum v4l2_ctrl_type	type;
	__u8			name[32];  /* Whatever */
	__u32           v4l2_pixelformat;
	__s32			minimum;   /* Note signedness */
	__s32			maximum;
	__s32			step;
	__u32			menu_skip_mask;
	__s32			default_value;
	__u32			flags;
	__u32     ipc_flags;
	__u32			reserved;
	__u8			is_volatile;
};

#define fh_to_ctx(__fh) container_of(__fh, struct aitcam_ctx, fh)

#define ait_err(fmt, args...)				\
	do {						\
		printk(KERN_ERR "%s:%d: " fmt,		\
		       __func__, __LINE__, ##args);	\
	} while (0)



int     aitcam_mem_getinfo (struct aitcam_dev *dev, enum ait_mem_idx mem_idx,
                            struct ait_mem_regions *available);
int     aitcam_mem_alloc (struct aitcam_dev *dev, enum ait_mem_idx mem_idx,
                            struct ait_mem_regions *alloc);
int aitcam_mem_release (struct aitcam_dev *dev, enum ait_mem_idx mem_idx);
int     aitcam_get_fov_crop_info (MMP_ULONG *base, MMP_ULONG *ratio);
void    aitcam_queue_wakeup(MMPF_VIDBUF_QUEUE *pVbq);

extern int debug_level;


#endif /* AIT_CAM_COMMON_H_ */
