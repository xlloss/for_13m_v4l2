
//#include <cstdlib>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <mach/ait_if.h>

#if AITCAM_IPC_EN
#include <mach/cpucomm/cpucomm_if.h>
#else
#define  CPU_LOCK_INIT()
#define  CPU_LOCK()
#define  CPU_UNLOCK()
#endif
#include "project_id_gen.h"
#include "includes_fw.h"
#include "mmpf_sensor.h"
#include "mmpf_vif.h"
#include "mmpf_ibc.h"
#include "mmpf_vidbuf.h"
#include "mmpf_videnc.h"
#include "mmpf_h264enc.h"
#include "mmpf_jpeg.h"
#include "mmpf_system.h"
#include "isp_if.h"
#include "mmpf_pio.h"
#include "gpio.h"
#include "ait_cam_common.h"
#include "ait_cam_ctls.h"
#include "ait_cam_v4l2.h"
#include "ait_cam_rawstream.h"
#if (AIT_ISP_VIDEO_IQ == 1)
#include "ait_iq_video.h"
#endif
#include "mmpf_graphics.h"
#include "mmpf_scaler.h"
#include "os_wrap.h"

#if (STATISTIC_EN == 1)
#include "stream_stat.h"

#define STAT_CYCLE_LOG2             (0)
#define STAT_CYCLE_MASK             ((1 << STAT_CYCLE_LOG2) - 1)

extern MMPF_OS_TIME_UNIT   aitcam_clockbase(void);
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#if AITCAM_IPC_EN
#include "ait_cam_ipc.h"
#endif
#define CPB_SIZE_CTRL_AS_LB_SIZE  (1)

#if CPB_SIZE_CTRL_AS_LB_SIZE
#define MAX_LBS (5000) // ms
#define DEF_LBS (1000)
#define MIN_LBS (500)
#else

/*
CPB <-> Leaky bucket size convert
*/
#define H264_CPBSize2LBSize(cpbsize,br_kbps)   (16*cpbsize) / (br_kbps)
#define H264_LBSize2CPBSize(lbsize,br_kbps)    (lbsize* (br_kbps)) / 16
#define MAX_LBS (5000) // ms
#define DEF_LBS (1000)
#define MIN_LBS (100)

#endif
#define MAX_BITRATE (60*1000*1000)
#define DEF_BITRATE (768*1000)
#define MIN_BITRATE (25*1000)

#define MAX_FRATE    (60)
/*
V4L2 cpb unit is Kb ( 1024 )
*/
#define Kb(bps)                 (bps >> 10     )
#define CPB2V4L_SIZE(cpb_size)  (cpb_size >> 10) //kb
#define V4L2CPB_SIZE(v4l_cpb )  (v4l_cpb  << 10)

int aitcam_v4l2_ctrl_initbyrtos(u32 id);

#if (STATISTIC_EN == 1)
extern char             stat_en;
extern struct stat_window      stat_win[AITCAM_NUM_CONTEXTS];
static int              frame_number[AITCAM_NUM_CONTEXTS];
static int     tag_timestamp[AITCAM_NUM_CONTEXTS];
#endif

#if (SUPPORT_DUAL_SNR_PRV)
extern int dual_snr_en;
#endif

MMP_BOOL gbHDR_en = 1;


extern int debug_ts ;
extern int force_rdo ;
extern int force_tnr ;

extern int cpu_ipc ;
extern int fdfr_osd ;

#if AITCAM_IPC_EN

#define DEF_STREAM_TYPE V4L2_CAMERA_AIT_REALTIME_TYPE
#define JPEG_S0_ID  (0)
#define H264_S0_ID  (1)
#define H264_S1_ID  (2)
#define H264_S2_ID  (3) // share with jpg, means can't start with jpg

#define GRAY_S0_ID  (4)
#define AAC_S0_ID   (5)
#define AAC_S1_ID   (6)
#define YUY2_S0_ID  (7)

#if USING_FIXED_MEM_POOL

//#define MAX_H264_FRAME_SIZE (119*4096)
#define MAX_H264_FRAME_SIZE (300*4096)
#define MAX_H264_FRAME_SLOT (4)

#define MAX_H264_S2_FRAME_SIZE (64*1024)
#define MAX_H264_S2_FRAME_SLOT (4)

#define USE_STANDARD_13M 1

// only slot buffer, hw buffer is in cpu-b
static struct aitcam_fmtpipe_map fmtpipe_map[] =
{
    
    {
        .format = V4L2_PIX_FMT_H264,
        .pipe = H264_S2_ID ,
        .base_addr = 0x400000,
        .max_width = 640,
        .max_height = 480,
        .max_pool_size = MAX_H264_S2_FRAME_SIZE * MAX_H264_S2_FRAME_SLOT ,
    },
  
    { // share with H264, it means can't active with h264
        .format = V4L2_PIX_FMT_MJPEG,
        .pipe = JPEG_S0_ID,
        .base_addr = 0, // auto assign
#if USE_5M
        .max_width = 2560,
        .max_height = 1920,
#else
#if USE_STANDARD_13M
        .max_width = 4192,
        .max_height = 3104,
#else
        .max_width = 4000,
        .max_height = 3000,
#endif
#endif

        .max_pool_size = MAX_H264_FRAME_SIZE*MAX_H264_FRAME_SLOT ,   
    },
    
    {
        .format = V4L2_PIX_FMT_H264, //
        .pipe = H264_S0_ID ,
        .base_addr = 0,
        .max_width = 1920,
        .max_height = 1080,
        .max_pool_size = MAX_H264_FRAME_SIZE*MAX_H264_FRAME_SLOT,
    },
    {
        .format = V4L2_PIX_FMT_H264, //
        .pipe = H264_S1_ID,
        .base_addr = 0x1DC000,
        .max_width = 1920,
        .max_height = 1080,
        .max_pool_size = MAX_H264_FRAME_SIZE*MAX_H264_FRAME_SLOT,
    },
    {
        .format = V4L2_PIX_FMT_MPEG,
        .pipe = AAC_S0_ID,
        .base_addr = 0x3B8000,
        .max_width = 4096,
        .max_height = 1,
        .max_pool_size = 1*4096*4,   
    },
    {
        .format = V4L2_PIX_FMT_MPEG,
        .pipe = AAC_S1_ID,
        .base_addr = 0x3BC000,
        .max_width = 4096,
        .max_height = 1,
        .max_pool_size = 1*4096*4,   
    },

    {
        .format = V4L2_PIX_FMT_GREY,
        .pipe = GRAY_S0_ID ,
        .base_addr = 0x3C0000,
        .max_width = 640,
        .max_height = 360,
        .max_pool_size = 57*4096*1,
    },

};

#define NUM_PIPES ARRAY_SIZE(fmtpipe_map)
#endif


#endif

static struct aitcam_video_fmt formats[] =
{
    {
        .name = "H264 Stream",
        .fourcc = V4L2_PIX_FMT_H264,
    },
    {
        .name = "MJPG Stream",
        .fourcc = V4L2_PIX_FMT_MJPEG,
    },
    {
        .name = "GREY Stream",
        .fourcc = V4L2_PIX_FMT_GREY,
    },
    {
        .name = "4:2:2 packed, UYVY",
        .fourcc = V4L2_PIX_FMT_UYVY,
    },
    {
        .name = "4:2:2 packed, YUYV",
        .fourcc = V4L2_PIX_FMT_YUYV,
    },
    {
        .name = "4:2:0 planar, Y-Cb-Cr",
        .fourcc = V4L2_PIX_FMT_YUV420,
    },
};
#define NUM_FORMATS ARRAY_SIZE(formats)


static struct aitcam_control controls[] =
{
    {
        .id = V4L2_CID_BRIGHTNESS,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = 0,
        .minimum = -100,
        .maximum = 100,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_CONTRAST,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = 0,
        .minimum = -100,
        .maximum = 100,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_HUE,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = 0,
        .minimum = -180,
        .maximum = 180,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_SATURATION,
        .type = V4L2_CTRL_TYPE_INTEGER,
   
        .v4l2_pixelformat = 0,
        .minimum = -100,
        .maximum = 100,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_POWER_LINE_FREQUENCY,
        .type = V4L2_CTRL_TYPE_MENU,
   
        .v4l2_pixelformat = 0,
        .minimum = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED,
        .maximum = V4L2_CID_POWER_LINE_FREQUENCY_AUTO,
        .default_value = V4L2_CID_POWER_LINE_FREQUENCY_60HZ,
        .menu_skip_mask = 0,
        .ipc_flags = V4L2_IPC_FLAG_RTOS_INIT ,
    },
    {
        .id = V4L2_CID_EXPOSURE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
  
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step = 1,
        .default_value = 1,
    },
    {
        .id = V4L2_CID_AUTO_WHITE_BALANCE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
 
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step = 1,
        .default_value = 1,
    },
    {
        .id = V4L2_CID_GAMMA,
        .type = V4L2_CTRL_TYPE_INTEGER,
 
        .v4l2_pixelformat = 0,
        .minimum = -100,
        .maximum = 100,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_SHARPNESS,
        .type = V4L2_CTRL_TYPE_INTEGER,
 
        .v4l2_pixelformat = 0,
        .minimum = -100,
        .maximum = 100,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_BACKLIGHT_COMPENSATION,
        .type = V4L2_CTRL_TYPE_BOOLEAN,

        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step = 1,
        .default_value = 0,
    },
    {
        .id = V4L2_CID_CAMERA_AIT_ORIENTATION,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "orientation",
        .v4l2_pixelformat = 0,
        .minimum = V4L2_CAMERA_AIT_ORTN_NORMAL,
        .maximum = V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR,
        .step = 1,
        .default_value = V4L2_CAMERA_AIT_ORTN_NORMAL,
        .ipc_flags = V4L2_IPC_FLAG_RTOS_INIT ,
    },
    #if 0
    {
        .id      = V4L2_CID_HFLIP,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 0,
    },
    {
        .id      = V4L2_CID_VFLIP,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 0,
    },
    #endif
    #if AITCAM_IPC_EN==0 // not open yet
    {
        .id = V4L2_CID_CAMERA_AIT_EXPOSURE_STATE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "exposure state"
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 128,
        .step = 1,
        .default_value = 0,
        .is_volatile = 1,
    },
    #endif
    {
        .id = V4L2_CID_CAMERA_AIT_MANUAL_R_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "r gain",
        .v4l2_pixelformat = 0,
        .minimum = 1,
        .maximum = 8 * 1024, //8x gain
        .step = 1,
        .default_value = 1024,
    },
    {
        .id = V4L2_CID_CAMERA_AIT_MANUAL_G_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "g gain",
        .v4l2_pixelformat = 0,
        .minimum = 1,
        .maximum = 8 * 1024, //8x gain
        .step = 1,
        .default_value = 1024,
    },
    {
        .id = V4L2_CID_CAMERA_AIT_MANUAL_B_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "b gain",
        .v4l2_pixelformat = 0,
        .minimum = 1,
        .maximum = 8 * 1024, //8x gain
        .step = 1,
        .default_value = 1024,
    },
    #if 1//AITCAM_IPC_EN==0 // not open yet
    {
        .id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE,
        .type = V4L2_CTRL_TYPE_MENU,
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE,
        .maximum = V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_BYTES,
        .default_value = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE,
        .menu_skip_mask = 0,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 1,
        .maximum = (1 << 16) - 1,
        .step = 1,
        .default_value = (1 << 16) - 1,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 1900,
        .maximum = (1 << 30) - 1,
        .step = 1,
        .default_value = (1 << 30) - 1,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = (1 << 16) - 1,
        .step = 1,
        .default_value = 0,
    },
    #endif
    {
        .id = V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 1,
        .step = 1,
        .default_value = 1,
    },
    {
        .id = V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "fps numerator",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 1,
        .maximum = (MAX_FRATE * 10),
        .step = 1,
        .default_value = 300,
        .ipc_flags = V4L2_IPC_FLAG_RTOS_INIT ,
    },
    {
        .id = V4L2_CID_MPEG_AIT_VIDEO_FPS_INCR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "fps denominator",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 1,
        .maximum = 100,
        .step = 1,
        .default_value = 10,
        .ipc_flags = V4L2_IPC_FLAG_RTOS_INIT ,
    },
    #if 1//AITCAM_IPC_EN==0 // 
    {
        .id = V4L2_CID_MPEG_VIDEO_GOP_SIZE,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = (1 << 16) - 1,
        .step = 1,
        .default_value = 60,
    },
    #endif
    {
        .id = V4L2_CID_MPEG_VIDEO_BITRATE,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = MIN_BITRATE,
        .maximum = MAX_BITRATE,
        .step = 1,
        .default_value = DEF_BITRATE,
    },
    /*	{
    		.id = V4L2_CID_MPEG_VIDEO_H264_CURBUFMODE,//cm
    		.type = V4L2_CTRL_TYPE_INTEGER,
    		.v4l2_pixelformat = V4L2_PIX_FMT_H264,
    		.minimum = 0,
    		.maximum = 2,
    		.step = 1,
    		.default_value = 0,
    	},*/
    {
        .id = V4L2_CID_MPEG_AIT_VIDEO_RC_FRAME_SKIP,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .name = "rc skip",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 1,
        .step = 1,
        .default_value = 0,
    },
    {
        // v4l2 unit is kb
        .id = V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
		#if CPB_SIZE_CTRL_AS_LB_SIZE
    		.minimum = MIN_LBS,
    		.maximum = MAX_LBS,
    		.step = 500,
    		.default_value = DEF_LBS,
		#else
        .minimum = 1 + CPB2V4L_SIZE( H264_LBSize2CPBSize(MIN_LBS , Kb(MIN_BITRATE) )),
        .maximum = CPB2V4L_SIZE( H264_LBSize2CPBSize(MAX_LBS   , Kb(MAX_BITRATE) )),
        .step = 1,
        .default_value = CPB2V4L_SIZE(H264_LBSize2CPBSize( DEF_LBS, Kb(DEF_BITRATE))),
		#endif
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_I_PERIOD,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "gop",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = (1 << 16) - 1,
        .step = 1,
        .default_value = 60,
        .is_volatile =( AITCAM_FLAG_EXE_ON_WR | AITCAM_FLAG_VOLATILE ),
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_PROFILE,
        .type = V4L2_CTRL_TYPE_MENU,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
        //.maximum = V4L2_MPEG_VIDEO_H264_PROFILE_MULTIVIEW_HIGH,
        .maximum = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH,
        #if (AITCAM_REALTIME_MODE)
        .default_value = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
        #else
        .default_value = V4L2_MPEG_VIDEO_H264_PROFILE_MAIN,
        #endif
        .menu_skip_mask = 0,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_LEVEL,
        .type = V4L2_CTRL_TYPE_MENU,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = V4L2_MPEG_VIDEO_H264_LEVEL_1_0,
        .maximum = V4L2_MPEG_VIDEO_H264_LEVEL_5_1,
        .default_value = V4L2_MPEG_VIDEO_H264_LEVEL_4_0,
        .menu_skip_mask = 0,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE,
        .type = V4L2_CTRL_TYPE_MENU,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
        .maximum = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC,
        .default_value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
        .menu_skip_mask = 0,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 51,
        .step = 1,
        .default_value = 36,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 51,
        .step = 1,
        .default_value = 36,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_MIN_QP,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 51,
        .step = 1,
        .default_value = 22,
    },
    {
        .id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP,
        .type = V4L2_CTRL_TYPE_INTEGER,

        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 51,
        .step = 1,
        .default_value = 36,
    },
    {
        .id = V4L2_CID_MPEG_AIT_VIDEO_FORCE_FRAME,
        .type = V4L2_CTRL_TYPE_MENU,
        .name = "Force frame type",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_NONE,
        .maximum = V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_I,
        .default_value = V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_NONE,
        .menu_skip_mask = 0,
        .is_volatile =( AITCAM_FLAG_EXE_ON_WR | AITCAM_FLAG_VOLATILE ),
    },
    {
        .id	 = V4L2_CID_JPEG_COMPRESSION_QUALITY,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .v4l2_pixelformat = V4L2_PIX_FMT_MJPEG,
        .minimum = 1,
        .maximum = 100,
        .step    = 1,
        .default_value = 10,
    },
    {
        .id	 = V4L2_CID_CAMERA_AIT_NIGHT_VISION,
        .type    = V4L2_CTRL_TYPE_MENU,
        .name = "NightVision",
        .v4l2_pixelformat = 0,
        .minimum = V4L2_CAMERA_AIT_NV_OFF,
        .maximum = V4L2_CAMERA_AIT_NV_AUTO,
        .step    = 1,
        .default_value = V4L2_CAMERA_AIT_NV_OFF,
        .menu_skip_mask = 0,
        .ipc_flags = V4L2_IPC_FLAG_RTOS_INIT ,
        .is_volatile =( AITCAM_FLAG_EXE_ON_WR | AITCAM_FLAG_VOLATILE ),
    },
    {
        .id	 = V4L2_CID_CAMERA_AIT_STREAMER_ALIVE,
        .type    = V4L2_CTRL_TYPE_MENU,
        .name = "Streamer Alive",
        .v4l2_pixelformat = 0,
        .minimum = V4L2_CAMERA_AIT_STREAMER_DOWN,
        .maximum = V4L2_CAMERA_AIT_STREAMER_UP,
        .step    = 1,
        .default_value = V4L2_CAMERA_AIT_STREAMER_DOWN,
        .menu_skip_mask = 0,
        .ipc_flags = V4L2_IPC_FLAG_RTOS_INIT ,
    },

    {
        .id	 = V4L2_CID_CAMERA_AIT_IR_LED,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .name = "ir_led",
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 0,
    },
    {
        .id	 = V4L2_CID_CAMERA_AIT_IR_SHUTTER,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .name = "ir_shutter",
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 0,
    },
    {
        .id	 = V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .name = "lightmode",
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 0,
        .is_volatile = 1,
    },
    {
        .id	 = V4L2_CID_CAMERA_AIT_LIGHT_CONDITION,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .name = "light",
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = (1 << 30) - 1,
        .step    = 1,
        .default_value = 0,
        .is_volatile = 1,
    },
    {
        .id      = V4L2_CID_FOCUS_AUTO,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 0,
    },
    #if 0
    {
        .id	 = V4L2_CID_CAMERA_AIT_IMAGE_EFFECT,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .name = "ImageEffect",
        .v4l2_pixelformat = 0,
        .minimum = V4L2_CAMERA_AIT_EFFECT_NORMAL,
        .maximum = V4L2_CAMERA_AIT_EFFECT_BW,
        .step    = 1,
        .default_value = V4L2_CAMERA_AIT_EFFECT_NORMAL,
        //   	.menu_skip_mask = 0,
    },
    #endif
    {
        .id	 = V4L2_CID_CAMERA_AIT_NR_GAIN,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .name = "nrgain",
        .v4l2_pixelformat = 0,
        .minimum = 0,
        .maximum = 10,
        .step    = 1,
        .default_value = 0,
        .is_volatile = 1,
    },

    #if H264ENC_TNR_EN
    {
        .id = V4L2_CID_MPEG_AIT_TNR ,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .name = "tnr.en",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = V4L2_H264_TNR_ZERO_MV_EN | V4L2_H264_TNR_LOW_MV_EN | V4L2_H264_TNR_HIGH_MV_EN,
        .step    = 1,
        .default_value = V4L2_H264_TNR_ZERO_MV_EN /*| V4L2_H264_TNR_LOW_MV_EN | V4L2_H264_TNR_HIGH_MV_EN*/,
    },
    #endif

    #if H264ENC_RDO_EN
    {
        .id = V4L2_CID_MPEG_AIT_RDO ,
        .type    = V4L2_CTRL_TYPE_BOOLEAN,
        .name = "rdo.en",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = 0,
        .maximum = 1,
        .step    = 1,
        .default_value = 1,  // 0 :disable RDO 1:enable RDO
    },
    {
        .id = V4L2_CID_MPEG_AIT_QSTEP3_P1 ,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .name = "rdo.p1",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = (__s32) 0x80000000,
        .maximum = (__s32)(0x7fffffff),
        .step    = 1,
        .default_value = 0x00334466 , //0x00223344,
    },
    {
        .id = V4L2_CID_MPEG_AIT_QSTEP3_P2 ,
        .type    = V4L2_CTRL_TYPE_INTEGER,
        .name = "rdo.p2",
        .v4l2_pixelformat = V4L2_PIX_FMT_H264,
        .minimum = (__s32) 0x80000000,
        .maximum = (__s32)(0x7fffffff),
        .step    = 1,
        .default_value = 0x222c,//0x0cdd,
    },
    #endif
    #if AITCAM_IPC_EN
    {
      .id   = V4L2_CID_CAMERA_AIT_STREAM_TYPE ,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "Stream Type",
      .v4l2_pixelformat = 0,//V4L2_PIX_FMT_H264,
      .minimum = 0,
      .maximum = 0xFF,
      .step = 1,
      .default_value = DEF_STREAM_TYPE,    
    },
    {
        .id = V4L2_CID_CAMERA_AIT_GET_TEMPERATURE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .name = "tempature",
        .v4l2_pixelformat = 0,
        .minimum = -100 ,
        .maximum =  100 ,
        .step = 1,
        .default_value = 0,
        .is_volatile = 1,
    },
    #endif
    
    #if 0 //AITCAM_IPC_EN
    {
      .id   = V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ ,
      .type = V4L2_CTRL_TYPE_INTEGER,
      .name = "AAC.Freq",
      .v4l2_pixelformat = V4L2_PIX_FMT_MPEG,
      .minimum = 16000,
      .maximum = 48000,
      .step = 16000,
      .default_value = 32000,    
    },
    {
      .id   = V4L2_CID_MPEG_AUDIO_AAC_BITRATE ,
      .type = V4L2_CTRL_TYPE_MENU,
      .name = "AAC.Bitrate",
      .v4l2_pixelformat = V4L2_PIX_FMT_MPEG,
      .minimum = V4L2_MPEG_AUDIO_AC3_BITRATE_32K,
      .maximum = V4L2_MPEG_AUDIO_AC3_BITRATE_640K,
      .step = 1,
      .default_value = V4L2_MPEG_AUDIO_AC3_BITRATE_128K,    
    },   
    #endif
};

#define NUM_CTRLS ARRAY_SIZE(controls)

static struct aitcam_ctx *aitcam_get_ctx_by_format(struct aitcam_dev *dev, u32 fmt)
{
    int i ;
    struct aitcam_ctx *ctx = 0;
    for(i = 0; i < AITCAM_NUM_CONTEXTS; i++)
    {
        ctx = dev->ctx[i] ;
        if(ctx)
        {
            if(ctx->output_format == fmt)
            {
                if(ctx->inst_state >= AITCAM_STAT_ALLOC)
                {
                    return (struct aitcam_ctx *)ctx;
                }
            }
        }
    }
    return (struct aitcam_ctx *)0 ;
}

#if (USING_FIXED_MEM_POOL)
__u32 aitcam_get_format_by_ctx_id(struct aitcam_dev *dev, int ctx_id)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    if(ctx_id >= AITCAM_NUM_CONTEXTS)
    {
        return   -1;
    }
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(ctx_id == map->ctxid)
        {
            return map->format ;
        }
    }
    return -1 ;
}

__u32 aitcam_get_pipe_by_ctx_id(struct aitcam_dev *dev, int ctx_id)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    if(ctx_id >= AITCAM_NUM_CONTEXTS)
    {
        return   -1;
    }
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(ctx_id == map->ctxid)
        {
            return map->pipe;
        }
    }
    return -1 ;
}

__u32 aitcam_get_baseaddr_by_ctx_id(struct aitcam_dev *dev, int ctx_id)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    if(ctx_id >= AITCAM_NUM_CONTEXTS)
    {
        return   -1;
    }
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
      
        map = &fmtpipe_map[i] ;

        if(ctx_id == map->ctxid)
        {

            return map->base_addr;
        }
    }
    return -1 ;
}


void aitcam_set_jpg_baseaddr(__u32 addr,__u32 max_pool )
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(V4L2_PIX_FMT_MJPEG == map->format)
        {
            map->base_addr     = addr;
            map->max_pool_size = max_pool ;
        }
    }
}

__u32 aitcam_set_h264_s2_baseaddr(__u32 addr  )
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if( (V4L2_PIX_FMT_H264 == map->format) && ( H264_S2_ID == map->pipe) )
        {
            map->base_addr     = addr;
            //map->disable = 0 ;
            return ( addr + map->max_pool_size );
        }
    }
    return 0;
}


__u32 aitcam_get_maxpoolsize_by_ctx_id(struct aitcam_dev *dev, int ctx_id)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    if(ctx_id >= AITCAM_NUM_CONTEXTS)
    {
        return   -1;
    }
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(ctx_id == map->ctxid)
        {
            pr_err("fmtpipe_map[%d].max_pool_size 0x%x\r\n", i, map->max_pool_size);
            return map->max_pool_size;
        }
    }
    return -1 ;
}

int aitcam_check_pipe_available(int pipe_id)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(map->ctxid != -1)
        {
            if(map->pipe == pipe_id)
                return -1;
        }
    }
    return 0;
}

int aitcam_pipefree(int ctxid)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    if(ctxid < 0)
        return -1 ;

    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(map->ctxid == ctxid)
        {
            map->ctxid = -1 ;
            return 0 ;
        }
    }
    return 0 ;
}

int aitcam_pipereset(void)
{
    int i ;
    struct aitcam_fmtpipe_map *map;
    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        map->ctxid = -1 ;
    }
    return 0 ;

}

int aitcam_pipealloc(int ctxid, __u32 format, int img_width, int img_height)
{
    int i ;
    struct aitcam_fmtpipe_map *map;

    pr_err("slash TEST 0.3 %s %d\r\n", __func__, __LINE__);

    if(format == 0)
    {
        pr_err("%s %d\r\n", __func__, __LINE__);
        return   aitcam_pipefree(ctxid) ;
    }

    for ( i = 0 ; i < NUM_PIPES ; i++)
    {
        map = &fmtpipe_map[i] ;
        if(format == map->format)
        {
            pr_err("%s %d img_width %d, img_height %d\r\n",
                __func__, __LINE__, img_width, img_height);

            pr_err("%s %d map->max_width %d, map->max_height %d\r\n",
                __func__, __LINE__, map->max_width, map->max_height);

            if( ((img_width <= map->max_width) && (img_height <= map->max_height)) || (format==V4L2_PIX_FMT_MPEG ))
            {
                pr_err("%s %d\r\n", __func__, __LINE__);
                if(map->ctxid == -1)
                {
                    pr_err("%s %d\r\n", __func__, __LINE__);
                    if(aitcam_check_pipe_available(map->pipe) == 0)
                    {
                        map->ctxid = ctxid ;

                        return map->pipe ;
                    }
                }
            }
        }
    }
    pr_err("%s %d\r\n", __func__, __LINE__);
    return -1 ;
}
#endif
#if AITCAM_IPC_EN

// loop check if opened stream include loop mode
static int aitcam_is_loopstream_opened(struct aitcam_ctx *ctx)
{
    int i ;
    struct aitcam_ctx *ctx_tmp = 0;
    struct aitcam_dev *dev = ctx->dev ;
    int ctx_n ;//= ctx->num - 1;
    u32 fmt ;//= ctx->
    if(!ctx) {
      return -1 ;
    }
    
    ctx_n = ctx->num - 1;
    fmt = ctx->output_format ;
   
    for(i = 0; i < ctx_n ; i++) {
        ctx_tmp = dev->ctx[i] ;
        if(ctx_tmp) {
            if((ctx_tmp->output_format == fmt) && (ctx->loop_recording) ) {
                if( (ctx_tmp->inst_state >= AITCAM_STAT_OPEN) && (ctx_tmp->loop_recording) ) {
                  return -1 ;  
                }
            }
        }
    }
    return 0 ;
}

#endif

static int aitcam_vidioc_querycap(struct file *file, void *priv,
                                  struct v4l2_capability *cap)
{
    struct aitcam_dev *dev = video_drvdata(file);

    strncpy(cap->driver, dev->plat_dev->name, sizeof(cap->driver) - 1);
    strncpy(cap->card, dev->plat_dev->name, sizeof(cap->card) - 1);
    cap->bus_info[0] = 0;
    cap->version = KERNEL_VERSION(1, 0, 0);
    cap->capabilities = V4L2_CAP_VIDEO_CAPTURE
                        | V4L2_CAP_STREAMING;
    return 0;
}

static int aitcam_vidioc_enum_fmt_vid_cap(struct file *file, void *pirv,
        struct v4l2_fmtdesc *f)
{
    struct aitcam_video_fmt *fmt;
    int i, j = 0;

    for (i = 0; i < ARRAY_SIZE(formats); ++i)
    {
        if (j == f->index)
        {
            fmt = &formats[i];
            strlcpy(f->description, fmt->name,
                    sizeof(f->description));
            f->pixelformat = fmt->fourcc;
            return 0;
        }
        ++j;
    }
    return -EINVAL;
}

static int aitcam_vidioc_g_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    struct v4l2_pix_format *pix_fmt = &f->fmt.pix;

    if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        ait_err("invalid buf type\n");
        return -EINVAL;
    }

    pix_fmt->width = ctx->img_width;
    pix_fmt->height = ctx->img_height;
    //pix_fmt->field = V4L2_FIELD_NONE;
    pix_fmt->pixelformat = ctx->output_format;
    pix_fmt->bytesperline = ctx->enc_dst_buf_size;
    pix_fmt->sizeimage = ctx->enc_dst_buf_size;
    pix_fmt->colorspace = 0;
    pix_fmt->priv = 0;

    return 0;
}

static int aitcam_vidioc_try_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
    struct v4l2_pix_format *pix_fmt = &f->fmt.pix;

    
    if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        ait_err("invalid buf type\n");
        return -EINVAL;
    }

    switch (pix_fmt->pixelformat)
    {
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_MV_AITHREC:
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
        #if AITCAM_IPC_EN
        case V4L2_PIX_FMT_MPEG:
        #endif  
            break;
        default:
            ait_err("failed to try output format\n");
            return -EINVAL;
    }
    //if (pix_fmt->sizeimage == 0) {
    //ait_err("must be set encoding output size\n");
    //return -EINVAL;
    //}

    switch (pix_fmt->pixelformat)
    {
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_MV_AITHREC:
            if (pix_fmt->sizeimage < 256 * 1024)
            {
                //ait_err("sizeimage error\n");
                pix_fmt->sizeimage = 256 * 1024;
                //return -EINVAL;
            }
            
            if( pix_fmt->sizeimage  > MAX_H264_FRAME_SIZE ) {
                pix_fmt->sizeimage =   MAX_H264_FRAME_SIZE ;
            }
            
            if( (pix_fmt->width <=640 ) && (pix_fmt->height <=480 ) ) {
                pix_fmt->sizeimage =   MAX_H264_S2_FRAME_SIZE ;
            }
            
            break;
        case V4L2_PIX_FMT_MJPEG:
            if (pix_fmt->width != ALIGN8(pix_fmt->width))
            {
                ait_err("width alignment error\n");
                return -EINVAL;
            }
            if (pix_fmt->height != ALIGN8(pix_fmt->height))
            {
                ait_err("height alignment error\n");
                return -EINVAL;
            }
            if (pix_fmt->sizeimage < 256 * 1024)
            {
                //ait_err("sizeimage error\n");
                pix_fmt->sizeimage = 256 * 1024;
                //return -EINVAL;
            }
            break;
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_GREY:
        {
            int rawsize = ait_calculate_rawdata_size(pix_fmt->pixelformat,
                          pix_fmt->width, pix_fmt->height)
                          + VIDBUF_FRAME_HDR_RESV;
            if (pix_fmt->sizeimage < rawsize)
            {
                //ait_err("sizeimage error\n");
                pix_fmt->sizeimage = rawsize;
                //return -EINVAL;
            }
            break;
        }
        #if AITCAM_IPC_EN
        case V4L2_PIX_FMT_MPEG:
        {
          //if (pix_fmt->sizeimage < 4 * 1024) {
          pix_fmt->sizeimage = 1536 ;  
          //}
          
          if ( (pix_fmt->height!=48000) && (pix_fmt->height!=32000) && (pix_fmt->height!=16000) ) {
              ait_err("sample rate is not support!");
              return -EINVAL ;
          }
        }
        break ;  
        #endif
        
        default:
            break;
    }

    return 0;
}

static int aitcam_vidioc_s_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
    //struct aitcam_dev *dev = video_drvdata(file);
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    struct v4l2_pix_format *pix_fmt = &f->fmt.pix;
    int ret = 0;

    dbg_printf(0,"[%d][SLOT.IN.S:%d]\n",ctx->num,pix_fmt->sizeimage);

    ret = aitcam_vidioc_try_fmt(file, priv, f);
    if (ret)
    {
        return ret;
    }

    if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        #if 1 // if current ctx is not virgin, clear previous resource/state
        if (ctx->inst_state == AITCAM_STAT_RUNNING)
        {
            return -EINVAL;
        }
        if (ctx->inst_state > AITCAM_STAT_OPEN)
        {
            #if AITCAM_IPC_EN==0
            switch (ctx->output_format)
            {
                case V4L2_PIX_FMT_H264:
                case V4L2_PIX_FMT_MV_AITHREC:
                    MMPF_VIDENC_DeinitInstance((MMP_ULONG)ctx->codec_handle);
                    break;
                case V4L2_PIX_FMT_MJPEG:
                    if (ctx->codec_handle)
                    {
                        MMPF_JPEG_DeinitInstance(ctx->codec_handle);
                        kfree(ctx->codec_handle);
                    }
                    break;
                case V4L2_PIX_FMT_GREY:
                case V4L2_PIX_FMT_YUV420:
                case V4L2_PIX_FMT_YUYV:
                case V4L2_PIX_FMT_UYVY:
                #if AITCAM_IPC_EN
                case V4L2_PIX_FMT_MPEG:
                #endif
                    break;
            } 
            #endif
            ctx->inst_state = AITCAM_STAT_OPEN;
        }
        #endif

        ctx->img_width  = pix_fmt->width;
        ctx->img_height = pix_fmt->height;
        ctx->output_format = pix_fmt->pixelformat;
        ctx->enc_dst_buf_size =	ALIGN_PAGE(pix_fmt->sizeimage);

        dbg_printf(0,"[%d][SLOT.OUT.S:%d]\n",ctx->num,ctx->enc_dst_buf_size );
        #if AITCAM_IPC_EN
        if (cpu_ipc) {
          aitcam_img_info img_info ;
          
          if ( aitcam_is_loopstream_opened(ctx) < 0 ) {
            dbg_printf(0,"#fmt:0x%08x loop mode NG\n",ctx->output_format);  
            return -EINVAL ;
          }
          /*
          ctx->img_pipe = ctx->num ;
          
          if(ctx->img_pipe < 0) {
              dbg_printf(0, "#this fmt[%d]:0x%x, no streamid..\r\n",ctx->num, ctx->output_format);
              return -EINVAL;
    
          }
          */
          
          img_info.streamid  = ctx->num;
          if (ctx->output_format==V4L2_PIX_FMT_H264) { 
            img_info.ctrl_flag  = (fdfr_osd)?CTRL_FLAG_OSD_EN : 0 ;
          }
          else {
            img_info.ctrl_flag  = 0 ;
          }
          pr_info("#ctrl_flag : %x\n",  img_info.ctrl_flag );
          img_info.img_width = ctx->img_width;
          img_info.img_height= ctx->img_height;
          img_info.img_fmt   = ctx->output_format;
          img_info.max_framesize = ctx->enc_dst_buf_size ;    
          img_info.streamtype = ctx->loop_recording ? V4L2_CAMERA_AIT_LOOP_RECORDING : 0 ;   
          if(aitcam_ipc_open(&img_info) < 0 ) {
              pr_info("aitcam_ipc_open fail\n");
              return -EINVAL;
          }
        }
        #endif
    
        #ifdef AITCAM_DBG_MSG
        if (debug_level >= 2)
        {
            switch (ctx->output_format)
            {
                case V4L2_PIX_FMT_H264:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_H264\n");
                    break;
                case V4L2_PIX_FMT_MV_AITHREC:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_MV_AITHREC\n");
                    break;
                case V4L2_PIX_FMT_MJPEG:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_MJPEG\n");
                    break;
                case V4L2_PIX_FMT_GREY:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_GREY\n");
                    break;
                case V4L2_PIX_FMT_YUV420:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_YUV420\n");
                    break;
                case V4L2_PIX_FMT_YUYV:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_YUYV\n");
                    break;
                case V4L2_PIX_FMT_UYVY:
                    dbg_printf(0, "format is  V4L2_PIX_FMT_UYVY\n");
                    break;
                #if AITCAM_IPC_EN
                case V4L2_PIX_FMT_MPEG:
                    dbg_printf(0,"format is V4L2_PIX_FMT_MPEG\n");  
                #endif    
            }
            dbg_printf(0, "enc_dst_buf_size = x%x\n", ctx->enc_dst_buf_size);
            dbg_printf(0, "stream(%d) is %d x %d\n", ctx->num, ctx->img_width, ctx->img_height);
        }
        #endif

    }

    return ret;
}

#if AITCAM_MULTI_STREAM_EN
#define TEST_GRA_PATH   1
#else
#define TEST_GRA_PATH   0
#endif

#define TEST_RAW_PATH   0

#if TEST_RAW_PATH == 1
MMP_ULONG glRawBufAddr;
static void _test_raw_path (void)
{
#include "mmp_reg_rawproc.h"
#include "mmp_reg_vif.h"
    AITPS_RAWPROC   pRAW = AITC_BASE_RAWPROC;
    AITPS_VIF       pVIF = AITC_BASE_VIF;
    extern MMPF_SENSOR_FUNCTION *gsSensorFunction;

    pRAW->RAWPROC_S_ADDR = glRawBufAddr;
    pRAW->RAWPROC_F_ADDR = glRawBufAddr;
    pRAW->RAWPROC_F_H_BYTE      = 1288;
    pRAW->RAWPROC_F_V_BYTE      = 722;
    pRAW->RAWPROC_F_ST_OFST     = 0;
    pRAW->RAWPROC_F_PIX_OFST    = 1;
    pRAW->RAWPROC_F_LINE_OFST   = 1288;
    pRAW->RAWPROC_F_TIME_CTL    = 0;
    pRAW->RAWPROC_F_EXTRA_LINE_TIME   = 2;
    pRAW->RAWPROC_MODE_SEL = RAWPROC_STORE_EN | RAWPROC_F_BURST_EN;
    pVIF->VIF_INT_HOST_SR[0] = 0xFF;
    pVIF->VIF_RAW_OUT_EN[0] = (VIF_2_RAW_EN/* | VIF_2_ISP_EN*/);

    //_dump_iomem(0xf0006900, 256);
    //_dump_iomem(0xf0005D00, 256);
    //_dump_iomem(0xf0006000, 256);
    //_dump_iomem(0xf0006f00, 256);
    //_dump_iomem(0xf0006400, 256);
    //_dump_iomem(0xf0006500, 256);
    //_dump_iomem(0xf0007900, 256);
    //_dump_iomem(0xf0006B00, 256);///raw0
    //_dump_iomem(0xf0007E00, 256);///raw1
    //_dump_iomem(0xf0007000, 512);

    MMPF_VIF_EnableInputInterface(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_TRUE);
    MMPF_VIF_EnableOutput(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_TRUE);

    while (1)
    {
        int timeout = 0, frm = 0;
        while ((pVIF->VIF_INT_HOST_SR[0]&VIF_INT_GRAB_END) == 0)
        {
            MMPF_OS_Sleep_MS(30);
            if (timeout++ > 100)
            {
                break;
            }
        }
        if (timeout > 100)
        {
            dbg_printf(0, "timeout !!!!!!\n");
            _dump_iomem(0xf0006000, 256);
            _dump_iomem(0xf0006B00, 256);///raw0
        }
        else
        {
            _dump_iomem(AIT_RAM_P2V(glRawBufAddr), 256);
        }

        dbg_printf(0, "fetch frame\n");
        *((AIT_REG_B*)0xF000700A) = *((AIT_REG_B*)0xF000700A) | (1 << 7);
        pRAW->RAWPROC_MODE_SEL |= RAWPROC_FETCH_EN;

        pVIF->VIF_INT_HOST_SR[0] = 0xFF;
        MMPF_OS_Sleep_MS(1000);
    }
}
#endif

#if TEST_GRA_PATH == 1
#if AITCAM_MULTI_STREAM_EN
extern LoopBackCallBackFunc *GraIBCLinkCallBackFunc[4];
extern MMP_USHORT  GraIBCLinkCallBackArgu0[4];
extern MMP_USHORT  GraIBCLinkCallBackArgu1[4];
#endif
extern MMP_ULONG   glPreviewBufAddr[MMPF_IBC_PIPE_MAX][4];
extern MMP_ULONG   glPreviewUBufAddr[MMPF_IBC_PIPE_MAX][4];
extern MMP_ULONG   glPreviewVBufAddr[MMPF_IBC_PIPE_MAX][4];
extern MMP_UBYTE   gbPreviewBufferCount[MMPF_IBC_PIPE_MAX];

extern MMP_USHORT  gsPreviewBufWidth[MMPF_IBC_PIPE_MAX][4];
extern MMP_USHORT  gsPreviewBufHeight[MMPF_IBC_PIPE_MAX][4];
extern MMP_USHORT  gsPreviewBufCurWidth[MMPF_IBC_PIPE_MAX];
extern MMP_USHORT  gsPreviewBufCurHeight[MMPF_IBC_PIPE_MAX];

extern MMP_UBYTE   gbExposureDone;
extern MMP_UBYTE   gbExposureDoneFrame[MMPF_IBC_PIPE_MAX];
void _dump_iomem(MMP_ULONG start, MMP_ULONG len);
static void _loopback_frame(MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    MMPF_GRAPHICS_BUFATTRIBUTE  m_GraSrcBufAttr =
    {
        0, 0,   //width, height
        0,      //line offset
        MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE,
        0, 0, 0 //YUV buf addr
    };
    MMPF_GRAPHICS_RECT  m_GraRect =
    {
        0, 0,   //top, left
        0, 0    //width, height
    };
    MMPF_IBC_PIPEID SrcPipe = MMPF_IBC_PIPE_0;
    MMPF_IBC_PIPEID DstPipe = MMPF_IBC_PIPE_1;
    MMP_ERR     ret;
    MMPF_GRAPHICS_BUFATTRIBUTE dummy;
    MMPF_SCALER_PATH        scalp = (MMPF_SCALER_PATH)DstPipe;
    MMPF_SCALER_FIT_RANGE   ftrge;
    MMPF_SCALER_GRABCONTROL grctl;
    MMPF_ICO_PIPEATTRIBUTE 		ico_pipeattribute;

    MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, scalp);

    m_GraSrcBufAttr.usWidth     = gsPreviewBufWidth[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.usHeight    = gsPreviewBufHeight[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.usLineOffset = m_GraSrcBufAttr.usWidth;
    m_GraSrcBufAttr.colordepth  = MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;
    m_GraSrcBufAttr.ulBaseAddr  = glPreviewBufAddr[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.ulBaseUAddr = glPreviewUBufAddr[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.ulBaseVAddr = glPreviewVBufAddr[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraRect.usWidth           = m_GraSrcBufAttr.usWidth;
    m_GraRect.usHeight          = m_GraSrcBufAttr.usHeight;
    m_GraRect.usLeft            = 0;
    m_GraRect.usTop             = 0;

    ret = MMPF_Graphics_SetScaleAttribute (&m_GraSrcBufAttr, 0, 0, &m_GraRect, 1, MMPF_GRAPHICS_SCAL_FB, MMPF_OS_LOCK_CTX_ISR);

    if (ret != MMP_ERR_NONE)
    {
        return;
    }

    dummy.ulBaseAddr = 0;

    ftrge.usFitResol = 30;
    ftrge.fitmode   = MMPF_SCALER_FITMODE_OUT;
    ftrge.usInWidth = m_GraRect.usWidth;
    ftrge.usInHeight = m_GraRect.usHeight;
    ftrge.usOutWidth = usWidth;
    ftrge.usOutHeight = usHeight;

    MMPF_IBC_SetMirrorEnable (DstPipe, MMP_FALSE , 0);

    // config ICO module and connect SCAL->ICO
    ico_pipeattribute.inputsel = DstPipe;
    ico_pipeattribute.bDlineEn = MMP_TRUE;
    ico_pipeattribute.usFrmWidth 	= usWidth;
    MMPF_ICON_SetDLAttributes(DstPipe, &ico_pipeattribute);

    MMPF_Scaler_SetEngine(MMP_FALSE, scalp, &ftrge, &grctl);
    MMPF_Scaler_SetLPF(scalp, &ftrge, &grctl);

    MMPF_Scaler_SetPixelLineDelay(scalp, 1, 1, 0x10);

    //MMPF_Scaler_SetPixelLineDelay(scalp, 0, 0, 0x10);

    #if AITCAM_MULTI_STREAM_EN
    //if((usWidth >= 1280) && (usHeight >= 720))
    {
        // 15/20 for G0 264MHz/ISP 400MHz
        ret = MMPF_Graphics_ScaleStart (m_GraSrcBufAttr, dummy, 0, 15, 20, 0xFFFF, MMPF_IBC_PIPE_0, MMPF_OS_LOCK_CTX_ISR);
    }
    //else
    #else
    {
        ret = MMPF_Graphics_ScaleStart (m_GraSrcBufAttr, dummy, 0, 1, 1, 0xFFFF, MMPF_IBC_PIPE_0, MMPF_OS_LOCK_CTX_ISR);
    }
    #endif
}
/*static void _test_gra_loopback (void)
{
    int i, timeout, qsize;
    void MMPF_Display_LoobackFrame(MMP_USHORT usWidth, MMP_USHORT usHeight);
    MMPF_VIDBUF_QUEUE *vq = MMPF_VIDBUF_GetHandle(0);

    for (i = 0; i < 4; i++) {
        qsize = vq->ready_vbq.size;

        MMPF_Display_LoobackFrame (640, 480);

        timeout = 0;
        while (vq->ready_vbq.size == qsize) {
            MMPF_OS_Sleep_MS(10);
            dbg_printf(0, "#,");
            if (timeout++ > 100) {
                dbg_printf(0, "timeout ~~~\n");
                break;
            }
        }
    }
}*/
#endif


#if (SUPPORT_DUAL_SNR_PRV)
extern Raw2GraLoopBackCallBackFunc *GraRAWLinkCallBackFunc[2];
extern MMPF_IBC_PIPEID  GraRAWLinkCallBackArgu0[2];
extern MMP_USHORT  GraRAWLinkCallBackArgu1[2];
extern MMP_USHORT  GraRAWLinkCallBackArgu2[2];
void _dump_iomem(MMP_ULONG start, MMP_ULONG len);

static void Raw2Gra_loopback_frame(MMP_UBYTE ubRawId, MMPF_IBC_PIPEID DstPipe, MMP_ULONG SrcAddr, MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    MMPF_GRAPHICS_BUFATTRIBUTE  m_GraSrcBufAttr =
    {
        0, 0,   //width, height
        0,      //line offset
        MMPF_GRAPHICS_COLORDEPTH_YUV422_UYVY,
        0, 0, 0 //YUV buf addr
    };
    MMPF_GRAPHICS_RECT  m_GraRect =
    {
        0, 0,   //top, left
        0, 0    //width, height
    };
    MMP_ERR     ret;
    MMPF_GRAPHICS_BUFATTRIBUTE dummy;
    MMPF_SCALER_PATH        scalp = (MMPF_SCALER_PATH)DstPipe;
    MMPF_SCALER_FIT_RANGE   ftrge;
    MMPF_SCALER_GRABCONTROL grctl;
    MMPF_ICO_PIPEATTRIBUTE 		ico_pipeattribute;
    MMP_USHORT  rawstoreW, rawstoreH;

    MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, scalp);

    MMPF_RAWPROC_GetStoreRange(ubRawId , &rawstoreW, &rawstoreH);
    //dbg_printf(0, "rawstoreW = %d, rawstoreH = %d\n", rawstoreW, rawstoreH);

    m_GraSrcBufAttr.usWidth     = rawstoreW / 2; // raw store use BAYER8 format, store width is 2 times of original width
    m_GraSrcBufAttr.usHeight    = rawstoreH;
    m_GraSrcBufAttr.usLineOffset = m_GraSrcBufAttr.usWidth * 2;
    m_GraSrcBufAttr.colordepth  = MMPF_GRAPHICS_COLORDEPTH_YUV422_YVYU;
    m_GraSrcBufAttr.ulBaseAddr  = SrcAddr;
    m_GraSrcBufAttr.ulBaseUAddr = SrcAddr;
    m_GraSrcBufAttr.ulBaseVAddr = SrcAddr;
    m_GraRect.usWidth           = m_GraSrcBufAttr.usWidth;
    m_GraRect.usHeight          = m_GraSrcBufAttr.usHeight;
    m_GraRect.usLeft            = 0;
    m_GraRect.usTop             = 0;

    ret = MMPF_Graphics_SetScaleAttribute (&m_GraSrcBufAttr, 0, 0, &m_GraRect, 1, MMPF_GRAPHICS_SCAL_FB, MMPF_OS_LOCK_CTX_ISR);

    if (ret != MMP_ERR_NONE)
    {
        return;
    }

    dummy.ulBaseAddr = 0;

    ftrge.usFitResol = 30;
    ftrge.fitmode   = MMPF_SCALER_FITMODE_OUT;
    ftrge.usInWidth = m_GraRect.usWidth;
    ftrge.usInHeight = m_GraRect.usHeight;
    ftrge.usOutWidth = usWidth;
    ftrge.usOutHeight = usHeight;
    //dbg_printf(0, "usInWidth = %d, usInHeight = %d, w=%d, h=%d\n", ftrge.usInWidth, ftrge.usInHeight, ftrge.usOutWidth, ftrge.usOutHeight);

    MMPF_IBC_SetMirrorEnable (DstPipe, MMP_FALSE , 0);

    // config ICO module and connect SCAL->ICO
    ico_pipeattribute.inputsel = DstPipe;
    ico_pipeattribute.bDlineEn = MMP_TRUE;
    ico_pipeattribute.usFrmWidth 	= usWidth;
    MMPF_ICON_SetDLAttributes(DstPipe, &ico_pipeattribute);

    MMPF_Scaler_SetEngine(MMP_FALSE, scalp, &ftrge, &grctl);
    MMPF_Scaler_SetLPF(scalp, &ftrge, &grctl);

    MMPF_Scaler_SetPixelLineDelay(scalp, 1, 1, 0x10);

#if 0//(STATISTIC_EN == 1)
    if (frame_number[0] == 0)
    {
            // Scaler
            _dump_iomem(0xF0006F00, 256);
            _dump_iomem(0xF0006400, 256);
            //_dump_iomem(0xF0004500, 256);
            frame_number[0]++;
    }
#endif

    ret = MMPF_Graphics_ScaleStart (m_GraSrcBufAttr, dummy, 0, 15, 20, 0xFFFF, DstPipe, MMPF_OS_LOCK_CTX_ISR);

}
/*static void _test_gra_loopback (void)
{
    int i, timeout, qsize;
    void MMPF_Display_LoobackFrame(MMP_USHORT usWidth, MMP_USHORT usHeight);
    MMPF_VIDBUF_QUEUE *vq = MMPF_VIDBUF_GetHandle(0);

    for (i = 0; i < 4; i++) {
        qsize = vq->ready_vbq.size;

        MMPF_Display_LoobackFrame (640, 480);

        timeout = 0;
        while (vq->ready_vbq.size == qsize) {
            MMPF_OS_Sleep_MS(10);
            dbg_printf(0, "#,");
            if (timeout++ > 100) {
                dbg_printf(0, "timeout ~~~\n");
                break;
            }
        }
    }
}*/
#endif


#ifdef AITCAM_DBG_MSG
void _dump_iomem(MMP_ULONG start, MMP_ULONG len)
{
    MMP_ULONG addr, l_offset;
    MMP_UBYTE l_buf[128], token_buf[8];

    for (addr = start, l_offset = 0; addr < (start + len); addr++)
    {
        if ((addr & 0xFF) == 0)
        {
            printk(KERN_ERR "x%08X :", addr);
        }
        if ((addr & 0xF) == 0)
        {
            l_offset = 0;
            snprintf(token_buf, 8, "%04X:", (0xFFFF & addr));
            strcpy(l_buf + l_offset, token_buf);
            l_offset += strlen(token_buf);
        }

        snprintf(token_buf, 8, " %02X", *(MMP_UBYTE*)addr);
        strcpy(l_buf + l_offset, token_buf);
        l_offset += strlen(token_buf);

        if ((addr & 0xF) == 0xF)
        {
            printk(KERN_ERR "%s\n", l_buf);
        }
    }
    RTNA_DBG_Str(0, "Dump End\n");
}
#endif

#if AITCAM_EVENT_QUEUE
static void aitcam_put_event(struct aitcam_dev *dev, u32 id)
{
    struct video_device *vdev = dev->vfd_cam ;
    struct v4l2_event event;
    struct v4l2_event_ctrl *ctrl = &event.u.ctrl;


    if(vdev)
    {
        memset(&event, 0, sizeof(event));
        event.type = V4L2_EVENT_CTRL;
        event.id = id ;
        switch (id)
        {
            case V4L2_CID_CAMERA_AIT_LIGHT_CONDITION:

                ctrl->value = (s32)ISP_IF_AE_GetLightCond();
                if(dev->lightcond == ctrl->value)
                {
                    return ;
                }
                dev->lightcond = ctrl->value ;
                //dbg_printf(0,"#LightCond[id=0x%08x]: new:0x%08x,old:0x%08x\r\n",id,(u32)ctrl->value,dev->lightcond);
                break;
            case V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE:
                ctrl->value = MMPF_Sensor_GetNightVisionMode(NV_CTRL_LIGHT_MODE);
                if(dev->is_night == ctrl->value)
                {
                    return ;
                }
                dev->is_night = ctrl->value ;
                break;
        }
        v4l2_event_queue(vdev, &event);
    }
}
#endif

void aitbh_isp_frame_start (void *arg)
{
  #if AITCAM_IPC_EN==0
    struct aitcam_dev *dev = container_of(arg, struct aitcam_dev, IspFrameStWork);
    #if AITCAM_EVENT_QUEUE
    struct video_device *vdev = dev->vfd_cam ;
    #endif

    #if (USE_TASK_DO3A == 1)
    MMPF_Sensor_WorkIspFrameSt(NULL);
    #endif

    #if (AIT_ISP_VIDEO_IQ == 1)
    mutex_lock(&dev->aitcam_mutex);
    if (dev->isp_vidiq_en)
    {
        struct aitcam_ctx *ctx = dev->ctx[dev->isp_vidiq_ctx_num];

        if ((ctx->inst_state == AITCAM_STAT_RUNNING)
                && (ctx->output_format == V4L2_PIX_FMT_H264))
        {
            ait_iq_video_frame_start(&dev->hVidIq);
        }
    }
    mutex_unlock(&dev->aitcam_mutex);
    #endif
    if (dev->isp_ir_refresh)
    {
        struct ait_camif_ir_ctrl g_ir;
        dev->isp_ir_refresh = 0;

        ait_param_ctrl(AIT_PARAM_CAM_IR_G_CTRL, &g_ir);

        //        ISP_IF_F_SetSaturation(g_ir.ir_on? ISP_SATURATION_LEVEL_GRAY: ISP_SATURATION_LEVEL_NOMAL);
        ISP_IF_F_SetImageEffect(g_ir.ir_on ? ISP_IMAGE_EFFECT_GREY : ISP_IMAGE_EFFECT_NORMAL); // Grey

    }
    #if AITCAM_EVENT_QUEUE
    if(vdev && vdev->ioctl_ops->vidioc_subscribe_event)
    {
        aitcam_put_event(dev, V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE);
        aitcam_put_event(dev, V4L2_CID_CAMERA_AIT_LIGHT_CONDITION);
    }
    #endif
  #endif
}

#if (AIT_OSD_EN == 1)
int aitcam_osd_rtc_sync (struct aitcam_ctx *ctx)
{
    if ((ctx->osd.resync_time)
            || (ctx->osd.local_time.uwSecond != ctx->dev->rtc.handle.uwSecond)
            || (ctx->osd.local_time.uwMinute != ctx->dev->rtc.handle.uwMinute))
    {
        /* The sec/min condition fails when system stuck for 60min */
        unsigned long flags;

        spin_lock_irqsave(&ctx->dev->rtc.time_rdlock, flags);
        ctx->osd.local_time = ctx->dev->rtc.handle;
        spin_unlock_irqrestore(&ctx->dev->rtc.time_rdlock, flags);

        if (ctx->osd.resync_time)
            ctx->osd.resync_time = 0;

        return 1;
    }

    return 0;
}

int aitcam_osd_paint_windows (struct aitcam_ctx *ctx, AIT_OSD_BUFDESC *TargetBuf)
{
    int active_cnt = 0;

    mutex_lock(&ctx->osd.mlock);

    if (ctx->osd.active_num > 0)   // need check again since mxlock does not be held first
    {
        int win, is_resync = aitcam_osd_rtc_sync(ctx);

        for (win = 0;
                (active_cnt < ctx->osd.active_num) && (win < MAX_OSD_INDEX);
                win++)
        {
            typeof(ctx->osd.win[win]) *pOsd = &(ctx->osd.win[win]);

            if (pOsd->type == AIT_OSD_TYPE_INACTIVATE)
            {
                continue;
            }
            else if (pOsd->type == AIT_OSD_TYPE_RTC)
            {
                if (is_resync)
                {
                    ait_rtc_ConvertDateTime2String(&ctx->osd.local_time, pOsd->str);
                }
            }

            if (pOsd->type == AIT_OSD_TYPE_WATERMARK)
            {
                #if (AIT_WATERMARK_EN)
                ait_osd_DrawWatermark(&(pOsd->hdl), TargetBuf);
                #endif
            }
            else if (pOsd->type == AIT_OSD_TYPE_FDFRAME)
            {
                #if (AIT_FDFRAME_EN)
                ait_osd_DrawFDFrame(&(pOsd->hdl), TargetBuf, pOsd->str, strlen(pOsd->str));
                #endif
            }
            else
            {
                ait_osd_DrawStr(&(pOsd->hdl), TargetBuf, pOsd->str, strlen(pOsd->str));
            }

            active_cnt++;
        }
    }

    mutex_unlock(&ctx->osd.mlock);

    return active_cnt;
}
#endif

#if AITCAM_IPC_EN==0
void aitbh_h264enc_enc_start (void *arg)
{
    struct aitcam_ctx *ctx = container_of(arg, struct aitcam_ctx, EncStartWork);

    #if (AIT_OSD_EN == 1)
    if (ctx->osd.active_num)
    {
        int num;
        MMPF_VIDENC_FRAME   *pSrcFrm;
        AIT_OSD_BUFDESC     TargetBuf;

        MMPF_VIDENC_GetParameter((MMP_ULONG)ctx->codec_handle,
                                 MMPF_VIDENC_ATTRIBUTE_CURBUF_ADDR, &pSrcFrm);

        TargetBuf.ulAddr[COLR_Y] = AIT_RAM_P2V(pSrcFrm->ulYAddr);
        TargetBuf.ulAddr[COLR_U] = AIT_RAM_P2V(pSrcFrm->ulUAddr);
        TargetBuf.ulAddr[COLR_V] = AIT_RAM_P2V(pSrcFrm->ulVAddr);
        TargetBuf.ulBufWidth     = ALIGN16(ctx->img_width);
        TargetBuf.ulBufHeight    = ALIGN16(ctx->img_height);
        num = aitcam_osd_paint_windows(ctx, &TargetBuf);
    }
    #endif

    MMPF_H264ENC_TriggerFrameDone(MMPF_H264ENC_GetHandle((MMP_ULONG)ctx->codec_handle),
                                  MMPF_OS_LOCK_CTX_TASK);

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 3)
    {
        struct timespec ct;
        getnstimeofday(&ct);
        printk(KERN_ERR "[%d].%d\n", (int)ct.tv_sec, (int)ct.tv_nsec / 1000000);
    }
    #endif

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 4)
    {
        extern struct timespec enc_start;
        extern void * enc_hdl;

        enc_hdl = ctx->codec_handle;
        getnstimeofday(&enc_start);
    }
    #endif
}

void calculate_scaling_fov(MMPF_SCALER_FIT_RANGE *pFitrange, MMPF_SCALER_GRABCONTROL *pGrab)
{
    #if (AIT_FOV_CROP_EN == 1)
    MMPF_SCALER_FIT_RANGE   scaled_fit = *pFitrange;
    MMP_ULONG   crop_base = 0, crop_ratio = 0;

    aitcam_get_fov_crop_info(&crop_base, &crop_ratio);

    if (crop_base && crop_ratio && (crop_ratio < crop_base))
    {

        scaled_fit.usOutWidth = (scaled_fit.usOutWidth * crop_base) / (crop_base - crop_ratio);
        scaled_fit.usOutHeight = (scaled_fit.usOutHeight * crop_base) / crop_base - crop_ratio;

        MMPF_Scaler_GetBestFitScale(&scaled_fit, pGrab);

        pGrab->usStartX += ((scaled_fit.usOutWidth - pFitrange->usOutWidth) >> 1);
        pGrab->usEndX   = pGrab->usStartX + pFitrange->usOutWidth - 1;
        pGrab->usStartY += ((scaled_fit.usOutHeight - pFitrange->usOutHeight) >> 1);
        pGrab->usEndY   = pGrab->usStartY + pFitrange->usOutHeight - 1;
    }
    else
    #endif // (AIT_FOV_CROP_EN == 1)
    {
        MMPF_Scaler_GetBestFitScale(pFitrange, pGrab);
    }
}

static MMP_ERR set_pipe_config(MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight,
                               MMP_USHORT usEncWidth, MMP_USHORT usEncHeight,
                               MMP_USHORT cropped_bottom,
                               MMPF_VIDENC_FRAME *FrameList,
                               MMP_USHORT FrameNum,
                               MMPF_FCTL_LINK *PipeLink,
                               MMP_USHORT  usScaleResol,
                               MMP_UBYTE mirror, MMP_UBYTE ubEncId, MMP_LONG lCtxNum, MMPF_SCALER_SOURCE usScaleSrc)
{
    MMPF_SCALER_FIT_RANGE           fitrange;
    MMPF_SCALER_GRABCONTROL         grabctl;
    MMPF_FCTL_PREVIEWATTRIBUTE  	fctlpreviewattribute;
    MMP_ULONG                       i;
    #if 1
    MMP_ULONG                       buf_offset;
    #endif

    fitrange.fitmode    = MMPF_SCALER_FITMODE_OUT;
    fitrange.usFitResol = usScaleResol;
    fitrange.usInWidth  = usSrcWidth;
    fitrange.usInHeight = usSrcHeight;
    fitrange.usOutWidth = usEncWidth;
    fitrange.usOutHeight = usEncHeight;
    if ((FrameNum) && (cropped_bottom))
    {
        fitrange.usOutHeight -= cropped_bottom;
    }
    calculate_scaling_fov(&fitrange, &grabctl);

    #if AITCAM_MULTI_STREAM_EN
    if (PipeLink->ibcpipeID == MMPF_IBC_PIPE_1)
    {
        switch (lCtxNum)
        {
            case 0:
                lCtxNum = 3;
                break;
            case 1:
                lCtxNum = 4;
                break;
            case 2:
                lCtxNum = 5;
                break;
            case 3:
                lCtxNum = 6;
                break;
        }
    }
    #endif
    //dbg_printf(0, ">>grabctl%d:M%d, N%d, X:%d/%d, Y:%d/%d\n", lCtxNum, grabctl.usScaleM, grabctl.usScaleN, grabctl.usStartX,
    //grabctl.usEndX, grabctl.usStartY, grabctl.usEndY);//cm

    fctlpreviewattribute.fctllink = *PipeLink;
    #if AITCAM_MULTI_STREAM_EN
    if (PipeLink->scalerpath == MMPF_SCALER_PATH_1)
    {
        fctlpreviewattribute.FctlLinkSrc = MMPF_SCALER_SOURCE_GRA;
    }
    else
    #endif
    {
        fctlpreviewattribute.FctlLinkSrc = usScaleSrc;
    }
    fctlpreviewattribute.usInputW = usSrcWidth;
    fctlpreviewattribute.usInputH = usSrcHeight;
    #if AITCAM_MULTI_STREAM_EN
    fctlpreviewattribute.lCtxNum = lCtxNum;
    #endif
    fctlpreviewattribute.grabctl = grabctl;
    fctlpreviewattribute.usBufferCount = FrameNum;
    for (i = 0; i < FrameNum; i++)
    {
        fctlpreviewattribute.ulBaseAddr[i] = FrameList[i].ulYAddr;
        fctlpreviewattribute.ulBaseUAddr[i] = FrameList[i].ulUAddr;
        fctlpreviewattribute.ulBaseVAddr[i] = FrameList[i].ulVAddr;

        if ((FrameNum) && (cropped_bottom))
        {
            ///fill NV12 buffer bottom to black
            #if 1
            //dbg_printf(0, "ulBaseAddr[%d] = x%x, ulBaseUAddr[%d] = x%x\n", i, fctlpreviewattribute.ulBaseAddr[i], i, fctlpreviewattribute.ulBaseUAddr[i]);
            buf_offset = (usEncHeight - cropped_bottom) * usEncWidth;
            //dbg_printf(0, "buf_offset = x%x\n", buf_offset);
            MEMSET((void*)AIT_RAM_P2V(fctlpreviewattribute.ulBaseAddr[i] + buf_offset),
                   0x00, cropped_bottom * usEncWidth);
            buf_offset >>= 1;
            MEMSET((void*)AIT_RAM_P2V(fctlpreviewattribute.ulBaseUAddr[i] + buf_offset),
                   0x80, (cropped_bottom >> 1)*usEncWidth);
            #endif
        }
    }
    fctlpreviewattribute.bMirrorEnable = mirror;
    if (FrameNum)
    {
        fctlpreviewattribute.IbcLinkFx = MMPF_IBC_FX_TOFB;
        fctlpreviewattribute.colormode = MMPF_IBC_COLOR_NV12;
    }
    else
    {
        fctlpreviewattribute.IbcLinkFx = MMPF_IBC_FX_H264;
        fctlpreviewattribute.colormode = MMPF_IBC_COLOR_I420;
    }

    MMPF_Fctl_SetPreivewAttributes(&fctlpreviewattribute);
    #if AITCAM_MULTI_STREAM_EN
    MMPF_Fctl_LinkPreviewToVideo(PipeLink->ibcpipeID, ubEncId, lCtxNum);
    if ((lCtxNum == 3) || (PipeLink->ibcpipeID != MMPF_IBC_PIPE_1))
        MMPF_Fctl_SetLPFMaster(PipeLink->ibcpipeID, lCtxNum);
    if ((lCtxNum == 3) && (PipeLink->scalerpath == MMPF_SCALER_PATH_1))
    {
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, PipeLink->scalerpath);
        MMPF_Scaler_SetOutputColor(PipeLink->scalerpath,
                                   MMP_FALSE, MMPF_SCALER_COLRMTX_BT601);
    }
    else if (PipeLink->scalerpath != MMPF_SCALER_PATH_1)
    {
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, PipeLink->scalerpath);
        MMPF_Scaler_SetOutputColor(PipeLink->scalerpath,
                                   MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
    }
    #else
    MMPF_Fctl_LinkPreviewToVideo(PipeLink->ibcpipeID, ubEncId, 0);
    MMPF_Fctl_SetLPFMaster(PipeLink->ibcpipeID, 0);

    MMPF_Scaler_SetPath(usScaleSrc, PipeLink->scalerpath);
    #if (CUSTOMER == CVN)
    MMPF_Scaler_SetOutputColor(PipeLink->scalerpath,
                               MMP_TRUE, MMPF_SCALER_COLRMTX_FULLRANGE);
    #else
    MMPF_Scaler_SetOutputColor(PipeLink->scalerpath,
                               MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
    #endif
    #endif

    return MMP_ERR_NONE;
}
#endif

#if AITCAM_MULTI_STREAM_EN
MMP_ULONG lRecord[5] = {4, 4, 4, 4, 4};
#endif

MMP_UBYTE ubCurSnrId = PRM_SENSOR;
#if (SUPPORT_DUAL_SNR_PRV)
MMP_BOOL  bRawPathInited = 0;
#endif

static int aitcam_vidioc_reqbufs(struct file *file, void *priv,
                                 struct v4l2_requestbuffers *reqbufs)
{
    struct  aitcam_ctx *ctx = fh_to_ctx(priv);
    struct  aitcam_dev *dev = ctx->dev;
    int         ret = 0;
    //MMP_USHORT  snr_width = 0, snr_height = 0;
    MMP_ULONG   dram_addr, sram_addr;
    struct ait_mem_regions          availmem;
    MMPF_SCALER_FIT_RANGE           fitrange;
    MMPF_SCALER_GRABCONTROL         grabctl;
    MMPF_FCTL_PREVIEWATTRIBUTE      fctl_attribute;
    #if AITCAM_MULTI_STREAM_EN
    MMP_ULONG   bufsize, i;
    MMPF_VIDENC_FRAME               gra_frame_list[2];
    #endif
    #if (SUPPORT_DUAL_SNR_PRV)
    MMP_ULONG   bufsize, i;
    MMPF_VIDENC_FRAME               raw_frame_list[2];
    //MMP_RAW_STORE_BUF rawbuf;
    MMP_ULONG         ulStoreW, ulStoreH;
    #endif
    MMPF_VIDENC_MODULE_CONFIG       HwConfig;
    #if 1
    // just return 0 in case the reqbuf count is 0.
    if(reqbufs)
    {
        if(!reqbufs->count)
        {
            dbg_printf(0, "[reqbufs].count=0,just return\n");
            return 0;
        }
    }
    #endif
    if ((reqbufs->memory != V4L2_MEMORY_MMAP) ||
            (reqbufs->type != V4L2_BUF_TYPE_VIDEO_CAPTURE))
    {
        return -EINVAL;
    }

    #if (SUPPORT_DUAL_SNR_PRV)
    if(ctx->snr_src_id)
    {
        ubCurSnrId = SCD_SENSOR;
    }
    else
    #endif
    {
        ubCurSnrId = PRM_SENSOR;
    }

    #if AITCAM_IPC_EN==0
    MMPF_Sensor_GetCurResolution(ubCurSnrId, &snr_width, &snr_height);

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 2)
    {
        dbg_printf(0, "sensor[%d] : %d x %d\n", ubCurSnrId, snr_width, snr_height);
        dbg_printf(0, "ctx%d : %d x %d\n", ctx->num, ctx->img_width, ctx->img_height);
        dbg_printf(0, "snr_src_id = %d\n", ctx->snr_src_id);
    }
    #endif
    #endif
    
#if 1
    ctx->qcnt = ctx->dqcnt = 0 ;
    ctx->ts =  0;
    ctx->buf_index = -1 ;
    ctx->realtime= (DEF_STREAM_TYPE & V4L2_CAMERA_AIT_REALTIME_TYPE)?1:0;

    if ( aitcam_is_loopstream_opened(ctx) < 0 ) {
      //dbg_printf(0,"#reqbuf.ctx id:%d,this fmt:0x%08x,loop mode already used\n",ctx->num,ctx->output_format);  
      return -EINVAL ;
    }

      
#endif

    #if (USING_FIXED_MEM_POOL)  // assigned pipe by specified format

    if(1)
    {
        int pipe =-1;
        
        if(ctx->img_pipe > 0)
        {
            pipe = ctx->img_pipe ;
        }
        else
        {

            pipe = aitcam_pipealloc(ctx->num, ctx->output_format, ctx->img_width, ctx->img_height);
            ctx->img_pipe = pipe ;
        }
        
        if(pipe < 0)
        {
            pr_err("#slash this fmt:0x%x, no pipe..\r\n", ctx->output_format);
            return -EINVAL;
        }

        ctx->pipe_link.scalerpath = MMPF_SCALER_PATH_0 + ctx->num;
        ctx->pipe_link.icopipeID  = MMPF_ICO_PIPE_0    + ctx->num;
        ctx->pipe_link.ibcpipeID  = MMPF_IBC_PIPE_0    + ctx->num;
        dbg_printf(0, "#pipe id:%d,for ctx->num:%d,fmt:0x%0x\r\n", pipe, ctx->num, ctx->output_format);

    }

    #else

    /// HARDWARE RESOURCE ALLOCATE =============================================
    #if AITCAM_MULTI_STREAM_EN
    if (ctx->output_format == V4L2_PIX_FMT_H264)
    {
        if (dev->num_reqbufs == 0)
        {
            ctx->pipe_link_1.scalerpath = MMPF_SCALER_PATH_0;
            ctx->pipe_link_1.icopipeID = MMPF_ICO_PIPE_0;
            ctx->pipe_link_1.ibcpipeID = MMPF_IBC_PIPE_0;
        }
        ctx->pipe_link.scalerpath = MMPF_SCALER_PATH_1;
        ctx->pipe_link.icopipeID = MMPF_ICO_PIPE_1;
        ctx->pipe_link.ibcpipeID = MMPF_IBC_PIPE_1;
    }
    else if (ctx->output_format == V4L2_PIX_FMT_MJPEG)
    {
        ctx->pipe_link.scalerpath = MMPF_SCALER_PATH_2;
        ctx->pipe_link.icopipeID = MMPF_ICO_PIPE_2;
        ctx->pipe_link.ibcpipeID = MMPF_IBC_PIPE_2;
    }
    #else
    ctx->pipe_link.scalerpath = MMPF_SCALER_PATH_0 + ctx->num;
    ctx->pipe_link.icopipeID = MMPF_ICO_PIPE_0 + ctx->num;
    ctx->pipe_link.ibcpipeID = MMPF_IBC_PIPE_0 + ctx->num;
    #endif
    #endif

    #if (SUPPORT_DUAL_SNR_PRV)
    if(dual_snr_en)
    {
        if(dev->num_streaming == 0)
        {
            if(1/*bRawPathInited == 0*/)
            {
                MMPF_VIF_GetGrabResolution(MMPF_Sensor_GetVIFPad(SCD_SENSOR), &ulStoreW, &ulStoreH);
                if ((ret = aitcam_mem_getinfo(dev, AIT_MEM_IDX_RAW, &availmem)))
                {
                    dbg_printf(0, "raw mem get fail\n");
                    goto ABORT_FMT_INIT_MOD;
                }
                HwConfig.RawModCfg.DramAddr = ALIGN32(availmem.base_addr[MEM_DRAM]);//Gra dram base
                HwConfig.RawModCfg.SramAddr = ALIGN32(availmem.base_addr[MEM_SRAM]);//Gra sram base
                HwConfig.RawModCfg.CurFrameList = raw_frame_list;
                HwConfig.RawModCfg.CurBufNum = 2;
                HwConfig.RawModCfg.MaxReservedWidth = ALIGN16(MAX_WIDTH_H264);
                HwConfig.RawModCfg.MaxReservedHeight = ALIGN16(MAX_HEIGHT_H264);

                if (0)  //debug_level) {
                {
                    dbg_printf(0, "raw phys start x%x/x%x\n", HwConfig.RawModCfg.DramAddr, HwConfig.RawModCfg.SramAddr);
                }

                {
                    dram_addr = ALIGN32(HwConfig.RawModCfg.DramAddr);
                    sram_addr = ALIGN32(HwConfig.RawModCfg.SramAddr);
                    if (HwConfig.RawModCfg.CurBufNum)   //only support DRAM
                    {

                        MMPF_RAWPROC_CalcBufSize(MMP_RAW_COLORFMT_BAYER8, ulStoreW, ulStoreH, &bufsize);  // use RAW8 to store 1280x480

                        for (i = 0; i < HwConfig.RawModCfg.CurBufNum; i++)
                        {
                            HwConfig.RawModCfg.CurFrameList[i].ulYAddr = ALIGN256(dram_addr);
                            dbg_printf(0, "@@@ RawAddr[%d]:%x\n", i, HwConfig.RawModCfg.CurFrameList[i].ulYAddr);
                            MMPF_RAWPROC_SetRawStoreBuffer(MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_RAW_STORE_PLANE0, i, HwConfig.RawModCfg.CurFrameList[i].ulYAddr);
                            dram_addr += bufsize;
                        }
                    }
                }

                availmem.end_addr[MEM_DRAM] = dram_addr;
                availmem.end_addr[MEM_SRAM] = sram_addr;
                if ((ret = aitcam_mem_alloc(dev, AIT_MEM_IDX_RAW, &availmem)))
                {
                    dbg_printf(0, "raw mem alloc fail\n");
                    goto ABORT_FMT_INIT_MOD;
                }

                if (0)  //debug_level) {
                {
                    dbg_printf(0, "raw phys end x%x/x%x\n", dram_addr, sram_addr);
                }

                MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_S0, MMP_TRUE);
                MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_S1, MMP_TRUE);
                MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_F, MMP_TRUE);
                if (HwConfig.RawModCfg.CurBufNum > 0)
                {
                    MMPF_RAWPROC_InitStoreBuffer(MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_RAW_STORE_PLANE0, HwConfig.RawModCfg.CurFrameList[0].ulYAddr);
                }

                bRawPathInited = 1;  //
            }
        }
    }
    #endif

    #if (SUPPORT_DUAL_SNR_PRV)
    if(dual_snr_en)
    {
        if(ctx->snr_src_id)
        {
            GraRAWLinkCallBackFunc[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = Raw2Gra_loopback_frame;
            GraRAWLinkCallBackArgu0[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = ctx->pipe_link.ibcpipeID;
            GraRAWLinkCallBackArgu1[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = ctx->img_width;
            GraRAWLinkCallBackArgu2[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = ctx->img_height;
            dbg_printf(0, ">>RawId = %d, Srcpipe = %d\n", MMPF_Sensor_GetVIFPad(SCD_SENSOR), ctx->pipe_link.ibcpipeID);
            dbg_printf(0, ">>ctx(%d):w(%d), h(%d)\n", ctx->num, GraRAWLinkCallBackArgu1[MMPF_Sensor_GetVIFPad(SCD_SENSOR)],
                       GraRAWLinkCallBackArgu2[MMPF_Sensor_GetVIFPad(SCD_SENSOR)]);
        }
    }
    #endif

    /// module common memory
    switch (ctx->output_format)
    {
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_MV_AITHREC:
            #if AITCAM_IPC_EN==0
            if (!MMPF_VIDENC_IsModuleInit(MMPF_VIDENC_MODULE_H264))
            {
                if(!cpu_ipc)
                {
                  if ((ret = aitcam_mem_getinfo(dev, AIT_MEM_IDX_H264, &availmem)))
                  {
                      goto ABORT_FMT_INIT_MOD;
                  }
  
                  HwConfig.H264ModCfg.DramAddr = ALIGN32(availmem.base_addr[MEM_DRAM]);
                  HwConfig.H264ModCfg.SramAddr = ALIGN32(availmem.base_addr[MEM_SRAM]);
                  //dbg_printf(0, "h264 phys start x%x/x%x\n", HwConfig.H264ModCfg.DramAddr, HwConfig.H264ModCfg.SramAddr);//cm
                  HwConfig.H264ModCfg.MaxReservedWidth = ALIGN16(MAX_WIDTH_H264);
                  HwConfig.H264ModCfg.MaxReservedHeight = ALIGN16(MAX_HEIGHT_H264);
                  HwConfig.H264ModCfg.RingBufSize = 0;
                  #if (AITCAM_REALTIME_MODE)
                  HwConfig.H264ModCfg.RTBufAllocMode = MMPF_VIDENC_BUFCFG_SRAM;
                  #else
                  HwConfig.H264ModCfg.RTBufAllocMode = MMPF_VIDENC_BUFCFG_NONE;
                  #endif
  
                  if (MMPF_VIDENC_InitModule(MMPF_VIDENC_MODULE_H264, &HwConfig)
                          != MMP_ERR_NONE)
                  {
                      ret = -ENOMEM;
                      goto ABORT_FMT_INIT_MOD;
                  }
  
                  availmem.end_addr[MEM_DRAM] = HwConfig.H264ModCfg.DramAddr;
                  availmem.end_addr[MEM_SRAM] = HwConfig.H264ModCfg.SramAddr;
                  //dbg_printf(0, "h264 phys end x%x/x%x\n", HwConfig.H264ModCfg.DramAddr, HwConfig.H264ModCfg.SramAddr);//cm
                  if ((ret = aitcam_mem_alloc(dev, AIT_MEM_IDX_H264, &availmem)))
                  {
                      goto ABORT_FMT_INIT_MOD;
                  }
                }
            }
            #endif
            
            #if AITCAM_MULTI_STREAM_EN
            if (dev->num_reqbufs == 0)
            {
                if ((ret = aitcam_mem_getinfo(dev, AIT_MEM_IDX_GRA, &availmem)))
                {
                    dbg_printf(0, "gra mem get fail\n");
                    goto ABORT_FMT_INIT_MOD;
                }
                HwConfig.GraModCfg.DramAddr = ALIGN32(availmem.base_addr[MEM_DRAM]);//Gra dram base
                HwConfig.GraModCfg.SramAddr = ALIGN32(availmem.base_addr[MEM_SRAM]);//Gra sram base
                HwConfig.GraModCfg.CurFrameList = gra_frame_list;
                HwConfig.GraModCfg.CurBufNum = 2;
                HwConfig.GraModCfg.MaxReservedWidth = ALIGN16(MAX_WIDTH_H264);
                HwConfig.GraModCfg.MaxReservedHeight = ALIGN16(MAX_HEIGHT_H264);

                if (0)  //debug_level) {
                {
                    dbg_printf(0, "gra phys start x%x/x%x\n", HwConfig.GraModCfg.DramAddr, HwConfig.GraModCfg.SramAddr);
                }

                {
                    dram_addr = ALIGN32(HwConfig.GraModCfg.DramAddr);
                    sram_addr = ALIGN32(HwConfig.GraModCfg.SramAddr);
                    if (HwConfig.GraModCfg.CurBufNum)   //only support DRAM
                    {
                        bufsize = HwConfig.GraModCfg.MaxReservedWidth * HwConfig.GraModCfg.MaxReservedHeight;
                        for (i = 0; i < HwConfig.GraModCfg.CurBufNum; i++)
                        {
                            HwConfig.GraModCfg.CurFrameList[i].ulYAddr = dram_addr;
                            dram_addr += bufsize;

                            HwConfig.GraModCfg.CurFrameList[i].ulUAddr = dram_addr;
                            dram_addr += (bufsize >> 2);
                            HwConfig.GraModCfg.CurFrameList[i].ulVAddr = dram_addr;
                            dram_addr += (bufsize >> 2);
                        }
                    }
                }

                availmem.end_addr[MEM_DRAM] = dram_addr;
                availmem.end_addr[MEM_SRAM] = sram_addr;
                if ((ret = aitcam_mem_alloc(dev, AIT_MEM_IDX_GRA, &availmem)))
                {
                    dbg_printf(0, "gra mem alloc fail\n");
                    goto ABORT_FMT_INIT_MOD;
                }

                if (0)  //debug_level) {
                {
                    dbg_printf(0, "gra phys end x%x/x%x\n", dram_addr, sram_addr);
                }
            }
            #endif
            break;
        case V4L2_PIX_FMT_MJPEG:
            #if AITCAM_IPC_EN==0
            if (!MMPF_JPEG_IsModuleInit())
            {
                if(!cpu_ipc)
                {
                  MMPF_JPEG_MODULE_CONFIG jcfg =
                  {
                      .LinebufAllocMode   = MMPF_JPEG_LINEBUF_SRAM,
                      .usReservedMaxWidth = MAX_WIDTH_JPEG,
                      .usReservedMaxHeight = MAX_HEIGHT_JPEG
                  };
  
                  if ((ret = aitcam_mem_getinfo(dev, AIT_MEM_IDX_JPEG, &availmem)))
                  {
                      goto ABORT_FMT_INIT_MOD;
                  }
                  jcfg.DramAddr = ALIGN32(availmem.base_addr[MEM_DRAM]);
                  jcfg.SramAddr = ALIGN32(availmem.base_addr[MEM_SRAM]);
  
                  if (MMPF_JPEG_InitModule(&jcfg) != MMP_ERR_NONE)
                  {
                      ret = -ENOMEM;
                      goto ABORT_FMT_INIT_MOD;
                  }
  
                  availmem.end_addr[MEM_DRAM] = jcfg.DramAddr;
                  availmem.end_addr[MEM_SRAM] = jcfg.SramAddr;
                  if ((ret = aitcam_mem_alloc(dev, AIT_MEM_IDX_JPEG, &availmem)))
                  {
                      goto ABORT_FMT_INIT_MOD;
                  }
                }
            }
            #endif
            break;
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
#if AITCAM_IPC_EN
        case V4L2_PIX_FMT_MPEG:
#endif          
            break;
        default:
            ret = -EINVAL;
            goto ABORT_FMT_INIT_MOD;
            break;
    }
    /// HARDWARE RESOURCE ALLOCATE END =========================================

    /// CONTEXT INDEPENDENT RESOURCE ALLOCATE ==================================
    if ((ret = aitcam_mem_getinfo(dev, ctx->num, &availmem)))
    {
        goto ABORT_FMT_INIT_MOD;
    }
    dram_addr = ALIGN32(availmem.base_addr[MEM_DRAM]);
    sram_addr = ALIGN32(availmem.base_addr[MEM_SRAM]);

    if (debug_level)
    {
        dbg_printf(0, "ctx%d start x%x/x%x\n", ctx->num, dram_addr, sram_addr);
    }

    if(!cpu_ipc)
    {
      #if AITCAM_IPC_EN==0
      switch (ctx->output_format)
      {
          case V4L2_PIX_FMT_H264:
          case V4L2_PIX_FMT_MV_AITHREC:
          {
              MMP_ULONG               enc_id = 0;
              #if AITCAM_MULTI_STREAM_EN
              MMP_LONG                lReqOrd = 0;
              #endif
              MMPF_OS_WORK_CTX        work;
              MMPF_VIDENC_RESOLUTION  resol =
              {
                  .usWidth    = ALIGN16(ctx->img_width),
                  .usHeight   = ALIGN16(ctx->img_height),
              };
              #if (AITCAM_REALTIME_MODE)
              MMPF_VIDENC_CURBUF_MODE_CTL curbuf_mode_ctl =
              {
                  .CurBufMode  = MMPF_VIDENC_CURBUF_RT,
                  .ubRTSrcPipe = ctx->num,
              };
              #endif
              MMPF_VIDENC_CROPPING    crop = {0, 0, 0, 0};
              MMPF_VIDENC_FRAME       frame_list[2];
              MMPF_VIDENC_INSTANCE_CONFIG EncConfig;
  
              EncConfig.H264InstCfg.DramAddr = dram_addr;
              EncConfig.H264InstCfg.SramAddr = sram_addr;
              EncConfig.H264InstCfg.CurFrameList = frame_list;
              #if AITCAM_MULTI_STREAM_EN
              EncConfig.H264InstCfg.CurBufNum = 1;
              #else
              EncConfig.H264InstCfg.CurBufNum = 2;
              #endif
              EncConfig.H264InstCfg.MaxReservedWidth = (resol.usWidth < MIN_WIDTH_H264) ?
                      MIN_WIDTH_H264 : resol.usWidth;
              EncConfig.H264InstCfg.MaxReservedHeight = (resol.usHeight < MIN_HEIGHT_H264) ?
                      MIN_HEIGHT_H264 : resol.usHeight;
              #if 1 ///reduce dram usage
              EncConfig.H264InstCfg.RefGenBufMode = MMPF_VIDENC_REFGENBUF_OVERWRITE;
              #else
              EncConfig.H264InstCfg.RefGenBufMode = MMPF_VIDENC_REFGENBUF_INDEPENDENT;
              #endif
              if (ctx->output_format == V4L2_PIX_FMT_H264)
              {
                  EncConfig.H264InstCfg.FilterMode = MMPF_VIDENC_FILTER_ENCODER;
              }
              else if (ctx->output_format == V4L2_PIX_FMT_MV_AITHREC)
              {
                  EncConfig.H264InstCfg.FilterMode = MMPF_VIDENC_FILTER_ME_REF_RECONSTRUCT;
              }
              else
              {
                  goto ABORT_FMT_INIT_MOD;
              }
  
              if (MMPF_VIDENC_InitInstance(&enc_id, &EncConfig, MMPF_VIDENC_MODULE_H264)
                      != MMP_ERR_NONE)
              {
                  dbg_printf(0, "#Err not available h264 instance\n");
                  ret = -EBUSY;
                  goto ABORT_FMT_INIT_MOD;
              }
              dram_addr = EncConfig.H264InstCfg.DramAddr;
              sram_addr = EncConfig.H264InstCfg.SramAddr;
  
              ctx->codec_handle = (void*)enc_id;
  
              work.Exec = aitbh_h264enc_enc_start;
              work.Task = dev->H264EncTask;
              work.Work = &(ctx->EncStartWork);
              MMPF_OS_RegisterWork(MMPF_OS_WORKID_ENC_ST_0 + enc_id, &work);
  
              #if AITCAM_MULTI_STREAM_EN
              if (dev->num_reqbufs == 0)
              {
                  set_pipe_config(snr_width, snr_height, 1920, 1088,
                                  0, gra_frame_list, HwConfig.GraModCfg.CurBufNum,
                                  &ctx->pipe_link_1, ctx->scale_resol,
                                  MMP_FALSE, enc_id, 0, 0);
              }
              for(i = 0; i < (AITCAM_NUM_CONTEXTS - 1); i++)
              {
                  if(GraIBCLinkCallBackFunc[i] == NULL)
                  {
                      if(i != ctx->num)
                          lReqOrd = i;
                      else
                          lReqOrd = ctx->num;
                      lRecord[ctx->num] = lReqOrd;
                      break;
                  }
              }
              GraIBCLinkCallBackFunc[lRecord[ctx->num]] = _loopback_frame;
              GraIBCLinkCallBackArgu0[lRecord[ctx->num]] = resol.usWidth;
              GraIBCLinkCallBackArgu1[lRecord[ctx->num]] = resol.usHeight;
              dbg_printf(0, ">>IRecord[%d] = %d\n", ctx->num, lRecord[ctx->num]);//cm
              dbg_printf(0, ">>ctx(%d):w(%d), h(%d)\n", ctx->num, GraIBCLinkCallBackArgu0[ctx->num], GraIBCLinkCallBackArgu1[ctx->num]);//cm
              #endif
  
              #if (AITCAM_REALTIME_MODE == 0)
              if (resol.usHeight > ctx->img_height)
              {
                  crop.usBottom = resol.usHeight - ctx->img_height;
              }
              #endif
  
              #if AITCAM_MULTI_STREAM_EN
              set_pipe_config(snr_width, snr_height, resol.usWidth, resol.usHeight,
                              crop.usBottom, frame_list, EncConfig.H264InstCfg.CurBufNum,
                              &ctx->pipe_link, ctx->scale_resol,
                              MMP_FALSE, enc_id, lRecord[ctx->num], 0);
              #else
  
              #if (SUPPORT_DUAL_SNR_PRV)
              if(ctx->snr_src_id)
              {
                  set_pipe_config(snr_width, snr_height, resol.usWidth, resol.usHeight,
                                  crop.usBottom, frame_list, EncConfig.H264InstCfg.CurBufNum,
                                  &ctx->pipe_link, ctx->scale_resol,
                                  MMP_FALSE, enc_id, 0, MMPF_SCALER_SOURCE_GRA);
              }
              else
              #endif
              {
                  set_pipe_config(snr_width, snr_height, resol.usWidth, resol.usHeight,
                                  crop.usBottom, frame_list, EncConfig.H264InstCfg.CurBufNum,
                                  &ctx->pipe_link, ctx->scale_resol,
                                  MMP_FALSE, enc_id, 0, MMPF_SCALER_SOURCE_ISP);
              }
              #endif
  
              MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_RESOLUTION, &resol);
              #if (AITCAM_REALTIME_MODE)
              MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, &curbuf_mode_ctl);//cm
              #endif
              MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_CROPPING, &crop);
              MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_RINGBUF_EN, (void*)MMP_FALSE);
              #if H264_SPSPPS_EVERY_FRM
              MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_PARSET_EVERY_FRM, (void*)MMP_TRUE);
              #endif
              MMPF_VIDMGR_SetOutputPipe(enc_id, ctx->num);
              #if (AIT_ISP_VIDEO_IQ == 1)
              #if (WTF_LOW_BR == 1)
              {
                  MMPF_H264ENC_MEMD_PARAM param;
                  MMPF_VIDENC_QP_BOUND_CTL QpBound;
  
                  MMPF_VIDENC_GetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_MEMD_PARAM, &param);
  
                  param.usMeStopThr[BD_HIGH]  = WTF_MAGIC_STOP_H;
                  param.usMeSkipThr[BD_HIGH]  = WTF_MAGIC_SKIP_H;
  
                  MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_MEMD_PARAM, &param);
  
  
                  QpBound.ubLayerID = 0;
                  MMPF_VIDENC_GetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_FRM_QP_BOUND, &QpBound);
  
                  QpBound.ubLayerID = 0;
                  QpBound.ubTypeBitMap = (1 << I_FRAME);
                  QpBound.ubQPBound[I_FRAME][BD_HIGH] = WTF_MAGIC_IQP_H;
                  MMPF_VIDENC_SetParameter(enc_id, MMPF_VIDENC_ATTRIBUTE_FRM_QP_BOUND, &QpBound);
              }
              #endif
              #endif
  
              #if AITCAM_MULTI_STREAM_EN
              dev->num_reqbufs++;
              #endif
  
              #if AITCAM_MULTI_STREAM_EN
              availmem.max_buf_size[MEM_DRAM] = (((ALIGN16(ctx->img_width) * ALIGN16(ctx->img_height) * 3) >> 1) * 2 + 1024 * 1024 + 230 * 1024);
              #else
              availmem.max_buf_size[MEM_DRAM] = (((ALIGN16(ctx->img_width) * ALIGN16(ctx->img_height) * 3) >> 1) * 3 + 1024 * 1024 + 230 * 1024);
              #endif
              availmem.max_buf_size[MEM_SRAM] = 0;
  
              break;
          }
          case V4L2_PIX_FMT_MJPEG:
          {
              MMPF_JPEG_CTL_RESOL jpeg_res =
              {
                  .usWidth    = ALIGN8(ctx->img_width),
                  .usHeight   = ALIGN8(ctx->img_height),
              };
  
              ctx->codec_handle = kzalloc(sizeof(MMPF_JPEG_INSTANCE), GFP_KERNEL);
              if (!ctx->codec_handle)
              {
                  ret = -ENOMEM;
                  goto ABORT_FMT_INIT_MOD;
              }
  
              MMPF_JPEG_InitInstance(ctx->codec_handle, NULL);
  
              if (MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_RESOLUTION,
                                         (void*)&jpeg_res) != MMP_ERR_NONE)
              {
                  ret = -EINVAL;
                  goto ABORT_FMT_INIT;
              }
              if (MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_SRC_PIPE_ID,
                                         (void*)ctx->pipe_link.ibcpipeID) != MMP_ERR_NONE)
              {
                  ret = -EINVAL;
                  goto ABORT_FMT_INIT;
              }
              if (MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_DST_PIPE_ID,
                                         (void*)ctx->num) != MMP_ERR_NONE)
              {
                  ret = -EINVAL;
                  goto ABORT_FMT_INIT;
              }
              //dbg_printf(0, ">%d", ctx->num);//cm
  
              fitrange.fitmode    = MMPF_SCALER_FITMODE_OUT;
              fitrange.usFitResol = ctx->scale_resol;
              fitrange.usInWidth  = snr_width;
              fitrange.usInHeight = snr_height;
              fitrange.usOutWidth = jpeg_res.usWidth;
              fitrange.usOutHeight = jpeg_res.usHeight;
              calculate_scaling_fov(&fitrange, &grabctl);
  
              fctl_attribute.fctllink = ctx->pipe_link;
              fctl_attribute.FctlLinkSrc = MMPF_SCALER_SOURCE_ISP;
              fctl_attribute.usInputW = snr_width;
              fctl_attribute.usInputH = snr_height;
              #if AITCAM_MULTI_STREAM_EN
              fctl_attribute.lCtxNum = 2;
              #endif
              fctl_attribute.grabctl  = grabctl;
              fctl_attribute.usBufferCount = 0;
              fctl_attribute.bMirrorEnable = MMP_FALSE;
              fctl_attribute.IbcLinkFx = MMPF_IBC_FX_JPG;
              fctl_attribute.colormode = MMPF_IBC_COLOR_YUV422;
              MMPF_Fctl_SetPreivewAttributes(&fctl_attribute);
              MMPF_Fctl_SetLPFMaster(ctx->pipe_link.ibcpipeID, 2);
              MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, ctx->pipe_link.scalerpath);
              MMPF_Scaler_SetOutputColor(ctx->pipe_link.scalerpath, MMP_FALSE,
                                         MMPF_SCALER_COLRMTX_FULLRANGE);
  
              availmem.max_buf_size[MEM_DRAM] = 0;
              availmem.max_buf_size[MEM_SRAM] = 0;
  			
              break;
          }
          case V4L2_PIX_FMT_GREY:
          case V4L2_PIX_FMT_YUV420:
          case V4L2_PIX_FMT_YUYV:
          case V4L2_PIX_FMT_UYVY:
              fitrange.fitmode    = MMPF_SCALER_FITMODE_OUT;
              fitrange.usFitResol = ctx->scale_resol;
              fitrange.usInWidth  = snr_width;
              fitrange.usInHeight = snr_height;
              fitrange.usOutWidth = ctx->img_width;
              fitrange.usOutHeight = ctx->img_height;
              calculate_scaling_fov(&fitrange, &grabctl);
  
              fctl_attribute.fctllink = ctx->pipe_link;
              #if (SUPPORT_DUAL_SNR_PRV)
              if(ctx->snr_src_id)
              {
                  fctl_attribute.FctlLinkSrc = MMPF_SCALER_SOURCE_GRA;
              }
              else
              #endif
              {
                  fctl_attribute.FctlLinkSrc = MMPF_SCALER_SOURCE_ISP;
              }
              fctl_attribute.usInputW = snr_width;
              fctl_attribute.usInputH = snr_height;
              fctl_attribute.grabctl  = grabctl;
              fctl_attribute.usBufferCount = 0; //not to internal module, direct to v4l2 buf
              fctl_attribute.bMirrorEnable = MMP_FALSE;
              fctl_attribute.IbcLinkFx = MMPF_IBC_FX_TOFB;
              switch (ctx->output_format)
              {
                  case V4L2_PIX_FMT_GREY:
                      fctl_attribute.colormode = MMPF_IBC_COLOR_YUV420_LUMA_ONLY;
                      break;
                  case V4L2_PIX_FMT_YUV420:
                      fctl_attribute.colormode = MMPF_IBC_COLOR_I420;
                      break;
                  case V4L2_PIX_FMT_YUYV:
                      fctl_attribute.colormode = MMPF_IBC_COLOR_YUV422_YUYV;
                      break;
                  case V4L2_PIX_FMT_UYVY:
                      fctl_attribute.colormode = MMPF_IBC_COLOR_YUV422;
                      break;
              }
  
              MMPF_Fctl_SetPreivewAttributes(&fctl_attribute);
              MMPF_Fctl_SetLPFMaster(ctx->pipe_link.ibcpipeID, 0);
              #if (SUPPORT_DUAL_SNR_PRV)
              if(ctx->snr_src_id)
              {
                  MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, ctx->pipe_link.scalerpath);
              }
              else
              #endif
              {
                  MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, ctx->pipe_link.scalerpath);
              }
              MMPF_Scaler_SetOutputColor(ctx->pipe_link.scalerpath, MMP_FALSE,
                                         MMPF_SCALER_COLRMTX_FULLRANGE);
  
              availmem.max_buf_size[MEM_DRAM] = 0;
              availmem.max_buf_size[MEM_SRAM] = 0;
  			
              break;
          default:
              ret = -EINVAL;
              goto ABORT_FMT_INIT_MOD;
      }
      #endif
    }
    else {
      availmem.max_buf_size[MEM_DRAM] = 0;
      availmem.max_buf_size[MEM_SRAM] = 0;
         
    }
    //slot buf
    dram_addr = ALIGN_PAGE(dram_addr);
    MMPF_VIDBUF_Initialize(ctx->num, dram_addr, reqbufs->count,
                           ctx->enc_dst_buf_size);
    dram_addr += ctx->enc_dst_buf_size * reqbufs->count;
    dram_addr = ALIGN_PAGE(dram_addr);

    availmem.end_addr[MEM_DRAM] = dram_addr;
    availmem.end_addr[MEM_SRAM] = sram_addr;

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 2)
    {
        dbg_printf(0, "ctx%d max_buf_size = x%x /x%x\n", ctx->num, availmem.max_buf_size[MEM_DRAM], availmem.max_buf_size[MEM_SRAM]);
        dbg_printf(0, "slotbuf --> x%x * %d \n", ctx->enc_dst_buf_size, reqbufs->count);
        dbg_printf(0, "slotbuf --> x%x/x%x(x%x), x%x/x%x\n", availmem.base_addr[MEM_DRAM], dram_addr, (dram_addr - availmem.base_addr[MEM_DRAM]), 
                                                                             		  availmem.base_addr[MEM_SRAM], sram_addr);//cm
    }
    #endif

    if ((ret = aitcam_mem_alloc(dev, ctx->num, &availmem)))
    {
        dbg_printf(0,"aitcam_mem_alloc[%d] : %d\n",ctx->num,ret);
        goto ABORT_FMT_INIT;
    }
    /// CONTEXT INDEPENDENT RESOURCE ALLOCATE END ==============================

    ctx->inst_state = AITCAM_STAT_ALLOC;

    if (debug_level)
    {
        dbg_printf(0, "ctx%d end x%x/x%x\n", ctx->num, availmem.end_addr[MEM_DRAM], availmem.end_addr[MEM_SRAM]);
    }

    #if TEST_RAW_PATH == 1
    glRawBufAddr = dram_addr;
    #endif

    return ret;

ABORT_FMT_INIT:
    #if AITCAM_IPC_EN==0
    switch (ctx->output_format)
    {
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_MV_AITHREC:
            MMPF_VIDENC_DeinitInstance((MMP_ULONG)ctx->codec_handle);
            break;
        case V4L2_PIX_FMT_MJPEG:
            if (ctx->codec_handle)
            {
                MMPF_JPEG_DeinitInstance(ctx->codec_handle);
                kfree(ctx->codec_handle);
            }
            break;
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
#if AITCAM_IPC_EN
        case V4L2_PIX_FMT_MPEG:
#endif          
        default:
            break;
    }
    #endif
ABORT_FMT_INIT_MOD:
    return ret;
}

static int aitcam_vidioc_querybuf(struct file *file, void *priv,
                                  struct v4l2_buffer *buf)
{
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    int ret = 0;
    MMP_ULONG   buf_num, buf_size, buf_addr;

    MMPF_Video_GetVidBufQueueInfo(ctx->num, &buf_addr, &buf_size, &buf_num);

    if ((buf->memory != V4L2_MEMORY_MMAP) ||
            (buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) ||
            (buf->index >= buf_num))
    {
        dbg_printf(0, "ctx %d querybuf fail %d/%d\n",
                   ctx->num, buf->index, buf_num);
        return -EINVAL;
    }

    ///buf->m.offset = buf_addr + buf_size*buf->index;
    buf->m.offset = (buf->index << PAGE_SHIFT);
    buf->length = buf_size;
    return ret;
}

static int aitcam_vidioc_qbuf(struct file *file, void *priv, struct v4l2_buffer *buf)
{
    CPU_LOCK_INIT();
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    unsigned long flags;

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 3)
    {
        printk(KERN_ERR "(%d)Qbuf st: %d\n", ctx->num, buf->index);
    }
    #endif
    ctx->qcnt++ ;
    //if(ctx->num==0) {
    //  dbg_printf(0,"     Q[%d]\n",buf->index);
    //}
    if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        MMP_ULONG   state = 0;
        MMP_ERR     ret;
        int depth   ;
        if(cpu_ipc) {
          CPU_LOCK();  
        }
        depth = MMPF_VIDBUF_GetDepth(ctx->cam_vbq, VIDBUF_FREE_QUEUE) ;
        
        ret = MMPF_VIDBUF_GetState(ctx->cam_vbq, buf->index, &state);
        
        if(cpu_ipc) {
          CPU_UNLOCK();
        }
        
        #if 0 // 20160316 , return 0 if queues are all free
        if ( depth == ctx->cam_vbq->buf_num) {
            dbg_printf(0,"[qbufoo]:%d\n",ctx->cam_vbq->buf_num);
            return 0;
        }
        #endif
       #if 0
        {
          printk("-q[%d],ts : %d\r\n",ctx->num,buf->timestamp);
        }
        #endif
 
      
        if ((ret != MMP_ERR_NONE) || (state & VIDBUF_STATE_LOCKED))
        {
            int i,depth ;
            MMP_ULONG s[4] ;
            for(i=0;i<ctx->cam_vbq->buf_num;i++) {
                MMPF_VIDBUF_GetState(ctx->cam_vbq, i, &s[i]);
            }
            dbg_printf(0,"      Q%d[%d] NG-[%d %d %d %d]\n",ctx->num,buf->index,s[0],s[1],s[2],s[3]);

            goto QBUF_FAIL ;
        }

    if(0/*ctx->num==0*/) {
      int i;
      MMP_ULONG s[4] ;
      for(i=0;i<ctx->cam_vbq->buf_num;i++) {
        MMPF_VIDBUF_GetState(ctx->cam_vbq, i, &s[i]);
      }
      dbg_printf(0,"     Q%d[%d]-[%d,%d,%d,%d]\n",ctx->num,buf->index,s[0],s[1],s[2],s[3] );
    }

        ret = MMPF_VIDBUF_SetState(ctx->cam_vbq, buf->index,
                                   VIDBUF_STATE_LOCKED, MMP_TRUE);


        if (ret != MMP_ERR_NONE)
        {
            printk(KERN_ERR " qbuf fail 1\n");
            goto QBUF_FAIL;
        }
#if 1 // set timestamp
        buf->timestamp.tv_usec = 0 ;
        buf->timestamp.tv_sec  = 0 ;
#endif        
        spin_lock_irqsave(&ctx->dev->irqlock, flags);
        if(cpu_ipc) {
          OS_LockCore();
        }

        MMPF_VIDBUF_PushVBQ(ctx->cam_vbq, VIDBUF_FREE_QUEUE, buf->index);
        if(cpu_ipc) {
          OS_UnlockCore();
        }
        spin_unlock_irqrestore(&ctx->dev->irqlock, flags);
        #ifdef AITCAM_DBG_MSG
        if (debug_level >= 3)
        {
            printk(KERN_ERR "(%d)Qbuf end\n", ctx->num);
        }
        #endif

        return 0;
    }

QBUF_FAIL:
    return -EINVAL;
}

static int aitcam_vidioc_dqbuf(struct file *file, void *priv, struct v4l2_buffer *buf)
{
    CPU_LOCK_INIT();
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    unsigned long flags;
    #if (KERN_DIV64 == 1)
    MMP_ULONG64 ullBit64;
    #endif

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 3)
    {
        printk(KERN_ERR "(%d)DQbuf st\n", ctx->num);
    }
    #endif
    ctx->dqcnt++ ;
    if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        MMP_ERR     ret;
        int retval;
        /// check ctx status is under streaming
        // pVbq->ready_vbq.size == 0
        #if AITCAM_IPC_EN==0
        
        if (MMPF_VIDBUF_GetDepth(ctx->cam_vbq, VIDBUF_READY_QUEUE) == 0)
        {
            if(file->f_flags & O_NONBLOCK)
            {
                // non-block mode return -EAGAIN
                printk(KERN_ERR "no frame available\n");
                return -EAGAIN;
            }
        }

        if( !(file->f_flags & O_NONBLOCK))
        {
            //printk(KERN_ERR "[down]...\n" );
            retval = down_interruptible(&ctx->cam_vbq->vbq_lock);
            //printk(KERN_ERR "[down]:%d\n",MMPF_VIDBUF_GetDepth(ctx->cam_vbq, VIDBUF_READY_QUEUE) );
        }
        #endif
        spin_lock_irqsave(&ctx->dev->irqlock, flags);
        if(cpu_ipc) {
          OS_LockCore();
        }
        buf->index = MMPF_VIDBUF_PopVBQ(ctx->cam_vbq, VIDBUF_READY_QUEUE);
        if(cpu_ipc) {
          OS_UnlockCore();
        }
        spin_unlock_irqrestore(&ctx->dev->irqlock, flags);
        
        
        if(buf->index == MMPF_VIDENC_MAX_QUEUE_SIZE)
        {
            printk(KERN_ERR "no frame available:%d\n",buf->index);
            return -EAGAIN;
        }

        #if 1 // return phy address for cpub-b
        buf->reserved = ctx->cam_vbq->buffers[buf->index].buf_start ;
        #endif
        buf->bytesused = ctx->cam_vbq->buffers[buf->index].used_size;

        #if 0
        if(1/*ctx->cam_vbq->buffers[buf->index].timestamp*/) {
          pr_info("# cpu-b sid : %d,lost 1st frame,got ts: %ul\n",ctx->num,(u32)ctx->cam_vbq->buffers[buf->index].timestamp);
        } 
        #endif
        
        if(debug_ts && (ctx->output_format==V4L2_PIX_FMT_H264) ) {
          
        pr_info("[%d]old ts : %06d, new ts %06d,diff:%d\n",ctx->num,
                                                      (MMP_ULONG)ctx->ts,
                                                      (MMP_ULONG)ctx->cam_vbq->buffers[buf->index].timestamp,
                                                      (MMP_ULONG)(ctx->cam_vbq->buffers[buf->index].timestamp-ctx->ts) );
        }
        
        #if 0
        {
          printk("+dq[%d],ts : %d ,avail# %d\r\n",ctx->num,
                                                  ctx->cam_vbq->buffers[buf->index].timestamp,
                                                  MMPF_VIDBUF_GetDepth(ctx->cam_vbq, VIDBUF_READY_QUEUE));
        }
        #endif
        
        if( (ctx->ts > 0) && (ctx->num==0) ) {
          MMP_ULONG diff ;
          diff = (MMP_ULONG)(ctx->cam_vbq->buffers[buf->index].timestamp - ctx->ts );
          if(diff==0) {
              dbg_printf(0,"  ***BAD.TS : %d,diff:%d\n",(MMP_ULONG)ctx->cam_vbq->buffers[buf->index].timestamp ,diff);
          }
          if(0/*diff >=40*/) {
              dbg_printf(0,"  ***BAD.TS ??? : %d,diff:%d\n",(MMP_ULONG)ctx->cam_vbq->buffers[buf->index].timestamp,diff);
          }
        }
        
        ctx->ts = ctx->cam_vbq->buffers[buf->index].timestamp ;
        

        #if (KERN_DIV64 == 1)
        ullBit64 = (MMP_ULONG64)ctx->cam_vbq->buffers[buf->index].timestamp;
        
        do_div(ullBit64, 1000);
        buf->timestamp.tv_sec = (long)ullBit64;
        #else
        buf->timestamp.tv_sec = (MMP_ULONG64)ctx->cam_vbq->buffers[buf->index].timestamp / 1000;
        #endif
        buf->timestamp.tv_usec = (ctx->cam_vbq->buffers[buf->index].timestamp -
                                  (buf->timestamp.tv_sec * 1000)) * 1000;

        #if (STATISTIC_EN == 1)
        if (stat_en)
        {
            struct stat_entry_info  entry;
            MMP_ULONG64   timestamp = 0;
            int nms, kbps = 0, f = 0, fps = 0;
	     int time_diff = 0;

            MMPF_OS_GetTimestamp(&timestamp, aitcam_clockbase());

            entry.size = buf->bytesused;
            entry.timestamp = (int)timestamp;
	     if((tag_timestamp[ctx->num] == 0) || (entry.timestamp < tag_timestamp[ctx->num]))
	     {
	         tag_timestamp[ctx->num] = entry.timestamp;
	     }	
	     time_diff = entry.timestamp - tag_timestamp[ctx->num];

            stat_window_add(&stat_win[ctx->num], &entry);

            {
                nms = stat_window_get_duration(&stat_win[ctx->num]);
                if (nms > 0)
                {
                    #if (KERN_DIV64 == 1)
                    ullBit64 = (MMP_ULONG64)(stat_win[ctx->num].entry_size_sum << 3) * 1000;
                    do_div(ullBit64, nms);
                    kbps = (int)ullBit64;
                    #else
                    kbps = ((MMP_ULONG64)(stat_win[ctx->num].entry_size_sum << 3) * 1000) / nms;
        	      #endif

                    f = kbps % 1000;
                    kbps /= 1000;

		      fps = (stat_win[ctx->num].depth * 1000) / nms;

                    if(time_diff >= 1000)
                    {
                        tag_timestamp[ctx->num] = entry.timestamp;
                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 2)
                        {
                            printk(KERN_ERR "[%d]> %d.%03d kbps, fps: %02d\n",  ctx->num, kbps, f, fps);
                        }
                        #endif
                    }
                    #if 0
                    if ((frame_number[ctx->num] & ((1 << STAT_CYCLE_LOG2 << 5) - 1)) ==
                            ((1 << STAT_CYCLE_LOG2 << 5) - 1))
                    {
                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 2)
                        {
                            printk(KERN_ERR "[%d]> %d.%03d kbps, fps: %02d\n",  ctx->num, kbps, f, fps);
                        }
                        #endif
                    }
		      #endif
                }
            }

            frame_number[ctx->num]++;
        }
        #endif


        #ifdef AITCAM_DBG_MSG
        if (debug_level >= 3)
        {
            printk(KERN_ERR "[%d]dqbuf[%d : %d]:%d bytes\n",  ctx->num, (unsigned int)buf->timestamp.tv_sec, (unsigned int)buf->timestamp.tv_usec,buf->bytesused );
            //printk(KERN_ERR "[%d]dqbuf[%d : %d], phy: x08%x\n",  ctx->num, (unsigned int)buf->timestamp.tv_sec, (unsigned int)buf->timestamp.tv_usec, (unsigned int)buf->reserved);
        }
        #endif
if(0/*!ctx->num*/) {
    int i;
    MMP_ULONG s[4] ;
    for(i=0;i<ctx->cam_vbq->buf_num;i++) {
      MMPF_VIDBUF_GetState(ctx->cam_vbq, i, &s[i]);
    }
    dbg_printf(0,"D%d[%d]-[%d,%d,%d,%d] ts(%d)\n",ctx->num,buf->index,s[0],s[1],s[2],s[3],(MMP_ULONG)ctx->cam_vbq->buffers[buf->index].timestamp);
}
      
      if( (ctx->buf_index >= 0) && (ctx->buf_index==buf->index) ) {
      //  dbg_printf(0,"    *****#same index : %d,ts:%d\n", buf->index ,(unsigned int)ctx->cam_vbq->buffers[buf->index].timestamp ) ; 
      }
      ctx->buf_index = buf->index ;


        ret = MMPF_VIDBUF_SetState(ctx->cam_vbq, buf->index,
                                   VIDBUF_STATE_LOCKED, MMP_FALSE);
                                   
        if (ret != MMP_ERR_NONE)
        {
            MMP_ULONG state=0;
            int i,depth = MMPF_VIDBUF_GetDepth(ctx->cam_vbq, VIDBUF_READY_QUEUE) ;
            printk(KERN_ERR " dqbuf fail 1,id:%d,depth:%d\n",buf->index,depth);
            for(i=0;i<ctx->cam_vbq->buf_num;i++) {
                MMPF_VIDBUF_GetState(ctx->cam_vbq, i, &state);
                dbg_printf(0,"buf[%d] : %x\n",i,state);
            }
            return -EINVAL;
        }

        #ifdef AITCAM_DBG_MSG
        if (debug_level >= 3)
        {
            printk(KERN_ERR "(%d)DQbuf end: %d, %d\n", ctx->num, buf->index, buf->bytesused);
        }
        #endif

        return 0;
    }

    return -EINVAL;
}

#if AITCAM_MULTI_STREAM_EN
MMP_LONG lMaxCtx = 0;
MMP_LONG NumStreaming = 0;
MMP_BOOL bCheckCtx[5] = {MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE};
extern MMP_ULONG lRecord[5];
#endif
#if AITCAM_IPC_EN==0
void DumpOPR(MMP_ULONG reg, MMP_ULONG length)
{
    int i;
    for(i = 0; i <= length; i++)
    {
        dbg_printf(0, "addr:%x val:%x\n", (int)reg, (int)ISP_IF_IQ_GetHwOpr(reg, 1));
        reg++;
    }
}
#endif

static int aitcam_vidioc_streamon(struct file *file, void *priv,
                                  enum v4l2_buf_type type)
{
  
  //extern __u32 isp_data_from_rtos[] ;
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    struct aitcam_dev *dev = ctx->dev;
    //MMP_BOOL bVifEn = MMP_FALSE;
    //MMP_ERR     ret;


    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 2)
        printk(KERN_ERR "aitcam_vidioc_streamon:%d\n", ctx->num);
    #endif

    dbg_printf(0, "[+++]:v4l2.ctrl.son.s,cpu_ipc:%d\n",cpu_ipc);
    // stop to return error for S_CTRL
    ctx->ctx_flags &= ~CTX_STOP_V4L2_RET_ERR ;    
    v4l2_ctrl_handler_setup(&ctx->ctrl_handler);
    // start to return error for S_CTRL 
    ctx->ctx_flags |= CTX_STOP_V4L2_RET_ERR ;
    
    dbg_printf(0, "[---]:v4l2.ctrl.son.e\n");

    if(!cpu_ipc)
    {
      #if AITCAM_IPC_EN==0
      #if (SUPPORT_DUAL_SNR_PRV)
      if(ctx->snr_src_id)
      {
  
          MMPF_VIF_IsInterfaceEnable(MMPF_Sensor_GetVIFPad(SCD_SENSOR), &bVifEn);
          if (bVifEn == MMP_FALSE)
          {
  
              MMPF_RAWPROC_EnablePath(SCD_SENSOR, MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_TRUE, MMP_RAW_IOPATH_VIF2RAW, MMP_RAW_COLORFMT_YUV422);
  
              //MMPF_Sensor_Set3AInterrupt(MMP_TRUE);
  
              MMPF_VIF_EnableInputInterface(MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_TRUE);
              MMPF_VIF_EnableOutput(MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_TRUE);
  
              //wait one frame end for isp dma opr
              if (MMPF_Sensor_CheckFrameEnd(MMPF_Sensor_GetVIFPad(SCD_SENSOR), 1) != MMP_ERR_NONE)
              {
                  ait_err("Sensor initial failed\n");
                  return -EINVAL;
              }
          }
      }
      else
      #endif
      {
          //if (dev->num_streaming == 0)
          {
              MMPF_VIF_IsInterfaceEnable(MMPF_Sensor_GetVIFPad(PRM_SENSOR), &bVifEn);
              if (bVifEn == MMP_FALSE)
              {
                  #if AITCAM_IPC_EN
                  if(force_demo_only) {
                    MMPF_Sensor_RestoreSnrSetting(&isp_data_from_rtos[1]);
                    dbg_printf(0,"#Set 1st snr \n");
                  }
                  #endif
                 MMPF_Sensor_Set3AInterrupt(MMP_TRUE);
  
                  MMPF_VIF_EnableInputInterface(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_TRUE);
                  MMPF_VIF_EnableOutput(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_TRUE);
  
                  //wait one frame end for isp dma opr
                  if (MMPF_Sensor_CheckFrameEnd(MMPF_Sensor_GetVIFPad(PRM_SENSOR), 1) != MMP_ERR_NONE)
                  {
                      ait_err("Sensor initial failed\n");
                      return -EINVAL;
                  }
                   
              }
          }
      }
  
  
      // Tune MCI priority
  
      switch (ctx->output_format)
      {
          case V4L2_PIX_FMT_H264:
          case V4L2_PIX_FMT_MV_AITHREC:
              #if AITCAM_MULTI_STREAM_EN
              if (dev->num_streamon == 0)
                  MMPF_Fctl_EnablePreview(ctx->pipe_link_1.ibcpipeID, MMP_TRUE, 0);
              MMPF_Fctl_EnablePreview(ctx->pipe_link.ibcpipeID, MMP_TRUE, lRecord[ctx->num]);
              #else
              MMPF_Fctl_EnablePreview(ctx->pipe_link.ibcpipeID, MMP_TRUE, 0);
              #endif
  
              ret = MMPF_VIDENC_Start((MMP_ULONG)ctx->codec_handle);
              if(ret != MMP_ERR_NONE)
                  return  -EINVAL;
  
              #if (AIT_ISP_VIDEO_IQ == 1)
              if (dev->isp_vidiq_ctx_num == ctx->num)
              {
                  AIT_IQ_VIDEO_CONFIG conf =
                  {
                      .usWidth = ALIGN16(ctx->img_width),
                      .usHeight = ALIGN16(ctx->img_height),
                      .VideoType = AIT_IQ_VIDEO_H264,
                      .VideoHandle = ctx->codec_handle
                  };
  
                  ait_iq_video_init(&conf, &(dev->hVidIq));
                  dev->isp_vidiq_en = 1;
              }
              #endif
  
              #if AITCAM_MULTI_STREAM_EN
              dev->num_streamon++;
              #endif
  
              break;
          case V4L2_PIX_FMT_MJPEG:
              MMPF_Fctl_EnablePreview(ctx->pipe_link.ibcpipeID, MMP_TRUE, 2);
              MMPF_JPEG_StartStream(ctx->codec_handle);
              break;
          case V4L2_PIX_FMT_GREY:
          case V4L2_PIX_FMT_YUV420:
          case V4L2_PIX_FMT_YUYV:
          case V4L2_PIX_FMT_UYVY:
          {
              struct aitcam_stream_param param;
  
              param.cmd = AITCAM_STREAM_S_START;
              param.arg = NULL;
              ait_rawstream_ioctl(ctx, &param);
              break;
          }
          default:
              break;
      }
      #endif

    }
    else {
      #if AITCAM_IPC_EN
      aitcam_ipc_info ipc_info ;
      ipc_info.img_info.streamid  = ctx->num;   
      if (ctx->output_format==V4L2_PIX_FMT_H264) { 
        ipc_info.img_info.ctrl_flag  = (fdfr_osd)?CTRL_FLAG_OSD_EN : 0 ;
        pr_info("#ctrl_flag : %x\n",  ipc_info.img_info.ctrl_flag );
      }
      ipc_info.img_info.img_width = ctx->img_width;
      ipc_info.img_info.img_height= ctx->img_height;
      ipc_info.img_info.img_fmt   = ctx->output_format;
      ipc_info.img_info.max_framesize = ctx->enc_dst_buf_size ;    
      ipc_info.img_info.streamtype = (ctx->loop_recording << 1 ) | (ctx->realtime);   
      if ( aitcam_ipc_streamon(&ipc_info) < 0 ) {
        pr_info("aitcam_ipc_streamon fail\n");
      }  
      #endif
    }

    dev->num_streaming++; ///locked by vfd mutex
    ctx->inst_state = AITCAM_STAT_RUNNING;

    #if (STATISTIC_EN == 1)
    frame_number[ctx->num] = 0;
    tag_timestamp[ctx->num] = 0;
    #endif

    #if AITCAM_MULTI_STREAM_EN
    if (ctx->output_format == V4L2_PIX_FMT_H264)
    {
        bCheckCtx[lRecord[ctx->num]] = MMP_TRUE;

        if (lRecord[ctx->num] > lMaxCtx)
            lMaxCtx = lRecord[ctx->num];

        NumStreaming++;
    }
    #endif

    #if TEST_RAW_PATH == 1
    _test_raw_path();
    #endif
    #if TEST_GRA_PATH
    //_test_gra_loopback();
    #endif

    /*
        #ifdef AITCAM_DBG_MSG
        if (debug_level >= 3) {
            extern void _dump_iomem(MMP_ULONG start, MMP_ULONG len);

            // Gra
            _dump_iomem(0xF0005000, 0x68);
            _dump_iomem(0xF0007900, 0x40);
            // Scaler
            _dump_iomem(0xF0006F00, 256);
            _dump_iomem(0xF0006400, 256);
            _dump_iomem(0xF0004500, 256);
            // ICON
            _dump_iomem(0xF0006c00, 0xa0);
    	 // IBC
            _dump_iomem(0xF0005600, 0xe0);
            _dump_iomem(0xF0005720, 0xb0);
            _dump_iomem(0xF0005820, 0x40);
        }
        #endif
    */
    return 0;
}

static int aitcam_vidioc_streamoff(struct file *file, void *priv,
                                   enum v4l2_buf_type type)
{
    struct aitcam_ctx *ctx = fh_to_ctx(priv);

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 2)
        printk(KERN_ERR "aitcam_vidioc_streamoff[%d]\n", ctx->num);
    #endif

    return aitcam_stop_stream(ctx);
}

static inline int h264_level(enum v4l2_mpeg_video_h264_level lvl)
{
    static unsigned int t[V4L2_MPEG_VIDEO_H264_LEVEL_5_1 + 1] =
    {
        /* V4L2_MPEG_VIDEO_H264_LEVEL_1_0   */ 10,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_1B    */ 9,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_1_1   */ 11,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_1_2   */ 12,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_1_3   */ 13,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_2_0   */ 20,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_2_1   */ 21,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_2_2   */ 22,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_3_0   */ 30,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_3_1   */ 31,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_3_2   */ 32,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_4_0   */ 40,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_4_1   */ 41,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_4_2   */ 42,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_5_0   */ 50,
        /* V4L2_MPEG_VIDEO_H264_LEVEL_5_1   */ 51
    };
    return t[lvl];
}

static inline int vui_sar_idc(enum v4l2_mpeg_video_h264_vui_sar_idc sar)
{
    static unsigned int t[V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_EXTENDED + 1] =
    {
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_UNSPECIFIED     */ 0,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_1x1             */ 1,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_12x11           */ 2,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_10x11           */ 3,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_16x11           */ 4,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_40x33           */ 5,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_24x11           */ 6,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_20x11           */ 7,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_32x11           */ 8,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_80x33           */ 9,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_18x11           */ 10,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_15x11           */ 11,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_64x33           */ 12,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_160x99          */ 13,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_4x3             */ 14,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_3x2             */ 15,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_2x1             */ 16,
        /* V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_EXTENDED        */ 255,
    };
    return t[sar];
}

#if AITCAM_IPC_EN
//
// Just implement necessary ctrl.
//
static int aitcam_ipc_s_ctrl(struct v4l2_ctrl *ctrl ,aitcm_ipc_ctrl_dir dir )
{
    struct aitcam_ctx *ctx = container_of(ctrl->handler, struct aitcam_ctx, ctrl_handler);
    struct aitcam_dev *dev = ctx->dev;

    aitcam_ipc_ctrl ipc_ctrl ;
    
    int ret = 0 ;    
    ipc_ctrl.streamid = ctx->num;
    ipc_ctrl.id  = ctrl->id ;
    ipc_ctrl.val = ctrl->val ; 
    strcpy(ipc_ctrl.name,ctrl->name);
    if(ctrl->id==V4L2_CID_CAMERA_AIT_STREAM_TYPE) {
      ctx->loop_recording = (ctrl->val & V4L2_CAMERA_AIT_LOOP_RECORDING)?1:0;
      ctx->realtime = (ctrl->val & V4L2_CAMERA_AIT_REALTIME_TYPE)?1:0;  
    }
    
    switch(ctrl->id) {
    case V4L2_CID_BRIGHTNESS:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->brightness == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->brightness = ctrl->val ;
        }
      }
      else {
        dev->brightness = ctrl->val ;
      }		
      ipc_ctrl.val = dev->brightness ;
      break ; 
    case V4L2_CID_CONTRAST:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->contrast == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->contrast = ctrl->val ;
        }
      }
      else {
        dev->contrast = ctrl->val ;
      }		
      ipc_ctrl.val = dev->contrast ;
      break ; 
    case V4L2_CID_GAMMA:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->gamma == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->gamma = ctrl->val ;
        }
      }
      else {
        dev->gamma = ctrl->val ;
      }		
      ipc_ctrl.val = dev->gamma ;
      break ; 
    case V4L2_CID_SHARPNESS:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->sharpness == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->sharpness = ctrl->val ;
        }
      }
      else {
        dev->sharpness = ctrl->val ;
      }		
      ipc_ctrl.val = dev->sharpness ;
      break ; 
    case V4L2_CID_BACKLIGHT_COMPENSATION:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->wdr == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->wdr = ctrl->val ;
        }
      }
      else {
        dev->wdr = ctrl->val ;
      }		
      ipc_ctrl.val = dev->wdr ;
      break ; 
    case V4L2_CID_HUE:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->hue == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->hue = ctrl->val ;
        }
      }
      else {
        dev->hue = ctrl->val ;
      }		
      ipc_ctrl.val = dev->hue ;
      break ; 
    case V4L2_CID_SATURATION:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->saturation == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->saturation = ctrl->val ;
        }
      }
      else {
        dev->saturation = ctrl->val ;
      }		
      ipc_ctrl.val = dev->saturation ;
      break ; 
    case V4L2_CID_POWER_LINE_FREQUENCY:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->powerfreq == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->powerfreq = ctrl->val ;
        }
      }
      else {
        dev->powerfreq = ctrl->val ;
      }		
      ipc_ctrl.val = dev->powerfreq ;
      break ; 
    case V4L2_CID_CAMERA_AIT_ORIENTATION:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->orientation == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->orientation = ctrl->val ;
        }
      }
      else {
        dev->orientation = ctrl->val ;
      }		
      ipc_ctrl.val = dev->orientation ;
      break ; 
    case V4L2_CID_CAMERA_AIT_NIGHT_VISION:
      if( !(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR ) ) {
        if(dev->nv_mode == DEV_TYPE_V4L2_RESET_VAL ) {	
        	dev->nv_mode = ctrl->val ;
        }
      }
      else {
        dev->nv_mode = ctrl->val ;
      }		
      ipc_ctrl.val = dev->nv_mode ;
      break ; 
    }
    if(  (!(ctx->ctx_flags  & CTX_STOP_V4L2_RET_ERR )) && ( aitcam_v4l2_ctrl_initbyrtos(ctrl->id)) ) {
      
    } else {
      if(ctrl->id==V4L2_CID_CAMERA_AIT_NIGHT_VISION) {
      }
      aitcam_ipc_set_ctrl( &ipc_ctrl ,dir);
      if(dir== IPC_V4L2_GET) {
        ctrl->val = ipc_ctrl.val ;
        //printk("[V4L2.Get] id : %08x, val = %d\r\n",ctrl->id,ctrl->val ); 
      }
    }
    return ret ;
}
#endif


static int aitcam_v4l2_s_ctrl(struct v4l2_ctrl *ctrl)
{
    //struct aitcam_ctx *ctx = container_of(ctrl->handler, struct aitcam_ctx, ctrl_handler);
    //struct aitcam_dev *dev = ctx->dev;
    //struct aitcam_params *p = &ctx->cam_params;
    //int ret = 0;
    // not support in ipc mode
    #if AITCAM_IPC_EN
    if(cpu_ipc  ) {
      // sean@20161205 , always pass to cpu-b
      if( 1/*(ctx->output_format == V4L2_PIX_FMT_H264) || (ctx->output_format == V4L2_PIX_FMT_MPEG) */) {
        aitcam_ipc_s_ctrl(ctrl,IPC_V4L2_SET) ;
      }
      return 0 ;  
    }
    #endif
    
    
    #if (AITCAM_IPC_EN==0)
    
    #if (SUPPORT_DUAL_SNR_PRV)
    if(ctx->snr_src_id)
    {
        ubCurSnrId = SCD_SENSOR;
    }
    else
    #endif
    {
        ubCurSnrId = PRM_SENSOR;
    }

    ///ISP related
    switch (ctrl->id)
    {
            #if H264ENC_RDO_EN
        case V4L2_CID_MPEG_AIT_RDO:
        {
            struct aitcam_ctx *ctx_h264 ;
            struct aitcam_dev *dev = ctx->dev;
            // search if H264 handle initialized
            ctx_h264 = (struct aitcam_ctx *)aitcam_get_ctx_by_format(dev, V4L2_PIX_FMT_H264);
            if(ctx_h264)
            {
                if(force_rdo)
                {
                    ctrl->val = force_rdo ;
                }
                MMPF_VIDENC_SetParameter((MMP_ULONG)ctx_h264->codec_handle, MMPF_VIDENC_ATTRIBUTE_RDO_EN, (void*)&ctrl->val);
            }
        }
        return 0;
        case V4L2_CID_MPEG_AIT_QSTEP3_P1:
        {
            struct aitcam_ctx *ctx_h264 ;
            struct aitcam_dev *dev = ctx->dev;
            // search if H264 handle initialized
            ctx_h264 = (struct aitcam_ctx *)aitcam_get_ctx_by_format(dev, V4L2_PIX_FMT_H264);
            if(ctx_h264)
            {

                MMPF_VIDENC_SetParameter((MMP_ULONG)ctx_h264->codec_handle, MMPF_VIDENC_ATTRIBUTE_RDO_QSTEP3_P1, (void*)&ctrl->val);
            }
        }
        return 0;
        case V4L2_CID_MPEG_AIT_QSTEP3_P2:
        {
            struct aitcam_ctx *ctx_h264 ;
            struct aitcam_dev *dev = ctx->dev;
            // search if H264 handle initialized
            ctx_h264 = (struct aitcam_ctx *)aitcam_get_ctx_by_format(dev, V4L2_PIX_FMT_H264);
            if(ctx_h264)
            {
                MMPF_VIDENC_SetParameter((MMP_ULONG)ctx_h264->codec_handle, MMPF_VIDENC_ATTRIBUTE_RDO_QSTEP3_P2, (void*)&ctrl->val);
            }

        }
        return 0;
        #endif
        #if H264ENC_TNR_EN
        case V4L2_CID_MPEG_AIT_TNR:
        {
            MMP_UBYTE tnr_level = 0 ;
            struct aitcam_ctx *ctx_h264 ;
            struct aitcam_dev *dev = ctx->dev;
            // search if H264 handle initialized
            ctx_h264 = (struct aitcam_ctx *)aitcam_get_ctx_by_format(dev, V4L2_PIX_FMT_H264);
            if(ctx_h264)
            {
                // force tnr to whatever when insmod
                if(force_tnr)
                {
                    ctrl->val = force_tnr ;
                }
                else
                {

                }

                if (ctrl->val & V4L2_H264_TNR_ZERO_MV_EN )
                    tnr_level |= TNR_ZERO_MV_EN ;

                if (ctrl->val & V4L2_H264_TNR_LOW_MV_EN )
                    tnr_level |= TNR_LOW_MV_EN ;

                if (ctrl->val & V4L2_H264_TNR_HIGH_MV_EN )
                    tnr_level |= TNR_HIGH_MV_EN ;

                //dbg_printf(0,"[tnr] : id : 0x%x,val:%x\n",ctrl->id,ctrl->val);
                MMPF_VIDENC_SetParameter((MMP_ULONG)ctx_h264->codec_handle, MMPF_VIDENC_ATTRIBUTE_TNR_EN, (void*)&tnr_level);
            }
        }
        return 0;
        #endif

        case V4L2_CID_BRIGHTNESS:
            ISP_IF_F_SetBrightness(ctrl->val);
            dev->brightness = ctrl->val;
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 3)
                printk(KERN_ERR "V4L2 BRIGHTNESS = %d\n", ctrl->val);
            #endif
            return 0;
        case V4L2_CID_CONTRAST:
            ISP_IF_F_SetContrast(ctrl->val);
            dev->contrast = ctrl->val;
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 3)
                printk(KERN_ERR "V4L2 CONTRAST = %d\n", ctrl->val);
            #endif
            return 0;
        case V4L2_CID_GAMMA:
            ISP_IF_F_SetGamma(ctrl->val);
            dev->gamma = ctrl->val;
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 3)
                printk(KERN_ERR "V4L2 GAMMA = %d\n", ctrl->val);
            #endif
            return 0;
        case V4L2_CID_SHARPNESS:
            ISP_IF_F_SetSharpness(ctrl->val);
            dev->sharpness = ctrl->val;
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 3)
                printk(KERN_ERR "V4L2 SHARPNESS = %d\n", ctrl->val);
            #endif
            return 0;
        case V4L2_CID_BACKLIGHT_COMPENSATION:
            ISP_IF_F_SetWDR((ctrl->val) ? 255 : 0);
            dev->wdr = ctrl->val;
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 3)
                printk(KERN_ERR "V4L2 WDR = %d\n", ctrl->val);
            #endif
            return 0;
        case V4L2_CID_HUE:
            ISP_IF_F_SetHue(ctrl->val);
            dev->hue = ctrl->val;
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 3)
                printk(KERN_ERR "V4L2 HUE = %d\n", ctrl->val);
            #endif
            return 0;
        case V4L2_CID_SATURATION:
        {
            unsigned long flags, lock = (dev->num_streaming > 0);
            if (lock)
            {
                spin_lock_irqsave(&ctx->dev->irqlock, flags);
            }
            if(ctrl->val  == -100)
            {
                #if (CUSTOMER == CVN)
                ISP_IF_IQ_SetSysMode(0);  // night mode
                if(ISP_IF_AE_GetSysMode() != 1)
                    ISP_IF_AE_SetSysMode(1);
                ISP_IF_F_SetImageEffect(ISP_IMAGE_EFFECT_GREY);  // Grey
                #endif
            }
            else
            {
                #if (CUSTOMER == CVN)
                ISP_IF_IQ_SetSysMode(1);  // normal mode
                if(ISP_IF_AE_GetSysMode() != 0)
                    ISP_IF_AE_SetSysMode(0);
                ISP_IF_F_SetImageEffect(ISP_IMAGE_EFFECT_NORMAL);  // Normal
                #endif
                ISP_IF_F_SetSaturation(ctrl->val);
            }
            dev->saturation = ctrl->val;
            if (lock)
            {
                spin_unlock_irqrestore(&ctx->dev->irqlock, flags);
            }
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 2)
                printk(KERN_ERR "V4L2 SATURATION = %d\n", ctrl->val);
            #endif
        }
        return 0;
        case V4L2_CID_POWER_LINE_FREQUENCY:
        {
            ISP_AE_FLICKER fkr = 0;
            switch (ctrl->val)
            {
                case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
                    fkr = ISP_AE_FLICKER_OFF;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 ISP_AE_FLICKER_OFF\n");
                    #endif
                    break;
                case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
                    fkr = ISP_AE_FLICKER_AUTO;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 ISP_AE_FLICKER_AUTO\n");
                    #endif
                    break;
                case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
                    fkr = ISP_AE_FLICKER_50HZ;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 ISP_AE_FLICKER_50HZ\n");
                    #endif
                    break;
                case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
                default:
                    fkr = ISP_AE_FLICKER_60HZ;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 ISP_AE_FLICKER_60HZ\n");
                    #endif
                    break;
            }
            if (fkr != ISP_IF_AE_GetFlicker())
            {
                ISP_IF_AE_SetFlicker(fkr);
            }
            return 0;
        }
        case V4L2_CID_EXPOSURE:
            ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AE, ctrl->val);
            return 0;
        case V4L2_CID_AUTO_WHITE_BALANCE:
            ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AWB, ctrl->val);
            return 0;
            #if 0
        case V4L2_CID_HFLIP:
            switch (dev->orientation)
            {
                case V4L2_CAMERA_AIT_ORTN_FLIP:
                    if(ctrl->val)
                    {
                        MMPF_Sensor_SetRotDirection(ubCurSnrId, MMPF_SENSOR_ROTATE_RIGHT_180);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR;
                    }
                    break;
                case V4L2_CAMERA_AIT_ORTN_MIRROR:
                    if(!ctrl->val)
                    {
                        MMPF_Sensor_SetRotDirection(ubCurSnrId, MMPF_SENSOR_ROTATE_NO_ROTATE);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_NORMAL;
                    }
                    break;
                case V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR :
                    if(!ctrl->val)
                    {
                        MMPF_Sensor_SetFlipDirection(ubCurSnrId, MMPF_SENSOR_COLUMN_FLIP);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_FLIP;
                    }
                    break;
                case V4L2_CAMERA_AIT_ORTN_NORMAL:
                default:
                    if(ctrl->val)
                    {
                        MMPF_Sensor_SetFlipDirection(ubCurSnrId, MMPF_SENSOR_ROW_FLIP);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_MIRROR;
                    }
                    break;
            }

            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 2)
                printk(KERN_ERR "V4L2_CID_HFLIP = %d\n", ctrl->val);
            #endif

            return 0;
        case V4L2_CID_VFLIP:
            switch (dev->orientation)
            {
                case V4L2_CAMERA_AIT_ORTN_FLIP:
                    if(!ctrl->val)
                    {
                        MMPF_Sensor_SetRotDirection(ubCurSnrId, MMPF_SENSOR_ROTATE_NO_ROTATE);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_NORMAL;
                    }
                    break;
                case V4L2_CAMERA_AIT_ORTN_MIRROR:
                    if(ctrl->val)
                    {
                        MMPF_Sensor_SetRotDirection(ubCurSnrId, MMPF_SENSOR_ROTATE_RIGHT_180);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR;
                    }
                    break;
                case V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR :
                    if(!ctrl->val)
                    {
                        MMPF_Sensor_SetFlipDirection(ubCurSnrId, MMPF_SENSOR_ROW_FLIP);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_MIRROR;
                    }
                    break;
                case V4L2_CAMERA_AIT_ORTN_NORMAL:
                default:
                    if(ctrl->val)
                    {
                        MMPF_Sensor_SetFlipDirection(ubCurSnrId, MMPF_SENSOR_COLUMN_FLIP);
                        dev->orientation = V4L2_CAMERA_AIT_ORTN_FLIP;
                    }
                    break;
            }

            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 2)
                printk(KERN_ERR "V4L2_CID_VFLIP = %d\n", ctrl->val);
            #endif

            return 0;
            #endif

        case V4L2_CID_FOCUS_ABSOLUTE:
            return 0;
        case V4L2_CID_FOCUS_AUTO:
            if(ctrl->val)
            {
                //ISP_IF_AF_Control(ISP_AF_START);
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 4)
                    printk(KERN_ERR "V4L2_CID_FOCUS_AUTO = %d\n", ctrl->val);
                #endif
            }
            return 0;

        case V4L2_CID_CAMERA_AIT_ORIENTATION:
            if (/*(dev->num_streaming == 0) &&*/
                (dev->orientation != ctrl->val))
            {
                dev->orientation = ctrl->val;
                switch (ctrl->val)
                {
                    case V4L2_CAMERA_AIT_ORTN_FLIP:
                        MMPF_Sensor_SetFlipDirection(ubCurSnrId, MMPF_SENSOR_COLUMN_FLIP);
                        break;
                    case V4L2_CAMERA_AIT_ORTN_MIRROR:
                        MMPF_Sensor_SetFlipDirection(ubCurSnrId, MMPF_SENSOR_ROW_FLIP);
                        break;
                    case V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR :
                        MMPF_Sensor_SetRotDirection(ubCurSnrId, MMPF_SENSOR_ROTATE_RIGHT_180);
                        break;
                    case V4L2_CAMERA_AIT_ORTN_NORMAL:
                    default:
                        MMPF_Sensor_SetRotDirection(ubCurSnrId, MMPF_SENSOR_ROTATE_NO_ROTATE);
                        break;
                }
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 2)
                    printk(KERN_ERR "V4L2 ORIENTATION = %d\n", ctrl->val);
                #endif

            }
            else if (dev->num_streaming)
            {
                // Keep sensor orientation setting after streaming
            }
            else
            {
                // Keep sensor orientation setting
            }
            return 0;
        case V4L2_CID_CAMERA_AIT_EXPOSURE_STATE:
            return 0;
        case V4L2_CID_CAMERA_AIT_MANUAL_R_GAIN:
        case V4L2_CID_CAMERA_AIT_MANUAL_G_GAIN:
        case V4L2_CID_CAMERA_AIT_MANUAL_B_GAIN:
            if (ISP_IF_3A_GetSwitch(ISP_3A_ALGO_AWB) == MMP_TRUE)
            {
                if (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_R_GAIN)
                    ctrl->val = ISP_IF_AWB_GetGainR();
                else if (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_G_GAIN)
                    ctrl->val = ISP_IF_AWB_GetGainG();
                else if (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_B_GAIN)
                    ctrl->val = ISP_IF_AWB_GetGainB();
            }
            else
            {
                ISP_UINT32  rgain, ggain, bgain;

                if (0)  //(dev->num_streaming == 0) {
                {
                    rgain = (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_R_GAIN) ? ctrl->val : ISP_IF_AWB_GetGainBase();
                    ggain = (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_G_GAIN) ? ctrl->val : ISP_IF_AWB_GetGainBase();
                    bgain = (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_B_GAIN) ? ctrl->val : ISP_IF_AWB_GetGainBase();
                }
                else
                {
                    rgain = (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_R_GAIN) ? ctrl->val : ISP_IF_AWB_GetGainR();
                    ggain = (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_G_GAIN) ? ctrl->val : ISP_IF_AWB_GetGainG();
                    bgain = (ctrl->id == V4L2_CID_CAMERA_AIT_MANUAL_B_GAIN) ? ctrl->val : ISP_IF_AWB_GetGainB();
                }
                ISP_IF_AWB_SetGains(rgain, ggain, bgain);
            }
            return 0;
        case V4L2_CID_CAMERA_AIT_NIGHT_VISION:
        {
            if (dev->nv_mode != ctrl->val)
            {
                dev->nv_mode = ctrl->val;
                MMPF_Sensor_SetNightVisionMode(NV_CTRL_MODE, ctrl->val);
            }
        }
        return 0;
        case V4L2_CID_CAMERA_AIT_IR_LED:
        {
            MMPF_Sensor_SetNightVisionMode(NV_CTRL_LED, ctrl->val);
        }
        return 0;
        case V4L2_CID_CAMERA_AIT_IR_SHUTTER:
        {
            MMPF_Sensor_SetNightVisionMode(NV_CTRL_SHUTTER, ctrl->val);
        }
        return 0;
        case V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE:
        case V4L2_CID_CAMERA_AIT_LIGHT_CONDITION:
        case V4L2_CID_CAMERA_AIT_NR_GAIN:
            return 0;

        case V4L2_CID_CAMERA_AIT_IMAGE_EFFECT:
        {
            char EffectMapping[V4L2_CAMERA_AIT_EFFECT_BW + 1] = {ISP_IMAGE_EFFECT_NORMAL,
                                                                 ISP_IMAGE_EFFECT_GREY,
                                                                 ISP_IMAGE_EFFECT_SEPIA,
                                                                 ISP_IMAGE_EFFECT_NEGATIVE,
                                                                 ISP_IMAGE_EFFECT_RED,
                                                                 ISP_IMAGE_EFFECT_GREEN,
                                                                 ISP_IMAGE_EFFECT_BLUE,
                                                                 ISP_IMAGE_EFFECT_YELLOW,
                                                                 ISP_IMAGE_EFFECT_BW
                                                                };
            ISP_IF_F_SetImageEffect(EffectMapping[ctrl->val]);
            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 4)
                printk(KERN_ERR "V4L2_CID_CAMERA_AIT_IMAGE_EFFECT = %d\n", ctrl->val);
            #endif
        }
        return 0;

        case V4L2_CID_JPEG_COMPRESSION_QUALITY:
            if(ctx->output_format == V4L2_PIX_FMT_MJPEG)
            {
                MMPF_VIDENC_BITRATE_CTL BrCtl;
                if(ctrl->val > 0)
                {
                    BrCtl.ubLayerBitMap = 0x01;
                    BrCtl.ulBitrate[0] = (ctrl->val) * 400 * 1000;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2_CID_JPEG_COMPRESSION_QUALITY = %d\n", BrCtl.ulBitrate[0]);
                    #endif
                    MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_RC_BITRATE, &BrCtl);
                }
            }
            return 0;
        case V4L2_CID_JPEG_CHROMA_SUBSAMPLING:
        case V4L2_CID_JPEG_RESTART_INTERVAL:
        case V4L2_CID_JPEG_ACTIVE_MARKER:
        case V4L2_CID_ZOOM_ABSOLUTE:
            return 0;
        default:
            break;
    }

    if (ctx->inst_state < AITCAM_STAT_ALLOC)
    {
        return ret; ///caching s_ctrl
    }

    ///Codec related
    switch (ctx->output_format)
    {
        case V4L2_PIX_FMT_H264:
            switch (ctrl->id)
            {
                    #if  H264ENC_TNR_EN
                case V4L2_CID_MPEG_AIT_TNR:
                    break;
                    #endif
                case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE:
                case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
                case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES:
                {
                    MMPF_VIDENC_SLICE_CTL SliceCtl;

                    if (ctrl->id == V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB)
                    {
                        p->slice_mb = ctrl->val;
                    }
                    else if (ctrl->id == V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES)
                    {
                        p->slice_byte = (ctrl->val << 3);
                    }
                    else
                    {
                        p->slice_mode = ctrl->val;
                    }

                    if (p->slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE)
                    {
                        SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_FRM;
                        SliceCtl.ulSliceSize = 0;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_SLICE_CTL,
                                                 &SliceCtl);
                    }
                    if ((p->slice_mode == V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_MB) &&
                            (p->slice_mb))
                    {
                        SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_MB;
                        SliceCtl.ulSliceSize = p->slice_mb;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_SLICE_CTL,
                                                 &SliceCtl);
                    }
                    if ((p->slice_mode == V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_BYTES) &&
                            (p->slice_byte))
                    {
                        SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_BYTE;
                        SliceCtl.ulSliceSize = p->slice_byte;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_SLICE_CTL,
                                                 &SliceCtl);
                    }
                }
                break;
                case V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB:
                    p->intra_refresh_mb = ctrl->val;
                    break;
                case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE://cm
                {
                    MMPF_VIDENC_RC_MODE_CTL RcModeCtl;

                    #if H264_LOW_BITRATE_CONTROL
                    RcModeCtl.RcMode = (ctrl->val) ? MMPF_VIDENC_RC_MODE_CBR :
                                       MMPF_VIDENC_RC_MODE_LOWBR;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                    {
                        if(ctrl->val)
                            printk(KERN_ERR "MMPF_VIDENC_RC_MODE_CBR\n");
                        else
                            printk(KERN_ERR "MMPF_VIDENC_RC_MODE_LOWBR\n");
                    }
                    #endif

                    #else
                    RcModeCtl.RcMode = (ctrl->val) ? MMPF_VIDENC_RC_MODE_CBR :
                                       MMPF_VIDENC_RC_MODE_CQP;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                    {
                        if(ctrl->val)
                            printk(KERN_ERR "MMPF_VIDENC_RC_MODE_CBR\n");
                        else
                            printk(KERN_ERR "MMPF_VIDENC_RC_MODE_CQP\n");
                    }
                    #endif
                    #endif

                    RcModeCtl.bLayerGlobalRc = MMP_FALSE;
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_RC_MODE,
                                             &RcModeCtl);
                }
                break;
                case V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL:
                case V4L2_CID_MPEG_AIT_VIDEO_FPS_INCR:
                {
                    MMPF_VIDENC_MAX_FPS_CTL FpsCtl;
                    if (ctrl->id == V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL)
                    {
                        p->fps_resolution = ctrl->val;
                    }
                    else
                    {
                        p->fps_increament = ctrl->val;
                    }
                    if (p->fps_resolution && p->fps_increament)
                    {
                        FpsCtl.ulMaxFpsResolution = p->fps_resolution;
                        FpsCtl.ulMaxFpsIncreament = p->fps_increament;
                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 2)
                            printk(KERN_ERR "V4L2 Set frame rate(H264) = %d(%d/%d)\n",
                                   (FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament),
                                   FpsCtl.ulMaxFpsResolution,
                                   FpsCtl.ulMaxFpsIncreament);
                        #endif
                        #if (AITCAM_FIX_SENSOR_FR)
                        ISP_IF_AE_SetFPS((FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament)); // set sensor fps

                        {
                            MMPF_VIDENC_GOP_CTL GopCtl;
                            GopCtl.usGopSize            = (FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament);
                            GopCtl.usMaxContBFrameNum   = 0;
                            GopCtl.SyncFrameType        = MMPF_VIDENC_SYNCFRAME_IDR;
                            GopCtl.bReset               = MMP_FALSE;
                            MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                     MMPF_VIDENC_ATTRIBUTE_GOP_CTL,
                                                     &GopCtl);
                        }
                        #endif
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_MAX_FPS,
                                                 &FpsCtl);
                    }
                }
                break;
                case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
                {
                    MMPF_VIDENC_GOP_CTL GopCtl;
                    GopCtl.usGopSize            = ctrl->val;
                    GopCtl.usMaxContBFrameNum   = 0;
                    GopCtl.SyncFrameType        = MMPF_VIDENC_SYNCFRAME_IDR;
                    GopCtl.bReset               = MMP_FALSE;
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_GOP_CTL,
                                             &GopCtl);
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 Set GOP size = %d\n", GopCtl.usGopSize);
                    #endif
                }
                break;
                case V4L2_CID_MPEG_VIDEO_BITRATE:
                {
                    u32 lbs ;
                    MMPF_VIDENC_BITRATE_CTL BrCtl;
                    
                    #if CPB_SIZE_CTRL_AS_LB_SIZE==0
                    s32 cpbsize = 0;
                    aitcam_v4l2_ctrl_val(ctx, V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE, &cpbsize);

                    lbs = H264_CPBSize2LBSize( V4L2CPB_SIZE(cpbsize) , Kb(ctrl->val));
                    //dbg_printf(0,"set br: cpb_size:%d bytes,bitrate:%d, new leakybucket:%d ms\n" ,  V4L2CPB_SIZE(cpbsize) , ctrl->val, lbs);
                    #endif
                    
                    BrCtl.ubLayerBitMap = 0x01;
                    BrCtl.ulBitrate[0] = ctrl->val;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 Set bitrate(H264) = %d\n", BrCtl.ulBitrate[0]);
                    #endif
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_BR,
                                             &BrCtl);
                    
                    #if CPB_SIZE_CTRL_AS_LB_SIZE==0
                    {
                        MMPF_VIDENC_LEAKYBUCKET_CTL LbCtl;

                        LbCtl.ubLayerBitMap = 0x01;
                        LbCtl.ulLeakyBucket[0] = lbs;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_LB_SIZE,
                                                 &LbCtl);
                    }
                    #endif
                    
                }
                break;
                case V4L2_CID_MPEG_AIT_VIDEO_RC_FRAME_SKIP:
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_RC_SKIPPABLE,
                                             (void*)((ctrl->val) ? MMP_TRUE : MMP_FALSE));
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 3)
                        printk(KERN_ERR "V4L2_RC_FRAME_SKIP : %d\n", ctrl->val);
                    #endif
                    break;
                case V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE:
                {

                    u32 lbs;
                    s32 bitrate = 0;
                    #if CPB_SIZE_CTRL_AS_LB_SIZE==0
                    aitcam_v4l2_ctrl_val(ctx, V4L2_CID_MPEG_VIDEO_BITRATE, &bitrate);

                    lbs = H264_CPBSize2LBSize( V4L2CPB_SIZE(ctrl->val) , Kb(bitrate));
                    #else
                    lbs = ctrl->val ;
                    #endif
                    //dbg_printf(0,"set cpb : cpb_size:%d bytes,bitrate:%d, new leakybucket:%d ms\n" ,  V4L2CPB_SIZE(ctrl->val) , bitrate, lbs);
                    {
                        MMPF_VIDENC_LEAKYBUCKET_CTL LbCtl;

                        LbCtl.ubLayerBitMap = 0x01;
                        LbCtl.ulLeakyBucket[0] = lbs;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_LB_SIZE,
                                                 &LbCtl);
                    }
                }
                break;
                case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
                {
                    H264Profile profile_flag ;

                    #if (AITCAM_REALTIME_MODE)
                    ctrl->val = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
                    #endif
                    switch (ctrl->val)
                    {
                        case V4L2_MPEG_VIDEO_H264_PROFILE_MAIN:
                            profile_flag = MAIN_P;
                            //MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                            //                MMPF_VIDENC_ATTRIBUTE_PROFILE,
                            //                (void*)MAIN_PROFILE);
                            break;
                        case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH:
                            profile_flag = HIGH_P;
                            //MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                            //                MMPF_VIDENC_ATTRIBUTE_PROFILE,
                            //                (void*)FREXT_HP);
                            break;
                        case V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE:
                            profile_flag = BASELINE_P;
                            //MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                            //                MMPF_VIDENC_ATTRIBUTE_PROFILE,
                            //                (void*)BASELINE_PROFILE);
                            break;
                        case V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE:
                        default:
                            profile_flag = CONSTRAINED_BASELINE_FLAG1 ;

                            //goto err_ctrl_param;
                    }
                    // ait internal:
                    // lsb = profile
                    // hsb = flag
                    //dbg_printf(0,"[sean]:Profile.Flag:0x%0x\r\n",profile_flag);
                    profile_flag = ( (profile_flag >> 8) & 0x00FF) | ( (profile_flag & 0x00FF) << 8 );
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_PROFILE,
                                             (void*)profile_flag);
                    break;
                }
                case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
                    p->codec.h264.level_v4l2 = ctrl->val;
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_LEVEL,
                                             (void*)h264_level(ctrl->val));
                    break;
                case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
                {
                    // put constraint check
                    s32 profile = 0;
                    aitcam_v4l2_ctrl_val(ctx, V4L2_CID_MPEG_VIDEO_H264_PROFILE, &profile);
                    //dbg_printf(0, "Cur Profile:%d\r\n", profile);

                    #if (AITCAM_REALTIME_MODE == 0)
                    if( (profile == V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE) ||
                            (profile == V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE) )
                    {

                        if(ctrl->val != V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC)
                        {
                            printk(KERN_ERR "V4L2 Cur H264 profile not allowed to cavac\n");
                            goto err_ctrl_param ;
                        }
                    }

                    if( (profile == V4L2_MPEG_VIDEO_H264_PROFILE_MAIN) ||
                            (profile == V4L2_MPEG_VIDEO_H264_PROFILE_HIGH) )
                    {

                        if(ctrl->val != V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC)
                        {
                            printk(KERN_ERR "V4L2 Cur H264 profile not allowed to cavlc\n");
                            goto err_ctrl_param ;
                        }
                    }
                    #else
                    ctrl->val = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC;
                    #endif
                    //dbg_printf(0,"Cur Entropy mode:%d\r\n",ctrl->val);
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_ENTROPY_MODE,
                                             (void*)((ctrl->val == V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC) ?
                                                     MMPF_H264ENC_ENTROPY_CAVLC : MMPF_H264ENC_ENTROPY_CABAC));
                    break;
                }
                case V4L2_CID_MPEG_VIDEO_H264_I_PERIOD:
                    p->gop_size = ctrl->val;
                    {
                        MMPF_VIDENC_GOP_CTL GopCtl;
                        GopCtl.usGopSize            = ctrl->val;
                        GopCtl.usMaxContBFrameNum   = 0;
                        GopCtl.SyncFrameType        = MMPF_VIDENC_SYNCFRAME_IDR;
                        GopCtl.bReset               = MMP_FALSE;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_GOP_CTL,
                                                 &GopCtl);

                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 2)
                            printk(KERN_ERR "V4L2 Set I frame period = %d\n", GopCtl.usGopSize);
                        #endif
                    }
                    break;
                case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
                case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
                {
                    MMPF_VIDENC_QP_CTL QpCtl;

                    memset(&QpCtl, 0, sizeof(QpCtl));

                    QpCtl.ubTID = 0;

                    if (ctrl->id == V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP)
                    {
                        QpCtl.ubTypeBitMap = (1 << 0);
                        QpCtl.ubQP[0] = ctrl->val;
                    }
                    else if (ctrl->id == V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP)
                    {
                        QpCtl.ubTypeBitMap = (1 << 1);
                        QpCtl.ubQP[1] = ctrl->val;
                    }
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_FRM_QP,
                                             &QpCtl);
                }
                break;
                case V4L2_CID_MPEG_VIDEO_H264_MIN_QP:
                case V4L2_CID_MPEG_VIDEO_H264_MAX_QP:
                {
                    MMPF_VIDENC_QP_BOUND_CTL QpBound;

                    memset(&QpBound, 0, sizeof(QpBound));

                    QpBound.ubLayerID = 0;
                    MMPF_VIDENC_GetParameter((MMP_ULONG)ctx->codec_handle, MMPF_VIDENC_ATTRIBUTE_FRM_QP_BOUND, &QpBound);

                    if (ctrl->id == V4L2_CID_MPEG_VIDEO_H264_MIN_QP)
                    {
                        QpBound.ubTypeBitMap = (1 << I_FRAME) | (1 << P_FRAME);
                        QpBound.ubQPBound[I_FRAME][BD_LOW] = ctrl->val;
                        QpBound.ubQPBound[P_FRAME][BD_LOW] = ctrl->val;
                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 3)
                            printk(KERN_ERR "V4L2 Set QP Lower bound(H264) = %d\n", ctrl->val);
                        #endif
                    }
                    else if (ctrl->id == V4L2_CID_MPEG_VIDEO_H264_MAX_QP)
                    {
                        QpBound.ubTypeBitMap = (1 << I_FRAME) | (1 << P_FRAME);
                        QpBound.ubQPBound[I_FRAME][BD_HIGH] = ctrl->val;
                        QpBound.ubQPBound[P_FRAME][BD_HIGH] = ctrl->val;
                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 3)
                            printk(KERN_ERR "V4L2 Set QP Upper bound(H264) = %d\n", ctrl->val);
                        #endif
                    }
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_FRM_QP_BOUND,
                                             &QpBound);

                }
                break;
                case V4L2_CID_MPEG_AIT_VIDEO_FORCE_FRAME:
                {
                    MMPF_VIDENC_PICCTL picctl = MMPF_VIDENC_PICCTL_NONE;
                    switch (ctrl->val)
                    {
                        case V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_IDR_RESYNC:
                            picctl = MMPF_VIDENC_PICCTL_IDR_RESYNC;
                            break;
                        case V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_IDR:
                            picctl = MMPF_VIDENC_PICCTL_IDR;
                            break;
                        case V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_I_RESYNC:
                            picctl = MMPF_VIDENC_PICCTL_I_RESYNC;
                            break;
                        case V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_I:
                            picctl = MMPF_VIDENC_PICCTL_I;
                            break;
                        default:
                            break;
                    }
                    if (picctl != MMPF_VIDENC_PICCTL_NONE)
                    {
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)picctl);
                        #if 1
                        ctrl->val = V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_NONE;
                        #endif
                    }
                }
                break;
                //	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
                //		p->codec.h264.loop_filter_mode = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA:
                //		p->codec.h264.loop_filter_alpha = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA:
                //		p->codec.h264.loop_filter_beta = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE:
                //		p->codec.h264.rc_mb = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_MIN_QP:
                //		p->codec.h264.rc_min_qp = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_MAX_QP:
                //		p->codec.h264.rc_max_qp = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_MFC51_VIDEO_PADDING:
                //		p->pad = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_MFC51_VIDEO_PADDING_YUV:
                //		p->pad_luma = (ctrl->val >> 16) & 0xff;
                //		p->pad_cb = (ctrl->val >> 8) & 0xff;
                //		p->pad_cr = (ctrl->val >> 0) & 0xff;
                //		break;
                //	case V4L2_CID_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE:
                //		ctx->force_frame_type = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_VBV_SIZE:
                //		p->vbv_size = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_HEADER_MODE:
                //		p->seq_hdr_mode = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE:
                //		p->frame_skip_mode = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_MFC51_VIDEO_RC_FIXED_TARGET_BIT:
                //		p->fixed_target_bit = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_B_FRAMES:
                //		p->num_b_frame = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_MFC51_VIDEO_H264_NUM_REF_PIC_FOR_P:
                //		p->codec.h264.num_ref_pic_4p = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM:
                //		p->codec.h264._8x8_transform = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
                //		p->codec.h264.rc_b_frame_qp = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE:
                //		p->codec.h264.vui_sar = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC:
                //		p->codec.h264.vui_sar_idc = vui_sar_idc(ctrl->val);
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH:
                //		p->codec.h264.vui_ext_sar_width = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT:
                //		p->codec.h264.vui_ext_sar_height = ctrl->val;
                //		break;
                //	case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE:
                //		p->codec.h264.open_gop = !ctrl->val;
                //		break;
                default:
                    goto err_ctrl_param;
            }
            break;
        case V4L2_PIX_FMT_MV_AITHREC:
            switch (ctrl->id)
            {
                case V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL:
                case V4L2_CID_MPEG_AIT_VIDEO_FPS_INCR:
                {
                    MMPF_VIDENC_MAX_FPS_CTL FpsCtl;
                    if (ctrl->id == V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL)
                    {
                        p->fps_resolution = ctrl->val;
                    }
                    else
                    {
                        p->fps_increament = ctrl->val;
                    }
                    if (p->fps_resolution && p->fps_increament)
                    {
                        FpsCtl.ulMaxFpsResolution = p->fps_resolution;
                        FpsCtl.ulMaxFpsIncreament = p->fps_increament;
                        MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                                 MMPF_VIDENC_ATTRIBUTE_MAX_FPS,
                                                 &FpsCtl);
                    }
                }
                break;
                case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
                case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
                {
                    MMPF_VIDENC_QP_CTL QpCtl;

                    memset(&QpCtl, 0, sizeof(QpCtl));

                    QpCtl.ubTID = 0;

                    if (ctrl->id == V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP)
                    {
                        QpCtl.ubTypeBitMap = (1 << 0);
                        QpCtl.ubQP[0] = ctrl->val;
                    }
                    else if (ctrl->id == V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP)
                    {
                        QpCtl.ubTypeBitMap = (1 << 1);
                        QpCtl.ubQP[1] = ctrl->val;
                    }
                    MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                             MMPF_VIDENC_ATTRIBUTE_FRM_QP,
                                             &QpCtl);
                }
                break;
                default:
                    break;
            }
            break;
        case V4L2_PIX_FMT_MJPEG:
            switch (ctrl->id)
            {
                case V4L2_CID_MPEG_AIT_VIDEO_RC_FRAME_SKIP:
                    MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_RC_SKIPPABLE,
                                           (void*)((ctrl->val) ? MMP_TRUE : MMP_FALSE));
                    break;
                case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE://cm
                {
                    MMPF_VIDENC_RC_MODE_CTL RcModeCtl;

                    RcModeCtl.RcMode = (ctrl->val) ? MMPF_VIDENC_RC_MODE_CBR :
                                       MMPF_VIDENC_RC_MODE_CQP;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                    {
                        if(ctrl->val)
                            printk(KERN_ERR "MMPF_VIDENC_RC_MODE_CBR\n");
                        else
                            printk(KERN_ERR "MMPF_VIDENC_RC_MODE_CQP\n");
                    }
                    #endif

                    RcModeCtl.bLayerGlobalRc = MMP_FALSE;
                    MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_RC_MODE, &RcModeCtl);
                }
                break;
                case V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL:
                case V4L2_CID_MPEG_AIT_VIDEO_FPS_INCR:
                {
                    MMPF_VIDENC_MAX_FPS_CTL FpsCtl;

                    if (ctrl->id == V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL)
                    {
                        p->fps_resolution = ctrl->val;
                    }
                    else
                    {
                        p->fps_increament = ctrl->val;
                    }
                    if (p->fps_resolution && p->fps_increament)
                    {
                        FpsCtl.ulMaxFpsResolution = p->fps_resolution;
                        FpsCtl.ulMaxFpsIncreament = p->fps_increament;
                        MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_MAX_FPS, &FpsCtl);

                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 2)
                            printk(KERN_ERR "V4L2 Set frame rate(MJPG) = %d(%d/%d)\n",
                                   (FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament),
                                   FpsCtl.ulMaxFpsResolution,
                                   FpsCtl.ulMaxFpsIncreament);
                        #endif
                        #if (AITCAM_FIX_SENSOR_FR)
                        ISP_IF_AE_SetFPS((FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament)); // set sensor fps
                        #endif
                    }
                }
                break;
                case V4L2_CID_MPEG_VIDEO_BITRATE://cm
                {
                    MMPF_VIDENC_BITRATE_CTL BrCtl;

                    BrCtl.ubLayerBitMap = 0x01;
                    BrCtl.ulBitrate[0] = ctrl->val;
                    #ifdef AITCAM_DBG_MSG
                    if (debug_level >= 2)
                        printk(KERN_ERR "V4L2 Set bitrate(MJPG) = %d\n", BrCtl.ulBitrate[0]);
                    #endif
                    MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_RC_BITRATE, &BrCtl);
                }
                break;
                default:
                    break;
            }
            break;
        case V4L2_PIX_FMT_GREY:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
            break;
        default:
            goto err_ctrl_param;
    }

    return ret;

err_ctrl_param:
    v4l2_err(&dev->v4l2_dev, "Invalid control, id=x%x, val=%d\n",
             ctrl->id, ctrl->val);
    return -EINVAL;
    #endif
    return 0;
}

static int aitcam_v4l2_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
  
    struct aitcam_ctx *ctx = container_of(ctrl->handler, struct aitcam_ctx, ctrl_handler);
    struct aitcam_dev *dev = ctx->dev;
    //struct aitcam_params *p = &ctx->cam_params;
    int ret = 0;


    #if AITCAM_IPC_EN
    if(cpu_ipc  ) {
      aitcam_ipc_s_ctrl(ctrl,IPC_V4L2_GET) ;
      return 0 ;  
    }
    #endif

    ///< ctrl->val equals the last val set by s_ctrl
    #if AITCAM_IPC_EN==0
    switch (ctrl->id)
    {
        case V4L2_CID_CAMERA_AIT_EXPOSURE_STATE:
            if (dev->num_streaming == 0)
            {
                ctrl->val = 0;
            }
            else
            {
                ctrl->val = ISP_IF_AE_GetGain() / ISP_IF_AE_GetGainBase();
            }
            break;
        case V4L2_CID_CAMERA_AIT_LIGHT_CONDITION:
            ctrl->val =  (dev->num_streaming == 0) ? 0 : (s32)ISP_IF_AE_GetLightCond() ;
            if(dev)
            {
                dev->lightcond = ctrl->val ;
            }
            break;
        case V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE:
            ctrl->val =  (dev->num_streaming == 0) ? 0 : MMPF_Sensor_GetNightVisionMode(NV_CTRL_LIGHT_MODE) ;
            if(dev)
            {
                dev->is_night = ctrl->val ;
            }
            break;
        case V4L2_CID_CAMERA_AIT_NR_GAIN:
            ctrl->val =  (dev->num_streaming == 0) ? 0 : (s32)ISP_IF_IQ_GetID(ISP_IQ_CHECK_CLASS_GAIN) ;
            if(dev)
            {
                dev->nr_gain = ctrl->val ;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 2)
                    printk(KERN_ERR "AIT_NR_GAIN = %d\n", ctrl->val);
                #endif
            }
            break;
        default:
            goto err_g_param;
    }
    #endif
    return ret;

err_g_param:
    v4l2_err(&dev->v4l2_dev, "Unknown control x%x\n",
             ctrl->id);
    return -EINVAL;
}

static const struct v4l2_ctrl_ops aitcam_v4l2_ctrl_ops =
{
    .s_ctrl = aitcam_v4l2_s_ctrl,
    .g_volatile_ctrl = aitcam_v4l2_g_volatile_ctrl
};

int aitcam_vidioc_s_parm(struct file *file, void *priv, struct v4l2_streamparm *sp)
{
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    struct v4l2_captureparm *cp = &sp->parm.capture;
    struct aitcam_dev *dev = ctx->dev;
    struct aitcam_params *p = &ctx->cam_params;
    unsigned long flags, lock = (dev->num_streaming > 0);
    MMPF_VIDENC_MAX_FPS_CTL FpsCtl;
    u32 tgt_fps;	/* target frames per secound */

    if (sp == NULL)
        return -EINVAL;

    if (sp->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    if (lock)
    {
        spin_lock_irqsave(&ctx->dev->irqlock, flags);
    }

    if ((cp->timeperframe.numerator == 0) ||
            (cp->timeperframe.denominator == 0))
    {
        /* reset framerate */
        cp->timeperframe.numerator = 1;
        cp->timeperframe.denominator = MAX_FRATE;
    }

    tgt_fps = cp->timeperframe.denominator / cp->timeperframe.numerator;

    if (tgt_fps > MAX_FRATE)
    {
        cp->timeperframe.denominator = MAX_FRATE;
        cp->timeperframe.numerator = 1;
    }
    else if (tgt_fps < 1)
    {
        cp->timeperframe.denominator = 1;
        cp->timeperframe.numerator = 1;
    }

    p->fps_increament = cp->timeperframe.numerator * 10;
    p->fps_resolution = cp->timeperframe.denominator * 10;
    cp->capability = V4L2_CAP_TIMEPERFRAME;

    if ((p->fps_resolution && p->fps_increament) && ctx->codec_handle)   // to fix kernel panic before reqbuf
    {
        FpsCtl.ulMaxFpsResolution = p->fps_resolution;
        FpsCtl.ulMaxFpsIncreament = p->fps_increament;
        #if AITCAM_IPC_EN==0
        if(!cpu_ipc)
        {
          switch (ctx->output_format)
          {
              case V4L2_PIX_FMT_H264:
                  #ifdef AITCAM_DBG_MSG
                  if (debug_level >= 2)
                      printk(KERN_ERR "aitcam_vidioc_s_parm(H264) = %d(%d/%d)\n",
                             (FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament),
                             FpsCtl.ulMaxFpsResolution,
                             FpsCtl.ulMaxFpsIncreament);
                  #endif
                  MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                           MMPF_VIDENC_ATTRIBUTE_MAX_FPS,
                                           &FpsCtl);
                  break;
              case V4L2_PIX_FMT_MV_AITHREC:
                  MMPF_VIDENC_SetParameter((MMP_ULONG)ctx->codec_handle,
                                           MMPF_VIDENC_ATTRIBUTE_MAX_FPS,
                                           &FpsCtl);
                  break;
              case V4L2_PIX_FMT_MJPEG:
                  MMPF_JPEG_SetParameter(ctx->codec_handle, MMPF_JPEG_PARAM_MAX_FPS, &FpsCtl);
  
                  #ifdef AITCAM_DBG_MSG
                  if (debug_level >= 2)
                      printk(KERN_ERR "aitcam_vidioc_s_parm(MJPG) = %d(%d/%d)\n",
                             (FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament),
                             FpsCtl.ulMaxFpsResolution,
                             FpsCtl.ulMaxFpsIncreament);
                  #endif
                  #if (AITCAM_FIX_SENSOR_FR)
                  ISP_IF_AE_SetFPS((FpsCtl.ulMaxFpsResolution / FpsCtl.ulMaxFpsIncreament)); // set sensor fps
                  #endif
                  break;
              default:
                  break;
          }
        }
        #endif        
    }
    else   // if(ctx->codec_handle == 0)
    {
        if (p->fps_resolution && p->fps_increament)
        {
            int i;

            #ifdef AITCAM_DBG_MSG
            if (debug_level >= 2)
                printk(KERN_ERR "aitcam_vidioc_s_parm = (%d/%d)\n",
                       p->fps_resolution,
                       p->fps_increament);
            #endif

            for (i = 0; i < NUM_CTRLS; i++)
            {
                if (controls[i].id == V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL)
                {
                    ctx->ctrls[i]->val = ctx->ctrls[i]->cur.val = p->fps_resolution;
                }
                else if (controls[i].id == V4L2_CID_MPEG_AIT_VIDEO_FPS_INCR)
                {
                    ctx->ctrls[i]->val = ctx->ctrls[i]->cur.val = p->fps_increament;
                }
            }
        }
    }

    if (lock)
    {
        spin_unlock_irqrestore(&ctx->dev->irqlock, flags);
    }

    return 0;
}

static int aitcam_vidioc_g_parm(struct file *file, void *priv, struct v4l2_streamparm *sp)
{
    struct aitcam_ctx *ctx = fh_to_ctx(priv);
    struct v4l2_captureparm *cp = &sp->parm.capture;
    //struct aitcam_dev *dev = ctx->dev;
    struct aitcam_params *p = &ctx->cam_params;

    if (sp->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    memset(sp, 0, sizeof(*sp));
    sp->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    cp->capability = V4L2_CAP_TIMEPERFRAME;
    cp->timeperframe.numerator = p->fps_increament / 10;
    cp->timeperframe.denominator = p->fps_resolution / 10;
    cp->capturemode = 0;
    cp->readbuffers = 2;

    return 0;
}


static	int aitcam_vidioc_sub_event (struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
    struct v4l2_ctrl *ctrl = NULL;
    switch (sub->type)
    {
        case V4L2_EVENT_CTRL:
            // send initial value
            if(sub->flags & V4L2_EVENT_SUB_FL_SEND_INITIAL)
            {
                ctrl = v4l2_ctrl_find(fh->ctrl_handler, sub->id);
                if(ctrl)
                {
                    #if AITCAM_IPC_EN==0
                    switch(sub->id)
                    {
                    
                        case V4L2_CID_CAMERA_AIT_LIGHT_CONDITION:
                            ctrl->val = (s32)ISP_IF_AE_GetLightCond() ;
                            break;
                        case V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE:
                            ctrl->val = (s32)MMPF_Sensor_GetNightVisionMode(NV_CTRL_LIGHT_MODE);
                            break ;
                    }
                    #endif
                    dbg_printf(0, "[sean.ipc.ng].sub_event[0x%0x],ctrl:0x%08x,val:%d\r\n", sub->id, (u32)ctrl, ctrl->val);
                }
            }
            return v4l2_event_subscribe(fh, sub, 0);
        default:
            return -EINVAL;
    }
}

static	int aitcam_vidioc_unsub_event(struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
    return v4l2_event_unsubscribe(fh, sub);
}


static const struct v4l2_ioctl_ops aitcam_ioctl_ops =
{
    .vidioc_querycap = aitcam_vidioc_querycap,
    .vidioc_enum_fmt_vid_cap = aitcam_vidioc_enum_fmt_vid_cap,
    .vidioc_g_fmt_vid_cap = aitcam_vidioc_g_fmt,
    .vidioc_try_fmt_vid_cap = aitcam_vidioc_try_fmt,
    .vidioc_s_fmt_vid_cap = aitcam_vidioc_s_fmt,
    .vidioc_reqbufs = aitcam_vidioc_reqbufs,
    .vidioc_querybuf = aitcam_vidioc_querybuf,
    .vidioc_qbuf = aitcam_vidioc_qbuf,
    .vidioc_dqbuf = aitcam_vidioc_dqbuf,
    .vidioc_streamon = aitcam_vidioc_streamon,
    .vidioc_streamoff = aitcam_vidioc_streamoff,
    .vidioc_s_parm = aitcam_vidioc_s_parm,
    .vidioc_g_parm = aitcam_vidioc_g_parm,
    .vidioc_subscribe_event = aitcam_vidioc_sub_event,
    .vidioc_unsubscribe_event = aitcam_vidioc_unsub_event,
};

int aitcam_stop_stream(struct aitcam_ctx *ctx)
{
    struct aitcam_dev *dev = ctx->dev;
    #if AITCAM_MULTI_STREAM_EN
    MMP_LONG i;
    #endif
    dbg_printf(0,"aitcam_stop_stream[%d],cpu_ipc:%d ,n_s :%d\n",ctx->num,cpu_ipc,dev->num_streaming);
    if(dev->num_streaming > 0)
    {
        // release the potential lock by select or dqbuf
        //
        // fixed kernel crash if no streamon called
        //
        if (ctx->inst_state == AITCAM_STAT_RUNNING)
        {
            aitcam_queue_wakeup( ctx->cam_vbq ) ;
        }
        
        if(!cpu_ipc)
        {
          #if AITCAM_IPC_EN==0
          switch (ctx->output_format)
          {
              case V4L2_PIX_FMT_H264:
              case V4L2_PIX_FMT_MV_AITHREC:
                  #if (AIT_ISP_VIDEO_IQ == 1)
                  if (dev->isp_vidiq_ctx_num == ctx->num)
                  {
                      dev->isp_vidiq_en = 0;
                      ait_iq_video_deinit(&(dev->hVidIq));
                  }
                  #endif
                  MMPF_VIDENC_Stop((MMP_ULONG)ctx->codec_handle);
                  flush_workqueue(dev->H264EncTask);
                  #if AITCAM_MULTI_STREAM_EN
                  //if (ctx->output_format == V4L2_PIX_FMT_H264) {
                  NumStreaming--;
                  dev->num_reqbufs--;
                  dev->num_streamon--;
  
                  bCheckCtx[lRecord[ctx->num]] = MMP_FALSE;
                  GraIBCLinkCallBackFunc[lRecord[ctx->num]] = NULL;
                  GraIBCLinkCallBackArgu0[lRecord[ctx->num]] = 0;
                  GraIBCLinkCallBackArgu1[lRecord[ctx->num]] = 0;
  
  
                  if ((NumStreaming != 0) && (lRecord[ctx->num] == lMaxCtx))
                  {
                      for (i = lRecord[ctx->num] - 1; i < lMaxCtx; i--)
                      {
                          if(i >= 0)
                          {
                              if (bCheckCtx[i])
                              {
                                  lMaxCtx = i;
                                  break;
                              }
                          }
                          else
                              break;
                      }
                  }
                  //printk(KERN_ERR ">>NumStreaming_OFF_%d\n", NumStreaming);//cm
  
                  if (dev->num_reqbufs == 0)
                      MMPF_Fctl_EnablePreview(ctx->pipe_link_1.ibcpipeID, MMP_FALSE, 0);
  
                  MMPF_Fctl_EnablePreview(ctx->pipe_link.ibcpipeID, MMP_FALSE, lRecord[ctx->num]);
                  #else
                  MMPF_Fctl_EnablePreview(ctx->pipe_link.ibcpipeID, MMP_FALSE, 0);
                  #endif
                  break;
              case V4L2_PIX_FMT_MJPEG:
                  MMPF_JPEG_StopStream(ctx->codec_handle);
                  MMPF_Fctl_EnablePreview(ctx->pipe_link.ibcpipeID, MMP_FALSE, 2);
                  break;
              case V4L2_PIX_FMT_GREY:
              case V4L2_PIX_FMT_YUV420:
              case V4L2_PIX_FMT_YUYV:
              case V4L2_PIX_FMT_UYVY:
              {
                  struct aitcam_stream_param param;
  
                  param.cmd = AITCAM_STREAM_S_STOP;
                  param.arg = NULL;
                  ait_rawstream_ioctl(ctx, &param);
                  break;
              }
              default:
                  break;
          }
          #endif
        }
        else {
          aitcam_ipc_streamoff(ctx->num);  
        }
        
        #if (SUPPORT_DUAL_SNR_PRV)
        if(dual_snr_en)
        {
            if(ctx->snr_src_id)
            {

                MMPF_RAWPROC_EnablePath(SCD_SENSOR, MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_FALSE, MMP_RAW_IOPATH_VIF2RAW, MMP_RAW_COLORFMT_YUV422);

                GraRAWLinkCallBackFunc[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = NULL;
                GraRAWLinkCallBackArgu0[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = 0;
                GraRAWLinkCallBackArgu1[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = 0;
                GraRAWLinkCallBackArgu2[MMPF_Sensor_GetVIFPad(SCD_SENSOR)] = 0;

                if(dev->num_streaming == 1)
                {
                    MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_S0, MMP_FALSE);
                    MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_S1, MMP_FALSE);
                    MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_F, MMP_FALSE);

                    bRawPathInited = 0;  //
                }
                ctx->snr_src_id = 0;
            }
        }
        #endif

        if (aitcam_mem_release(dev, ctx->num))
        {
            return 1;
        }

        #if (USING_FIXED_MEM_POOL)
        ctx->img_pipe = -1 ;
        aitcam_pipefree(ctx->num);
        #endif

        ctx->inst_state = AITCAM_STAT_FINISH;
        dev->num_streaming--; ///locked by vfd mutex

    }

    return 0;
}

const struct v4l2_ioctl_ops *aitcam_get_ioctl_ops(void)
{
    return &aitcam_ioctl_ops;
}

static const char * const aitcam_force_frame_qmenu[] =
{
    "No force I frame",
    "Force IDR frame with SPS/PPS",
    "Force IDR frame",
    "Force I frame with SPS/PPS",
    "Force I frame",
    NULL,
};

int aitcam_v4l2_ctrl_val(struct aitcam_ctx *ctx, u32 id, s32 *val)
{
    struct v4l2_ctrl *ctrl ;
    int i ;
    for(i = 0; i < NUM_CTRLS; i++)
    {
        ctrl = ctx->ctrls[i] ;
        if(ctrl->id == id)
        {
            *val = ctrl->val ;
            return 0 ;
        }
    }
    return -EINVAL ;
}


int aitcam_v4l2_ctrl_initbyrtos(u32 id)
{
    struct v4l2_ctrl *ctrl ;
    int i ;
    for(i = 0; i < NUM_CTRLS; i++) {
      if (controls[i].id == id ) {
        if( controls[i].ipc_flags & V4L2_IPC_FLAG_RTOS_INIT ) {
          return 1 ;
        }
      }
    }
    return 0 ;

}


#define IS_AITCAM_PRIV(x) (((V4L2_CTRL_ID2CLASS(x) == V4L2_CTRL_CLASS_MPEG) \
                            || (V4L2_CTRL_ID2CLASS(x) == V4L2_CTRL_CLASS_CAMERA)) \
                           && V4L2_CTRL_DRIVER_PRIV(x))
int aitcam_v4l2_ctrls_setup(struct aitcam_ctx *ctx)
{
    struct v4l2_ctrl_config cfg;
    int i;

    v4l2_ctrl_handler_init(&ctx->ctrl_handler, NUM_CTRLS);
    if (ctx->ctrl_handler.error)
    {
        ait_err("v4l2_ctrl_handler_init failed\n");
        return ctx->ctrl_handler.error;
    }
    for (i = 0; i < NUM_CTRLS; i++)
    {
        if (IS_AITCAM_PRIV(controls[i].id))
        {
            /*
            if (controls[i].id == V4L2_CID_CAMERA_AIT_ORIENTATION)
            {
                controls[i].default_value = ctx->dev->orientation;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def ORIENTATION = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_CAMERA_AIT_NIGHT_VISION)
            {
                controls[i].default_value = ctx->dev->nv_mode;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def AIT_NIGHT_VISION = %d\n", controls[i].default_value);
                #endif
            }
            */
            cfg.ops = &aitcam_v4l2_ctrl_ops;
            cfg.id = controls[i].id;
            cfg.min = controls[i].minimum;
            cfg.max = controls[i].maximum;
            cfg.def = controls[i].default_value;
            cfg.name = controls[i].name;
            cfg.type = controls[i].type;
            cfg.flags = 0;

            if (cfg.type == V4L2_CTRL_TYPE_MENU)
            {
                cfg.step = 0;
                cfg.menu_skip_mask = controls[i].menu_skip_mask;
                cfg.qmenu = aitcam_force_frame_qmenu;
            }
            else
            {
                cfg.step = controls[i].step;
                cfg.menu_skip_mask = 0;
            }
            ctx->ctrls[i] = v4l2_ctrl_new_custom(&ctx->ctrl_handler,
                                                 &cfg, NULL);
        }
        else
        {
            /*
            if (controls[i].id == V4L2_CID_POWER_LINE_FREQUENCY)
            {
                switch (ISP_IF_AE_GetFlicker())
                {
                    case ISP_AE_FLICKER_AUTO:
                        controls[i].default_value = V4L2_CID_POWER_LINE_FREQUENCY_AUTO;
                        break;
                    case ISP_AE_FLICKER_OFF:
                        controls[i].default_value = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED;
                        break;
                    case ISP_AE_FLICKER_50HZ:
                        controls[i].default_value = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;
                        break;
                    case ISP_AE_FLICKER_60HZ:
                    default:
                        controls[i].default_value = V4L2_CID_POWER_LINE_FREQUENCY_60HZ;
                        break;
                }
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def POWER_LINE_FREQUENCY = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_SATURATION)
            {
                controls[i].default_value = ctx->dev->saturation;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def SATURATION = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_BRIGHTNESS)
            {
                controls[i].default_value = ctx->dev->brightness;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def BRIGHTNESS = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_CONTRAST)
            {
                controls[i].default_value = ctx->dev->contrast;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def CONTRAST = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_GAMMA)
            {
                controls[i].default_value = ctx->dev->gamma;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def GAMMA = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_SHARPNESS)
            {
                controls[i].default_value = ctx->dev->sharpness;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def SHARPNESS = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_HUE)
            {
                controls[i].default_value = ctx->dev->hue;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def HUE = %d\n", controls[i].default_value);
                #endif
            }
            else if (controls[i].id == V4L2_CID_BACKLIGHT_COMPENSATION)
            {
                controls[i].default_value = ctx->dev->wdr;
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 3)
                    printk(KERN_ERR "Def BACKLIGHT_COMPENSATION = %d\n", controls[i].default_value);
                #endif
            }
            */
            if (controls[i].type == V4L2_CTRL_TYPE_MENU)
            {
                ctx->ctrls[i] = v4l2_ctrl_new_std_menu(
                                    &ctx->ctrl_handler,
                                    &aitcam_v4l2_ctrl_ops, controls[i].id,
                                    controls[i].maximum, controls[i].menu_skip_mask,
                                    controls[i].default_value);
            }
            else
            {
                ctx->ctrls[i] = v4l2_ctrl_new_std(
                                    &ctx->ctrl_handler,
                                    &aitcam_v4l2_ctrl_ops, controls[i].id,
                                    controls[i].minimum,
                                    controls[i].maximum, controls[i].step,
                                    controls[i].default_value);
            }
        }
        if (ctx->ctrl_handler.error)
        {
            ait_err("Adding control (%d) failed\n", i);
            return ctx->ctrl_handler.error;
        }
        if ( ctx->ctrls[i])
        {
          if(controls[i].is_volatile & AITCAM_FLAG_VOLATILE) {
            ctx->ctrls[i]->flags |=  V4L2_CTRL_FLAG_VOLATILE ;   
          } 
          #if V4L2_CTRL_FLAG_EXECUTE_ON_WRITE
          if(controls[i].is_volatile & AITCAM_FLAG_EXE_ON_WR ) {
            ctx->ctrls[i]->flags |=  V4L2_CTRL_FLAG_EXECUTE_ON_WRITE ;
          }
          #endif
        }
    }
    return 0;
}

void aitcam_v4l2_ctrls_delete(struct aitcam_ctx *ctx)
{
    int i;

    v4l2_ctrl_handler_free(&ctx->ctrl_handler);
    for (i = 0; i < NUM_CTRLS; i++)
        ctx->ctrls[i] = NULL;
}

