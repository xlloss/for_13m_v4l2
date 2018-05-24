//==============================================================================
//
//  File        : mmpf_dma.h
//  Description : INCLUDE File for the Firmware Graphic Driver (DMA portion).
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_DMA_H_
#define _MMPF_DMA_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmpf_graphics.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#if (CHIP == VSN_V2)
#define DMA_M_NUM 1
#define DMA_R_NUM 1
#endif
#if (CHIP == VSN_V3) || (CHIP == MERCURY)
#define DMA_M_NUM 2
#define DMA_R_NUM 1
#endif
#if (CHIP == P_V2) || (CHIP == MCR_V2)
#define DMA_M_NUM 2
#define DMA_R_NUM 2
#endif

#if (CHIP == P_V2) || (CHIP == VSN_V2) || (CHIP == VSN_V3) || (CHIP == MERCURY)
#define DMA_OFFSET 0x10
#endif
#if (CHIP == MCR_V2)
#define DMA_OFFSET 0x50
#endif
#define DMA_SEM_WAITTIME 0xF0000000

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPF_DMA_M_ID
{
    MMPF_DMA_M_0 = 0,
    MMPF_DMA_M_1,
    MMPF_DMA_M_MAX
} MMPF_DMA_M_ID;

typedef enum _MMPF_DMA_R_ID
{ 
    MMPF_DMA_R_0 = 0,
    #if (CHIP == P_V2) || (CHIP == MCR_V2)
    MMPF_DMA_R_1,
    #endif
    MMPF_DMA_R_MAX
} MMPF_DMA_R_ID;

typedef enum _MMPF_DMA_R_BPP
{
    MMPF_DMA_R_BPP8 = 0,
    MMPF_DMA_R_BPP16,
    MMPF_DMA_R_BPP24,
    MMPF_DMA_R_BPP32,
    MMPF_DMA_R_BPP_MAX
} MMPF_DMA_R_BPP;

typedef enum _MMPF_DMA_R_BLOCK
{
    #if (CHIP == P_V2) || (CHIP == VSN_V2) || (CHIP == VSN_V3)
    MMPF_DMA_R_BLOCK8 = 0,
    MMPF_DMA_R_BLOCK16
    #endif
    #if (CHIP == MCR_V2)
    MMPF_DMA_R_BLOCK64 = 0,
    MMPF_DMA_R_BLOCK128
    #endif
} MMPF_DMA_R_BLOCK;

typedef enum _MMPF_DMA_R_TYPE
{
    MMPF_DMA_R_NO = 0,
    MMPF_DMA_R_90,
    MMPF_DMA_R_180,
    MMPF_DMA_R_270
} MMPF_DMA_R_TYPE;

typedef enum _MMPF_DMA_R_MIRROR_TYPE
{
    MMPF_DMA_R_MIRROR_H = 0,
    MMPF_DMA_R_MIRROR_V,
    MMPF_DMA_R_NO_MIRROR
} MMPF_DMA_R_MIRROR_TYPE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPF_DMA_ROT_DATA{
	MMP_SHORT               BufferNum;
	MMP_SHORT               BufferIndex;
    MMP_SHORT               SrcWidth[3];
    MMP_SHORT               SrcHeight[3];
    MMP_ULONG               SrcAddr[3];
    MMP_ULONG               DstAddr[3];
    MMP_USHORT              SrcLineOffset[3];
    MMP_USHORT              DstLineOffset[3];
    MMP_SHORT               BytePerPixel[3];
    MMPF_DMA_R_TYPE         RotateType;
    MMPF_GRAPHICS_COLORDEPTH ColorDepth;
    MMP_BOOL                MirrorEnable;
    MMPF_DMA_R_MIRROR_TYPE  MirrorType;
} MMPF_DMA_ROT_DATA;

typedef struct _MMPF_DMA_M_LOFFS_DATA{
	MMP_ULONG SrcWidth;
	MMP_ULONG SrcOffset;
	MMP_ULONG DstWidth;
	MMP_ULONG DstOffset;
} MMPF_DMA_M_LOFFS_DATA;

typedef void DmaCallBackFunc(void *argu);

typedef struct _MMPF_DMA_M_2NDROUND{
	MMP_ULONG              ulSrcaddr;
	MMP_ULONG              ulDstaddr;
	MMP_ULONG              ulCount;
	MMPF_DMA_M_LOFFS_DATA* ptrLineOffset;
	DmaCallBackFunc        *CallBackFunc;
    void                   *Argu;
}MMPF_DMA_M_2NDROUND;
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPF_DMA_Initialize(void);
MMP_ERR MMPF_DMA_MoveData(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr, 
                          MMP_ULONG ulCount, DmaCallBackFunc *CallBackFunc, void *callBackArgument, 
                          MMP_BOOL bEnableLOFFS, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset, MMPF_OS_LOCK_CTX LockCtx);
MMP_ERR MMPF_DMA_RotateImageBuftoBuf(MMPF_GRAPHICS_BUFATTRIBUTE *srcBufAttr, 
                                    MMPF_GRAPHICS_RECT          *srcrect, 
                                    MMPF_GRAPHICS_BUFATTRIBUTE  *dstBufAttr, 
                                    MMP_USHORT                  usDststartx,
                                    MMP_USHORT                  usDststarty,
                                    MMPF_GRAPHICS_ROTATE_TYPE   rotatetype,
                                    DmaCallBackFunc             *CallBackFunc,
                                    void                        *CallBackArgu,
                                    MMP_BOOL                    mirrorEnable,
                                    MMPF_DMA_R_MIRROR_TYPE      mirrortype,
                                    MMPF_OS_LOCK_CTX            LockCtx);
MMP_ERR MMPF_DMA_WaitAllFree(void);
MMP_ERR MMPF_DMA_SetAllFree(void);

#endif // _MMPF_DMA_H_
