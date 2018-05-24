
#include "includes_fw.h"

#include "mmpf_vidmgr.h"
#include "mmpf_videnc.h"
#include "mmpf_vidbuf.h"
#include "ait_cam_common.h"

#if AITCAM_IPC_EN
#include <mach/cpucomm/cpucomm_if.h>
#else
#define  CPU_LOCK_INIT()
#define  CPU_LOCK()
#define  CPU_UNLOCK()
#endif


static MMPF_VIDBUF_QUEUE m_vbqueue[AITCAM_NUM_CONTEXTS];
static void *m_vbqueue_ipc_virt_addr = 0 ;
static unsigned long   m_vbqueue_ipc_phy_addr ;

void MMPF_VIDBUF_SetQueueBase(void *base_virt,unsigned long base_phy)
{
  m_vbqueue_ipc_virt_addr = base_virt ;
  m_vbqueue_ipc_phy_addr  = base_phy  ;
  #if 0
  dbg_printf(0,"[VIDBUF]IPC.Queue Base(virt,phy)=0x%08x,0x%08x\n",(unsigned int)base_virt,(unsigned int)base_phy);
  
  dbg_printf(0,"sizeof(MMPF_VIDBUF_QUEUE) : %d\n",sizeof(MMPF_VIDBUF_QUEUE) );
  dbg_printf(0,"sizeof(MMPF_VIDBUF_INFO ) : %d\n",sizeof(MMPF_VIDBUF_INFO ) );
  dbg_printf(0,"sizeof(MMPF_VIDENC_QUEUE) : %d\n",sizeof(MMPF_VIDENC_QUEUE) );
  dbg_printf(0,"sizeof(wait_queue_head_t) : %d\n",sizeof(wait_queue_head_t) );
  dbg_printf(0,"sizeof(struct semaphore ) : %d\n",sizeof(struct semaphore ) );
  #endif
  
}

void MMPF_VIDBUF_Initialize(MMP_UBYTE pipe, MMP_ULONG buf_addr,
                                MMP_ULONG buf_num, MMP_ULONG buf_size)
{
    MMP_ULONG i;
    MMPF_VIDBUF_QUEUE *queue ;//= MMPF_VIDBUF_GetHandle(pipe) ;

    if ((pipe >= AITCAM_NUM_CONTEXTS) || (buf_num > MMPF_VIDENC_MAX_QUEUE_SIZE)) {
        return;
    }

    queue = MMPF_VIDBUF_GetHandle(pipe) ;
    
    queue->buf_addr = buf_addr;
    queue->buf_num = buf_num;
    queue->buf_size = buf_size;

    queue->free_vbq.head = 0;
    queue->free_vbq.size = 0;
    queue->ready_vbq.head = 0;
    queue->ready_vbq.size = 0;

	//init_waitqueue_head(&m_vbqueue[pipe].free_waitq);
    init_waitqueue_head(&queue->ready_waitq);

    for (i = 0; i < buf_num; i++) {
        queue->buffers[i].buf_start = buf_addr + i*buf_size;
        queue->buffers[i].used_size = 0;
        queue->buffers[i].timestamp = 0;
        queue->buffers[i].flags = VIDBUF_STATE_CLEAR;
    }
}

void MMPF_VIDBUF_Reset(MMP_UBYTE pipe)
{
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;
    int i ;
    if(queue) {
        queue->free_vbq.head = 0;
        queue->free_vbq.size = 0;
        queue->ready_vbq.head = 0;
        queue->ready_vbq.size = 0;
        //init_waitqueue_head(&queue->ready_waitq);
        for (i = 0; i < queue->buf_num; i++) {
            queue->buffers[i].buf_start = queue->buf_addr + i*queue->buf_size;
            queue->buffers[i].used_size = 0;
            queue->buffers[i].timestamp = 0;
            queue->buffers[i].flags = VIDBUF_STATE_CLEAR;
        }
    
    }
}

void *MMPF_VIDBUF_GetHandle(MMP_UBYTE pipe)
{
    if (pipe >= AITCAM_NUM_CONTEXTS) {
        return NULL;
    }
    if(m_vbqueue_ipc_virt_addr) {
        MMPF_VIDBUF_QUEUE *queue = (MMPF_VIDBUF_QUEUE *)m_vbqueue_ipc_virt_addr ;
        return (void *) (queue + pipe );
    }
    return &m_vbqueue[pipe];
}

void *MMPF_VIDBUF_GetHandlePhy(MMP_UBYTE pipe)
{
    if (pipe >= AITCAM_NUM_CONTEXTS) {
        return NULL;
    }
    if(m_vbqueue_ipc_phy_addr) {
        MMPF_VIDBUF_QUEUE *queue = (MMPF_VIDBUF_QUEUE *)m_vbqueue_ipc_phy_addr ;
        return (void *) (queue + pipe );
    }
    return 0;
}

void MMPF_VIDBUF_PushVBQ(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE q_select,
                         MMP_UBYTE buf_id)
{
    if (buf_id >= pVbq->buf_num)
        return;

    if (q_select == VIDBUF_FREE_QUEUE) {
        MMPF_VIDENC_PushQueue(&pVbq->free_vbq, buf_id);
    } else {
        MMPF_VIDENC_PushQueue(&pVbq->ready_vbq, buf_id);
    }

    return;
}

MMP_UBYTE MMPF_VIDBUF_PopVBQ(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE q_select)
{
    if (q_select == VIDBUF_FREE_QUEUE) {
        return MMPF_VIDENC_PopQueue(&pVbq->free_vbq, 0);
    } else {
        return MMPF_VIDENC_PopQueue(&pVbq->ready_vbq, 0);
    }
}

MMP_ERR MMPF_VIDBUF_SetState(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE buf_id, MMP_ULONG flags, MMP_BOOL bSet)
{
    if (!pVbq || (buf_id >= pVbq->buf_num)) {
        return MMP_VIDBUF_ERR_PARAM;
    }

    if (bSet) {
        pVbq->buffers[buf_id].flags |= flags;
    } else {
        pVbq->buffers[buf_id].flags &= ~flags;
    }

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIDBUF_GetState(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE buf_id, MMP_ULONG *flags)
{
    if (!pVbq || (buf_id >= pVbq->buf_num)) {
        return MMP_VIDBUF_ERR_PARAM;
    }

    *flags = pVbq->buffers[buf_id].flags;

    return MMP_ERR_NONE;
}

MMP_ULONG MMPF_VIDBUF_GetDepth(MMPF_VIDBUF_QUEUE *pVbq, MMP_UBYTE q_select)
{
    if (!pVbq) {
        return MMP_VIDBUF_ERR_PARAM;
    }

    if (q_select == VIDBUF_FREE_QUEUE) {
        return pVbq->free_vbq.size;
    }
    else {
        return pVbq->ready_vbq.size;
    }
}

void MMPF_Video_SignalFrameDone(MMP_UBYTE pipe)
{
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;
    if(queue) {
      aitcam_queue_wakeup(queue);
    }
    return;
}

void* MMPF_Video_CurWrPtr(MMP_UBYTE pipe)
{
    MMP_UBYTE head;
    void *cur_wr ;
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;
    CPU_LOCK_INIT() ;
    if (pipe >= AITCAM_NUM_CONTEXTS)
        return 0;
    CPU_LOCK();
    head = queue->free_vbq.buffers[queue->free_vbq.head];
    cur_wr =  (void*)(queue->buffers[head].buf_start);
    CPU_UNLOCK();
    return cur_wr ;
}

void MMPF_Video_UpdateWrPtr(MMP_UBYTE pipe)
{
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;
    CPU_LOCK_INIT() ;
    if (pipe >= AITCAM_NUM_CONTEXTS)
        return;
    CPU_LOCK();
    MMPF_VIDBUF_PushVBQ(queue, VIDBUF_READY_QUEUE,
                       MMPF_VIDBUF_PopVBQ(queue, VIDBUF_FREE_QUEUE));
    CPU_UNLOCK();
                       
    return;
}

MMP_ULONG MMPF_Video_GetFreeSize(MMP_UBYTE pipe)
{
    MMP_ULONG free_size ;
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;
    CPU_LOCK_INIT() ;
    if (pipe >= AITCAM_NUM_CONTEXTS)
        return 0;
    CPU_LOCK();
    if (queue->free_vbq.size) {
        free_size = queue->buf_size;
    }
    else {
        free_size = 0;
    }
    CPU_UNLOCK();
    return free_size ;
}

MMP_BOOL MMPF_Video_IsFull(MMP_UBYTE pipe)
{
    return (MMPF_Video_GetFreeSize(pipe) == 0);
}

void MMPF_Video_GetBufBound(MMP_UBYTE pipe, MMP_ULONG* low, MMP_ULONG* high)
{
    MMP_UBYTE head;
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;

    if (pipe >= AITCAM_NUM_CONTEXTS)
        return;
    if (queue->free_vbq.size == 0) {
        *low = 0;
        *high = 0;
        return;
    }

    head = queue->free_vbq.buffers[queue->free_vbq.head];

    *low    = queue->buffers[head].buf_start;
    *high   = queue->buffers[head].buf_start + queue->buf_size;

    return;
}

void MMPF_Video_GetVidBufQueueInfo(MMP_UBYTE pipe, MMP_ULONG *addr,
                                    MMP_ULONG *buf_size, MMP_ULONG *buf_num)
{
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(pipe) ;
    if (pipe >= AITCAM_NUM_CONTEXTS) {
        *addr = 0;
        *buf_size = 0;
        *buf_num = 0;
        return;
    }
    *addr       = queue->buf_addr;
    *buf_size   = queue->buf_size;
    *buf_num    = queue->buf_num;
    return;
}

void MMPF_Video_FillPayloadHeader(MMP_UBYTE *frame_ptr, MMP_ULONG framelength,
                              MMP_ULONG frameseq, MMP_ULONG flag,
                              MMP_ULONG64 timestamp, MMP_USHORT w, MMP_USHORT h,
                              MMP_USHORT framerate, MMP_UBYTE ubPipe)
{
    MMP_UBYTE head;
    MMPF_VIDBUF_QUEUE *queue = MMPF_VIDBUF_GetHandle(ubPipe) ;
    CPU_LOCK_INIT();
    if (ubPipe >= AITCAM_NUM_CONTEXTS) {
        return;
    }
    CPU_LOCK();
    head = queue->free_vbq.buffers[queue->free_vbq.head];
    queue->buffers[head].used_size = framelength;
    queue->buffers[head].timestamp = timestamp;
    CPU_UNLOCK();
    #if (VIDBUF_FRAME_HDR_RESV > 0)
    #endif

    return;
}

