//==============================================================================
//
//  File        : mmpf_jpeg.h
//  Description :
//  Author      :
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_JPEG_H_
#define _MMPF_JPEG_H_

#include "mmpf_fpsctl.h"
#include "mmpf_rate_control.h"


//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

#define JPEG_FPS_CTL                 (1)

#define INIT_JPEG_QTABL              (1)
#if (CHIP == VSN_V3) || (CHIP == MERCURY) || (CHIP == MCR_V2)
#define INIT_JPEG_QFACTOR_0          (128)
#define INIT_JPEG_QFACTOR_1          (128)
#endif

#define BD_LOW                       (0)
#define BD_HIGH                      (1)
#define JPEG_MAX_MB_QFACTOR          (63)
#define JPEG_MIN_MB_QFACTOR          (2)

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

#if AITCAM_MULTI_STREAM_EN
#define MAX_NUM_JPEG                (5)
#else
#define MAX_NUM_JPEG                (3)
#endif

#define JPEGENC_OP_NONE             (0)
#define JPEGENC_OP_START            (1)
#define JPEGENC_OP_STOP             (2)

#define MMPF_JPEGENC_STATUS_NONE    (0)
#define MMPF_JPEGENC_STATUS_START   (1)
#define MMPF_JPEGENC_STATUS_STOP    (2)


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_JPEG_PARAM {
    MMPF_JPEG_PARAM_RESOLUTION = 0,
    MMPF_JPEG_PARAM_SRC_PIPE_ID,
    MMPF_JPEG_PARAM_DST_PIPE_ID,
    MMPF_JPEG_PARAM_QFACTOR,
    MMPF_JPEG_PARAM_QTABLE,
    MMPF_JPEG_PARAM_MAX_FPS,
    MMPF_JPEG_PARAM_RC_MODE,
    MMPF_JPEG_PARAM_RC_BITRATE,
    MMPF_JPEG_PARAM_RC_SKIPPABLE
} MMPF_JPEG_PARAM;

typedef enum _MMPF_JPEG_CACHE_FLG {
    MMPF_JPEG_CACHE_FLG_NONE        = 0,
    MMPF_JPEG_CACHE_FLG_QFACTOR     = (1 << 0),
    MMPF_JPEG_CACHE_FLG_QTABLE      = (1 << 1),
    MMPF_JPEG_CACHE_FLG_MAX_FPS     = (1 << 2),
    MMPF_JPEG_CACHE_FLG_RC_BITRATE  = (1 << 3),
    MMPF_JPEG_CACHE_FLG_RC_MODE     = (1 << 4)
} MMPF_JPEG_CACHE_FLG;

typedef enum _MMPF_JPEG_LINEBUF_MODE {
    MMPF_JPEG_LINEBUF_SRAM = 0,
    MMPF_JPEG_LINEBUF_DRAM,
    MMPF_JPEG_LINEBUF_INTLV,
    MMPF_JPEG_LINEBUF_MAX
} MMPF_JPEG_LINEBUF_MODE;

typedef struct _MMPF_JPEG_CTL_RESOL {
    MMP_USHORT  usWidth;
    MMP_USHORT  usHeight;
} MMPF_JPEG_CTL_RESOL;

typedef struct _MMPF_JPEG_CTL_QFACTOR {
    MMP_USHORT  usQFactors[2];
} MMPF_JPEG_CTL_QFACTOR;

struct _MMPF_JPEG_MODULE;
typedef struct _MMPF_JPEG_INSTANCE {
    MMP_UBYTE   eid;
    MMP_UBYTE   ubSrcId;
    MMP_UBYTE   ubDstId;
    MMP_BOOL    bInitialized;   ///< 1: resource(mem,sem) allocated
    MMP_USHORT  usWidth;
    MMP_USHORT  usHeight;
    MMP_USHORT  usQFactor[2];
    MMP_UBYTE   *QTable;
    MMP_ULONG   MinQTMem;
    MMP_ULONG   QMultiple;
    //MMP_UBYTE   QTableIdx;
    MMP_ULONG   ulBsAddrLow;
    MMP_ULONG   ulBsAddrHigh;
    MMP_ULONG   ulFrameCount;
    MMP_BOOL    rc_skippable;   ///< false, rc should not skip ant frames
    MMP_ULONG   stream_bitrate; ///< total bitrate
    MMP_ULONG   lb_size;        ///< leakybucket size in ms


    #if (JPEG_FPS_CTL == 1)
    MMPF_FPS_INFO   FPSMax;
    MMPF_FPS_INFO   FPSSrc;
    MMPF_FPS_INFO   FPSOut;
    #endif

    MMP_USHORT  JpgCtlOpr;

    MMP_USHORT  Operation;
    MMP_USHORT  Status;
    MMPF_OS_EVENT_ACTION    FrameDoneAct;
    MMPF_OS_EVENT_ACTION    FrameSkipAct;

    RC_CONFIG_PARAM         rc_config;
    void                    *jpeg_rc_hdl;      ///< rc handler

    MMP_LONG                MbQfactorBound[2]; ///<for I only
    MMP_LONG                CurRcQFactor;      ///<for I only

    struct {
        MMPF_OS_SEMID       Lock;
        MMPF_JPEG_CACHE_FLG flags;

        MMP_USHORT          usQFactor[2];
        MMP_UBYTE           *QTable;
        MMP_ULONG           stream_bitrate;
        #if (JPEG_FPS_CTL == 1)
        MMPF_FPS_INFO       FPSMax;
        #endif
        MMP_ULONG           rc_mode;
    } cache;

    void    *priv_data;
} MMPF_JPEG_INSTANCE;

typedef struct _MMPF_JPEG_MODULE {
    ///< indication for current h/w configuration
    MMP_BOOL            checkid[MAX_NUM_JPEG];
    MMPF_JPEG_INSTANCE  *Inst;
    MMP_UBYTE           *QTable;

    ///< share for every instance
    MMPF_OS_SEMID       Lock;
    MMP_ULONG           ulLockTimeout;
    MMP_USHORT          usResvMaxWidth;
    MMP_USHORT          usResvMaxHeight;
    MMPF_JPEG_LINEBUF_MODE  LinebufMode;
    MMP_ULONG           ulLineBufAddr[2];
    MMP_BOOL            bInitialized;       ///< 1: resource(mem,sem) allocated
} MMPF_JPEG_MODULE;

typedef struct _MMPF_JPEG_MODULE_CONFIG {
    MMPF_JPEG_LINEBUF_MODE  LinebufAllocMode;
    MMP_USHORT              usReservedMaxWidth;
    MMP_USHORT              usReservedMaxHeight;
    MMP_ULONG               SramAddr;
    MMP_ULONG               DramAddr;
} MMPF_JPEG_MODULE_CONFIG;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void        MMPF_JPEG_ISR(void);
MMP_ERR     MMPF_JPEG_InitModule(MMPF_JPEG_MODULE_CONFIG *Config);
MMP_ERR     MMPF_JPEG_DeinitModule(void);
MMP_BOOL    MMPF_JPEG_IsModuleInit(void);
MMP_ERR     MMPF_JPEG_InitInstance(MMPF_JPEG_INSTANCE *JpegInst, void *priv);
MMP_ERR     MMPF_JPEG_DeinitInstance(MMPF_JPEG_INSTANCE *JpegInst);
MMP_ERR     MMPF_JPEG_InitRCConfig (MMPF_JPEG_INSTANCE *JpegInst);
MMP_ERR     MMPF_JPEG_SetQfactorBound(MMPF_JPEG_INSTANCE *JpegInst, MMPF_3GPMGR_FRAME_TYPE type, MMP_LONG lMinQfactor, MMP_LONG lMaxQfactor);
MMP_ERR     MMPF_JPEG_SetParameter(MMPF_JPEG_INSTANCE *JpegInst, MMPF_JPEG_PARAM ParamId, void *Arg);
MMP_ERR     MMPF_JPEG_EncodeFrame(MMPF_JPEG_INSTANCE *JpegInst, MMPF_OS_LOCK_CTX LockCtx);
MMP_ERR     MMPF_JPEG_StartStream(MMPF_JPEG_INSTANCE *JpegInst);
MMP_ERR     MMPF_JPEG_StopStream(MMPF_JPEG_INSTANCE *JpegInst);
//MMPF_JPEG_INSTANCE * MMPF_JPEG_GetInstance (MMP_UBYTE InstId);

#endif // _MMPF_JPEG_H_
