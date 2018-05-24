//==============================================================================
//
//  File        : mmpf_ibc.h
//  Description : INCLUDE File for the Firmware IBC control
//  Author      : Jerry Lai
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_IBC_H_
#define _MMPF_IBC_H_

#include    "includes_fw.h"
#include    "mmpf_icon.h"

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
#define IBCP_OFFSET 0x100


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_IBC_LINK_TYPE
{
	MMPF_IBC_LINK_NONE 		= 0x00,
	MMPF_IBC_LINK_DISPLAY	= 0x01,
	MMPF_IBC_LINK_VIDEO		= 0x02,
	MMPF_IBC_LINK_ROTATE	= 0x04,
	MMPF_IBC_LINK_FDTC      = 0x08,
	MMPF_IBC_LINK_GRAPHIC   = 0x10,
	MMPF_IBC_LINK_VID_FROM_GRA = 0x20,
	MMPF_IBC_LINK_CALLBACK     = 0x40,
	#if (CHIP == MCR_V2)
	MMPF_IBC_LINK_LDC		= 0x80,
	#endif
	MMPF_IBC_LINK_MASK		= 0xFF
} MMPF_IBC_LINK_TYPE;

typedef enum _MMPF_IBC_COLOR
{
    MMPF_IBC_COLOR_RGB565	   = 0,
    MMPF_IBC_COLOR_YUV422	   = 1,
    MMPF_IBC_COLOR_RGB888	   = 2,
    MMPF_IBC_COLOR_I420		   = 3,
    MMPF_IBC_COLOR_YUV420_LUMA_ONLY = 4,
    MMPF_IBC_COLOR_NV12		   = 5,
    MMPF_IBC_COLOR_NV21		   = 6,
    MMPF_IBC_COLOR_M420_CBCR	        = 7,
    MMPF_IBC_COLOR_M420_CRCB	        = 8,
    MMPF_IBC_COLOR_YUV422_YUYV = 9,
    MMPF_IBC_COLOR_YUV422_UYVY = 10,
    MMPF_IBC_COLOR_YUV422_YVYU = 11,
    MMPF_IBC_COLOR_YUV422_VYUY = 12,
    MMPF_IBC_COLOR_YUV444_2_YUV422_YUYV = 13,
    MMPF_IBC_COLOR_YUV444_2_YUV422_YVYU = 14,
    MMPF_IBC_COLOR_YUV444_2_YUV422_UYVY = 15,
    MMPF_IBC_COLOR_YUV444_2_YUV422_VYUY = 16,
    MMPF_IBC_COLOR_Y_ONLY            = 17,
    MMPF_IBC_COLOR_MAX_NUM    
} MMPF_IBC_COLOR;

typedef enum _MMPF_IBC_FX
{
    MMPF_IBC_FX_TOFB = 0,
    MMPF_IBC_FX_JPG,
    MMPF_IBC_FX_RING_BUF,
    MMPF_IBC_FX_H264
} MMPF_IBC_FX;

typedef enum _MMPF_IBC_PIPEID
{
    MMPF_IBC_PIPE_0 = 0,
    MMPF_IBC_PIPE_1,
    MMPF_IBC_PIPE_2,
    MMPF_IBC_PIPE_3,
    MMPF_IBC_PIPE_4,
    MMPF_IBC_PIPE_MAX = 8
} MMPF_IBC_PIPEID;

typedef enum _MMPF_IBC_EVENT
{
    MMPF_IBC_EVENT_FRM_ST	  = 0,
    MMPF_IBC_EVENT_FRM_RDY	  = 1,
    MMPF_IBC_EVENT_FRM_END	  = 2,
    MMPF_IBC_EVENT_FRM_PRERDY = 3,
    MMPF_IBC_EVENT_MAX		  = 4
} MMPF_IBC_EVENT;

typedef struct _MMPF_IBC_PIPEATTRIBUTE
{
    MMP_ULONG           ulBaseAddr;
    MMP_ULONG           ulBaseUAddr;
    MMP_ULONG           ulBaseVAddr;
    MMPF_IBC_COLOR      colorformat;
    MMPF_IBC_FX         function;
    MMP_ULONG           ulBaseEndAddr;  // only ring buf need config
    MMP_ULONG           ulLineOffset;
    MMP_ULONG           ulCbrLineOffset;
    MMP_BOOL            bMirrorEnable;
    MMP_USHORT          usMirrorWidth;
    MMPF_ICO_PIPEID     InputSource;
    MMP_USHORT          usPipeCnf;
} MMPF_IBC_PIPEATTRIBUTE;

typedef void IbcCallBackFunc(void);
typedef void IbcLinkCallBackFunc(MMPF_IBC_PIPEID IbcPipe);
typedef void LoopBackCallBackFunc(MMP_USHORT usWidth, MMP_USHORT usHeight);

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if (CHIP == MERCURY)
MMP_ERR MMPF_IBC_SetP3Attributes(MMPF_IBC_PIPEATTRIBUTE *pipeAttr);
MMP_ERR MMPF_IBC_SetMCI_UrgentMode(MMPF_IBC_PIPEID pipeID, MMP_BOOL ubEn, MMP_USHORT ubThd);
#endif
#if (CHIP == MERCURY) || (CHIP == MCR_V2)
MMP_ERR MMPF_IBC_SetH264RT_Enable(MMP_BOOL ubEn);
MMP_ERR MMPF_IBC_SetRtPingPongAddr(MMPF_IBC_PIPEID pipeID, 
                                   MMP_ULONG ulYAddr,  MMP_ULONG ulUAddr, MMP_ULONG ulVAddr,
                                   MMP_ULONG ulY1Addr, MMP_ULONG ulU1Addr, MMP_ULONG ulV1Addr, MMP_USHORT usFrmW);
MMP_ERR MMPF_IBC_SetMCI_ByteCount(MMPF_IBC_PIPEID pipeID, MMP_USHORT ubByteCntSel);
#endif

MMP_ERR MMPF_IBC_SetColorFmt(MMPF_IBC_PIPEID pipeID, MMPF_IBC_COLOR fmt);
MMP_ERR MMPF_IBC_InitModule(MMPF_IBC_PIPEID IbcPipe);
MMP_ERR MMPF_IBC_StopPreview(MMP_USHORT usIBCPipe);
MMP_ERR MMPF_IBC_SetAttributes(MMPF_IBC_PIPEID pipeID, MMPF_IBC_PIPEATTRIBUTE *pipeAttr);
MMP_ERR MMPF_IBC_GetAttributes(MMPF_IBC_PIPEID pipeID, MMPF_IBC_PIPEATTRIBUTE *pipeAttr);
MMP_ERR MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable);
MMP_ERR MMPF_IBC_SetSingleFrmEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable);
MMP_ERR MMPF_IBC_SetStoreBuffer(MMPF_IBC_PIPEID pipeID, MMP_ULONG ulY, MMP_ULONG ulCbr, MMP_ULONG ulCr);
MMP_ERR MMPF_IBC_SetPreFrameRdy(MMPF_IBC_PIPEID pipeID, MMP_USHORT usLineNum, MMP_BOOL bEnable);
MMP_ERR MMPF_IBC_SetMirrorEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable, MMP_USHORT usWidth);
MMP_ERR MMPF_IBC_SetInterruptEnable(MMPF_IBC_PIPEID pipeID, MMPF_IBC_EVENT event, MMP_BOOL bEnable);
MMP_ULONG MMPF_IBC_GetRingBufWrPtr(MMPF_IBC_PIPEID pipeID);
MMP_ERR MMPF_IBC_RegisterEventAction(MMPF_IBC_PIPEID ibcpipe, MMPF_IBC_EVENT event,
                                     MMPF_OS_EVENT_ACTION *Act);
MMP_ERR MMPF_IBC_UnRegisterEventAction(MMPF_IBC_PIPEID ibcpipe, MMPF_IBC_EVENT event);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

#endif //_MMPF_IBC_H_
