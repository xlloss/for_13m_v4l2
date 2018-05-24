/**
 @file mmpf_rate_control.h
 @brief Header function of video driver related define
 @author
 @version 0.0
*/

#ifndef _MMPF_RATE_CONTROL_H_
#define _MMPF_RATE_CONTROL_H_

#include "mmpf_vidcmn.h"

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

#if (RATE_CONTROL_EN == 1)

#define WINDOW_SIZE                         (20)
#define MAX_SUPPORT_LAYER                   (MAX_NUM_TMP_LAYERS)
#define MAX_SUPPORT_ENC_NUM                 (MAX_NUM_ENC_SET)
#define PEAK_WINDOW_SIZE                    (30)
#define SUPPORT_PEAK_BR_CONTROL             (0)

typedef struct {
    MMP_ULONG   LayerBitRate[MAX_SUPPORT_LAYER];
    MMP_ULONG   BitRate[MAX_SUPPORT_LAYER];
    MMP_ULONG   VBV_LayerSize[MAX_SUPPORT_LAYER];
    MMP_ULONG   VBV_size[MAX_SUPPORT_LAYER];
    MMP_LONG    VBV_fullness[MAX_SUPPORT_LAYER];
    MMP_ULONG   TargetVBVLevel[MAX_SUPPORT_LAYER];
    MMP_ULONG   TargetVBV[MAX_SUPPORT_LAYER];
    MMP_ULONG   VBVRatio[MAX_SUPPORT_LAYER];
} VBV_PARAM;

typedef struct {
    MMP_LONG    bitrate;
    MMP_LONG    intra_period;
    MMP_LONG    VBV_fullness;
    MMP_LONG    VBV_size;
    MMP_LONG    target_framesize;

    MMP_LONG    bits_budget;
    MMP_LONG    total_frames;
    MMP_LONG    left_frames[3];
    MMP_LONG    Iframe_num;
    MMP_LONG    Pframe_num;
    MMP_LONG    total_framesize[3];
    //MMP_LONG    total_I_size;
    //MMP_LONG    total_P_size;
    //MMP_LONG    total_B_size;
    //MMP_LONG    last_qp;
    //MMP_LONG    lastXP;

    MMP_LONG    last_X[3];
    MMP_LONG    last_X2[3];
    unsigned long long    X[3];
    MMP_LONG    X_Count[3];
    unsigned long long    X2[3][WINDOW_SIZE];
    MMP_LONG    X_Idx[3];
    MMP_LONG    count[3];
    MMP_LONG    frame_count;
    MMP_LONG    target_P;
    MMP_LONG    last_bits;
    //MMP_LONG    last_IQP;
    //MMP_LONG    last_Bqp;
    MMP_LONG    lastQP[3];
    //MMP_LONG    clip_qp;
    #if H264_LOW_BITRATE_CONTROL
    MMP_LONG    LowBrQpBias;
    MMP_ULONG   LowBrQpBiasTimes;
    #endif

    MMP_LONG    prev_window_size[3];
    MMP_LONG    window_size[3];

    //MMP_LONG    AlphaI;
    //MMP_LONG    AlphaB;
    MMP_LONG    Alpha[3];
    //MMP_LONG    avg_xp;
    MMP_LONG    avg_qp[3];

    //MMP_LONG    is_vbr_mode;
    MMP_LONG    rc_mode;
    MMP_LONG    enable_vbr_mode;
    MMP_LONG    GOP_frame_count;
    MMP_LONG    GOP_count;
    MMP_LONG    GOP_totalbits;
    MMP_LONG    QP_sum;
    MMP_LONG    GOP_QP[3];
    MMP_LONG    GOP_left_frames[3];

    MMP_LONG    GOP_num_per_I_period;
    MMP_LONG    GOP_count_inside_I_period;

    MMP_LONG    last_headerbits[3];
    MMP_ULONG   nP;
    MMP_ULONG   nB;

    MMP_ULONG   header_bits[3][WINDOW_SIZE];
    MMP_ULONG   header_index[3];
    MMP_ULONG   header_count[3];
    MMP_ULONG   avg_header[3];

    MMP_ULONG   avgXP[3];
    //test
    MMP_LONG    budget;
    MMP_ULONG   frametype_count[3];
    MMP_ULONG   targetPSize;

    MMP_LONG    vbr_budget;
    MMP_LONG    vbr_total_frames;
    MMP_ULONG   framerate;
    MMP_BOOL    SkipFrame;

    MMP_LONG    GOPLeftFrames;
    MMP_BOOL    bResetRC;
    MMP_ULONG   light_condition;

    //MMP_LONG  TargetLowerBound;
    //MMP_LONG  TargetUpperBound;

    MMP_ULONG   MaxQPDelta[3];
    MMP_ULONG   MaxWeight[3];       //1.5 * 1000
    MMP_ULONG   MinWeight[3];       //1.0 * 1000
    MMP_ULONG   VBV_Delay;          //500 ms
    MMP_ULONG   TargetVBVLevel;     //250 ms
    MMP_ULONG   FrameCount;
    MMP_BOOL    SkipPrevFrame;
    MMP_ULONG   SkipFrameThreshold;

    MMP_ULONG   m_LowerQP[3];
    MMP_ULONG   m_UpperQP[3];
    MMP_ULONG   m_VideoFormat;
    MMP_ULONG   m_ResetCount;
    MMP_ULONG   m_GOP;
    MMP_ULONG   m_Budget;
    MMP_ULONG64 m_AvgHeaderSize;
    MMP_LONG    m_lastQP;
    MMP_UBYTE   m_ubFormatIdx;
    MMP_ULONG   MBWidth;
    MMP_ULONG   MBHeight;
    MMP_ULONG   TargetVBV;
    MMP_BOOL    bPreSkipFrame;

    MMP_ULONG   VBVRatio;
    MMP_ULONG   bUseInitQP;

    MMP_ULONG   m_LastType;

    #if SUPPORT_PEAK_BR_CONTROL
    MMP_ULONG   ulPeakByteCnt;
    MMP_ULONG   ulWatchFrame;
    MMP_ULONG   ulPeakBufFrameCnt;
    MMP_ULONG   ulFrameSize[PEAK_WINDOW_SIZE];
    MMP_UBYTE   ubPeakWIdx;
    MMP_ULONG   ulTotalByte;
    #endif

    //++Will RC
    void*       pGlobalVBV;
    MMP_ULONG   LayerRelated;
    MMP_ULONG   Layer;
    //--Will RC
} RC;

typedef struct {
    MMP_ULONG   MaxIWeight;         //1.5 * 1000
    MMP_ULONG   MinIWeight;         //1.0 * 1000
    MMP_ULONG   MaxBWeight;         //1.0 * 1000
    MMP_ULONG   MinBWeight;         //0.5 * 1000
    MMP_ULONG   VBV_Delay;          //500 ms
    MMP_ULONG   TargetVBVLevel;     //250 ms
    MMP_ULONG   InitWeight[3];
    MMP_ULONG   MaxQPDelta[3];
    MMP_ULONG   SkipFrameThreshold;
    MMP_ULONG   MBWidth;
    MMP_ULONG   MBHeight;
    MMP_ULONG   InitQP[3];
    MMP_ULONG   rc_mode;
    MMP_ULONG   bPreSkipFrame;

    #if SUPPORT_PEAK_BR_CONTROL
    MMP_ULONG   ulPeakBR;
    #endif
    //++Will RC
    MMP_ULONG   LayerRelated;
    MMP_ULONG   Layer;
    MMP_ULONG   EncID;
    //--Will RC
} RC_CONFIG_PARAM;
#endif

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
#if (RATE_CONTROL_EN == 1)
MMP_LONG MMPF_VidRateCtl_Get_VOP_QP(void* RCHandle,MMP_LONG vop_type,MMP_ULONG *target_size, MMP_ULONG *qp_delta, MMP_BOOL *bSkipFrame, MMP_ULONG ulMaxFrameSize);
MMP_ERR MMPF_VidRateCtl_ForceQP(void* RCHandle,MMP_LONG vop_type, MMP_ULONG QP);
MMP_ULONG MMPF_VidRateCtl_UpdateModel(void* handle,MMP_LONG vop_type, MMP_ULONG CurSize,MMP_ULONG HeaderSize,MMP_ULONG last_QP, MMP_BOOL bForceSkip, MMP_BOOL *bSkipFrame,MMP_ULONG *pending_bytes);
void MMPF_VidRateCtl_Init(void* *handle,MMP_ULONG idx,MMP_USHORT gsVidRecdFormat, MMP_LONG targetsize, MMP_LONG framerate, MMP_ULONG nP, MMP_ULONG nB, /*MMP_ULONG InitQP,*/ MMP_BOOL PreventBufOverflow, /*MMP_LONG is_vbr_mode,*/ RC_CONFIG_PARAM     RcConfig);
void MMPF_VidRateCtl_ResetBitrate(void* handle,MMP_LONG bit_rate, MMP_ULONG framerate,MMP_BOOL ResetParams, MMP_ULONG ulVBVSize, MMP_BOOL bResetBufUsage);
void MMPF_VidRateCtl_SetQPBoundary(void* handle,MMP_ULONG frame_type,MMP_LONG QP_LowerBound,MMP_LONG QP_UpperBound);
void MMPF_VidRateCtl_GetQPBoundary(void* handle,MMP_ULONG frame_type,MMP_LONG *QP_LowerBound,MMP_LONG *QP_UpperBound);
void MMPF_VidRateCtl_ResetBufSize(void* RCHandle, MMP_LONG BufSize);
#endif

#endif //_MMPF_RATE_CONTROL_H_
