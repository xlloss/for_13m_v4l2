//==============================================================================
//
//  File        : mmpf_sensor.h
//  Description : INCLUDE File for the Sensor Driver Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_SENSOR_H_
#define _MMPF_SENSOR_H_

#include "includes_fw.h"
#include "mmpf_i2cm.h"
#include "mmpf_vif.h"
#include "mmp_snr_inc.h"

/** @addtogroup MMPF_Sensor
@{
*/
//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

// Select sensor model for multi-sensor configuration.
#define PRM_SENSOR           	(0)
#define SCD_SENSOR           	(1)
#define UNDEF_SENSOR			(0xFF)
#define SENSOR_MAX_NUM        	(2)         ///< Maximum sensor support count
#define SENSOR_DSC_MODE 	    (1)
#define SENSOR_VIDEO_MODE 	    (2)

#define MAX_SENSOR_RES_MODE     (12)
#define SENSOR_DELAY_REG        (0xFFFF)
#ifndef VR_MAX
#define VR_MAX(a, b)            ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef VR_MIN
#define VR_MIN(a, b)            ( ((a) < (b)) ? (a) : (b) )
#endif

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

/* Scene Mode */
typedef enum {
	AIT_SCENE_MODE_AUTO				    = 0,
	AIT_SCENE_MODE_PORTRAIT			    = 1,
	AIT_SCENE_MODE_LANDSCAPE			= 2,
	AIT_SCENE_MODE_SPORTS				= 3,
	AIT_SCENE_MODE_SUNSET				= 4,
	AIT_SCENE_MODE_DUSK				    = 5,
	AIT_SCENE_MODE_DAWN				    = 6,
	AIT_SCENE_MODE_NIGHT_SHOT			= 7,
	AIT_SCENE_MODE_AGAINST_LIGHT		= 8,
	AIT_SCENE_MODE_TEXT				    = 9,
	AIT_SCENE_MODE_MANUAL				= 10,
	AIT_SCENE_MODE_INDOOR				= 11,
	AIT_SCENE_MODE_SNOW				    = 12,
	AIT_SCENE_MODE_FALL				    = 13,
	AIT_SCENE_MODE_WAVE				    = 14,
	AIT_SCENE_MODE_FIREWORKS			= 15,
	AIT_SCENE_MODE_SHOW_WIN			    = 16,
	AIT_SCENE_MODE_CANDLE				= 17,
	AIT_SCENE_MODE_NONE				    = 18		// camera mode
} AIT_SCENE_MODE;

/* AF Mode */
typedef enum {
	AIT_AF_MODE_AUTO					= 0,
	AIT_AF_MODE_MANUAL				    = 1,
	AIT_AF_MODE_MACRO					= 2,
	AIT_AF_MODE_FULL					= 3,
	AIT_AF_MODE_MOTOR_TEST              = 4,
	AIT_AF_MODE_NULL					= 5
} AIT_AF_MODE;

/* Effect (Color Tone) */
typedef enum {
	AIT_IMAGE_EFFECT_NORMAL			    = 0,
	AIT_IMAGE_EFFECT_GREY				= 1,
	AIT_IMAGE_EFFECT_SEPIA			    = 2,
	AIT_IMAGE_EFFECT_NEGATIVE			= 3,
	AIT_IMAGE_EFFECT_ANTIQUE			= 4,
	AIT_IMAGE_EFFECT_WATERCOLOR		    = 5,
	AIT_IMAGE_EFFECT_PORTRAIT			= 6,
	AIT_IMAGE_EFFECT_LANDSCAPE		    = 7,
	AIT_IMAGE_EFFECT_SUNSET			    = 8,
	AIT_IMAGE_EFFECT_DUSK				= 9,
	AIT_IMAGE_EFFECT_DAWN				= 10,
	AIT_IMAGE_EFFECT_RED				= 11,
	AIT_IMAGE_EFFECT_GREEN			    = 12,
	AIT_IMAGE_EFFECT_BLUE				= 13,
	AIT_IMAGE_EFFECT_YELLOW			    = 15,
	AIT_IMAGE_EFFECT_EMBOSS			    = 17,
	AIT_IMAGE_EFFECT_OIL				= 18,
	AIT_IMAGE_EFFECT_BW				    = 19,
	AIT_IMAGE_EFFECT_SKETCH			    = 20,
	AIT_IMAGE_EFFECT_CRAYONE			= 21,
	AIT_IMAGE_EFFECT_WHITEBOARD		    = 22,
	AIT_IMAGE_EFFECT_BLACKBOARD		    = 23,
	AIT_IMAGE_EFFECT_VIVID              = 24,
	AIT_IMAGE_EFFECT_END				= 25
} AIT_IMAGE_EFFECT;

/* Exposure Level */
typedef enum
{
    AIT_BRIGHTNESS_LEVEL_0 = 0,
    AIT_BRIGHTNESS_LEVEL_1,
    AIT_BRIGHTNESS_LEVEL_2,
    AIT_BRIGHTNESS_LEVEL_3,
    AIT_BRIGHTNESS_LEVEL_4,
    AIT_BRIGHTNESS_LEVEL_5,
    AIT_BRIGHTNESS_LEVEL_6,
    AIT_BRIGHTNESS_LEVEL_7,
    AIT_BRIGHTNESS_LEVEL_8,
    AIT_BRIGHTNESS_LEVEL_9
} AIT_BRIGHTNESS_LEVEL;

/* AE ISO Definition */
typedef enum {
	AIT_ISO_AUTO                = 0,
	AIT_ISO_50					= 1,
	AIT_ISO_100					= 2,
	AIT_ISO_200					= 3,
	AIT_ISO_400					= 4,
	AIT_ISO_800					= 5,
	AIT_ISO_1600                = 6,
	AIT_ISO_3200                = 7,
	AIT_ISO_END					= 8
} AIT_AE_ISO;

/* AWB Mode */
typedef enum {
	AIT_AWB_MODE_BYPASS						= 0,
	AIT_AWB_MODE_AUTO						= 1,
	AIT_AWB_MODE_CLOUDY						= 2,
	AIT_AWB_MODE_DAYLIGHT					= 3,
	AIT_AWB_MODE_COOLWHITE					= 4,
	AIT_AWB_MODE_ALIGHT						= 5,
	AIT_AWB_MODE_FLUORSENT					= 6,
	AIT_AWB_MODE_EFFECT						= 7,
	AIT_AWB_MODE_DAWN						= 8,
	AIT_AWB_MODE_SUNSET						= 9
} AIT_AWB_MODE;

/* Sensor Parameter */
typedef enum {
    MMPF_SENSOR_RAWPATH_PREVIEW_ENABLE      = 0,
    MMPF_SENSOR_RAWPATH_STORE_DOWNSAMPLE,
    MMPF_SENSOR_RAWPATH_ZEROSHUTTERLAG_ENABLE,
    MMPF_SENSOR_ISP_FRAME_COUNT,
    MMPF_SENSOR_REG_CALLBACK_ISP_FRM_END,
    MMPF_SENSOR_REG_CALLBACK_SNR_TASK,
    MMPF_SENSOR_SCALE_STATE_INFO,
    MMPF_SENSOR_AE_FAST_CONVERGE
} MMPF_SENSOR_PARAM;

/* Sensor Rotate type */
typedef enum {
    MMPF_SENSOR_ROTATE_NO_ROTATE      = 0,
    MMPF_SENSOR_ROTATE_RIGHT_90,
    MMPF_SENSOR_ROTATE_RIGHT_180,
    MMPF_SENSOR_ROTATE_RIGHT_270
} MMPF_SENSOR_ROTATE_TYPE;

/* Sensor Flip Mode */
typedef enum _MMPF_SENSOR_FLIP_TYPE 
{
    MMPF_SENSOR_NO_FLIP = 0,    
    MMPF_SENSOR_COLUMN_FLIP,    
    MMPF_SENSOR_ROW_FLIP,
    MMPF_SENSOR_COLROW_FLIP
} MMPF_SENSOR_FLIP_TYPE;

typedef enum {
    MMPF_SENSOR_3A_RESET=0,
    MMPF_SENSOR_3A_SET
} MMPF_SENSOR_3A_STATE;

typedef enum _MMPF_SENSOR_SCALE_STATE {
    MMPF_SENSOR_SCALE_STATE_NONE = 0,
    MMPF_SENSOR_SCALE_STATE_DOWN,
    MMPF_SENSOR_SCALE_STATE_NONE_5M,
    MMPF_SENSOR_SCALE_STATE_MAX
} MMPF_SENSOR_SCALE_STATE;


/* Sensor resolution */
typedef enum _MMPF_SENSOR_RESOL 
{
    MMPF_SENSOR_RESOL_NULL          	= 0x00,
	MMPF_SENSOR_RESOL_320x240			= 0x01, // QVGA
	MMPF_SENSOR_RESOL_640x480			= 0x02, // VGA
	MMPF_SENSOR_RESOL_800x600			= 0x03,
	MMPF_SENSOR_RESOL_1024x768			= 0x04,
	MMPF_SENSOR_RESOL_1280x720			= 0x05, // HD
	MMPF_SENSOR_RESOL_1280x960			= 0x06,
	MMPF_SENSOR_RESOL_1280x1024			= 0x07, // 1.3M
	MMPF_SENSOR_RESOL_1600x900			= 0x08,
	MMPF_SENSOR_RESOL_1600x1200			= 0x09, // 2M
	MMPF_SENSOR_RESOL_1920x1080			= 0x0A,	// Full HD
	MMPF_SENSOR_RESOL_2304x1296			= 0x0B, // 3M : 16:9
	MMPF_SENSOR_RESOL_2048x1536			= 0x0C, // 3M
	MMPF_SENSOR_RESOL_2560x1920			= 0x0D, // 5M
	MMPF_SENSOR_RESOL_3264x1836			= 0x0E, // 6M : 16:9
	MMPF_SENSOR_RESOL_3264x2448			= 0x0F, // 8M
	MMPF_SENSOR_RESOL_3840x2160			= 0x10,	// 8M : 16:9
	MMPF_SENSOR_RESOL_NUM
} MMPF_SENSOR_RESOL;

/* ISP Mode */
typedef enum _MMPF_SENSOR_ISP_MODE 
{
	MMPF_SENSOR_MODE_INIT				= 0,
	MMPF_SENSOR_PREVIEW			        = 1,
	MMPF_SENSOR_SNAPSHOT			    = 2,
	MMPF_SENSOR_OTHERS			        = 3
} MMPF_SENSOR_ISP_MODE;

typedef enum _MMPF_SENSOR_NIGHT_VISION_MODE
{
    MMPF_SENSOR_NV_OFF = 0 ,
    MMPF_SENSOR_NV_ON  = 1 ,
    MMPF_SENSOR_NV_AUTO= 2 ,
    MMPF_SENSOR_NV_NUM  
    
} MMPF_SENSOR_NIGHT_VISION_MODE;
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPF_SENSOR_RESOLUTION {
	MMP_UBYTE	ubSensorModeNum;
	MMP_UBYTE	ubDefPreviewMode;
	MMP_UBYTE	ubDefCaptureMode;
	MMP_USHORT  usVifGrabStX[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usVifGrabStY[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usVifGrabW[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usVifGrabH[MAX_SENSOR_RES_MODE];
	#if (CHIP == MCR_V2)
	MMP_USHORT  usBayerDummyInX[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usBayerDummyInY[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usBayerOutW[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usBayerOutH[MAX_SENSOR_RES_MODE];	
	#endif
	MMP_USHORT  usScalInputW[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usScalInputH[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usTargetFpsx10[MAX_SENSOR_RES_MODE];
	MMP_USHORT  usVsyncLine[MAX_SENSOR_RES_MODE];			
	MMP_UBYTE  ubWBinningN[MAX_SENSOR_RES_MODE]; // N/M pixels to 1 pixel in X direction.
	MMP_UBYTE  ubWBinningM[MAX_SENSOR_RES_MODE];
	MMP_UBYTE  ubHBinningN[MAX_SENSOR_RES_MODE]; // N/M pixels to 1 pixel in Y direction.
	MMP_UBYTE  ubHBinningM[MAX_SENSOR_RES_MODE];
} MMPF_SENSOR_RESOLUTION;

typedef struct _MMPF_SENSOR_OPR_TABLE {
	MMP_USHORT  usInitSize;
	MMP_USHORT* uspInitTable;	
	MMP_USHORT  usSize[MAX_SENSOR_RES_MODE];
	MMP_USHORT* uspTable[MAX_SENSOR_RES_MODE];	
} MMPF_SENSOR_OPR_TABLE;

typedef struct _MMPF_SENSOR_AWBTIMIMG {
	MMP_UBYTE   ubPeriod;
	MMP_UBYTE   ubDoAWBFrmCnt;	
	MMP_UBYTE   ubDoCaliFrmCnt;
} MMPF_SENSOR_AWBTIMIMG;

typedef struct _MMPF_SENSOR_AETIMIMG {
	MMP_UBYTE   ubPeriod;
	MMP_UBYTE   ubFrmEndGetAccFrmCnt;	
	MMP_UBYTE   ubFrmStSetShutFrmCnt;
	MMP_UBYTE   ubFrmStSetGainFrmCnt;
} MMPF_SENSOR_AETIMIMG;

typedef struct _MMPF_SENSOR_AFTIMIMG {
	MMP_UBYTE   ubPeriod;
	MMP_UBYTE   ubDoAFFrmCnt;	
} MMPF_SENSOR_AFTIMIMG;

typedef struct _MMPF_SENSOR_POWER_ON {
    MMP_BOOL    bTurnOnExtPower;
    MMP_USHORT  usExtPowerPin;

	MMP_BOOL    bFirstEnPinHigh;
	MMP_UBYTE   ubFirstEnPinDelay;      //Uint:ms
	MMP_BOOL    bNextEnPinHigh;
	MMP_UBYTE   ubNextEnPinDelay;       //Uint:ms
	MMP_BOOL    bTurnOnClockBeforeRst;	
	MMP_BOOL    bFirstRstPinHigh;
	MMP_UBYTE   ubFirstRstPinDelay;	    //Uint:ms
	MMP_BOOL    bNextRstPinHigh;
	MMP_UBYTE   ubNextRstPinDelay;      //Uint:ms
} MMPF_SENSOR_POWER_ON;

typedef struct _MMPF_SENSOR_POWER_OFF {
    MMP_BOOL    bEnterStandByMode;
    MMP_USHORT  usStandByModeReg;
    MMP_USHORT  usStandByModeMask;
	MMP_BOOL    bEnPinHigh;
	MMP_UBYTE   ubEnPinDelay;           //Uint:ms
	MMP_BOOL    bTurnOffMClock;
	MMP_BOOL    bTurnOffExtPower;
	MMP_USHORT  usExtPowerPin;
} MMPF_SENSOR_POWER_OFF;

typedef struct _MMPF_CUSTOM_COLOR_ID {
	MMP_BOOL		   bUseCustomId;
	MMPF_VIF_COLOR_ID  		Rot0d_Id[MAX_SENSOR_RES_MODE];
	MMPF_VIF_COLOR_ID  		Rot90d_Id[MAX_SENSOR_RES_MODE];
	MMPF_VIF_COLOR_ID  		Rot180d_Id[MAX_SENSOR_RES_MODE];
	MMPF_VIF_COLOR_ID  		Rot270d_Id[MAX_SENSOR_RES_MODE];
	MMPF_VIF_COLOR_ID  		H_Flip_Id[MAX_SENSOR_RES_MODE];
	MMPF_VIF_COLOR_ID  		V_Flip_Id[MAX_SENSOR_RES_MODE];
	MMPF_VIF_COLOR_ID  		HV_Flip_Id[MAX_SENSOR_RES_MODE];
} MMPF_CUSTOM_COLOR_ID;

typedef struct _MMPF_SENSOR_COLOR_ID {
	MMPF_VIF_COLOR_ID       VifColorId;
	MMPF_CUSTOM_COLOR_ID 	CustomColorId;
} MMPF_SENSOR_COLOR_ID;

typedef struct _MMPF_SENSOR_VIF_SETTING {
    MMPF_VIF_SNR_TYPE       SnrType;
    MMPF_VIF_IF             OutInterface;
    MMPF_VIF_MDL_ID			VifPadId;
	MMPF_SENSOR_POWER_ON    powerOnSetting;
	MMPF_SENSOR_POWER_OFF   powerOffSetting;
	MMPF_VIF_MCLK_ATTR      clockAttr;
	MMPF_VIF_PARAL_ATTR     paralAttr;
	MMPF_MIPI_RX_ATTR       mipiAttr;
	MMPF_SENSOR_COLOR_ID	colorId;
	MMPF_MIPI_VC_ATTR		vcAttr;
	MMPF_VIF_YUV_ATTR       yuvAttr;
} MMPF_SENSOR_VIF_SETTING;

typedef struct _MMPF_SENSOR_FUNCTIION {
	//0    
    MMP_ERR (*MMPF_Sensor_InitCustFunc)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_Initialize)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_InitializeVIF)(MMP_UBYTE ubSnrSel,MMP_BOOL skipSnrPowerOn);
    MMP_ERR (*MMPF_Sensor_InitializeISP)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_PowerDown)(MMP_UBYTE ubSnrSel,MMP_BOOL skipSnrPowerDwn );
    //5
    MMP_ERR (*MMPF_Sensor_ChangeMode)(MMP_UBYTE ubSnrSel, MMP_USHORT usNewMode, MMP_USHORT usWaitFrames);
    MMP_ERR (*MMPF_Sensor_SetPreviewMode)(MMP_UBYTE ubSnrSel, MMP_USHORT usPreviewMode);
    MMP_ERR (*MMPF_Sensor_SetReg)(MMP_UBYTE ubSnrSel, MMP_USHORT usAddr, MMP_USHORT usData);
    MMP_ERR (*MMPF_Sensor_GetReg)(MMP_UBYTE ubSnrSel, MMP_USHORT usAddr, MMP_USHORT *usData);
    MMP_ERR (*MMPF_Sensor_DoAWBOperation)(MMP_UBYTE ubSnrSel);
    //10
    MMP_ERR (*MMPF_Sensor_DoAEOperation_ST)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_DoAEOperation_END)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_DoAFOperation)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_DoIQOperation)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_SetEV)(MMP_LONG ev);
    //15
    MMP_ERR (*MMPF_Sensor_SetExposureLimit)(MMP_ULONG ulBufaddr, MMP_ULONG ulDatatypeByByte, MMP_ULONG ulSize);
    MMP_ERR (*MMPF_Sensor_SetGain)(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain);
    MMP_ERR (*MMPF_Sensor_SetShutter)(MMP_UBYTE ubSnrSel, MMP_ULONG shutter, MMP_ULONG vsync);    
    MMP_ERR (*MMPF_Sensor_SetSensorFlip)(MMP_UBYTE ubSnrSel, MMP_USHORT usMode, MMPF_SENSOR_FLIP_TYPE ubFlipType);
    MMP_ERR (*MMPF_Sensor_SetSensorRotate)(MMP_UBYTE ubSnrSel, MMP_USHORT usMode, MMPF_SENSOR_ROTATE_TYPE RotateType);
    //20
    MMP_ERR (*MMPF_Sensor_Set3AStatus)(MMP_BOOL bEnable);
    MMP_ERR (*MMPF_Sensor_SetColorID)(MMPF_SENSOR_ROTATE_TYPE RotateType);    
    MMP_UBYTE (*MMPF_Sensor_GetAFEnable)(void);
    MMP_UBYTE (*MMPF_Sensor_GetEV)(void);
    MMP_UBYTE (*MMPF_Sensor_GetRealFPS)(MMP_UBYTE ubSnrSel);
    //25
	MMP_ERR (*MMPF_Sensor_GetScalInputRes)(MMP_UBYTE ubSnrSel, MMP_USHORT usSensorMode, 
										   MMP_ULONG *ulWidth, MMP_ULONG *ulHeight);
    MMP_UBYTE (*MMPF_Sensor_GetCurVifPad)(MMP_UBYTE ubSnrSel);
    MMPF_VIF_SNR_TYPE (*MMPF_Sensor_GetSnrType)(MMP_UBYTE ubSnrSel);
    MMP_ERR (*MMPF_Sensor_LinkISPSelect)(MMP_UBYTE ubSnrSel, MMP_BOOL bLinkISP);
    
} MMPF_SENSOR_FUNCTION;

typedef struct _MMPF_SENSOR_CUSTOMER {
    
    void (*MMPF_Cust_InitConfig)(void);
    void (*MMPF_Cust_DoAEOperation_ST)(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt);
    void (*MMPF_Cust_DoAEOperation_END)(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt);
    void (*MMPF_Cust_DoAWBOperation)(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt);
    void (*MMPF_Cust_DoIQOperation)(MMP_UBYTE ubSnrSel, MMP_ULONG ulFrameCnt);
    void (*MMPF_Cust_SetGain)(MMP_UBYTE ubSnrSel, MMP_ULONG ulGain);
    void (*MMPF_Cust_SetShutter)(MMP_UBYTE ubSnrSel, MMP_ULONG ulShutter, MMP_ULONG ulVsync);
    void (*MMPF_Cust_SetFlip)(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode);
    void (*MMPF_Cust_SetRotate)(MMP_UBYTE ubSnrSel, MMP_UBYTE ubMode);
    void (*MMPF_Cust_CheckVersion)(MMP_UBYTE ubSnrSel, MMP_ULONG *pulVersion);
    void (*MMPF_Cust_SetIRLED)(MMP_UBYTE ubSnrSel, MMP_UBYTE ubOn);
    void (*MMPF_Cust_SetICR)(MMP_UBYTE ubSnrSel, MMP_UBYTE ubOn);
    void (*MMPF_Cust_NightVision_Control)(MMP_UBYTE ubSnrSel, MMP_UBYTE NightMode);
    
    MMPF_SENSOR_RESOLUTION*     ResTable;
    MMPF_SENSOR_OPR_TABLE*      OprTable;
    MMPF_SENSOR_VIF_SETTING*    VifSetting; 
    MMPF_I2CM_ATTRIBUTE*        i2cmAttr;
    MMPF_SENSOR_AWBTIMIMG*      pAwbTime;
    MMPF_SENSOR_AETIMIMG*       pAeTime;
    MMPF_SENSOR_AFTIMIMG*       pAfTime;
    MMP_SNR_PRIO                sPriority;
} MMPF_SENSOR_CUSTOMER;

typedef struct _MMPF_SENSOR_INSTANCE {
    MMPF_SENSOR_3A_STATE b3AInitState;
    MMP_BOOL b3AStatus;
    MMP_BOOL bIspAeFastConverge;
    MMP_BOOL bIspAwbFastConverge;
    MMP_ULONG ulIspAeConvCount;
    MMP_ULONG ulIspAwbConvCount;
} MMPF_SENSOR_INSTANCE;

#define NV_CTRL_MODE    (0) // set
#define NV_CTRL_LED     (1) // set
#define NV_CTRL_SHUTTER (2) // set  
#define NV_CTRL_LIGHT_MODE    (3) //get

typedef struct _MMPF_SENSOR_NIGNT_VISION_CTRL
{
    MMPF_SENSOR_NIGHT_VISION_MODE nv_mode;
    MMP_BOOL    ir_led_on ;
    MMP_BOOL    ir_shutter_on;  
    MMP_BOOL    is_night ;
} MMPF_SENSOR_NIGNT_VISION_CTRL;

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

extern MMP_USHORT			gsSensorMode[];
extern MMPF_SENSOR_RESOLUTION m_SensorRes;
extern MMPF_SENSOR_FUNCTION *gsSensorFunction;  
extern MMPF_SENSOR_CUSTOMER SensorCustFunc;
extern MMP_ULONG            gSnrLineCntPerSec[];
extern MMP_BOOL             m_bLinkISPSel[];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
int SNR_Module_Init(void);
int SubSNR_Module_Init(void);
MMP_ERR MMPF_SensorDrv_Register(MMP_UBYTE ubSnrSel, MMPF_SENSOR_CUSTOMER* pCust);

MMP_UBYTE MMPF_Sensor_GetVIFPad(MMP_UBYTE ubSnrSel);
MMP_UBYTE MMPF_Sensor_GetSnrIdFromVifId(MMP_UBYTE ubVifId);
MMP_UBYTE MMPF_Sensor_GetSensorRealFPS(MMP_UBYTE ubSnrSel);
MMP_ERR MMPF_Sensor_GetParam(MMP_UBYTE ubSnrSel, MMPF_SENSOR_PARAM param_type, void* param);
MMP_ERR MMPF_Sensor_SetParam(MMP_UBYTE ubSnrSel, MMPF_SENSOR_PARAM param_type, void* param);
MMP_ERR MMPF_Sensor_Initialize(MMP_USHORT usSensorID, MMP_UBYTE ubFrameRate,MMP_UBYTE ubPreviewMode,MMP_BOOL skipSnrPowerOn);
MMP_ERR MMPF_Sensor_AutoFocus(void);
MMP_ERR MMPF_Sensor_GetCurResolution (MMP_UBYTE ubSnrSel, MMP_USHORT *usCurWidth, MMP_USHORT*usCurHeight);
MMP_ERR MMPF_Sensor_GetPreviewEffectNum(MMP_UBYTE *ubEffectNum);
MMP_ERR MMPF_Sensor_SetExposureValuel(MMP_USHORT usEV);

MMP_ERR MMPF_Sensor_CheckFrameStart(MMP_USHORT usFrameCount);
MMP_ERR MMPF_Sensor_CheckFrameEnd(MMP_UBYTE ubVifId, MMP_USHORT usFrameCount);

MMP_ERR MMPF_Sensor_WaitFrame(MMP_USHORT usFrameCount);
MMP_ERR MMPF_Sensor_SetSensorReg(MMP_UBYTE ubSnrSel, MMP_USHORT usAddr, MMP_USHORT usData);
MMP_ERR MMPF_Sensor_GetSensorReg(MMP_UBYTE ubSnrSel, MMP_USHORT usAddr,MMP_USHORT *usData) ;
MMP_ERR MMPF_Sensor_SetVIFInterrupt(MMP_UBYTE ubFlag, MMP_BOOL bEnable);
MMP_ERR MMPF_Sensor_SetISPInterrupt(MMP_ULONG ulFlag, MMP_BOOL bEnable);
MMP_ERR MMPF_Sensor_Set3AInterrupt(MMP_BOOL bEnable);
MMP_ERR MMPF_Sensor_LinkFunctionTable(void);
MMP_ERR MMPF_Sensor_PowerDown(MMP_USHORT usSensorID,MMP_BOOL skipSnrPowerDwn);
MMP_ERR MMPF_Sensor_SetFlipDirection(MMP_UBYTE ubSnrSel, MMPF_SENSOR_FLIP_TYPE Direction);
MMP_ERR MMPF_Sensor_SetRotDirection(MMP_UBYTE ubSnrSel, MMPF_SENSOR_ROTATE_TYPE Direction);

MMP_ERR MMPF_Sensor_Set3AState(MMPF_SENSOR_3A_STATE state);
MMP_ERR MMPF_ISP_UpdateInputSize(MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPF_Sensor_AllocateBuffer(MMP_ULONG ulStartAddr, MMP_ULONG ulDmaBufAddr);
MMPF_SENSOR_3A_STATE MMPF_Sensor_Get3AState(void);
MMP_ERR MMPF_Sensor_GetHWBufferSize (MMP_ULONG *ulBufSize, MMP_ULONG *ulDmaSize);
MMP_ERR MMPF_Sensor_SetScaleResol (MMP_USHORT usInitWidth, MMP_USHORT usInitHeight);

void MMPF_Sensor_WorkIspFrameSt(void *Arg);
MMP_ERR MMPF_Sensor_SetNightVisionMode(MMP_UBYTE ctrl_id,MMP_UBYTE val) ;
MMP_UBYTE MMPF_Sensor_GetNightVisionMode(MMP_UBYTE ctrl_id) ;
MMP_ERR MMPF_Sensor_RestoreSnrSetting(MMP_ULONG *data);
#endif // _MMPF_SENSOR_H_
/// @}
