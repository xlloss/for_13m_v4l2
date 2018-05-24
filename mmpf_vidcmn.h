/**
 @file mmpf_vidcmn.h
 @brief Header function of video driver related define
 @author
 @version 0.0
*/

#ifndef _MMPF_VIDCMN_H_
#define _MMPF_VIDCMN_H_

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
#define TEST_SIMULCAST          (0)

#define MGR_DBG_MSG             (0)

#define MAX_NUM_CONT_B_FRAME    (2)

#define RC_JPEG_TARGET_SIZE     (400 * 1024)

#define RATE_CONTROL_EN         (1)
#define FPS_CTL                 (1)

#define ENCODER_ID_MASK         (0x0F)
#define TEMPORAL_ID_MASK        (0x07)

#if (CHIP == VSN_V2) || (CHIP == VSN_V3) || (CHIP == MERCURY) || (CHIP == MCR_V2)
#define QP_SUM_PATCH            (0)
#endif
#if (CHIP == P_V2)
#define QP_SUM_PATCH            (1)
#endif

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#undef H264_LOW_BITRATE_CONTROL
#define H264_LOW_BITRATE_CONTROL        (1)

#define MMPF_VIDENC_MAX_QUEUE_SIZE          (16)

#define MMPF_MP4VENC_FORMAT_OTHERS      (MMPF_VIDENC_FORMAT_OTHERS)
#define MMPF_MP4VENC_FORMAT_H263        (MMPF_VIDENC_FORMAT_H263)
#define MMPF_MP4VENC_FORMAT_MP4V        (MMPF_VIDENC_FORMAT_MP4V)
#define MMPF_MP4VENC_FORMAT_H264        (MMPF_VIDENC_FORMAT_H264)
#define MMPF_MP4VENC_FORMAT_MJPEG       (MMPF_VIDENC_FORMAT_MJPEG)
#define MMPF_MP4VENC_FORMAT_YUV422      (MMPF_VIDENC_FORMAT_YUV422)
#define MMPF_MP4VENC_FORMAT_YUV420      (MMPF_VIDENC_FORMAT_YUV420)

// video operation status
#define MMPF_VIDENC_FW_STATUS_RECORD        0x0000  ///< status of video encoder
#define MMPF_VIDENC_FW_STATUS_START         0x0001  ///< status of START
#define MMPF_VIDENC_FW_STATUS_PAUSE         0x0002  ///< status of PAUSE
#define MMPF_VIDENC_FW_STATUS_RESUME        0x0003  ///< status of RESUME
#define MMPF_VIDENC_FW_STATUS_STOP          0x0004  ///< status of STOP
#define MMPF_VIDENC_FW_STATUS_ERROR         0x0005  ///< Error

#define VIDEO_RECORD_NONE                   0x00
#define VIDEO_RECORD_START                  0x01
#define VIDEO_RECORD_PAUSE                  0x02
#define VIDEO_RECORD_RESUME                 0x03
#define VIDEO_RECORD_STOP                   0x04

#define I_FRAME                             (0)
#define P_FRAME                             (1)
#define B_FRAME                             (2)

#if AITCAM_MULTI_STREAM_EN
#define MAX_NUM_ENC_SET                     (5)
#else
#define MAX_NUM_ENC_SET                     (4) //(3)
#endif
#define MAX_NUM_TMP_LAYERS                  (3)
#define MAX_NUM_TMP_LAYERS_LOW_BR           (2) //H264_LOW_BITRATE_CONTROL
#define MAX_NUM_PARAM_CTL                   (16)

#define RC_MIN_VBV_FRM_NUM                  ((RC_MAX_WEIGHT_I+2000)/1000)
#define RC_PSEUDO_GOP_SIZE                  (1000)
#if H264_LOW_BITRATE_CONTROL
#define RC_MAX_WEIGHT_I                     (2000)//(6000)// GGYY (3000)//(3500)              ///< 3000, 5000
#define RC_INIT_WEIGHT_I                    (1000)//(4000)// GGYY (2500)              ///< 2000, 4000
#else
#define RC_MAX_WEIGHT_I                     (3500)              ///< 3000, 5000
#define RC_INIT_WEIGHT_I                    (2500)              ///< 2000, 4000
#endif

#define FRAME_NUM_HIGH_BOUND                (0xFFFFFFFF)
#define FRAME_CODED_MAX_SIZE                (0x01000000)

#define FREE_SPACE_TO_RC_TH_ADJ             (1)
#define FREE_SPACE_TO_RC_TH_M_SHIFT         (1)


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef void VidEncEndCallBackFunc(void *);

typedef enum _MMPF_VIDENC_ATTRIBUTE {
    MMPF_VIDENC_ATTRIBUTE_PROFILE = 0,
    MMPF_VIDENC_ATTRIBUTE_LEVEL,
    MMPF_VIDENC_ATTRIBUTE_ENTROPY_MODE,
    MMPF_VIDENC_ATTRIBUTE_LAYERS,
    MMPF_VIDENC_ATTRIBUTE_PADDING_INFO,
    MMPF_VIDENC_ATTRIBUTE_RC_MODE,
    MMPF_VIDENC_ATTRIBUTE_RC_SKIPPABLE,
    MMPF_VIDENC_ATTRIBUTE_FRM_QP,
    MMPF_VIDENC_ATTRIBUTE_FRM_QP_BOUND,
    MMPF_VIDENC_ATTRIBUTE_BR,
    MMPF_VIDENC_ATTRIBUTE_LB_SIZE,
    MMPF_VIDENC_ATTRIBUTE_PIC_MAX_WEIGHT,
    MMPF_VIDENC_ATTRIBUTE_CROPPING,
    MMPF_VIDENC_ATTRIBUTE_GOP_CTL,
    MMPF_VIDENC_ATTRIBUTE_FORCE_I,
    MMPF_VIDENC_ATTRIBUTE_VIDEO_FULL_RANGE,
    MMPF_VIDENC_ATTRIBUTE_MAX_FPS,
    MMPF_VIDENC_ATTRIBUTE_SLICE_CTL,
    MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE,
    MMPF_VIDENC_ATTRIBUTE_CURBUF_ADDR,
    MMPF_VIDENC_ATTRIBUTE_SWITCH_CURBUF_MODE,
    MMPF_VIDENC_ATTRIBUTE_RTFCTL_MODE,
    MMPF_VIDENC_ATTRIBUTE_RINGBUF_EN,
    MMPF_VIDENC_ATTRIBUTE_POC_TYPE,
    MMPF_VIDENC_ATTRIBUTE_REG_CALLBACK_ENC_START,
    MMPF_VIDENC_ATTRIBUTE_REG_CALLBACK_ENC_RESTART,
    MMPF_VIDENC_ATTRIBUTE_REG_CALLBACK_ENC_END,
    MMPF_VIDENC_ATTRIBUTE_RESOLUTION,
    MMPF_VIDENC_ATTRIBUTE_PARSET_EVERY_FRM,
    MMPF_VIDENC_ATTRIBUTE_PRIORITY_ID,
	MMPF_VIDENC_ATTRIBUTE_SEI_CTL,
    MMPF_VIDENC_ATTRIBUTE_PNALU_MODE,
    MMPF_VIDENC_ATTRIBUTE_ME_ITR_MAX_STEPS,
    
    MMPF_VIDENC_ATTRIBUTE_TNR_EN,
    MMPF_VIDENC_ATTRIBUTE_TNR_LOW_MV_THR,
    MMPF_VIDENC_ATTRIBUTE_TNR_ZERO_MV_THR,
    MMPF_VIDENC_ATTRIBUTE_TNR_ZERO_LUMA_PXL_DIFF_THR,
    MMPF_VIDENC_ATTRIBUTE_TNR_ZERO_CHROMA_PXL_DIFF_THR,
    MMPF_VIDENC_ATTRIBUTE_TNR_ZERO_MV_4x4_CNT_THR,
    MMPF_VIDENC_ATTRIBUTE_TNR_LOW_MV_FILTER,
    MMPF_VIDENC_ATTRIBUTE_TNR_ZERO_MV_FILTER,
    MMPF_VIDENC_ATTRIBUTE_TNR_HIGH_MV_FILTER,

    // RDO control
    MMPF_VIDENC_ATTRIBUTE_RDO_EN,
    MMPF_VIDENC_ATTRIBUTE_RDO_QSTEP3_P1,
    MMPF_VIDENC_ATTRIBUTE_RDO_QSTEP3_P2,
    MMPF_VIDENC_ATTRIBUTE_MEMD_PARAM
} MMPF_VIDENC_ATTRIBUTE;

/// Video frame type
typedef enum _MMPF_3GPMGR_FRAME_TYPE {
    MMPF_3GPMGR_FRAME_TYPE_I = 0,
    MMPF_3GPMGR_FRAME_TYPE_P,
    MMPF_3GPMGR_FRAME_TYPE_B,
    MMPF_3GPMGR_FRAME_TYPE_MAX
} MMPF_3GPMGR_FRAME_TYPE;

typedef enum _MMPF_VIDENC_SYNCFRAME_TYPE {
    MMPF_VIDENC_SYNCFRAME_IDR = 0,
    MMPF_VIDENC_SYNCFRAME_I,
    MMPF_VIDENC_SYNCFRAME_GDR,
    MMPF_VIDENC_SYNCFRAME_LT_IDR,
    MMPF_VIDENC_SYNCFRAME_LT_I,
    MMPF_VIDENC_SYNCFRAME_LT_P,
    MMPF_VIDENC_SYNCFRAME_MAX
} MMPF_VIDENC_SYNCFRAME_TYPE;

typedef enum _MMPF_VIDENC_PROTECTION_MODE {
    MMPF_VIDENC_PROTECTION_TASK_MODE = 0,
    MMPF_VIDENC_PROTECTION_ISR_MODE
} MMPF_VIDENC_PROTECTION_MODE;

typedef enum _MMPF_VIDENC_FORMAT {
    MMPF_VIDENC_FORMAT_OTHERS = 0,
    MMPF_VIDENC_FORMAT_H263,
    MMPF_VIDENC_FORMAT_MP4V,
    MMPF_VIDENC_FORMAT_H264,
    MMPF_VIDENC_FORMAT_MJPEG,
    MMPF_VIDENC_FORMAT_YUV422,
    MMPF_VIDENC_FORMAT_YUV420
} MMPF_VIDENC_FORMAT;

typedef enum _MMPF_VIDENC_COLORMODE {
    MMPF_VIDENC_COLORMODE_NV12 = 0,
    MMPF_VIDENC_COLORMODE_I420,
    MMPF_VIDENC_COLORMODE_MAX
} MMPF_VIDENC_COLORMODE;

typedef enum _MMPF_VIDENC_CURBUF_MODE {
    MMPF_VIDENC_CURBUF_FRAME = 0,
    MMPF_VIDENC_CURBUF_RT,
    MMPF_VIDENC_CURBUF_MAX
} MMPF_VIDENC_CURBUF_MODE;

typedef enum _MMPF_VIDENC_RTFCTL_MODE {
    MMPF_VIDENC_RTFCTL_PASSIVE = 0,
    MMPF_VIDENC_RTFCTL_ACTIVE,
    MMPF_VIDENC_RTFCTL_MAX
} MMPF_VIDENC_RTFCTL_MODE;

typedef enum _MMPF_VIDENC_BUFCFG {
    MMPF_VIDENC_BUFCFG_SRAM = 0,
    MMPF_VIDENC_BUFCFG_DRAM,
    MMPF_VIDENC_BUFCFG_INTLV,
    MMPF_VIDENC_BUFCFG_NONE
} MMPF_VIDENC_BUFCFG;

typedef enum _MMPF_VIDENC_REFGENBUF_MODE {
    MMPF_VIDENC_REFGENBUF_INDEPENDENT = 0,
    MMPF_VIDENC_REFGENBUF_OVERWRITE,
    MMPF_VIDENC_REFGENBUF_ROTATE,
    MMPF_VIDENC_REFGENBUF_NONE,
    MMPF_VIDENC_REFGENBUF_MAX
} MMPF_VIDENC_REFGENBUF_MODE;

typedef enum _MMPF_VIDENC_OUTPUT_SYNC_MODE {
    MMPF_VIDENC_OUTPUT_FRAME_SYNC = 0,
    MMPF_VIDENC_OUTPUT_SLICE_SYNC
} MMPF_VIDENC_OUTPUT_SYNC_MODE;

typedef enum _MMPF_VIDENC_RC_MODE {
    MMPF_VIDENC_RC_MODE_CBR = 0,
    MMPF_VIDENC_RC_MODE_VBR = 1,
    MMPF_VIDENC_RC_MODE_CQP = 2,
    MMPF_VIDENC_RC_MODE_LOWBR = 3,
    MMPF_VIDENC_RC_MODE_MAX = 4
} MMPF_VIDENC_RC_MODE;

typedef enum _MMPF_VIDENC_PICCTL {
    MMPF_VIDENC_PICCTL_NONE,
    MMPF_VIDENC_PICCTL_IDR_RESYNC,
    MMPF_VIDENC_PICCTL_IDR,
    MMPF_VIDENC_PICCTL_I_RESYNC,
    MMPF_VIDENC_PICCTL_I,
    MMPF_VIDENC_PICCTL_MAX
} MMPF_VIDENC_PICCTL;

typedef enum _MMPF_VIDENC_BUF_TYPE {
    MMPF_VIDENC_BUF_TYPE_SINGLE,
    MMPF_VIDENC_BUF_TYPE_RING
} MMPF_VIDENC_BUF_TYPE;

typedef enum _MMPF_VIDENC_SLICE_MODE {
    MMPF_VIDENC_SLICE_MODE_FRM,
    MMPF_VIDENC_SLICE_MODE_MB,
    MMPF_VIDENC_SLICE_MODE_BYTE,
    MMPF_VIDENC_SLICE_MODE_ROW
} MMPF_VIDENC_SLICE_MODE;

typedef enum _MMPF_VIDENC_FILTER_MODE {
    MMPF_VIDENC_FILTER_ENCODER = 0,
    MMPF_VIDENC_FILTER_ME_REF_RECONSTRUCT,
    MMPF_VIDENC_FILTER_ME_REF_LOSSLESS
} MMPF_VIDENC_FILTER_MODE;

typedef struct _MMPF_VIDENC_LINE_BUF {
    MMP_ULONG ulY[12];
    MMP_ULONG ulU[12];
    MMP_ULONG ulV[12];
    MMP_ULONG ulUP[4];
    MMP_ULONG ulDeblockRow;
} MMPF_VIDENC_LINE_BUF;

typedef struct _MMPF_VIDENC_RESOLUTION {
    MMP_USHORT  usWidth;
    MMP_USHORT  usHeight;
} MMPF_VIDENC_RESOLUTION;

typedef struct _MMPF_VIDENC_CURBUF_MODE_CTL {
    MMPF_VIDENC_CURBUF_MODE CurBufMode;
    MMP_UBYTE               ubRTSrcPipe;
} MMPF_VIDENC_CURBUF_MODE_CTL;

typedef struct _MMPF_VIDENC_RC_MODE_CTL {
    MMPF_VIDENC_RC_MODE RcMode;
    MMP_BOOL            bLayerGlobalRc;
} MMPF_VIDENC_RC_MODE_CTL;

typedef struct _MMPF_VIDENC_GOP_CTL {
    MMP_USHORT  usGopSize;
    MMP_USHORT  usMaxContBFrameNum;
    MMPF_VIDENC_SYNCFRAME_TYPE SyncFrameType;
    MMP_BOOL    bReset;
} MMPF_VIDENC_GOP_CTL;

typedef struct _MMPF_VIDENC_CROPPING {
    MMP_USHORT  usTop;
    MMP_USHORT  usBottom;
    MMP_USHORT  usLeft;
    MMP_USHORT  usRight;
} MMPF_VIDENC_CROPPING;

/*
changed to packed for Linux Action Cam
*/

typedef packed_struct _MMPF_VIDENC_QUEUE {
    MMP_ULONG   buffers[MMPF_VIDENC_MAX_QUEUE_SIZE];  ///< queue for buffer ready to encode, in display order
    MMP_UBYTE   head;
    MMP_UBYTE   size;
} MMPF_VIDENC_QUEUE;

typedef struct _MMPF_VIDENC_FRAME {
    MMP_ULONG   ulYAddr;
    MMP_ULONG   ulUAddr;
    MMP_ULONG   ulVAddr;
} MMPF_VIDENC_FRAME;

typedef struct _MMPF_VIDENC_FRAMEBUF_BD {
    MMPF_VIDENC_FRAME   LowBound;
    MMPF_VIDENC_FRAME   HighBound;
} MMPF_VIDENC_FRAMEBUF_BD;

typedef struct _MMPF_VIDENC_BITRATE_CTL {   ///< bitrate param control
    MMP_UBYTE ubLayerBitMap;                ///< 0'b111 means all temporal layers
    MMP_ULONG ulBitrate[MAX_NUM_TMP_LAYERS];///< bitrate, bits
} MMPF_VIDENC_BITRATE_CTL;

typedef struct _MMPF_VIDENC_LEAKYBUCKET_CTL { ///< leacky bucket param control
    MMP_UBYTE ubLayerBitMap;                ///< 0'b111 means all temporal layers
    MMP_ULONG ulLeakyBucket[MAX_NUM_TMP_LAYERS];///< in ms
} MMPF_VIDENC_LEAKYBUCKET_CTL;

typedef struct _MMPF_VIDENC_QP_CTL {        ///< QP control, for initail QP and CQP
    MMP_UBYTE ubTID;                        ///< 0'b111 means all temporal layers
    MMP_UBYTE ubTypeBitMap;                 ///< 0: I, 1: P, 2: B
    MMP_UBYTE ubQP[3];
    MMP_LONG  CbrQpIdxOffset[3];            ///< Chroma QP index offset
    MMP_LONG  CrQpIdxOffset[3];             ///< 2nd chroma QP index offset
} MMPF_VIDENC_QP_CTL;

typedef struct _MMPF_VIDENC_QP_BOUND_CTL {  ///< QP Boundary
    MMP_UBYTE ubLayerID;                    ///< 0'b111 means all temporal layers
    MMP_UBYTE ubTypeBitMap;                 ///< 0: I, 1: P, 2: B
    MMP_UBYTE ubQPBound[3][2];
} MMPF_VIDENC_QP_BOUND_CTL;

typedef struct _MMPF_VIDENC_PIC_WEIGHT_CTL {///< RC picture weighting control
    MMP_UBYTE ubLayerID;                    ///< layer id
    MMP_UBYTE ubTypeBitMap;                 ///< 0: I, 1: P, 2: B
    MMP_ULONG ulIWeight;                    ///< P weight awalys 1000
    MMP_ULONG ulBWeight;
} MMPF_VIDENC_PIC_WEIGHT_CTL;

typedef struct _MMPF_VIDNC_MAX_FPS_CTL {
    MMP_ULONG   ulMaxFpsResolution;
    MMP_ULONG   ulMaxFpsIncreament;
} MMPF_VIDENC_MAX_FPS_CTL;

typedef struct _MMPF_VIDENC_SLICE_CTL {
    MMPF_VIDENC_SLICE_MODE  SliceMode;
    MMP_ULONG               ulSliceSize;
} MMPF_VIDENC_SLICE_CTL;

typedef struct _MMPF_VIDENC_PRIORITY_ID_CTL {
    MMP_USHORT  LayerId;
    MMP_USHORT  PriorityId;
} MMPF_VIDENC_PRIORITY_ID_CTL;

typedef struct _MMPF_VIDENC_PARAM_CTL {
    MMPF_VIDENC_ATTRIBUTE Attrib;
    void (*CallBack)(MMP_ERR);
    union {
        MMPF_VIDENC_RC_MODE_CTL     RcMode;
        MMPF_VIDENC_BITRATE_CTL     Bitrate;
        MMPF_VIDENC_LEAKYBUCKET_CTL CpbSize;
        MMPF_VIDENC_QP_CTL          Qp;
        MMPF_VIDENC_GOP_CTL         Gop;
    } Ctl;
} MMPF_VIDENC_PARAM_CTL;


#endif //_MMPF_VIDCMN_H_
