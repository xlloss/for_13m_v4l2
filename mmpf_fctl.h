/// @ait_only
//==============================================================================
//
//  File        : mmpf_fctl.h
//  Description : INCLUDE File for the Host Flow Control Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpf_fctl.h
 *  @brief The header File for the Host Flow Control Driver
 *  @author Penguin Torng
 *  @version 1.0
 */


#ifndef _MMPD_FCTL_H_
#define _MMPD_FCTL_H_

#include "mmpf_scaler.h"
#include "mmpf_icon.h"
#include "mmpf_ibc.h"

/** @addtogroup MMPD_FCtl
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


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPF_FCTL_STATUS {
    MMPF_FCTL_STATUS_PREVIEW_OFF = 0,
    MMPF_FCTL_STATUS_PREVIEW_ON
} MMPF_FCTL_STATUS;

typedef struct  _MMPF_FCTL_LINK
{
    MMPF_SCALER_PATH    scalerpath;
    MMPF_ICO_PIPEID     icopipeID;
    MMPF_IBC_PIPEID     ibcpipeID;
} MMPF_FCTL_LINK;

typedef struct _MMPF_FCTL_PREVIEWATTRIBUTE {
    MMPF_IBC_COLOR	    colormode;
    MMPF_FCTL_LINK      fctllink;
	MMP_USHORT          usInputW;
	MMP_USHORT          usInputH;
    #if AITCAM_MULTI_STREAM_EN
	MMP_LONG            lCtxNum;
	#endif
	MMPF_SCALER_GRABCONTROL grabctl;
    MMP_ULONG           ulBaseAddr[4];
    MMP_ULONG           ulBaseUAddr[4];
    MMP_ULONG           ulBaseVAddr[4];
    MMPF_IBC_FX         IbcLinkFx;
    MMP_BOOL            bMirrorEnable;
    MMP_USHORT          usBufferCount;
    MMPF_SCALER_SOURCE  FctlLinkSrc;
    MMPF_FCTL_STATUS    PreviewStatus;
} MMPF_FCTL_PREVIEWATTRIBUTE;

typedef enum _MMPF_FCTL_PATH {
    MMPF_FCTL_PATH_NONE = 0,
    MMPF_FCTL_PATH_RT,
    MMPF_FCTL_PATH_LOOPBACK
} MMPF_FCTL_PATH;

typedef enum _MMPF_FCTL_LB2RT_STATE {
    MMPF_FCTL_LB2RT_DONE = 0,
    MMPF_FCTL_LB2RT_PHASE_1,
    MMPF_FCTL_LB2RT_PHASE_2,
    MMPF_FCTL_LB2RT_PHASE_3
} MMPF_FCTL_LB2RT_STATE;

typedef enum _MMPF_FCTL_PREVIEWMODE
{
    MMPF_PREVIEW_MODE_P_LCD = 0,
    MMPF_PREVIEW_MODE_FLM,
    MMPF_PREVIEW_MODE_RGB_LCD,
    MMPF_PREVIEW_MODE_TV,
    MMPF_PREVIEW_MODE_HDMI,
    MMPF_PREVIEW_MODE_CCIR
} MMPF_FCTL_PREVIEWMODE;

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
MMP_ERR MMPF_Fctl_SetPreivewAttributes(MMPF_FCTL_PREVIEWATTRIBUTE *previewattribute);
MMPF_FCTL_PREVIEWATTRIBUTE * MMPF_Fctl_GetPreviewAttributes(MMPF_IBC_PIPEID pipeID, MMP_LONG lCtxNum);
MMP_ERR MMPF_Fctl_SetLPFMaster(MMPF_IBC_PIPEID pipeID, MMP_LONG lCtxNum);
MMPF_FCTL_STATUS MMPF_Fctl_GetPreviewStatus(MMPF_IBC_PIPEID pipeID);
MMP_ERR MMPF_Fctl_EnablePreview(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable, MMP_LONG CxtNum);
MMP_ERR MMPF_Fctl_LinkPreviewToVideo(MMPF_IBC_PIPEID pipeID, MMP_UBYTE ubEncId, MMP_LONG lCtxNum);
MMP_ERR MMPF_Fctl_LinkPreviewToVidFromGra(MMPF_IBC_PIPEID pipeID, MMP_UBYTE ubEncIdList[], MMP_UBYTE ubEncNum);
MMP_ERR MMPF_Fctl_UnlinkPreview(MMPF_IBC_PIPEID pipeID, MMPF_IBC_LINK_TYPE LinkType);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _INCLUDES_H_
/// @end_ait_only

