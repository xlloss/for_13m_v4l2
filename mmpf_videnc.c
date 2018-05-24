/// @ait_only
/**
 @file mmpf_mp4venc.c
 @brief Control functions of video encoder
 @author Will Tseng
 @version 1.0
*/

#include "includes_fw.h"
#include "ait_cam_common.h"

#include "mmp_reg_gbl.h"
#include "mmpf_vidmgr.h"
#include "mmpf_videnc.h"
#include "mmpf_h264enc.h"

#include "mmp_reg_h264enc.h"
#include "mmp_reg_h264dec.h"

/** @addtogroup MMPF_VIDENC
 *  @{
 */
extern MMPF_OS_TIME_UNIT   aitcam_clockbase(void);

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
static MMPF_VIDENC_INSTANCE     m_VidInstance[MAX_NUM_ENC_SET];
static MMPF_VIDENC_MODULE       m_VidModule[MMPF_VIDENC_MODULE_MAX];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#if 0
void dump_reg(MMP_ULONG start, MMP_ULONG len)
{
    MMP_ULONG addr;
    for (addr = start; addr < (start+len); addr++) {
        if ((addr & 0xF) == 0) {
            RTNA_DBG_Str(0, "\r\n");
            RTNA_DBG_Long(0, addr);
            RTNA_DBG_Str(0, " : ");
        }
        RTNA_DBG_Byte(0, *(MMP_UBYTE*)addr);
    }
    RTNA_DBG_Str(0, "\r\n Dump End\r\n");
}
#endif

static MMPF_VIDENC_FORMAT module_format_map (MMPF_VIDENC_MODULE_ID ModId)
{
    switch (ModId) {
    case MMPF_VIDENC_MODULE_H264:
        return MMPF_VIDENC_FORMAT_H264;
    default:
        return MMPF_VIDENC_FORMAT_OTHERS;
    }
}

MMPF_VIDENC_MODULE * MMPF_VIDENC_GetModule(MMPF_VIDENC_MODULE_ID ModId)
{
    return (ModId < MMPF_VIDENC_MODULE_MAX)? &(m_VidModule[ModId])
            : &(m_VidModule[0]);

}

/** @brief Returns pointers to videnc instance structure
 @param[in] InstId instance ID of encoder to get handle
 @retval MMP_ERR_NONE Success.
*/
MMPF_VIDENC_INSTANCE * MMPF_VIDENC_GetInstance (MMP_UBYTE InstId)
{
    return (InstId < MAX_NUM_ENC_SET)? &(m_VidInstance[InstId])
            : &(m_VidInstance[0]);
}

/**
 @brief Push 1 element to end of queue
 @param[in] queue current queue
 @param[in] queue_size current queue size
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPF_VIDENC_PushQueue(MMPF_VIDENC_QUEUE *queue, MMP_UBYTE buffer)
{
    if (queue->size >= MMPF_VIDENC_MAX_QUEUE_SIZE) {
        DBG_S(0, "Queue overflow\r\n");
        return MMP_ERR_NONE;
    }
    queue->buffers[(queue->head + queue->size++) % MMPF_VIDENC_MAX_QUEUE_SIZE] = buffer;
    return MMP_ERR_NONE;
}

/**
 @brief Pop 1 element from queue
 @param[in] queue current queue
 @param[in] offset the offset for pop the queue element
 @retval MMP_ERR_NONE Success.
*/
MMP_UBYTE MMPF_VIDENC_PopQueue(MMPF_VIDENC_QUEUE *queue, MMP_UBYTE offset)
{
    MMP_UBYTE buffer, target, source;
    if (queue->size == 0) {
        //DBG_S(0, "Queue underflow\r\n");
        printk(KERN_ERR "Queue underflow\n");
        return MMPF_VIDENC_MAX_QUEUE_SIZE;
    }
    target = (queue->head + offset) % MMPF_VIDENC_MAX_QUEUE_SIZE; //position for pop data
    buffer = queue->buffers[target];
    while (target != queue->head) { //shift elements before offset
        source = (target)? (target-1): (MMPF_VIDENC_MAX_QUEUE_SIZE-1);
        queue->buffers[target] = queue->buffers[source];
        target = source;
    }
    queue->size--;
    queue->head = (queue->head + 1) % MMPF_VIDENC_MAX_QUEUE_SIZE;

//        #ifdef AITCAM_DBG_MSG
//        if (debug_level >= 2) {
//            printk(KERN_ERR "PopQ: %d\n", buffer);
//        }
//        #endif

    return buffer;
}

#if AITCAM_IPC_EN==0
/**
 @brief Fill slice desc. structure from enc handle
 @param[out] pDesc descriptor of the slice/frame for mgr
 @param[in] pEnc encoder instance
 @retval none
*/
MMP_ERR MMPF_VIDENC_InitCodedDataDesc(MMPF_VIDMGR_CODED_DESC *pDesc,
                                        MMP_UBYTE   (*XhdrBuf)[MAX_XHDR_SIZE],
                                        MMP_ULONG   *XhdrSize,
                                        MMP_ULONG   *DataBuf,
                                        MMP_ULONG   *DataSize)
{
    pDesc->ulSliceNum = 0;
    pDesc->ulOutputSize = 0;
    pDesc->ulHwCodedSize = 0;

    pDesc->XhdrBuf  = XhdrBuf;
    pDesc->XhdrSize = XhdrSize;
    pDesc->DataBufAddr = DataBuf;
    pDesc->DataSize = DataSize;

    pDesc->ulTimestamp  = 0;
    pDesc->FrameType    = MMPF_3GPMGR_FRAME_TYPE_MAX;
    pDesc->Flags        = MMPF_VIDMGR_DESC_FLAG_NONE;

    pDesc->usLayerId    = 0;
    pDesc->ubEncId      = 0;

    return MMP_ERR_NONE;
}

/**
 @brief Get next frame type by enc frame num
 @param[in] Encoded frame number
 @retval none
*/
MMPF_3GPMGR_FRAME_TYPE MMPF_VIDENC_GetFrameType(MMP_ULONG ulEncFrameNum,
                                                MMP_USHORT gop_size, MMP_USHORT b_frame_num)
{
    MMP_ULONG ulLocalFrameNum;

    if (gop_size == 0) {
        ulLocalFrameNum = ulEncFrameNum;
    }
    else if (ulEncFrameNum < gop_size) {
        ulLocalFrameNum = ulEncFrameNum;
    }
    else {
        ulLocalFrameNum = ulEncFrameNum % gop_size;
    }

    if (ulLocalFrameNum == 0) {
        return MMPF_3GPMGR_FRAME_TYPE_I;
    }
    else {
        if ((ulLocalFrameNum-1) % (1+b_frame_num))
            return MMPF_3GPMGR_FRAME_TYPE_B;
        else
            return MMPF_3GPMGR_FRAME_TYPE_P;
    }
}



/**
 @brief Set current frame ready encoder id
 @param[in] ubEncId ID of the encoder
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPF_VIDENC_SetFrameReady(MMP_ULONG InstId)
{
    #if 1
    MMPF_OS_IssueWork(MMPF_OS_WORKID_ENC_ST_0+InstId);
    #elif 0
    MMPF_H264ENC_TriggerFrameDone(MMPF_H264ENC_GetHandle(InstId), MMPF_OS_LOCK_CTX_ISR);
    #else
    if (InstId== 0) {
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_ENC_FRM_0, MMPF_OS_FLAG_SET);
    }
    else {
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_ENC_FRM_1, MMPF_OS_FLAG_SET);
    }
    #endif

    return MMP_ERR_NONE;
}

/**
 @brief Control the flow about deciding encode frame type, frame order
 @param[in] ubEncID : ID of the encoder
 @param[in][out] pbCurBuf. point to index of IBC exposure done frame buffer
 @param[in][out] pbIBCBuf. point to index of IBC current output frame buffer
 @param[in] ubBufCount. IBC frame buffer count
 @param[in] ulCurYBufAddr. Frame buffer addresses of Y component
 @param[in] ulCurUBufAddr. Frame buffer addresses of U component
 @param[in] ulCurVBufAddr. Frame buffer addresses of V component
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPF_VIDENC_TriggerFrameDone(MMP_UBYTE ubEncID,
                                    MMP_UBYTE *pbCurBuf,
                                    MMP_UBYTE *pbIBCBuf,
                                    MMP_UBYTE ubBufCount,
                                    MMP_ULONG ulCurYBufAddr[],
                                    MMP_ULONG ulCurUBufAddr[],
                                    MMP_ULONG ulCurVBufAddr[])
{
    MMPF_VIDENC_INSTANCE *pVeInst = MMPF_VIDENC_GetInstance(ubEncID);

    if (pVeInst && !pVeInst->bInit) {
        return MMP_VIDE_ERR_INVAL_OP;
    }

    switch (get_vidinst_format(pVeInst)) {
    case MMPF_VIDENC_FORMAT_H264:
    {
        MMPF_H264ENC_ENC_INFO *pEnc = &(pVeInst->h264e);
        MMPF_VIDENC_CURBUF_MODE InputBufMode =
                (pEnc->CurRTModeSwitchingSig == MMPF_VIDENC_CURBUF_MAX)?
                        pEnc->CurBufMode: pEnc->CurRTModeSwitchingSig;

        if (pEnc->CurRTModeSwitchingSig == MMPF_VIDENC_CURBUF_FRAME) {
            pEnc->init_buffered_frm = pEnc->b_frame_num + 1; //re-fill queue
        }

        if (InputBufMode == MMPF_VIDENC_CURBUF_FRAME) {
            if (ubBufCount == 0) {
                RTNA_DBG_Str(0, "#Error : No cur buf\r\n");
                return MMP_VIDE_ERR_INVAL_PARAM;
            }
        }

        if (pEnc->Status == MMPF_VIDENC_FW_STATUS_START) {
            MMP_BOOL                bWorking;
            MMP_ULONG64             ullValue64[2];

            bWorking = (pEnc->module->bWorking && (pEnc->module->pH264Inst == pEnc));

            if (InputBufMode == MMPF_VIDENC_CURBUF_FRAME) {
                *pbCurBuf = *pbIBCBuf; // set exposur done frame to ibc_cur_buf
            }

            #if (FPS_CTL == 1)
            ullValue64[0] = (MMP_ULONG64)pEnc->ulFpsInputInc*pEnc->ulFpsOutputRes*pEnc->ulFpsInputAcc;
            ullValue64[1] = (MMP_ULONG64)pEnc->ulFpsInputRes*pEnc->ulFpsOutputInc*pEnc->ulFpsOutputAcc;

            if (ullValue64[0] < ullValue64[1]) {
                pEnc->ulFpsInputAcc += pEnc->ulFpsInputInc;
                if (pEnc->ulFpsInputAcc >= pEnc->ulFpsInputRes) {
                    pEnc->ulFpsInputAcc -= pEnc->ulFpsInputRes;
                    pEnc->ulFpsOutputAcc -= pEnc->ulFpsOutputRes;
                }

                //#if (KITE_ONLY_VIDEO_PATCH == 1)
                //if (gbCurH264Type != FRAMEBASE_H264) { // TBD
                //    MMPF_USB_ReleaseDm();
                //}
                //#endif
				if ((InputBufMode == MMPF_VIDENC_CURBUF_RT)
				    && (pEnc->CurRTFctlMode == MMPF_VIDENC_RTFCTL_PASSIVE)) {
                    if (pEnc->EncReStartCallback) {
                        pEnc->EncReStartCallback(pEnc);
                    }
				}

        	    return MMP_ERR_NONE;
        	}

            pEnc->ulFpsInputAcc += pEnc->ulFpsInputInc;
            pEnc->ulFpsOutputAcc += pEnc->ulFpsOutputInc;
            if (pEnc->ulFpsInputAcc >= pEnc->ulFpsInputRes) {
                pEnc->ulFpsInputAcc -= pEnc->ulFpsInputRes;
                pEnc->ulFpsOutputAcc -= pEnc->ulFpsOutputRes;
            }
            #endif //FPS_CTL == 1

            if (bWorking) {
                AITPS_H264DEC_DBLK_ROT   pH264REC = AITC_BASE_H264DEC_DBLK_ROT;
                MMP_UBYTE                mb_y = pH264REC->ENC_TIMEOUT_MB_Y_OFST;

                #if 0
                if ((mb_y) && (mb_y <= (pEnc->mb_h >> 3)))
                #else
                if ((mb_y) && (mb_y <= (pEnc->mb_h >> 1)))
                #endif
                {
                    dbg_printf(0, "h264 busy %d\r\n", ubEncID);
    				if ((InputBufMode == MMPF_VIDENC_CURBUF_RT)
    				    && (pEnc->CurRTFctlMode == MMPF_VIDENC_RTFCTL_PASSIVE)) {
                        if (pEnc->EncReStartCallback) {
                            pEnc->EncReStartCallback(pEnc);
                        }
    				}
                    return MMP_ERR_NONE;
                }
            }

            // move to task
            pEnc->OpForceFrameType = pEnc->pic_ctl_flag;
            if ((pEnc->pic_ctl_flag == MMPF_VIDENC_PICCTL_IDR_RESYNC)
                || (pEnc->pic_ctl_flag == MMPF_VIDENC_PICCTL_IDR)) {
                pEnc->total_frames = 0; //SEANADD    
                pEnc->OpIdrPic = MMP_TRUE;
                pEnc->OpFrameType = MMPF_3GPMGR_FRAME_TYPE_I;
            }
            else if ((pEnc->pic_ctl_flag == MMPF_VIDENC_PICCTL_I_RESYNC)
                    || (pEnc->pic_ctl_flag == MMPF_VIDENC_PICCTL_I)) {
                pEnc->OpIdrPic = MMP_FALSE;
                pEnc->OpFrameType = MMPF_3GPMGR_FRAME_TYPE_I;
            }
            else {
                if (pEnc->total_frames == 0) { //1st frame
                    pEnc->OpFrameType = MMPF_3GPMGR_FRAME_TYPE_I;
                    pEnc->OpIdrPic = MMP_TRUE;
                } else {
                    if (pEnc->RefGenBufMode == MMPF_VIDENC_REFGENBUF_NONE) {
                        pEnc->OpFrameType = MMPF_3GPMGR_FRAME_TYPE_I;
                    } else {
                        pEnc->OpFrameType = MMPF_VIDENC_GetFrameType(pEnc->gop_frame_num+(bWorking?1:0),
                                                pEnc->gop_size, pEnc->b_frame_num);
                    }
                    pEnc->OpIdrPic = ((pEnc->OpFrameType == MMPF_3GPMGR_FRAME_TYPE_I)
                                        && (pEnc->GopType == MMPF_VIDENC_SYNCFRAME_IDR));
                }
            }
            if (pEnc->OpFrameType == MMPF_3GPMGR_FRAME_TYPE_I) {
                if ((pEnc->pic_ctl_flag == MMPF_VIDENC_PICCTL_IDR)
                    || (pEnc->pic_ctl_flag == MMPF_VIDENC_PICCTL_I)) {
                    pEnc->OpInsParset = MMP_FALSE;
                }
                else {
                    pEnc->OpInsParset = MMP_TRUE;
                }
                #if 0
                if (pEnc->OpIdrPic) {
                    RTNA_DBG_Str(3, "#I, ");
                }
                else {
                    RTNA_DBG_Str(3, "#NonIdrI, ");
                }
                #endif
            }
            else {
                pEnc->OpInsParset = MMP_FALSE;
            }
            if (pEnc->pic_ctl_flag != MMPF_VIDENC_PICCTL_NONE) {
                pEnc->pic_ctl_flag = MMPF_VIDENC_PICCTL_NONE;
            }

            /* FRM <-> RT switch, time jitters */
#if (GET_TIMESTAMP_AFTER_H264ENC == 0)
            MMPF_OS_GetTimestamp(&(pEnc->timestamp), aitcam_clockbase());
#endif

            if (InputBufMode == MMPF_VIDENC_CURBUF_FRAME) {
                MMPF_VIDENC_PushQueue(&(pEnc->cur_frm_queue), *pbIBCBuf);
                if (pEnc->init_buffered_frm) { // to buffer gsBFrameCnt frames
                    pEnc->init_buffered_frm--;
                    *pbIBCBuf = (*pbIBCBuf + 1) % ubBufCount;
                    if (pEnc->init_buffered_frm == 0) { // trigger 1st encode
                        pEnc->enc_frm_buf = MMPF_VIDENC_PopQueue(&(pEnc->cur_frm_queue), 0);
                        pEnc->cur_frm.ulYAddr = ulCurYBufAddr[pEnc->enc_frm_buf];
                        pEnc->cur_frm.ulUAddr = ulCurUBufAddr[pEnc->enc_frm_buf];
                        pEnc->cur_frm.ulVAddr = ulCurVBufAddr[pEnc->enc_frm_buf];
                	    MMPF_VIDENC_SetFrameReady(ubEncID);
                    }
                    return MMP_ERR_NONE;
                }
                *pbIBCBuf = pEnc->enc_frm_buf;
                pEnc->enc_frm_buf = MMPF_VIDENC_PopQueue(&(pEnc->cur_frm_queue),
                                        (pEnc->OpFrameType==MMPF_3GPMGR_FRAME_TYPE_P)? pEnc->b_frame_num: 0);
                pEnc->cur_frm.ulYAddr = ulCurYBufAddr[pEnc->enc_frm_buf];
                pEnc->cur_frm.ulUAddr = ulCurUBufAddr[pEnc->enc_frm_buf];
                pEnc->cur_frm.ulVAddr = ulCurVBufAddr[pEnc->enc_frm_buf];
        	    MMPF_VIDENC_SetFrameReady(ubEncID);
        	}
        	else {
        	    if (pEnc->CurRTFctlMode == MMPF_VIDENC_RTFCTL_PASSIVE) {
        	        MMPF_H264ENC_TriggerFrameDone(MMPF_H264ENC_GetHandle(ubEncID),
        	                                        MMPF_OS_LOCK_CTX_ISR);
        	    } else {
        	        MMPF_VIDENC_SetFrameReady(ubEncID);
        	    }
        	}
        }
        else {
            if (InputBufMode == MMPF_VIDENC_CURBUF_FRAME) {
                if ((*pbCurBuf = *pbCurBuf + 1) >= ubBufCount) {
                    (*pbCurBuf) -= ubBufCount;
                }
                if ((*pbIBCBuf = *pbIBCBuf + 1) >= ubBufCount) {
                    (*pbIBCBuf) -= ubBufCount;
                }
            }
        }
        break;
    }
    default:
        break;
    }

    return MMP_ERR_NONE;
}

//#define GBL_CHIP_VER_MASK                   0x1F
#define GBL_CHIP_FEATURE_MASK           0x1C

/**
 @brief Check encode capability according to CHIP ID
*/
MMP_BOOL MMPF_VIDENC_CheckCapability(MMP_ULONG total_mb, MMP_ULONG fps)
{
    AITPS_GBL pGBL = AITC_BASE_GBL;
    MMP_UBYTE bFeature = 0;
    MMP_BOOL    support = MMP_FALSE;
    MMP_ULONG   mb_rate = total_mb * fps;

    bFeature = (pGBL->GBL_CHIP_VER & GBL_CHIP_FEATURE_MASK);
    //dbg_printf(0, "bFeature = 0x%x\r\n", bFeature);
    
//    pGBL->GBL_ID_RD_CTL |= GBL_PROJECT_ID_RD_EN;

//    gbSystemCoreID = pGBL->GBL_PROJECT_ID;
//    if ((bFeature == 0x18) || (bFeature == 0x14)) {
//        gbSystemCoreID += 0x40;
//    }
//    pGBL->GBL_ID_RD_CTL &= ~(GBL_PROJECT_ID_RD_EN);

    if ((bFeature == 0x18) || (bFeature == 0x14)) {  // CHIP_CORE_ID_MCR_V2_LE
        // MB count of 1296p = (2304/16) * (1296/16) = 11664
//        if (mb_rate <= (11664 * 30))  // 1296p@30fps
        if (mb_rate <= (8160 * 30))    // 1080p@30fps
            support = MMP_TRUE;
    }
    else {  // CHIP_CORE_ID_MCR_V2  // 1080p@60fps  // bFeature = 0x10
        // MB count of 1080p = (1920/16) * (1088/16) = 8160
        if (mb_rate <= (8160 * 60))
            support = MMP_TRUE;
    }

    if (support == MMP_FALSE)
        RTNA_DBG_Str(0, "UnSupported encode capability\n");

    return support;
}

MMP_ERR MMPF_VIDENC_SetParameter (MMP_UBYTE ubEncId, MMPF_VIDENC_ATTRIBUTE attrib, void *arg)
{
    MMPF_VIDENC_INSTANCE *pInst = MMPF_VIDENC_GetInstance(ubEncId);

    switch (get_vidinst_format(pInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        return MMPF_H264ENC_SetParameter(&(pInst->h264e), attrib, arg);
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    };
}

MMP_ERR MMPF_VIDENC_GetParameter(MMP_UBYTE ubEncId, MMPF_VIDENC_ATTRIBUTE attrib, void *arg)
{
    MMPF_VIDENC_INSTANCE *pInst = MMPF_VIDENC_GetInstance(ubEncId);

    switch (get_vidinst_format(pInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        return MMPF_H264ENC_GetParameter(&(pInst->h264e), attrib, arg);
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    };
}


MMP_ERR MMPF_VIDENC_InitModule(MMPF_VIDENC_MODULE_ID ModId,
                                MMPF_VIDENC_MODULE_CONFIG *ModuleConfig)
{
    MMPF_VIDENC_MODULE  *pMod = &(m_VidModule[ModId]);

    if (pMod->bInit) {
        return MMP_VIDE_ERR_INVAL_OP;
    }

    switch (ModId) {
    case MMPF_VIDENC_MODULE_H264:
        if (MMPF_H264ENC_InitModule(&(pMod->H264EMod),
                &(ModuleConfig->H264ModCfg)) != MMP_ERR_NONE)
        {
            return MMP_VIDE_ERR_INVAL_PARAM;
        }
        MMPF_VIDMGR_Initialization(0); ///assign mgr 0 to h264e
        break;
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    }

	#if (OS_TYPE == OS_UCOSII)
    {
        AITPS_AIC pAIC = AITC_BASE_AIC;
        RTNA_AIC_Open(pAIC, AIC_SRC_H264ENC, h264enc_isr_a,
                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
        RTNA_AIC_IRQ_En(pAIC, AIC_SRC_H264ENC);
    }
	#endif

	pMod->bInit        = MMP_TRUE;
	pMod->Format       = module_format_map(ModId);

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIDENC_DeinitModule(MMPF_VIDENC_MODULE_ID ModId)
{
    MMPF_VIDENC_MODULE  *pMod = &(m_VidModule[ModId]);

    if (!pMod->bInit) {
        return MMP_VIDE_ERR_INVAL_OP;
    }

    switch (ModId) {
    case MMPF_VIDENC_MODULE_H264:
        if (MMPF_H264ENC_DeinitModule(&(pMod->H264EMod)) != MMP_ERR_NONE) {
            return MMP_VIDE_ERR_INVAL_OP;
        }
        break;
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    }

    pMod->bInit = MMP_FALSE;

    return MMP_ERR_NONE;
}

MMP_BOOL MMPF_VIDENC_IsModuleInit(MMPF_VIDENC_MODULE_ID ModId)
{
    return m_VidModule[ModId].bInit;
}

MMP_ERR MMPF_VIDENC_InitInstance(MMP_ULONG *InstId,
            MMPF_VIDENC_INSTANCE_CONFIG *InstConfig,
            MMPF_VIDENC_MODULE_ID ModuleId)
{
    MMP_ULONG               inst_id;
    MMPF_VIDENC_INSTANCE    *Inst;
    MMPF_VIDENC_MODULE      *pMod;

    if (ModuleId >= MMPF_VIDENC_MODULE_MAX) {
        return MMP_VIDE_ERR_INVAL_PARAM;
    }
    pMod = &(m_VidModule[ModuleId]);

    if (!pMod->bInit) {
        return MMP_VIDE_ERR_INVAL_OP;
    }

    for (inst_id = 0; inst_id < MAX_NUM_ENC_SET; inst_id++) {
        Inst = MMPF_VIDENC_GetInstance(inst_id);
        if (!Inst->bInit) {
            break;
        }
    }
    if (inst_id >= MAX_NUM_ENC_SET) {
        return MMP_VIDE_ERR_INVAL_OP;
    }
    *InstId = inst_id;

    Inst->bInit = MMP_TRUE;
    Inst->Module = pMod;

    switch (pMod->Format) {
    case MMPF_VIDENC_FORMAT_H264:
        return MMPF_H264ENC_InitInstance(&(Inst->h264e),
                    &(InstConfig->H264InstCfg), &(pMod->H264EMod), Inst);
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    }

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIDENC_DeinitInstance(MMP_ULONG InstId)
{
    MMPF_VIDENC_INSTANCE *pEncInst = MMPF_VIDENC_GetInstance(InstId);

    if (!(pEncInst->bInit)) {
        return MMP_VIDE_ERR_INVAL_OP;
    }

    #if 0
    switch (get_vidinst_format(pEncInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        break;
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    }
    #endif

    pEncInst->Module = NULL;
    pEncInst->bInit = MMP_FALSE;

    return MMP_ERR_NONE;
}

/**
 @brief Return video engine status by Mergr3GP task.
 @retval 0x0001 MMPF_VIDENC_FW_STATUS_START
 @retval 0x0002 MMPF_VIDENC_FW_STATUS_PAUSE
 @retval 0x0003 MMPF_VIDENC_FW_STATUS_RESUME
 @retval 0x0004 MMPF_VIDENC_FW_STATUS_STOP
 @note The return value can not be changed because it sync with the host video
 status definitions.
*/
MMP_USHORT MMPF_VIDENC_GetStatus(MMP_UBYTE ubEncId)
{
    MMPF_VIDENC_INSTANCE *pEncInst = MMPF_VIDENC_GetInstance(ubEncId);

    switch (get_vidinst_format(pEncInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        return MMPF_H264ENC_GetStatus(&(pEncInst->h264e));
    default:
        return MMPF_VIDENC_FW_STATUS_ERROR;
    }
}

/**
 @brief STOP operation of video encoder.

 This function stops video engine. It also disables the audio engine and sets the
 end frame being done.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPF_VIDENC_Stop(MMP_UBYTE ubEncId)
{
    MMPF_VIDENC_INSTANCE *pEncInst = MMPF_VIDENC_GetInstance(ubEncId);
    MMP_ULONG timeout = 0;

    switch (get_vidinst_format(pEncInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        MMPF_H264ENC_Stop(&(pEncInst->h264e));
        break;
    default:
        return MMP_VIDE_ERR_INVAL_PARAM;
    }

    if (MMPF_VIDENC_GetStatus(ubEncId) == MMPF_VIDENC_FW_STATUS_PAUSE) {
        MMPF_VIDENC_Abort(ubEncId);
    }

    while (MMPF_VIDENC_GetStatus(ubEncId) != MMPF_VIDENC_FW_STATUS_STOP) {
        MMPF_OS_Sleep_MS(8);
        if (timeout++ > 500) {
            break;
        }
    }
    if (timeout > 500) {
        RTNA_DBG_Str(0, "Stop encode timeout\n");
        MMPF_VIDENC_Abort(ubEncId);
    }

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIDENC_Start (MMP_UBYTE ubEncId)
{
    MMPF_VIDENC_INSTANCE *pEncInst = MMPF_VIDENC_GetInstance(ubEncId);

    switch (get_vidinst_format(pEncInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        return MMPF_H264ENC_Start(&(pEncInst->h264e));
    default:
        return MMP_VIDE_ERR_INVAL_OP;
    }
}

void MMPF_VIDENC_Abort(MMP_UBYTE ubEncId)
{
    MMPF_VIDENC_INSTANCE *pEncInst = MMPF_VIDENC_GetInstance(ubEncId);

    switch (get_vidinst_format(pEncInst)) {
    case MMPF_VIDENC_FORMAT_H264:
        MMPF_H264ENC_Abort(&(pEncInst->h264e));
        return;
    default:
        return;
    }
}
#endif

#if (OS_TYPE == OS_UCOSII)
/**
 @brief Main routine of video recorder task.
*/
void VIDENC_Task(void *p_arg)
{
	MMPF_OS_FLAGS flags;
    MMPF_OS_FLAGS waitFlags;

	RTNA_DBG_Str(0, "VIDENC_Task ()\r\n");

    waitFlags = (SYS_FLAG_ENC_FRM_0 | SYS_FLAG_ENC_FRM_1);

    while (TRUE) {
        MMPF_OS_WaitFlags(SYS_Flag_Hif, waitFlags,
                         (MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME),
                         0, &flags);
        if (flags & SYS_FLAG_ENC_FRM_0) {
            MMPF_H264ENC_TriggerFrameDone(MMPF_H264ENC_GetHandle(0), MMPF_OS_LOCK_CTX_TASK);
        }
        if (flags & SYS_FLAG_ENC_FRM_1) {
            MMPF_H264ENC_TriggerFrameDone(MMPF_H264ENC_GetHandle(1), MMPF_OS_LOCK_CTX_TASK);
        }
    }
    return;
}
#endif

/// @}
/// @end_ait_only
