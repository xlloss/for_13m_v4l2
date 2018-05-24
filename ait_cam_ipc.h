#ifndef AIT_CAM_IPC_H_
#define AIT_CAM_IPC_H_

#include "ait_cam_common.h"
#include "ait_cam_v4l2.h"



typedef struct
{
	unsigned int cmd_type;
	unsigned int data_length;
	unsigned int sysmode;
	unsigned int page;
	unsigned int type;
	unsigned int index;
	unsigned int buf_ptr_size;
	void* buf_ptr;
} EZ_MODE_IQ_DATA_BUF;

#define EZ_IQ_HEADER_SIZE  (sizeof( EZ_MODE_IQ_DATA_BUF ) - sizeof(int) )


typedef enum
{
    IPC_V4L2_OPEN,
    IPC_V4L2_RELEASE,
    IPC_V4L2_STREAMON,
    IPC_V4L2_STREAMOFF,
    IPC_V4L2_CTRL,
    IPC_V4L2_ISR,
    IPC_IQ_TUNING,
    IPC_V4L2_GET_JPG,
    IPC_V4L2_NOT_SUPPORT
    
} aitcam_ipc_cmd ;

#define STORAGE_STREAM  (0)
#define REALTIME_STREAM (1)

#define CTRL_FLAG_OSD_EN  (1<<0)
typedef packed_struct _aitcam_img_info {
  int streamid ;
  unsigned int ctrl_flag ;
  packed_union {
  int img_width ;
  int bitrate ;
  } ;
  packed_union {
  int img_height;
  int samplerate;
  } ;
  unsigned long img_fmt ;  
  unsigned long max_framesize ;  
  int streamtype;
} aitcam_img_info ;
 
typedef packed_struct _aitcam_ipc_info {
  aitcam_img_info img_info ;
  unsigned long vbq_virt_addr ;
  unsigned long vbq_phy_addr  ; 
  int remote_q_size ; 
} aitcam_ipc_info ;

typedef packed_struct _aitcam_ipc_ctrl {
  int streamid ;
  unsigned long id ;
  int val ;  
  char name[32] ;
} aitcam_ipc_ctrl ;

typedef enum 
{
  IPC_V4L2_SET = 0 ,
  IPC_V4L2_GET
} aitcm_ipc_ctrl_dir ;

int aitcam_ipc_open(aitcam_img_info *img_info);
int aitcam_ipc_streamon ( aitcam_ipc_info *ipc_info) ;
int aitcam_ipc_streamoff( int streamid ) ;
int aitcam_ipc_release(int streamid,void *) ;
int aitcam_ipc_set_ctrl(aitcam_ipc_ctrl *ctrl,aitcm_ipc_ctrl_dir dir);
int aitcam_ipc_register(int debug_en) ;
int aitcam_ipc_unregister(void) ;
int aitcam_ipc_alloc_ctrl(struct aitcam_ctx *ctx);
int aitcam_ipc_free_ctrl(struct aitcam_ctx *ctx);
struct ait_fdframe_config_ipc *aitcam_ipc_osd_base(void);
// called by uvc-camera, need this name
ISP_CMD_STATUS ISP_IF_APTUNING_Para(EZ_MODE_IQ_DATA_BUF *iq);
int aitcam_ipc_get_jpg(struct ait_vidbuf_info *jpg_info);
#endif