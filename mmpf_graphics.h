//==============================================================================
//
//  File        : mmpf_graphic2.h
//  Description : INCLUDE File for the Firmware Graphic Driver.
//  Author      : Rogers Chen
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_GRAPHICS_H_
#define _MMPF_GRAPHICS_H_


//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mmpf_vidcmn.h"


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

#define	GRAPHICS_FIFO_RD_THRESHOLD	    (8)
#define	GRAPHICS_FIFO_WR_THRESHOLD	    (8)
#define	GRAPHICS_FIFO_RD_WR_THRESHOLD	(0x0808)
#define GRAPHICS_SEM_TIMEOUT	        (0x100)

#define SUPPORT_GRAPHICS_FIFO_MODE      (0)
		
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_GRAPHICS_COLORDEPTH
{
    MMPF_GRAPHICS_COLORDEPTH_8 		            = 1,
    MMPF_GRAPHICS_COLORDEPTH_16 		            = 2,
    MMPF_GRAPHICS_COLORDEPTH_24 		            = 3,
    MMPF_GRAPHICS_COLORDEPTH_32 		            = 4,
    MMPF_GRAPHICS_COLORDEPTH_YUV422_UYVY 	    = 5,
    MMPF_GRAPHICS_COLORDEPTH_YUV422_VYUY         = 6,
    MMPF_GRAPHICS_COLORDEPTH_YUV422_YUYV         = 7,
    MMPF_GRAPHICS_COLORDEPTH_YUV422_YVYU         = 8,
	MMPF_GRAPHICS_COLORDEPTH_YUV420 	            = 9,
	MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE   = 10,
    MMPF_GRAPHICS_COLORDEPTH_UNSUPPORT
} MMPF_GRAPHICS_COLORDEPTH;

typedef enum _MMPF_GRAPHICS_ROTATE_TYPE
{
    MMPF_GRAPHICS_ROTATE_NO_ROTATE = 0,
    MMPF_GRAPHICS_ROTATE_RIGHT_90,
    MMPF_GRAPHICS_ROTATE_RIGHT_180,
    MMPF_GRAPHICS_ROTATE_RIGHT_270
} MMPF_GRAPHICS_ROTATE_TYPE;

typedef enum _MMPF_GRAPHICS_KEYCOLOR
{
    MMPF_GRAPHICS_FG_COLOR = 0,
    MMPF_GRAPHICS_BG_COLOR
} MMPF_GRAPHICS_KEYCOLOR;

typedef enum _MMPF_GRAPHICS_RECTFILLTYPE {
    MMPF_GRAPHICS_SOLID_FILL = 0,
    MMPF_GRAPHICS_LINE_FILL
} MMPF_GRAPHICS_RECTFILLTYPE;

typedef enum _MMPF_GRAPHICS_SCAL_SRC
{
  MMPF_GRAPHICS_SCAL_FIFO = 0,
  MMPF_GRAPHICS_SCAL_FB
} MMPF_GRAPHICS_SCAL_SRC;

typedef enum _MMPF_GRAPHICS_DELAY_TYPE
{
    MMPF_GRAPHICS_DELAY_CHK_SCA_BUSY = 0,
    MMPF_GRAPHICS_DELAY_CHK_LINE_END
} MMPF_GRAPHICS_DELAY_TYPE;

typedef enum _MMPF_GRAPHICS_ROP
{
    MMPF_GRAPHICS_ROP_BLACKNESS     = 0,
    MMPF_GRAPHICS_ROP_NOTSRCERASE   = 0x01,	// ~(S+D)
    MMPF_GRAPHICS_ROP_NOTSRCCOPY    = 0x03,	// ~S
    MMPF_GRAPHICS_ROP_SRCERASE      = 0x04,	// S.~D
    MMPF_GRAPHICS_ROP_DSTINVERT     = 0x05,	// ~D
    MMPF_GRAPHICS_ROP_SRCINVERT     = 0x06,	// S^D
    MMPF_GRAPHICS_ROP_SRCAND        = 0x08,	// S.D
    MMPF_GRAPHICS_ROP_MERGEPAINT    = 0x0B,	// ~S+D
    MMPF_GRAPHICS_ROP_SRCCOPY       = 0x0C,	// S
    MMPF_GRAPHICS_ROP_SRCPAINT      = 0x0E,	// S+D
    MMPF_GRAPHICS_ROP_WHITENESS     = 0x0F
} MMPF_GRAPHICS_ROP;

typedef struct _MMPF_GRAPHICS_BUFATTRIBUTE {
    MMP_USHORT                   usWidth;
    MMP_USHORT                   usHeight;
    MMP_USHORT                   usLineOffset;
    MMPF_GRAPHICS_COLORDEPTH     colordepth;
	MMP_ULONG                    ulBaseAddr;
    MMP_ULONG                    ulBaseUAddr;
    MMP_ULONG                    ulBaseVAddr;
} MMPF_GRAPHICS_BUFATTRIBUTE;

typedef struct _MMPF_GRAPHICS_RECT {
    MMP_USHORT usLeft;
    MMP_USHORT usTop;
    MMP_USHORT usWidth;
    MMP_USHORT usHeight;
} MMPF_GRAPHICS_RECT;

typedef struct _MMPF_GRAPHICS_DRAWRECT_ATTR {
    MMPF_GRAPHICS_RECTFILLTYPE type;
    MMP_BOOL            bUseRect;
    MMP_USHORT          usWidth;        // Buffer Width
    MMP_USHORT          usHeight;       // Buffer Height
    MMP_USHORT          usLineOfst;     // Buffer LineOffset
	MMP_ULONG           ulBaseAddr;     // Buffer Base Address
    MMPF_GRAPHICS_COLORDEPTH colordepth;
    MMP_ULONG           ulColor;
    MMP_ULONG           ulPenSize;
    MMPF_GRAPHICS_ROP   ropcode;
} MMPF_GRAPHICS_DRAWRECT_ATTR;

#if 1//AITCAM_MULTI_STREAM_EN // alyways on
typedef struct _MMPF_GRAPHICS_MODULE_CONFIG {
    MMP_ULONG               DramAddr;
    MMP_ULONG               SramAddr;
    MMPF_VIDENC_FRAME       *CurFrameList;
    MMP_USHORT              CurBufNum;
    MMP_USHORT              MaxReservedWidth;
    MMP_USHORT              MaxReservedHeight;
} MMPF_GRAPHICS_MODULE_CONFIG;
#endif

typedef void GraphicRotDMACallBackFunc(void);


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
MMP_ERR MMPF_Graphics_Init(void);
void    MMPF_Graphics_ISR(void);

MMP_ERR MMPF_Graphics_SetDelay(MMPF_GRAPHICS_DELAY_TYPE     *ubType,
                               MMP_UBYTE                    *ubPixDelay,
                               MMP_USHORT                   *usLineDelay);
MMP_ERR MMPF_Graphics_SetScaleAttribute(MMPF_GRAPHICS_BUFATTRIBUTE *bufAttr,
                        				MMP_ULONG                  ulScalBufForFIFO, 
                        				MMPF_GRAPHICS_COLORDEPTH   incolormode,                       
						                MMPF_GRAPHICS_RECT         *rect,
										MMP_USHORT                 usUpscale, 
						                MMPF_GRAPHICS_SCAL_SRC     srcsel,
                                        MMPF_OS_LOCK_CTX           LockCtx);
MMP_ERR MMPF_Graphics_ScaleStart(MMPF_GRAPHICS_BUFATTRIBUTE src,
                            MMPF_GRAPHICS_BUFATTRIBUTE dst,
                            GraphicRotDMACallBackFunc  *GRACallBack,
                            MMP_UBYTE                  ubPixelDelayN,
                            MMP_UBYTE                  ubPixelDelayM,
                            MMP_USHORT                 usWaitMs,
                            MMP_UBYTE                  ubIBCPipeNum,
                            MMPF_OS_LOCK_CTX           LockCtx);
#if(CHIP == P_V2) || (CHIP == MCR_V2)
MMP_ERR MMPF_Graphics_CopyImageBuftoBuf(MMPF_GRAPHICS_BUFATTRIBUTE 	*srcBufAttr, 
                        				MMPF_GRAPHICS_RECT 			*srcrect, 
                        				MMPF_GRAPHICS_BUFATTRIBUTE	*dstBufAttr, 
										MMP_USHORT usDstStartx, 
										MMP_USHORT usDstStarty, 
										MMPF_GRAPHICS_ROP ropcode, 
										MMP_UBYTE ubTranspActive, MMP_ULONG ulKeyColor);

MMP_ERR MMPF_Graphics_CopyImageBuftoFIFO(MMP_USHORT                 *usHostbufaddr,
										MMPF_GRAPHICS_BUFATTRIBUTE  *bufAttr, 
										MMPF_GRAPHICS_RECT          *srcrect, 
										MMP_ULONG                   ulKeyColor);

MMP_ERR MMPF_Graphics_CopyImageFIFOtoBuf(MMP_USHORT                 *usHostbufaddr,
										MMPF_GRAPHICS_COLORDEPTH    colordepth, 
										MMP_USHORT                  usHostLineOffset,
										MMPF_GRAPHICS_BUFATTRIBUTE  *bufAttr,
                        				MMPF_GRAPHICS_RECT          *srcrect, 
                        				MMP_USHORT                  usDstStartx, 
                       	 				MMP_USHORT                  usDstStarty, 
                       	 				MMPF_GRAPHICS_ROP           ropcode,  
                       	 				MMP_UBYTE ubTranspActive, MMP_ULONG ulKeyColor);

MMP_ERR MMPF_Graphics_RotateImageBuftoBuf(MMPF_GRAPHICS_BUFATTRIBUTE *srcBufAttr,
						                  MMPF_GRAPHICS_RECT         *srcrect, 
						                  MMPF_GRAPHICS_BUFATTRIBUTE *dstBufAttr, 
						                  MMP_USHORT                usDstStartx, 
						                  MMP_USHORT                usDstStarty, 
						                  MMPF_GRAPHICS_ROP         ropcode,
						                  MMPF_GRAPHICS_ROTATE_TYPE rotate, 
						                  MMP_UBYTE ubTranspActive, MMP_ULONG ulKeyColor);

MMP_ERR MMPF_Graphics_RotateImageFIFOtoBuf(MMP_USHORT                   *usHostbufaddr,
						                   MMPF_GRAPHICS_COLORDEPTH     colordepth, 
						                   MMP_USHORT                   usHostLineOffset,
						                   MMPF_GRAPHICS_BUFATTRIBUTE   *bufattribute,
						                   MMPF_GRAPHICS_RECT           *srcrect,
						                   MMP_USHORT                   usDstStartx, 
						                   MMP_USHORT                   usDstStarty, 
						                   MMPF_GRAPHICS_ROP            ropcode,
						                   MMPF_GRAPHICS_ROTATE_TYPE    rotate, 
						                   MMP_UBYTE ubTranspActive, MMP_ULONG ulKeyColor);

MMP_ERR MMPF_Graphics_DrawRectToBuf(MMPF_GRAPHICS_DRAWRECT_ATTR *drawAttr, 
                                    MMPF_GRAPHICS_RECT          *rect,
                                    MMP_ULONG                   *pOldColor);

MMP_ULONG MMPF_Graphics_SetKeyColor(MMPF_GRAPHICS_KEYCOLOR keyColorSel, MMP_ULONG ulColor);
MMP_ERR MMPF_Graphics_SetPixDelay(MMP_UBYTE ubPixDelayN, MMP_UBYTE ubPixDelayM);
MMP_ERR MMPF_Graphics_SetLineDelay(MMP_USHORT usLineDelay);
MMP_ERR MMPF_Graphics_SetDelayType(MMPF_GRAPHICS_DELAY_TYPE ubType);
#endif
MMP_ERR MMPF_GRA_SetMCI_ByteCount(MMP_USHORT ubByteCntSel);


#endif // _MMPF_GRAPHICS_H_
