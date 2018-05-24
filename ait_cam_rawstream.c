
#include "includes_fw.h"
#include "mmpf_sensor.h"
#include "mmpf_vif.h"
#include "mmpf_vidbuf.h"
#include "mmpf_videnc.h"
#include "mmpf_h264enc.h"
#include "mmpf_jpeg.h"
#include "mmpf_system.h"
#include "isp_if.h"

#include "ait_cam_common.h"
#include "ait_cam_ctls.h"
#include "ait_cam_v4l2.h"

extern MMPF_OS_TIME_UNIT   aitcam_clockbase(void);
int aitth_rawstream_store (struct aitcam_ctx *ctx);

#if AITCAM_IPC_EN==0
static void aitth_rawstream_restart (void *arg)
{
    struct aitcam_ctx *ctx = arg;

    MMPF_IBC_SetInterruptEnable(ctx->pipe_link.ibcpipeID,
            MMPF_IBC_EVENT_FRM_ST, MMP_FALSE);
    MMPF_IBC_UnRegisterEventAction(ctx->pipe_link.ibcpipeID,
            MMPF_IBC_EVENT_FRM_ST);

    aitth_rawstream_store(ctx);
}

static void aitth_rawstream_handle_frame (void *arg)
{
    struct aitcam_ctx *ctx = arg;
    MMP_ULONG64   timestamp = 0;

    MMPF_IBC_SetStoreEnable(ctx->pipe_link.ibcpipeID, MMP_FALSE); //temp for non-single store

    if (ctx->RawStreamCtx.status == AITCAM_STREAM_START) {

        MMPF_OS_GetTimestamp(&timestamp, aitcam_clockbase());

        MMPF_Video_FillPayloadHeader(
                (MMP_UBYTE *)MMPF_Video_CurWrPtr(ctx->num),
                ctx->RawStreamCtx.frame_size,
                0/*ulFrameCount*/,
                0/*flag*/,
                timestamp,
                ctx->img_width, ctx->img_height,
                0/*FPS*/,
                ctx->num
        );

        MMPF_Video_UpdateWrPtr(ctx->num);

        MMPF_Video_SignalFrameDone(ctx->num);

        aitth_rawstream_store(ctx);
    }
}

static void aitth_rawstream_close (void *arg)
{
    struct aitcam_ctx *ctx = arg;

    MMPF_Scaler_SetEnable(ctx->pipe_link.scalerpath, MMP_FALSE);

    //MMPF_IBC_SetStoreEnable(ctx->pipe_link.ibcpipeID, MMP_FALSE);

    MMPF_IBC_SetInterruptEnable(ctx->pipe_link.ibcpipeID, MMPF_IBC_EVENT_FRM_RDY, MMP_FALSE);

    MMPF_IBC_SetInterruptEnable(ctx->pipe_link.ibcpipeID, MMPF_IBC_EVENT_FRM_END, MMP_FALSE);

    MMPF_IBC_UnRegisterEventAction(ctx->pipe_link.ibcpipeID, MMPF_IBC_EVENT_FRM_END);

    MMPF_IBC_UnRegisterEventAction(ctx->pipe_link.ibcpipeID, MMPF_IBC_EVENT_FRM_RDY);

    ctx->RawStreamCtx.status = AITCAM_STREAM_NONE;
}

int aitth_rawstream_store (struct aitcam_ctx *ctx)
{
    MMP_ULONG ulMaxFreeSize;

    if (ctx->RawStreamCtx.operation != AITCAM_STREAM_START) {
        ctx->RawStreamCtx.status = ctx->RawStreamCtx.operation;
        return 0;
    }

    ulMaxFreeSize = MMPF_Video_GetFreeSize(ctx->num);
    ulMaxFreeSize = (ulMaxFreeSize > VIDBUF_FRAME_HDR_RESV)?
                        (ulMaxFreeSize - VIDBUF_FRAME_HDR_RESV): 0;

        //dbg_printf(0, "MFS=x%x\n", ulMaxFreeSize);

    if (ulMaxFreeSize == 0) { //try_fmt checked the size
        MMPF_OS_EVENT_ACTION act = {
            .Exec = aitth_rawstream_restart,
            .Arg = ctx
        };

        MMPF_IBC_RegisterEventAction(ctx->pipe_link.ibcpipeID,
                MMPF_IBC_EVENT_FRM_ST, &act);
        MMPF_IBC_SetInterruptEnable(ctx->pipe_link.ibcpipeID,
                MMPF_IBC_EVENT_FRM_ST, MMP_TRUE);
        return 0;
    }

    ctx->RawStreamCtx.frame_addr[0] = (MMP_ULONG)MMPF_Video_CurWrPtr(ctx->num)
                                        + VIDBUF_FRAME_HDR_RESV;
    switch (ctx->output_format) {
    case V4L2_PIX_FMT_YUV420:
    {
        int ysize = ctx->img_width * ctx->img_height;
        ctx->RawStreamCtx.frame_addr[1] = ctx->RawStreamCtx.frame_addr[0]
                                            + ysize;
        ctx->RawStreamCtx.frame_addr[2] = ctx->RawStreamCtx.frame_addr[1]
                                            + (ysize >> 2);
        break;
    }
    case V4L2_PIX_FMT_GREY:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
    default:
        ctx->RawStreamCtx.frame_addr[1] = 0;
        ctx->RawStreamCtx.frame_addr[2] = 0;
        break;
    }

    MMPF_IBC_SetStoreBuffer(ctx->pipe_link.ibcpipeID,
            ctx->RawStreamCtx.frame_addr[0], ctx->RawStreamCtx.frame_addr[1],
            ctx->RawStreamCtx.frame_addr[2]);

    MMPF_IBC_SetStoreEnable(ctx->pipe_link.ibcpipeID, MMP_TRUE);

        //dbg_printf(0, "Y[%d]=x%x\n", ctx->num, ctx->RawStreamCtx.frame_addr[0]);

    return 0;
}


int ait_rawstream_ioctl (struct aitcam_ctx *ctx, struct aitcam_stream_param *param)
{
    int ret = 0;

    switch (ctx->output_format) {
    case V4L2_PIX_FMT_GREY:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
        break;
    default:
        ret = -EINVAL;
        goto RAWSTR_ERR_RET;
    }

    switch (param->cmd) {
    case AITCAM_STREAM_S_START:
    {
        MMPF_OS_EVENT_ACTION act;

        if (ctx->RawStreamCtx.status == AITCAM_STREAM_START) {
            goto RAWSTR_ERR_RET;
        }

        act.Exec = aitth_rawstream_handle_frame;
        act.Arg = ctx;
        MMPF_IBC_RegisterEventAction(ctx->pipe_link.ibcpipeID,
                MMPF_IBC_EVENT_FRM_RDY, &act);
        MMPF_IBC_SetInterruptEnable(ctx->pipe_link.ibcpipeID,
                MMPF_IBC_EVENT_FRM_RDY, MMP_TRUE);

        ctx->RawStreamCtx.frame_size = ait_calculate_rawdata_size(ctx->output_format,
                                        ctx->img_width, ctx->img_height);
        ctx->RawStreamCtx.operation = AITCAM_STREAM_START;
        ctx->RawStreamCtx.status = AITCAM_STREAM_START;

        MMPF_Scaler_SetEnable(ctx->pipe_link.scalerpath, MMP_TRUE);

        aitth_rawstream_store(ctx);
        break;
    }
    case AITCAM_STREAM_S_STOP:
    {
        MMPF_OS_EVENT_ACTION act;
        int timeout = 0;

        ctx->RawStreamCtx.operation = AITCAM_STREAM_STOP;
        while (ctx->RawStreamCtx.status != AITCAM_STREAM_STOP) {
            MMPF_OS_Sleep_MS(8);
            if (timeout++ > 500) {
                break;
            }
        }
        if (timeout > 500) {
            RTNA_DBG_Str(0, "Stop RAW stream timeout\n");
        }

        act.Exec = aitth_rawstream_close;
        act.Arg = ctx;
        MMPF_IBC_RegisterEventAction(ctx->pipe_link.ibcpipeID, MMPF_IBC_EVENT_FRM_END, &act);
        MMPF_IBC_SetInterruptEnable(ctx->pipe_link.ibcpipeID, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);
        while (ctx->RawStreamCtx.status != AITCAM_STREAM_NONE) {
            MMPF_OS_Sleep_MS(8);
            if (timeout++ > 500) {
                break;
            }
        }
        if (timeout > 500) {
            RTNA_DBG_Str(0, "Close RAW stream timeout\n");
        }
        break;
    }
    case AITCAM_STREAM_G_STATE:
        *(enum aitcam_stream_state *)(param->arg) = ctx->RawStreamCtx.status;
        break;
    default:
        ret = -EINVAL;
        goto RAWSTR_ERR_RET;
    }

    return ret;

RAWSTR_ERR_RET:
    return ret;
}
#endif

int ait_calculate_rawdata_size (__u32 fmt, int width, int height)
{
    switch (fmt) {
    case V4L2_PIX_FMT_YUV420:
        return ((width * height * 3) >> 1);
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_UYVY:
        return ((width * height) << 1);
    case V4L2_PIX_FMT_GREY:
        return width * height;
    default:
        return 0;
    }
}
