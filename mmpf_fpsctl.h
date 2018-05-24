/**
 @file mmpf_vidtop.h
 @brief Header File for the Host 3GP RECORDER API.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPF_FPSCTL_H_
#define _MMPF_FPSCTL_H_

//===============================================================================
//
//                               COMPILER OPTION
//
//===============================================================================


//===============================================================================
//
//                               CONSTANTS
//
//===============================================================================


//===============================================================================
//
//                               STRUCTURES
//
//===============================================================================

typedef struct _MMPF_FPS_INFO {
    MMP_ULONG   TimeRes;
    MMP_ULONG   TimeInc;
    MMP_ULONG   TimeAcc;
} MMPF_FPS_INFO;


//===============================================================================
//
//                               FUNCTION PROTOTYPES
//
//===============================================================================

MMP_ERR     MMPF_FPSCTL_Init(MMPF_FPS_INFO *info, MMP_ULONG TimeResolution, MMP_ULONG TimeIncrement);
MMP_BOOL    MMPF_FPSCTL_CheckSourceState(MMPF_FPS_INFO *in, MMPF_FPS_INFO *out, MMPF_FPS_INFO *max);
MMP_BOOL    MMPF_FPSCTL_PutFrame (MMPF_FPS_INFO *in, MMPF_FPS_INFO *out);

//===============================================================================
//
//                               MACRO FUNCTIONS
//
//===============================================================================


#endif //  _MMPF_FPSCTL_H_

