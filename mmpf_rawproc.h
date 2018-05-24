//==============================================================================
//
//  File        : mmpf_rawproc.h
//  Description : INCLUDE File for the Firmware Rawproc Control driver function
//  Author      : Ted
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_RAWPROC_H_
#define _MMPF_RAWPROC_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmp_scal_inc.h"
#include "mmp_rawproc_inc.h"
#include "mmpf_vidcmn.h"
#include "mmpf_ibc.h"

//#define AIT_OPR_PHYS_BASE   (0x80000000)
//#define AIT_OPR_VIRT_BASE   (0xF0000000)
//#define AIT_OPR_P2V(x)      (((x)- AIT_OPR_PHYS_BASE+AIT_OPR_VIRT_BASE))
#define AITC_BASE_HDR       ((AITPS_HDR)                AIT_OPR_P2V(0x800024D0)) // HDR      Base Address


#define AITC_BASE_RAW_OFST_TBL      ((AIT_REG_B*)           	0x80013400) // Raw Offset Table  Base Address
#define ALIGN256(_a)    (((_a) + 255) >> 8 << 8)


typedef struct _MMPF_RAWPROC_MODULE_CONFIG {
    MMP_ULONG               DramAddr;
    MMP_ULONG               SramAddr;
    MMPF_VIDENC_FRAME       *CurFrameList;
    MMP_USHORT              CurBufNum;
    MMP_USHORT              MaxReservedWidth;
    MMP_USHORT              MaxReservedHeight;
    
} MMPF_RAWPROC_MODULE_CONFIG;

typedef void Raw2GraLoopBackCallBackFunc(MMP_UBYTE ubRawId, 
	                                                                              MMPF_IBC_PIPEID DstPipe, 
	                                                                              MMP_ULONG SrcAddr, 
	                                                                              MMP_USHORT usWidth, 
	                                                                              MMP_USHORT usHeight);

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPF_RAWPROC_ResetModule(MMP_UBYTE ubRawIdx);
MMP_ERR MMPF_RAWPROC_RegisterIntrCallBack(MMP_UBYTE 		ubRawIdx, 
										  MMP_UBYTE 		ubPlane, 
										  MMP_RAW_EVENT 	event, 
										  RawCallBackFunc 	*pCallBack,
										  void              *pArgument);
MMP_ERR MMPF_RAWPROC_EnableInterrupt(MMP_UBYTE ubRawIdx, MMP_UBYTE ubPlane,
									 MMP_UBYTE ubFlag, MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_OpenInterrupt(MMP_BOOL bEnable);

MMP_RAW_FETCH_ATTR* MMPF_RAWPROC_GetFetchAttr(void);
MMP_UBYTE MMPF_RAWPROC_CalcFetchPixelDelay(void);
MMP_ERR MMPF_RAWPROC_SetFetchPixelDelay(MMP_UBYTE ubPixelDelay);
MMP_ERR MMPF_RAWPROC_SetFetchLineDelay(MMP_USHORT usLineDelay);
MMP_ERR MMPF_RAWPROC_SetFetchRange (MMP_USHORT  usLeft,
                                    MMP_USHORT  usTop,
                                    MMP_USHORT  usWidth,
                                    MMP_USHORT  usHeight,
                                    MMP_USHORT  usLineOffset);
MMP_BOOL MMPF_RAWPROC_IsFetchBusy(void);
MMP_ERR MMPF_RAWPROC_EnableStorePath(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_RAW_STORE_SRC ubRawSrc, MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_EnableFetchPath(MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_UpdateStoreAddr(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane);
MMP_ERR MMPF_RAWPROC_FetchData (MMP_UBYTE                   ubSnrSel,
                                MMP_UBYTE                   ubRawId,
                                MMP_RAW_FETCH_ROTATE_TYPE  	RotateType,
                                MMP_RAW_FETCH_MIRROR_TYPE  	mirrorType,
                                MMP_BOOL                    bDynamicFetch);
MMP_ERR MMPF_RAWPROC_FetchRotatedData(	MMP_UBYTE   ubSnrSel,
                                		MMP_USHORT	usOffsetX,
                               	 		MMP_USHORT	usOffsetY,
                               	 		MMP_ULONG	ulFetchAddr);
MMP_ERR MMPF_RAWPROC_WaitFetchDone(void);
MMP_ERR MMPF_RAWPROC_ClearStoreDone(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane);
MMP_ERR MMPF_RAWPROC_WaitStoreDone(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane);
MMP_ERR MMPF_RAWPROC_GetStoreRange(MMP_UBYTE ubRawId, MMP_USHORT* pusW, MMP_USHORT* pusH);
MMP_ERR MMPF_RAWPROC_GetGrabRange(MMP_UBYTE ubRawId, MMP_USHORT* pusW, MMP_USHORT* pusH);

// YUV Function
MMP_ERR MMPF_RAWPROC_SetInterruptForYUV420Fmt(MMP_UBYTE ubRawId);
MMP_ERR MMPF_RAWPROC_SetInterruptForYUV422Fmt(MMP_UBYTE ubRawId);
MMP_ERR MMPF_RAWPROC_SetDeInterlaceBuf(MMP_ULONG ulYAddr, MMP_ULONG ulUAddr, MMP_ULONG ulVAddr);
MMP_ERR MMPF_RAWPROC_TriggerGraToPipe(MMP_UBYTE ubRawId);
MMP_ERR MMPF_RAWPROC_DmaDeInterlace(MMP_UBYTE ubRawId);

// HDR Function
MMP_ERR MMPF_HDR_InitModule(MMP_USHORT usSubW, MMP_USHORT usSubH);
MMP_ERR MMPF_HDR_UnInitModule(MMP_UBYTE ubSnrSel);
MMP_ERR MMPF_HDR_InitRawStoreSetting(MMP_UBYTE ubSnrSel);
MMP_ERR MMPF_HDR_UpdateStoreFrameAddr(MMP_UBYTE ubVcChannel, MMP_UBYTE ubFrameIdx, MMP_USHORT usFrameW, MMP_USHORT usFrameH);
MMP_ERR MMPF_HDR_UpdateFetchFrameAddr(void);
MMP_ERR MMPF_HDR_Preview(MMP_USHORT usFrameW, MMP_USHORT usFrameH);
MMP_ERR MMPF_HDR_SetBufEnd(MMP_ULONG ulEndAddr);

// Buffer Function
void MMPF_RAWPROC_ResetBufIndex(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane);
void MMPF_RAWPROC_SetFetchBufIndex(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_ULONG ulIndex);
void MMPF_RAWPROC_UpdateStoreBufIndex(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_ULONG ulOffset);
void MMPF_RAWPROC_UpdateFetchBufIndex(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_ULONG ulOffset);

MMP_ERR MMPF_RAWPROC_SetCaptureStoreAddr(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane);
MMP_ERR MMPF_RAWPROC_GetCaptureStoreAddr(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_ULONG *ulAddr);
MMP_ERR MMPF_RAWPROC_SetRawStoreBuffer(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane,
									   MMP_UBYTE ubBufIdx, MMP_ULONG ulAddr);
MMP_ERR MMPF_RAWPROC_SetRawStoreBufferEnd(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane,
										  MMP_UBYTE ubBufIdx, MMP_ULONG ulAddr);
MMP_ULONG MMPF_RAWPROC_GetCurRawStoreBuf(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane);
MMP_ERR MMPF_RAWPROC_InitStoreBuffer(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_ULONG ulAddr);
MMP_ERR MMPF_RAWPROC_CalcBufSize(MMP_RAW_COLORFMT colorFormat, MMP_USHORT usWidth, MMP_USHORT usHeight, MMP_ULONG* ulSize);

// Property Function
MMP_ERR MMPF_RAWPROC_CalcRawFetchTiming(MMP_SCAL_PIPEID ubScalerPath);
MMP_ERR MMPF_RAWPROC_EnableRingStore(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_BOOL bRingBufEn);
MMP_ERR MMPF_RAWPROC_EnableDownsample(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, 
								      MMP_BOOL bEnable,  MMP_UBYTE ubRatio);
MMP_ERR MMPF_RAWPROC_SetRawStoreGrabRange(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, 
										  MMP_BOOL  bEnable,
										  MMP_USHORT usXst, MMP_USHORT usYst, 
										  MMP_USHORT usW, MMP_USHORT usH);
MMP_ERR MMPF_RAWPROC_SetStoreLineOffset(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, 
									    MMP_BOOL bEnable, MMP_ULONG ulOffset);
MMP_ERR MMPF_RAWPROC_SetStoreVCSelect(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, 
									  MMP_BOOL bEnable, MMP_UBYTE ubSrc);
MMP_ERR MMPF_RAWPROC_SetStoreByteCount(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_UBYTE ubByteCntSel);
MMP_ERR MMPF_RAWPROC_SetFetchByteCount(MMP_UBYTE ubByteCntSel);
MMP_ERR MMPF_RAWPROC_SetStoreUrgentMode(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_BOOL bEnable, MMP_USHORT usThd);
MMP_ERR MMPF_RAWPROC_SetFetchUrgentMode(MMP_BOOL bEnable, MMP_USHORT usThd);
MMP_ERR MMPF_RAWPROC_EnableFetchBusyMode(MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_SetFrontFetchTiming(MMP_USHORT usFrontLn, MMP_USHORT usFrontTiming, MMP_USHORT usExtraTiming);
MMP_ERR MMPF_RAWPROC_EnableDPCM(MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_SetRollingOffset(MMP_UBYTE ubBankIdx, MMP_UBYTE* ubTable);
MMP_ERR MMPF_RAWPROC_EnableRollingOfstComp(MMP_BOOL bEnable, MMP_BOOL bInitZero, MMP_USHORT usStartLine);
MMP_ERR MMPF_RAWPROC_SetStoreBitMode(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, 
									 MMP_RAW_COLORFMT colorFormat);
MMP_ERR MMPF_RAWPROC_SetFetchBitMode(MMP_RAW_COLORFMT colorFormat);
MMP_ERR MMPF_RAWPROC_SetStoreFrameSync(MMP_UBYTE ubRawId, MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_ISR(void);
MMP_ERR MMPF_RAWPROC_SetStoreYUV420Enable(MMP_UBYTE ubRawId, MMP_UBYTE ubPlane, MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_EnablePath(MMP_UBYTE ubSnrSel,
                                MMP_UBYTE ubRawId,
                                MMP_BOOL 			bEnable,
                                MMP_RAW_IOPATH 	 	pathSelect,
                                MMP_RAW_COLORFMT 	colorFormat);
#endif // _MMPF_RAWPROC_H_

