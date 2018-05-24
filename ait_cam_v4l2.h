
#ifndef _AIT_V4L2_H_
#define _AIT_V4L2_H_

#include <linux/videodev2.h>

#define AIT_IOC_MAGIC           0xF7

#define MAX_OSD_INDEX           (10)
#define MAX_OSD_STR_LEN         (32)

struct ait_mem_io {
    unsigned long phys_addr;
    int size;
    char data[16];
};

struct ait_driver_info {
    unsigned short IspYear;
    unsigned short IspMonth;
    unsigned short IspDay;
};

struct ait_rtc_config {
    unsigned short  usYear;
    unsigned short  usMonth;
    unsigned short  usDay;
    unsigned short  usHour;
    unsigned short  usMinute;
    unsigned short  usSecond;
    char            bOn;
};

struct ait_rtc_config_ipc {
    unsigned long  usYear;
    unsigned long  usMonth;
    unsigned long  usDay;
    unsigned long  usHour;
    unsigned long  usMinute;
    unsigned long  usSecond;
    char            bOn;
};
enum ait_osd_type {
    AIT_OSD_TYPE_INACTIVATE = 0,
    AIT_OSD_TYPE_RTC,
    AIT_OSD_TYPE_CUST_STRING,
    AIT_OSD_TYPE_WATERMARK,
    AIT_OSD_TYPE_FDFRAME
};

enum ait_osd_axis {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_MAX
};

struct ait_osd_config {
    unsigned char           index;
    enum ait_osd_type       type;

    unsigned short          pos[AXIS_MAX];
    unsigned short          TextColorY;
    unsigned short          TextColorU;
    unsigned short          TextColorV;

    char                    str[MAX_OSD_STR_LEN];
};

struct ait_osd_config_ipc {
    unsigned long           index;
    unsigned long           type;

    unsigned long          pos[AXIS_MAX];
    unsigned long          TextColorY;
    unsigned long          TextColorU;
    unsigned long          TextColorV;

    char                    str[MAX_OSD_STR_LEN];
};

struct ait_osd_info {
    unsigned short  font_h_pix;
    unsigned short  font_v_pix;
    unsigned short  datetime_h_pix;
    unsigned short  datetime_v_pix;
};

struct ait_vidbuf_info {
    unsigned long   buf_addr;
    unsigned long   buf_size;
    unsigned long   buf_num;
};

struct ait_fdframe_config {
	unsigned char           index;
	enum ait_osd_type       type;

	unsigned short          pos[AXIS_MAX];
	unsigned short          width;
	unsigned short          height;
	unsigned short          TextColorY;
	unsigned short          TextColorU;
	unsigned short          TextColorV;

  char                    str[MAX_OSD_STR_LEN];
};
// align for ipc usage
struct ait_fdframe_config_ipc {
    unsigned long   index;
    unsigned long   type;
    unsigned long   pos[AXIS_MAX];
    unsigned long   width;
    unsigned long   height;
    unsigned long   TextColorY;
    unsigned long   TextColorU;
    unsigned long   TextColorV;
    char            str[MAX_OSD_STR_LEN];
};

struct ait_sensor_config {
    unsigned char           snr_src_id;
};

#define AIT_IOCGOPR             _IOWR(AIT_IOC_MAGIC, 0, struct ait_mem_io)
#define AIT_IOCSOPR             _IOW(AIT_IOC_MAGIC, 1, struct ait_mem_io)
#define AIT_IOCGISPOPR          _IOWR(AIT_IOC_MAGIC, 2, struct ait_mem_io)
#define AIT_IOCSISPOPR          _IOW(AIT_IOC_MAGIC, 3, struct ait_mem_io)
#define AIT_IOCGDRVINFO         _IOR(AIT_IOC_MAGIC, 4, struct ait_driver_info)
#define AIT_IOCS_RTC            _IOW(AIT_IOC_MAGIC, 5, struct ait_rtc_config)
#define AIT_IOCS_OSD_CFG        _IOW(AIT_IOC_MAGIC, 6, struct ait_osd_config)
#define AIT_IOCG_OSD_INFO       _IOR(AIT_IOC_MAGIC, 7, struct ait_osd_info)
#define AIT_IOCGSNROPR          _IOWR(AIT_IOC_MAGIC, 8, struct ait_mem_io)
#define AIT_IOCSSNROPR          _IOW(AIT_IOC_MAGIC, 9, struct ait_mem_io)
#define AIT_IOCG_VIDBUF_INFO       _IOR(AIT_IOC_MAGIC, 10, struct ait_vidbuf_info)
#define AIT_IOCS_FDFRAME_CFG        _IOW(AIT_IOC_MAGIC, 11, struct ait_fdframe_config)
#define AIT_IOCS_SNR_CFG        _IOW(AIT_IOC_MAGIC, 12, struct ait_sensor_config)
#define AIT_IOCG_IMAGE_DATA     _IOWR(AIT_IOC_MAGIC, 13, struct ait_vidbuf_info)
#define AIT_IOC_MAXNR           (13)

/*  MPEG-class control IDs specific to the AIT-CAM driver as defined by V4L2 */
#define V4L2_CID_MPEG_AIT_BASE                  (V4L2_CTRL_CLASS_MPEG | 0x1800)

#define V4L2_CID_MPEG_AIT_VIDEO_FORCE_FRAME     (V4L2_CID_MPEG_AIT_BASE+0)
enum v4l2_mpeg_ait_video_force_frame_mode{
    V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_NONE        = 0,
    V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_IDR_RESYNC  = 1,
    V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_IDR         = 2,
    V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_I_RESYNC    = 3,
    V4L2_MPEG_AIT_VIDEO_FORCE_FRAME_I           = 4
};

#define V4L2_CID_MPEG_AIT_VIDEO_FPS_RESOL       (V4L2_CID_MPEG_AIT_BASE+1)
#define V4L2_CID_MPEG_AIT_VIDEO_FPS_INCR        (V4L2_CID_MPEG_AIT_BASE+2)
#define V4L2_CID_MPEG_AIT_VIDEO_RC_FRAME_SKIP   (V4L2_CID_MPEG_AIT_BASE+3)
#define V4L2_CID_MPEG_AIT_GOP_LENGTH            (V4L2_CID_MPEG_AIT_BASE+4) // gop length in 100ms
// TNR
#define V4L2_CID_MPEG_AIT_TNR                   (V4L2_CID_MPEG_AIT_BASE+5) // en tnr
enum v4l2_h264_tnr_mode {
    V4L2_H264_TNR_LOW_MV_EN     = 1 ,
    V4L2_H264_TNR_ZERO_MV_EN    = 2 ,
    V4L2_H264_TNR_HIGH_MV_EN    = 4   
} ;
#define V4L2_CID_MPEG_AIT_TNR_DUMP              (V4L2_CID_MPEG_AIT_BASE+6) // dump tnr opr
#define V4L2_CID_MPEG_AIT_TNR_LOW_MV_THR        (V4L2_CID_MPEG_AIT_BASE+7) // low motion mv thr
#define V4L2_CID_MPEG_AIT_TNR_ZERO_MV_THR       (V4L2_CID_MPEG_AIT_BASE+8) // zero motion mv thr
#define V4L2_CID_MPEG_AIT_TNR_ZERO_LUMA_PXL_DIFF_THR    (V4L2_CID_MPEG_AIT_BASE+9) 
#define V4L2_CID_MPEG_AIT_TNR_ZERO_CHROMA_PXL_DIFF_THR  (V4L2_CID_MPEG_AIT_BASE+10) 
#define V4L2_CID_MPEG_AIT_TNR_ZERO_MV_4x4_CNT_THR       (V4L2_CID_MPEG_AIT_BASE+11)
/*
mv_filter : 4 bytes definition
mv_filter[0] = luma_4x4 
mv_filter[1] = chroma_4x4
mv_filter[2~3] = 8x8 thr (mv_filter[2] | mv_filter[3] << 8)
*/
#define V4L2_CID_MPEG_AIT_TNR_LOW_MV_FILTER             (V4L2_CID_MPEG_AIT_BASE+12)
#define V4L2_CID_MPEG_AIT_TNR_ZERO_MV_FILTER             (V4L2_CID_MPEG_AIT_BASE+13)
#define V4L2_CID_MPEG_AIT_TNR_HIGH_MV_FILTER             (V4L2_CID_MPEG_AIT_BASE+14)
/*
H264 RDO enable/disable
*/
#define V4L2_CID_MPEG_AIT_RDO                   (V4L2_CID_MPEG_AIT_BASE+15)
/*
complicate scene <---> smooth scene
qstep = { 0 ,1,2,3,4,5,6,7,8,9 } ;
each 0~9 use nibble byte ( 4 bits)
complicate scene :increase qstep
smooth scene : decrease qstep

increase qstep : ex: 1,2,3,4...
decrease qstep : ex: (16-1):-1,(16-2):-2,(16-3):-3....

*/
#define V4L2_CID_MPEG_AIT_QSTEP3_P1             (V4L2_CID_MPEG_AIT_BASE+16)        
#define V4L2_CID_MPEG_AIT_QSTEP3_P2             (V4L2_CID_MPEG_AIT_BASE+17)

/*  Camera class control IDs specific to the AIT-CAM driver as defined by V4L2 */
#define V4L2_CID_CAMERA_AIT_BASE                (V4L2_CTRL_CLASS_CAMERA | 0x1800)
#define V4L2_CID_CAMERA_AIT_ORIENTATION         (V4L2_CID_CAMERA_AIT_BASE+0)
enum v4l2_camera_ait_orientation {
    V4L2_CAMERA_AIT_ORTN_NORMAL = 0,
    V4L2_CAMERA_AIT_ORTN_FLIP,
    V4L2_CAMERA_AIT_ORTN_MIRROR,
    V4L2_CAMERA_AIT_ORTN_FLIP_MIRROR
};
#define V4L2_CID_CAMERA_AIT_EXPOSURE_STATE      (V4L2_CID_CAMERA_AIT_BASE+1)
#define V4L2_CID_CAMERA_AIT_MANUAL_R_GAIN       (V4L2_CID_CAMERA_AIT_BASE+2)
#define V4L2_CID_CAMERA_AIT_MANUAL_G_GAIN       (V4L2_CID_CAMERA_AIT_BASE+3)
#define V4L2_CID_CAMERA_AIT_MANUAL_B_GAIN       (V4L2_CID_CAMERA_AIT_BASE+4)
#define V4L2_CID_CAMERA_AIT_NIGHT_VISION        (V4L2_CID_CAMERA_AIT_BASE+5) // off/on/auto
#define V4L2_CID_CAMERA_AIT_IR_LED              (V4L2_CID_CAMERA_AIT_BASE+6) // off/on
#define V4L2_CID_CAMERA_AIT_IR_SHUTTER          (V4L2_CID_CAMERA_AIT_BASE+7) // off/on
// Get 
#define V4L2_CID_CAMERA_AIT_LIGHT_MODE_STATE    (V4L2_CID_CAMERA_AIT_BASE+8) // day/night
#define V4L2_CID_CAMERA_AIT_LIGHT_CONDITION     (V4L2_CID_CAMERA_AIT_BASE+9) // 32bits light value for advance control

enum v4l2_camera_ait_night_vision {
    V4L2_CAMERA_AIT_NV_OFF = 0 ,
    V4L2_CAMERA_AIT_NV_ON  = 1 ,
    V4L2_CAMERA_AIT_NV_AUTO= 2   
} ;

#define V4L2_CID_CAMERA_AIT_IMAGE_EFFECT      (V4L2_CID_CAMERA_AIT_BASE+10)  // ISP_IMAGE_EFFECT
enum v4l2_camera_ait_image_effect {
    V4L2_CAMERA_AIT_EFFECT_NORMAL = 0,
    V4L2_CAMERA_AIT_EFFECT_GREY = 1,
    V4L2_CAMERA_AIT_EFFECT_SEPIA = 2,
    V4L2_CAMERA_AIT_EFFECT_NEGATIVE = 3,
    V4L2_CAMERA_AIT_EFFECT_RED = 4,
    V4L2_CAMERA_AIT_EFFECT_GREEN = 5,
    V4L2_CAMERA_AIT_EFFECT_BLUE = 6,
    V4L2_CAMERA_AIT_EFFECT_YELLOW = 7,
    V4L2_CAMERA_AIT_EFFECT_BW = 8,
};

#define V4L2_CID_CAMERA_AIT_NR_GAIN     (V4L2_CID_CAMERA_AIT_BASE+11) // 0 ~ 0x0A(low lux)

enum v4l2_camera_ait_stream_type {
  V4L2_CAMERA_AIT_STORAGE_TYPE  =0 << 0,
  V4L2_CAMERA_AIT_REALTIME_TYPE =1 << 0,
  V4L2_CAMERA_AIT_LOOP_RECORDING=1 << 1,
} ;
#define V4L2_CID_CAMERA_AIT_STREAM_TYPE (V4L2_CID_CAMERA_AIT_BASE+12) 

enum v4l2_camera_ait_streamer_alive {
  V4L2_CAMERA_AIT_STREAMER_DOWN  =0 ,
  V4L2_CAMERA_AIT_STREAMER_UP    =1 ,
} ;
#define V4L2_CID_CAMERA_AIT_STREAMER_ALIVE (V4L2_CID_CAMERA_AIT_BASE+19) 
#define V4L2_CID_CAMERA_AIT_GET_TEMPERATURE (V4L2_CID_CAMERA_AIT_BASE+20) 


#define V4L2_PIX_FMT_MV_AITHREC                 v4l2_fourcc('A', 'M', 'V', '0')

#endif // _AIT_V4L2_H_

