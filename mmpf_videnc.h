/// @ait_only
//==============================================================================
//
//  File        : mmpf_mp4venc.h
//  Description : Header function of video codec
//  Author      : Will Tseng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_VIDENC_H_
#define _MMPF_VIDENC_H_

#include "ait_cam_common.h"
#include "mmpf_vidcmn.h"
#include "mmpf_h264enc.h"
#include "mmpf_vidmgr.h"
#include "mmpf_graphics.h"
#include "mmpf_rawproc.h"

/** @addtogroup MMPF_VIDENC
 *  @{
 */

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#if (VID_TIME_SYNC_ST == 1)
#define VID_TIME_TO_PTS(ms, ns)				(ms*10000+ns/100)	// 100ns/unit
#endif

//// audio format (only used in recoder)
//#define	MMPF_AUD_FORMAT_OTHERS		        0x00
//#define	MMPF_AUD_FORMAT_AMR			        0x01
//#define	MMPF_AUD_FORMAT_MP4A		        0x02

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_VIDENC_MODULE_ID {
    MMPF_VIDENC_MODULE_H264 = 0,
    #if (CHIP == P_V2)
    MMPF_VIDENC_MODULE_MP4V,
    #endif
    MMPF_VIDENC_MODULE_MAX
} MMPF_VIDENC_MODULE_ID;

typedef struct _MMPF_VIDENC_MODULE {
    MMP_BOOL                bInit;
    MMPF_VIDENC_FORMAT      Format;

    union {
        MMPF_H264ENC_MODULE H264EMod;
    };
} MMPF_VIDENC_MODULE;

typedef struct _MMPF_VIDNEC_INSTANCE {
    MMP_BOOL                bInit;
    MMPF_VIDENC_MODULE      *Module;

    union {
        MMPF_H264ENC_ENC_INFO h264e;
    };
} MMPF_VIDENC_INSTANCE;

typedef union _MMPF_VIDENC_MODULE_CONFIG {
    MMPF_H264ENC_MODULE_CONFIG      H264ModCfg;
#if AITCAM_MULTI_STREAM_EN
    MMPF_GRAPHICS_MODULE_CONFIG     GraModCfg;
#endif
#if (SUPPORT_DUAL_SNR_PRV)
    MMPF_RAWPROC_MODULE_CONFIG     RawModCfg;
#endif
} MMPF_VIDENC_MODULE_CONFIG;

typedef union _MMPF_VIDENC_INSTANCE_CONFIG {
    MMPF_H264ENC_INSTANCE_CONFIG    H264InstCfg;
} MMPF_VIDENC_INSTANCE_CONFIG;


//==============================================================================
//
//                              VARIABLES
//
//==============================================================================



//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#define MAX(a,b)                    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)                    (((a) < (b)) ? (a) : (b))
#define CLIP(a,i,s)                 (((a) > (s)) ? (s) : MAX(a,i))

#define get_vidinst_format(_p)      ((_p)->Module->Format)
#define get_vidinst_id(_p)          ((MMPF_VIDENC_INSTANCE*)(_p) - MMPF_VIDENC_GetInstance(0))

MMPF_VIDENC_MODULE * MMPF_VIDENC_GetModule(MMPF_VIDENC_MODULE_ID ModId);
MMPF_VIDENC_INSTANCE * MMPF_VIDENC_GetInstance (MMP_UBYTE InstId);

MMP_ERR     MMPF_VIDENC_InitModule(MMPF_VIDENC_MODULE_ID ModId, MMPF_VIDENC_MODULE_CONFIG *ModuleConfig);
MMP_ERR     MMPF_VIDENC_DeinitModule(MMPF_VIDENC_MODULE_ID ModId);
MMP_BOOL    MMPF_VIDENC_IsModuleInit(MMPF_VIDENC_MODULE_ID ModId);

MMP_ERR     MMPF_VIDENC_InitInstance(MMP_ULONG *InstId, MMPF_VIDENC_INSTANCE_CONFIG *InstConfig, MMPF_VIDENC_MODULE_ID ModuleId);
MMP_ERR     MMPF_VIDENC_DeinitInstance(MMP_ULONG InstId);

MMP_ERR     MMPF_VIDENC_PushQueue(MMPF_VIDENC_QUEUE *queue, MMP_UBYTE buffer);
MMP_UBYTE   MMPF_VIDENC_PopQueue(MMPF_VIDENC_QUEUE *queue, MMP_UBYTE offset);
MMP_ERR     MMPF_VIDENC_InitCodedDataDesc(MMPF_VIDMGR_CODED_DESC *pDesc,
                                        MMP_UBYTE   (*XhdrBuf)[MAX_XHDR_SIZE],
                                        MMP_ULONG   *XhdrSize,
                                        MMP_ULONG   *DataBuf,
                                        MMP_ULONG   *DataSize);
MMPF_3GPMGR_FRAME_TYPE MMPF_VIDENC_GetFrameType(MMP_ULONG ulEncFrameNum,
                                                MMP_USHORT gop_size, MMP_USHORT b_frame_num);
MMP_ERR MMPF_VIDENC_SetFrameReady(MMP_ULONG InstId);
MMP_ERR MMPF_VIDENC_SetParameter (MMP_UBYTE ubEncId, MMPF_VIDENC_ATTRIBUTE attrib, void *arg);
MMP_ERR MMPF_VIDENC_GetParameter(MMP_UBYTE ubEncId, MMPF_VIDENC_ATTRIBUTE attrib, void *arg);
MMP_ERR MMPF_VIDENC_TriggerFrameDone(MMP_UBYTE ubEncID, MMP_UBYTE *pbCurBuf, MMP_UBYTE *pbIBCBuf, MMP_UBYTE ubBufCount,
                                    MMP_ULONG ulCurYBufAddr[], MMP_ULONG ulCurUBufAddr[], MMP_ULONG ulCurVBufAddr[]);

// functions of video encoder
MMP_USHORT  MMPF_VIDENC_GetStatus(MMP_UBYTE ubEncId);
MMP_ERR     MMPF_VIDENC_Start(MMP_UBYTE ubEncId);
MMP_ERR     MMPF_VIDENC_Stop(MMP_UBYTE ubEncId);
void        MMPF_VIDENC_Abort(MMP_UBYTE ubEncId);

#endif	// _MMPF_MP4VENC_H_
/// @}
/// @end_ait_only
