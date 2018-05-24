//==============================================================================
//
//  File        : mmpf_scaler.h
//  Description : INCLUDE File for the Firmware Scaler Control driver function, including LCD/TV/Win
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_SCALER_H_
#define _MMPF_SCALER_H_


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

#if (CHIP == MCR_V2)
#define TOTAL_SCALER_PATH               (5)
#define	SCALER_PATH0_MAX_WIDTH	        (4608)
#define	SCALER_PATH1_MAX_WIDTH	        (1920)
#define	SCALER_PATH2_MAX_WIDTH	        (1280)
#define	SCALER_PATH3_MAX_WIDTH	        (1920)
#define	SCALER_PATH4_MAX_WIDTH	        (1920)

#define	LPF_MAX_WIDTH			        (SCALER_PATH0_MAX_WIDTH)
//#define SCALER_ISR_EN                   (0)
#elif (CHIP == MERCURY)
#define TOTAL_SCALER_PATH               (4)
#define	SCALER_PATH0_MAX_WIDTH	        (4352)
#define	SCALER_PATH1_MAX_WIDTH	        (1920)
#define	SCALER_PATH2_MAX_WIDTH	        (1280)
#define	SCALER_PATH3_MAX_WIDTH	        (1920)
#define	LPF_MAX_WIDTH			        (SCALER_PATH0_MAX_WIDTH)
#elif (CHIP == VSN_V3)
#define TOTAL_SCALER_PATH               (3)
#define	SCALER_PATH0_MAX_WIDTH	        (3328)
#define	SCALER_PATH1_MAX_WIDTH	        (1920)
#define	SCALER_PATH2_MAX_WIDTH	        (1280)
#define	LPF_MAX_WIDTH			        (SCALER_PATH0_MAX_WIDTH)
#else
#define TOTAL_SCALER_PATH               (3)
#define	SCALER_PATH0_MAX_WIDTH	        (2592)
#define	SCALER_PATH1_MAX_WIDTH	        (1280)
#define	SCALER_PATH2_MAX_WIDTH	        (800)
#define	LPF_MAX_WIDTH			        (SCALER_PATH0_MAX_WIDTH)
#endif

#define BAYER_SCALER_OUT_WIDTH			(1920)
#define BAYER_SCALER_OUT_HEIGHT			(1088)

#define DEFAULT_MAX_GRA_SRC_W			(1152)
#define DEFAULT_MAX_GRA_SRC_H			(648)

#define SCALER_STOP_SRC_H264            (0x01)
#define SCALER_STOP_SRC_JPEG            (0x02)
#define SCALER_STOP_SRC_TNR             (0x04)

#if (CHIP == VSN_V3) || (CHIP==MCR_V2)
#define USE_SCA_FRM_END_SWITCH_PATH     (0)
#endif

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MMPF_SCALER_SOURCE 
{
    MMPF_SCALER_SOURCE_ISP = 0,
    MMPF_SCALER_SOURCE_JPG,
    MMPF_SCALER_SOURCE_GRA,
    #if (CHIP == MCR_V2)
    MMPF_SCALER_SOURCE_LDC,
    #endif
    MMPF_SCALER_SOURCE_P0,
    MMPF_SCALER_SOURCE_P1,
    MMPF_SCALER_SOURCE_P2,
    #if (CHIP == MCR_V2)
    MMP_SCAL_SOURCE_P3,
    MMP_SCAL_SOURCE_YUV,
    #endif
    MMPF_SCALER_SOURCE_MAX
} MMPF_SCALER_SOURCE;

typedef enum _MMPF_SCALER_FIT_MODE
{
    MMPF_SCALER_FITMODE_OUT = 0,
    MMPF_SCALER_FITMODE_IN,
    MMPF_SCALER_FITMODE_OPTIMAL,
    MMPF_SCALER_FITMODE_NUM
} MMPF_SCALER_FIT_MODE;

typedef enum _MMPF_SCALER_SCALER_TYPE
{
    MMPF_SCALER_TYPE_SCALER = 0,
    MMPF_SCALER_TYPE_BAYERSCALER,
    MMPF_SCALER_TYPE_NUM
} MMPF_SCALER_SCALER_TYPE;

typedef enum _MMPF_SCALER_PATH
{
    MMPF_SCALER_PATH_0 = 0,
    MMPF_SCALER_PATH_1,
    MMPF_SCALER_PATH_2,
    #if (CHIP == MCR_V2)    
    MMPF_SCALER_PATH_3,
    MMPF_SCALER_PATH_4,
    #endif
    MMPF_SCALER_PATH_MAX
} MMPF_SCALER_PATH;

typedef enum _MMPF_SCALER_LPF_SR 
{
    MMPF_SCALER_USE_LPF = 0,
    MMPF_SCALER_SKIP_LPF
} MMPF_SCALER_LPF_SR;

typedef enum _MMPF_SCALER_GRAB_STAGE 
{
	MMPF_SCALER_GRAB_STAGE_LPF = 1,	
	MMPF_SCALER_GRAB_STAGE_SCA,		
	MMPF_SCALER_GRAB_STAGE_OUT		
} MMPF_SCALER_GRAB_STAGE;

typedef enum _MMPF_SCALER_COLRMTX_MODE 
{
    MMPF_SCALER_COLRMTX_BT601 = 0,
    MMPF_SCALER_COLRMTX_FULLRANGE,
    #if (CHIP == MCR_V2)  
    MMPF_SCALER_COLRMTX_RGB,  
    #endif

    MMPF_SCALER_COLRMTX_MAX
} MMPF_SCALER_COLRMTX_MODE;

typedef enum _MMPF_SCALER_ZOOMDIRECTION
{
    MMPF_SCALER_ZOOMIN = 0,
	MMPF_SCALER_ZOOMOUT,
	MMPF_SCALER_ZOOMSTOP
} MMPF_SCALER_ZOOMDIRECTION;

typedef enum _MMPF_SCALER_OP
{
    MMPF_SCALER_OP_FINISH = 0,
    MMPF_SCALER_OP_GOING
} MMPF_SCALER_OP;

typedef enum _MMPF_SCALER_COLORMODE
{
    MMPF_SCALER_COLOR_RGB565 = 0,
    MMPF_SCALER_COLOR_RGB888,
    MMPF_SCALER_COLOR_YUV444,
    MMPF_SCALER_COLOR_YUV422
} MMPF_SCALER_COLORMODE;

typedef enum _MMPF_SCA_EVENT 
{
    MMPF_SCA_EVENT_FRM_ST 		= 0,
    MMPF_SCA_EVENT_FRM_END 		= 1,
    MMPF_SCA_EVENT_INPUT_END 	= 2,
    MMPF_SCA_EVENT_DBL_FRM_ST   = 3,
    MMPF_SCA_EVENT_MAX
} MMPF_SCA_EVENT;

typedef enum _MMPF_SCAL_USER_DEF_TYPE
{
    MMPF_SCAL_USER_DEF_TYPE_OUT,
    MMPF_SCAL_USER_DEF_TYPE_IN_OUT,
} MMPF_SCAL_USER_DEF_TYPE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPF_SCALER_FIT_RANGE {
    MMPF_SCALER_FIT_MODE   fitmode;
    MMP_USHORT  usFitResol;
    MMP_USHORT  usInWidth;
    MMP_USHORT  usInHeight;
    MMP_USHORT  usOutWidth;
    MMP_USHORT  usOutHeight;
} MMPF_SCALER_FIT_RANGE;

typedef struct _MMPF_SCALER_GRABCONTROL {   //cm//ok
    MMP_USHORT  usScaleN;
    MMP_USHORT  usScaleM;
    MMP_USHORT  usStartX;
    MMP_USHORT  usStartY;
    MMP_USHORT  usEndX;
    MMP_USHORT  usEndY;
} MMPF_SCALER_GRABCONTROL;

typedef struct _MMPF_SCALER_ZOOM_INFO {
    MMP_USHORT                 usStepX;
    MMP_USHORT                 usStepY;
    MMP_USHORT                 usInputWidth;
    MMP_USHORT                 usInputHeight;
    MMPF_SCALER_GRABCONTROL    grabCtl;          ///< For Preview Grab Control
    MMP_USHORT                 usRangeMinN;      ///< To constrain the N-Value for zoom in and zoom out
    MMP_USHORT                 usRangeMaxN;      ///< To constrain the N-Value for zoom in and zoom out
    MMP_USHORT                 usBaseN;          ///< The real zoom base minimum , not floating whih zoom in and zoom out
    MMPF_SCALER_ZOOMDIRECTION  direction;
    MMP_USHORT                 usZoomRate;
} MMPF_SCALER_ZOOM_INFO;

typedef struct _MMPF_SCALER_PANTILT_INFO {
    MMP_LONG 				ulTargetPan;
    MMP_LONG 				ulTargetTilt;
    MMP_USHORT 				usPanEnd;
    MMP_USHORT 				usTiltEnd;
    MMP_USHORT 				usStep;
    MMPF_SCALER_GRABCONTROL grabCtl;          ///< For Preview Grab Control
} MMPF_SCALER_PANTILT_INFO;

typedef struct _MMPF_SCALER_IN_MUX_ATTR {
    MMP_UBYTE 				ubFrmSyncEn;
    MMP_UBYTE 				ubFrmSyncSel;
} MMPF_SCALER_IN_MUX_ATTR;

typedef struct _MMPF_SCAL_FIT_RANGE {
    MMPF_SCALER_FIT_MODE  	fitmode;
    MMPF_SCALER_SCALER_TYPE	scalerType;
    MMP_ULONG 	ulFitResol;
    MMP_ULONG  	ulInWidth;
    MMP_ULONG 	ulInHeight;
    MMP_ULONG  	ulOutWidth;
    MMP_ULONG 	ulOutHeight;

    MMP_ULONG 	ulInGrabX;
    MMP_ULONG	ulInGrabY;
    MMP_ULONG 	ulInGrabW;
    MMP_ULONG	ulInGrabH;
    
    MMP_ULONG 	ulDummyInPixelX;
    MMP_ULONG	ulDummyInPixelY;
    MMP_ULONG	ulDummyOutPixelX;
    MMP_ULONG	ulDummyOutPixelY;
   
    MMP_ULONG	ulInputRatioH;
    MMP_ULONG	ulInputRatioV;
    MMP_ULONG	ulOutputRatioH;
    MMP_ULONG	ulOutputRatioV;
}MMPF_SCAL_FIT_RANGE;

typedef struct _MMPF_SCALER_GRAB_CTRL {
    MMP_ULONG	ulOutStX;
    MMP_ULONG 	ulOutStY;
    MMP_ULONG	ulOutEdX;
    MMP_ULONG 	ulOutEdY;

    MMP_ULONG 	ulScaleN;
    MMP_ULONG 	ulScaleM;
    MMP_ULONG	ulScaleXN;
    MMP_ULONG 	ulScaleXM;
    MMP_ULONG  	ulScaleYN;
    MMP_ULONG 	ulScaleYM; 
} MMPF_SCALER_GRAB_CTRL;

typedef void ScalCallBackFunc(MMPF_SCALER_PATH path);

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
void MMPF_Scaler_ISR(void);
#if 0
MMP_ERR MMPF_BayerScaler_SetEngine(	MMP_BOOL 				bUserdefine, 
                        			MMPF_SCALER_FIT_RANGE 	*fitrange,
                        			MMPF_SCALER_GRABCONTROL *grabctlin, /* Only for User Define */
                        			MMPF_SCALER_GRABCONTROL *grabctlout);
MMP_ERR MMPF_BayerScaler_SetEnable(MMP_BOOL bEnable);
#endif
MMP_ERR MMPF_Scaler_SetStopEnable(MMPF_SCALER_PATH pathsel, MMP_UBYTE ubStopSrc, MMP_BOOL bEn);
MMP_ERR MMPF_Scaler_SetBusyMode(MMPF_SCALER_PATH pathsel, MMP_BOOL bEn);

MMP_ERR MMPF_Scaler_RegisterIntrCallBack(MMPF_SCALER_PATH scalpath, MMPF_SCA_EVENT event, ScalCallBackFunc *pCallBack);
MMP_ERR MMPF_Scaler_SetInterruptEnable(MMPF_SCALER_PATH pipeID, MMPF_SCA_EVENT event, MMP_BOOL bEnable);
MMP_ERR MMPF_Scaler_InitModule(MMPF_SCALER_PATH scalerpath);
MMP_ERR MMPF_Scaler_SetEngine(MMP_BOOL bUserDefine, MMPF_SCALER_PATH pathsel,
                              MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl);

MMP_ERR MMPF_Scaler_SetLPF(MMPF_SCALER_PATH pathsel, MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl);
MMP_ERR MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH pathsel, MMP_BOOL bEnable, MMPF_SCALER_COLRMTX_MODE MatrixMode);
MMP_ERR MMPF_Scaler_SetOutputFormat(MMPF_SCALER_PATH pathsel, MMPF_SCALER_COLORMODE outcolor);
MMP_ERR MMPF_Scaler_SetInputMuxAttr(MMPF_SCALER_PATH pathsel, MMPF_SCALER_IN_MUX_ATTR* pAttr);

MMP_ERR MMPF_Scaler_SetZoomParams(MMPF_SCALER_PATH pathsel, MMPF_SCALER_GRABCONTROL *grabctl,
                                  MMP_USHORT usInputWidth, MMP_USHORT usInputHeight, MMP_USHORT usStepX, MMP_USHORT usStepY);
MMP_ERR MMPF_Scaler_SetZoomRange(MMPF_SCALER_PATH pathsel, MMP_USHORT usZoomMin, MMP_USHORT usZoomMax);
MMP_ERR MMPF_Scaler_SetDigitalZoom(MMPF_SCALER_PATH pathsel, MMPF_SCALER_ZOOMDIRECTION zoomdir,
								   MMP_USHORT zoomrate, MMP_BOOL bStartOP);
MMP_ERR MMPF_Scaler_SetZoomOP(void);
MMPF_SCALER_ZOOM_INFO* MMPF_Scaler_GetCurZoomInfo(MMPF_SCALER_PATH PathSel);
MMP_ERR MMPF_Scaler_GetZoomInfo(MMPF_SCALER_PATH pathsel, MMP_USHORT *usBaseN, MMP_USHORT *usCurrentN);
MMP_ERR MMPF_Scaler_GetZoomOutPos(MMPF_SCALER_PATH pathsel, MMPF_SCALER_GRABCONTROL *SetgrabCtl);
MMP_ERR MMPF_Scaler_GetScaleUpHBlanking(MMPF_SCALER_PATH pathsel, MMP_BYTE pixel_delay, MMP_ULONG *HBlanking);
MMP_ERR MMPF_Scaler_EnableDNSMP(MMPF_SCALER_PATH pathsel, MMP_BYTE bDownSample);
MMP_ERR MMPF_Scaler_SetDigitalZoomOP(MMPF_SCALER_PATH pathsel, MMPF_SCALER_ZOOMDIRECTION zoomdir,
                                     MMP_USHORT zoomrate);

MMPF_SCALER_PANTILT_INFO* MMPF_Scaler_GetCurPanTiltInfo(MMPF_SCALER_PATH PathSel);
MMP_ERR MMPF_Scaler_SetPanTiltOP(void);
MMP_ERR MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_SetPanTiltParams(MMPF_SCALER_PATH pathsel,MMP_LONG targetPan,MMP_LONG targetTilt,MMP_USHORT steps,MMP_LONG *pan_off,MMP_LONG *tilt_off);
MMP_ERR MMPF_Scaler_SetZoomSinglePipe(MMPF_SCALER_PATH mainpipe);
MMP_ERR MMPF_Scaler_EnableZoom(MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_SetPTZSinglePipe(MMPF_SCALER_PATH mainpipe, MMP_USHORT outimage_w, MMP_USHORT outimage_h ,MMP_USHORT x, MMP_USHORT y, MMP_USHORT TargetWidth, MMP_USHORT TargetHeight);

MMP_ERR MMPF_Scaler_GetBestFitScale(MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl);
MMP_ERR MMPF_Scaler_SetEnable(MMPF_SCALER_PATH pathsel, MMP_BOOL bEnable);
MMP_ERR MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE source, MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_CheckZoomComplete(MMP_BOOL *bComplete);
MMP_ERR MMPF_Scaler_CheckPanTiltComplete(MMP_BOOL *bComplete);

MMP_ERR MMPF_Scaler_SetPTZSinglePipe1(MMPF_SCALER_PATH mainpipe,MMP_USHORT in_image_w,MMP_USHORT in_image_h,
									  MMP_USHORT x, MMP_USHORT y, MMP_USHORT TargetWidth, MMP_USHORT TargetHeight);
MMP_ERR MMPF_Scaler_SetEngine1(MMP_SHORT startx, MMP_SHORT starty, MMP_USHORT usMaxInW, MMP_USHORT usMaxInH,
                               MMPF_SCALER_PATH pathsel, MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl);

MMP_ERR MMPF_Scaler_GetGCDBestFitScale(MMPF_SCAL_FIT_RANGE *fitrange, MMPF_SCALER_GRAB_CTRL *grabctl);

MMP_ERR MMPF_Scaler_SetPixelLineDelay(MMPF_SCALER_PATH pathsel, MMP_UBYTE ubPixelDelayN, MMP_UBYTE ubPixelDelayM, MMP_UBYTE ubLineDelay);
void ChangePreviewColorToBlack(MMP_UBYTE enable);
MMP_ERR MMPF_Scaler_ResetModule(MMPF_SCALER_PATH pipeID);
MMP_ERR MMPF_Scaler_GetGrabPosition(MMPF_SCALER_PATH pipeID, MMPF_SCALER_GRAB_STAGE grabstage,
									MMP_USHORT *usHStartPos, MMP_USHORT *usHEndPos, 
									MMP_USHORT *usVStartPos, MMP_USHORT *usVEndPos);
MMP_ERR MMPF_Scaler_SetLPF1( MMPF_SCALER_PATH pathsel, MMP_USHORT x,MMP_USHORT y, MMPF_SCALER_FIT_RANGE *fitrange,
                            MMPF_SCALER_GRABCONTROL *grabctl);
MMP_ERR MMPF_Scaler_SetGrabPosition(MMPF_SCALER_PATH 		pathsel,
									MMPF_SCALER_GRAB_STAGE 	grabstage,
									MMP_USHORT 				usHStartPos, 
									MMP_USHORT 				usHEndPos, 
									MMP_USHORT 				usVStartPos, 
									MMP_USHORT 				usVEndPos);
void MMPF_Scaler_DebugOn(MMP_BOOL en);
MMP_USHORT Greatest_Common_Divisor(MMP_USHORT a, MMP_USHORT b);
MMP_BOOL MMPF_Scaler_GetEnable(MMPF_SCALER_PATH pathsel);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _MMPD_SCALER_H_
