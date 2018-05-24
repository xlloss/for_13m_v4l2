
#ifndef _MMPF_VIDBUF_H_
#define _MMPF_VIDBUF_H_

#include <linux/wait.h>
#include <linux/semaphore.h>
#include "mmpf_vidcmn.h"

#define     VIDBUF_READY_QUEUE      (0)
#define     VIDBUF_FREE_QUEUE       (1)

#define     VIDBUF_STATE_CLEAR      (0x00000000)
#define     VIDBUF_STATE_LOCKED     (0x00000001)


#define     VIDBUF_FRAME_HDR_RESV   (0)


/*
changed to packed for Linux Action Cam
*/
typedef packed_struct _MMPF_VIDBUF_INFO {
    MMP_ULONG used_size;
    MMP_ULONG buf_start;
    MMP_ULONG64 timestamp;
    MMP_ULONG flags;
} MMPF_VIDBUF_INFO;

/*
changed to packed for Linux Action Cam
*/
typedef packed_struct _MMPF_VIDBUF_QUEUE {
    MMP_ULONG   buf_num, buf_size;
    MMP_ULONG   buf_addr;

    MMPF_VIDBUF_INFO buffers[MMPF_VIDENC_MAX_QUEUE_SIZE];

    MMPF_VIDENC_QUEUE   free_vbq, ready_vbq;
    wait_queue_head_t   /*free_waitq, */ready_waitq;
    
    // mutex lock for blocking mode
	struct semaphore    vbq_lock ;
    int             non_blocking ;
} MMPF_VIDBUF_QUEUE;

void MMPF_VIDBUF_SetQueueBase(void *base_virt,unsigned long base_phy);
void        MMPF_VIDBUF_Reset(MMP_UBYTE pipe);
void        MMPF_VIDBUF_Initialize(MMP_UBYTE pipe, MMP_ULONG buf_addr,
                                   MMP_ULONG buf_num, MMP_ULONG buf_size);
void*       MMPF_VIDBUF_GetHandle(MMP_UBYTE pipe);
void*       MMPF_VIDBUF_GetHandlePhy(MMP_UBYTE pipe);

void        MMPF_VIDBUF_PushVBQ(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE q_select, MMP_UBYTE buf_id);
MMP_UBYTE   MMPF_VIDBUF_PopVBQ(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE q_select);
MMP_ERR     MMPF_VIDBUF_SetState(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE buf_id, MMP_ULONG flags, MMP_BOOL bSet);
MMP_ERR     MMPF_VIDBUF_GetState(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE buf_id, MMP_ULONG *flags);
MMP_ULONG   MMPF_VIDBUF_GetDepth(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE q_select);


void        MMPF_Video_SignalFrameDone(MMP_UBYTE);
void*       MMPF_Video_CurWrPtr(MMP_UBYTE pipe);
void        MMPF_Video_UpdateWrPtr(MMP_UBYTE pipe);
MMP_ULONG   MMPF_Video_GetFreeSize(MMP_UBYTE pipe);
MMP_BOOL    MMPF_Video_IsFull(MMP_UBYTE pipe);
void        MMPF_Video_GetBufBound(MMP_UBYTE pipe, MMP_ULONG* low, MMP_ULONG* high);
void        MMPF_Video_GetVidBufQueueInfo(MMP_UBYTE pipe, MMP_ULONG *addr,
                                    MMP_ULONG *buf_size, MMP_ULONG *buf_num);
void        MMPF_Video_FillPayloadHeader(MMP_UBYTE *frame_ptr, MMP_ULONG framelength,
                                  MMP_ULONG frameseq, MMP_ULONG flag,
                                  MMP_ULONG64 timestamp, MMP_USHORT w, MMP_USHORT h,
                                  MMP_USHORT framerate, MMP_UBYTE ubPipe);

#endif

