//==============================================================================
//
//  File        : mmpf_bayerscaler.h
//  Description : INCLUDE File for the Firmware Scaler Control driver function, including LCD/TV/Win
//  Author      : Eroy Yang
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_BAYERSCALER_H_
#define _MMPF_BAYERSCALER_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"
#include "mmpf_scaler.h"

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPF_BAYER_SCALER_MODE
{
    MMPF_BAYER_SCAL_BYPASS = 0,
    MMPF_BAYER_SCAL_DOWN,
    MMPF_BAYER_SCAL_RAW_FETCH,
    MMPF_BAYER_SCAL_MAX_MODE
} MMPF_BAYER_SCALER_MODE;

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _MMPF_BAYER_SCALER_INFO 
{
    MMPF_SCAL_FIT_RANGE	sFitRange;
    MMPF_SCALER_GRAB_CTRL sGrabCtl;
} MMPF_BAYER_SCALER_INFO;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPF_BayerScaler_SetEngine(	MMP_BOOL 				bUserdefine, 
                        			MMPF_SCAL_FIT_RANGE 	*fitrange,
                        			MMPF_SCALER_GRAB_CTRL 	*grabctlout);
MMP_ERR MMPF_BayerScaler_SetEnable(MMP_BOOL bEnable);
MMP_ERR MMPF_BayerScaler_GetResolution(MMP_UBYTE ubInOut, MMP_USHORT *pusW, MMP_USHORT *pusH);

MMP_ERR MMPF_BayerScaler_GetZoomInfo(MMPF_BAYER_SCALER_MODE nBayerMode, 
								     MMPF_SCAL_FIT_RANGE 	*fitrange, 
								     MMPF_SCALER_GRAB_CTRL 	*grabctl);                        			 
MMP_ERR MMPF_BayerScaler_SetZoomInfo(MMPF_BAYER_SCALER_MODE nBayerMode, 
									 MMPF_SCALER_FIT_MODE 	sFitMode,
				                     MMP_ULONG ulSnrInputW, MMP_ULONG ulSnrInputH, 
				                     MMP_ULONG ulFovInputW, MMP_ULONG ulFovInputH, 
				                     MMP_ULONG ulOutputW,   MMP_ULONG ulOutputH, 
				                     MMP_ULONG ulDummyOutX, MMP_ULONG ulDummyOutY);

#endif // _MMPF_BAYERSCALER_H_
