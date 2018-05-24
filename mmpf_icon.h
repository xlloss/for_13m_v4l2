//==============================================================================
//
//  File        : mmpf_ico.h
//  Description : INCLUDE File for the Firmware ICON/ICO engine control
//  Author      : Jerry Lai
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_ICON_H_
#define _MMPF_ICON_H_

#include    "includes_fw.h"
#include    "mmpf_scaler.h"

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
#define ICON_OFFSET 0x20
#define MAX_ICON_NUM (10)

#if (CHIP == MCR_V2)
#define ICON0_MAX_DELAYLINE		(2048)
#define ICON1_MAX_DELAYLINE		(2048)
#define ICON2_MAX_DELAYLINE		(1024)
#define ICON3_MAX_DELAYLINE		(1024) // TBD ???
#endif
#if (CHIP == MERCURY)
#define ICON0_MAX_DELAYLINE		(2048)
#define ICON1_MAX_DELAYLINE		(1024)
#define ICON2_MAX_DELAYLINE		(256)
#define ICON3_MAX_DELAYLINE		(512)
/* For MERCURY_V1 
#define ICON0_MAX_DELAYLINE		(2048)
#define ICON1_MAX_DELAYLINE		(1024)
#define ICON2_MAX_DELAYLINE		(1024)
#define ICON3_MAX_DELAYLINE		(1024)
*/
#endif
#if (CHIP == VSN_V3)
#define ICON0_MAX_DELAYLINE		(512)
#define ICON1_MAX_DELAYLINE		(512)
#define ICON2_MAX_DELAYLINE		(512)
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_ICON_COLOR
{
    MMPF_ICON_COLOR_INDEX8 = 0,
    MMPF_ICON_COLOR_RGB565,
    #if (CHIP == MCR_V2)
    MMPF_ICON_COLOR_INDEX1,
    MMPF_ICON_COLOR_INDEX2,
    #endif
    MMPF_ICON_COLOR_MAX
} MMPF_ICON_COLOR;

typedef enum _MMPF_ICO_PIPEID
{
    MMPF_ICO_PIPE_0 = 0,
    MMPF_ICO_PIPE_1,
    MMPF_ICO_PIPE_2,
#if (CHIP == MCR_V2)
    MMPF_ICO_PIPE_3,
    MMPF_ICO_PIPE_4,
#endif    
    MMPF_ICO_PIPE_MAX
} MMPF_ICO_PIPEID;

typedef enum _MMPF_ICO_SEL
{
    MMPF_ICO_SEL_DISP = 0,
    MMPF_ICO_SEL_JPEG
} MMPF_ICO_SEL;

typedef enum _MMPF_JPEG_ICO_INDEX
{
    #if (CHIP == MCR_V2)    
    MMPF_JICON_0 = 0,
    MMPF_JICON_1,
    MMPF_JICON_2,
    MMPF_JICON_3,
    #endif    
    MMPF_JICON_MAX
} MMPF_JPEG_ICO_INDEX;

typedef struct _MMPF_ICON_BUFATTRIBUTE
{
	MMP_UBYTE			ubStickerId;
    MMP_ULONG           ulBaseAddr;
    MMP_USHORT          usStartX;
    MMP_USHORT          usStartY;
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMPF_ICON_COLOR     colorformat;
    MMP_ULONG           ulTpColor;
    MMP_BOOL            bTpEnable;
    MMP_BOOL            bSemiTpEnable;
    MMP_UBYTE           ubIconWeight;
    MMP_UBYTE           ubDstWeight;
} MMPF_ICON_BUFATTRIBUTE;

typedef struct _MMPF_ICO_PIPEATTRIBUTE
{
    MMPF_SCALER_PATH    inputsel;
    MMP_BOOL            bDlineEn;
    MMP_USHORT         	usFrmWidth;
} MMPF_ICO_PIPEATTRIBUTE;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR  MMPF_ICON_InitModule(MMPF_ICO_PIPEID IconPipe);
#if(CHIP == MCR_V2)
MMP_ERR  MMPF_ICON_GetAttributes(MMP_UBYTE ubIconID, MMP_UBYTE ubIconSel, MMPF_ICON_BUFATTRIBUTE *pBufAttr);
MMP_ERR  MMPF_ICON_SetAttributes(MMP_UBYTE ubIconID, MMP_UBYTE ubIconSel,       
                                MMPF_ICON_BUFATTRIBUTE *pBufAttr);

MMP_ERR  MMPF_ICON_SetSemiTP(MMP_UBYTE ubIconID, MMP_UBYTE ubIconSel,
                             MMP_BOOL bSemiTPEn, MMP_ULONG ulWeight);
MMP_ERR  MMPF_ICON_GetSemiTP_Weight(MMP_UBYTE ubIconID, MMP_UBYTE ubIconSel, MMP_USHORT *usWeight);
MMP_ERR  MMPF_ICON_SetTransparent(MMP_UBYTE ubIconID, MMP_UBYTE ubIconSel,
                                  MMP_BOOL bTranspActive, MMP_ULONG ulTranspColor);
MMP_ERR  MMPF_ICON_SetEnable(MMP_UBYTE ubIconID, MMP_UBYTE ubIconSel, MMP_BOOL bEnable);
#endif
MMP_ERR  MMPF_ICON_SetDLAttributes(MMPF_ICO_PIPEID pipeID, MMPF_ICO_PIPEATTRIBUTE *pPipeAttr);
MMP_BOOL MMPF_ICON_IsDelayLineOvf(MMPF_ICO_PIPEID pipeID);
MMP_ERR  MMPF_ICON_SetDLEnable(MMPF_ICO_PIPEID pipeID, MMP_BOOL bEnable);
MMP_ERR MMPF_Icon_LoadIndexColorTable(MMP_UBYTE ubIconID, MMPF_ICON_COLOR ubColor,
									  MMP_USHORT* pLUT, MMP_USHORT usColorNum);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

#endif //_MMPF_ICON_H_
