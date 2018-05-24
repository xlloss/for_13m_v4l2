#include <linux/errno.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/module.h>

#include <mach/mmpf_typedef.h>
#include <mach/mmp_reg_gbl.h>
#include <mach/mmp_register.h>
#include <mach/cpucomm/cpucomm_api.h>
#include <mach/cpucomm/cpucomm_if.h>
#include <mach/ait_cpu_sharemem.h>

#include "includes_fw.h"
#include "ait_cam_common.h"
#include "ait_cam_ipc.h"
#include "mmpf_vidbuf.h"
//#include "ait_cam_v4l2.h"

#define OSD_MEM_EN    (0)
#define IQ_TUNING_EN  (1)
extern int fdfr_osd ;
extern void *cpub_malloc(int size,gfp_t flags, dma_addr_t *h_dma) ;
extern void cpub_free(int size,void *va,dma_addr_t h_dma) ;


#if 1 
#define MS_FUNC(who,statement)  statement
#else
#define MS_FUNC(who,statement)				\
do {										              \
	MMP_ULONG64 t1,t2 ;                   \
	MMPF_OS_GetTimestamp(&t1,MMPF_OS_TIME_UNIT_JIFFIES);    \
	statement ;								                            \
	MMPF_OS_GetTimestamp(&t2,MMPF_OS_TIME_UNIT_JIFFIES);		\
	pr_info("%s time : %d\r\n",who,t2 - t1 );\
} while (0)
#endif
#define AITCAM_IPC_TIMEOUT		(1000)
#define COM_OBJ_SZ    sizeof(struct cpu_comm_transfer_data )
#define COM_DMA(addr)             (unsigned long)addr                                

#define COM_SEND()          do {                \
                                MS_FUNC("s:",err = CpuComm_DataSend( cpucmm_id, (MMP_UBYTE*)data, COM_OBJ_SZ,AITCAM_IPC_TIMEOUT ) );         \
                                if(! err ) {	\
                                	MS_FUNC("r:",err = CpuComm_DataReceive( cpucmm_id, (MMP_UBYTE*)data,  COM_OBJ_SZ,AITCAM_IPC_TIMEOUT,0) );   \
                                } \
                            } while(0)
                            
#define COM_PAYLOAD_ALLOC(size) do {    \
                                    payload = cpub_malloc(size,GFP_KERNEL,&h_dma);    \
                                } while (0)

#define COM_PAYLOAD_FREE(size) cpub_free(size,payload,h_dma )

#define COM_PAYLOAD_FREE_GLOBAL(size,payload,h_dma) cpub_free(size,payload,h_dma )

static int cmd_seq_cnt ;
static CPU_COMM_ID cpucmm_id = -1;

static int debug = 0 ;
#if 1//OSD_MEM_EN==1
static void *m_osd_ipc_virt_addr = 0 ;
static dma_addr_t  m_osd_ipc_phy_addr ;
#endif

static	void *m_ipc_virt_addr  = 0;
static	dma_addr_t m_ipc_phy_addr ;

static void *m_iq_ipc_virt_addr    = 0 ;
static dma_addr_t m_iq_ipc_phy_addr= 0 ;


struct ait_fdframe_config_ipc *aitcam_ipc_osd_base(void)
{
    return (struct ait_fdframe_config_ipc *)m_osd_ipc_virt_addr ;
}

int aitcam_ipc_get_jpg(struct ait_vidbuf_info *jpg_info)
{

  CPU_COMM_ERR err ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
	if(debug) {
    pr_info("aitcam_ipc_get_jpg\n");
  }
  
	data->command 	= IPC_V4L2_GET_JPG;
	data->phy_addr	= 0;
	data->size			= 0;
	data->seq			= cmd_seq_cnt++;
	data->flag		= CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	if(data->result) {
	  pr_info("aitcam_ipc_get_jpg result:%lu\n", data->result);
	  return -EINVAL ; 
	}
	else {
	  jpg_info->buf_addr = (unsigned long)AIT_RAM_P2V( data->phy_addr ) ;
	  jpg_info->buf_size = data->size ;
	  jpg_info->buf_num  = 1 ;  
	}
  return 0;
}


int aitcam_ipc_open(aitcam_img_info *img_info)
{
  //dma_addr_t h_dma ;
  CPU_COMM_ERR err ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
	void *payload ;
	dma_addr_t h_dma ;
	if(debug) {
    pr_info("aitcam_ipc_open\n");
  }
  COM_PAYLOAD_ALLOC(sizeof(aitcam_img_info)) ;
  if(!payload) {
    return -ENOMEM ;  
  }
  
  *(aitcam_img_info *)payload = *img_info ;
	data->command 	= IPC_V4L2_OPEN;
	data->phy_addr	= COM_DMA(h_dma);
	data->size			= sizeof(aitcam_img_info);
	data->seq			= cmd_seq_cnt++;
	data->flag		= CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	COM_PAYLOAD_FREE(sizeof(aitcam_img_info)) ; // BUG : 2017_05_02 add 
	if (err ) {
	  return -EINVAL ;
	}
	if(data->result) {
	  pr_info("aitcam_ipc_open result:%lu\n", data->result);
	  return -EINVAL ; 
	}
	
	return 0;	  
}

int aitcam_ipc_streamon ( aitcam_ipc_info *ipc_info) 
{
  CPU_COMM_ERR err ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
	void *payload ;
	dma_addr_t h_dma ;
	
	ipc_info->vbq_phy_addr  =  (unsigned long)MMPF_VIDBUF_GetHandlePhy(ipc_info->img_info.streamid) ;
	ipc_info->vbq_virt_addr =  (unsigned long)MMPF_VIDBUF_GetHandle(ipc_info->img_info.streamid)    ;
	ipc_info->remote_q_size = sizeof(MMPF_VIDBUF_QUEUE) ;
	if(debug) {
    pr_info("aitcam_ipc_streamon : streamid: %d,(w=%d,h=%d),fmt:0x%08lx\n",ipc_info->img_info.streamid,ipc_info->img_info.img_width,ipc_info->img_info.img_height,ipc_info->img_info.img_fmt);
    pr_info("aitcam_ipc_streamon : queue(phy,virt):(0x%08lx,0x%08lx),q_size:%d\n",ipc_info->vbq_phy_addr,ipc_info->vbq_virt_addr,ipc_info->remote_q_size);
  }
  
  COM_PAYLOAD_ALLOC(sizeof(aitcam_ipc_info)) ;
  if(!payload) {
    return -ENOMEM ;  
  }
  *(aitcam_ipc_info *)payload = *ipc_info ;
	data->command 	= IPC_V4L2_STREAMON;
	data->phy_addr	= COM_DMA(h_dma);
	data->size			= sizeof(aitcam_ipc_info);
	data->seq			= cmd_seq_cnt++;
	data->flag		= CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	COM_PAYLOAD_FREE(sizeof(aitcam_ipc_info)) ;
	if (err ) {
	  return -EINVAL ;
	}

	if(data->result) {
	  pr_info("ipc_streamon result:%lu\n", data->result);
	  return -EINVAL ; 
	}
	
	return 0;	  
}


int aitcam_ipc_streamoff( int streamid ) 
{
  CPU_COMM_ERR err ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
	if(debug)
  pr_info("aitcam_ipc_streamoff\n");
  
	data->command 	= IPC_V4L2_STREAMOFF;
	data->phy_addr	= streamid;
	data->size			= 0;
	data->seq			= cmd_seq_cnt++;
	data->flag		= CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	if (err ) {
	  return -EINVAL ;
	}
	if(data->result) {
	  pr_info("aitcam_ipc_streamoff result:%lu\n", data->result);
	  return -EINVAL ; 
	}
	return 0;	 
}

int aitcam_ipc_release(int streamid,void *priv_data) 
{
  CPU_COMM_ERR err ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
	if(debug)
  pr_info("aitcam_ipc_release\n");
  
	data->command 	= IPC_V4L2_RELEASE;
	data->phy_addr	= streamid;
	data->size			= 0;
	data->seq			= cmd_seq_cnt++;
	data->flag		= CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	if (err ) {
	  return -EINVAL ;
	}
	if(data->result) {
	  pr_info("aitcam_ipc_release result:%lu\n", data->result);
	  return -EINVAL ; 
	}
	if(data->size&&priv_data) {
	  //int i ;
	  unsigned int *remotedata = (unsigned int  *)AIT_RAM_P2V(data->phy_addr) ; 
	  //for( i = 0 ; i < 7 ; i++) {   
	  //  pr_info("isp_data->size : %d , [%d] = %d\n",data->size,i,remotedata[i] );
	  //}
	  memcpy(priv_data,remotedata,data->size) ;
	}
	return 0;	  
}

int aitcam_ipc_set_ctrl(aitcam_ipc_ctrl *ctrl,aitcm_ipc_ctrl_dir dir)
{
  int ret = 0 ;
  CPU_COMM_ERR err ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
	void *payload ;
	dma_addr_t h_dma ;
	if(debug) {
	  pr_info("ipc_ctrl[%d] : name:%s,id:0x%08lx\n", ctrl->streamid,ctrl->name,ctrl->id);
  }
  COM_PAYLOAD_ALLOC(sizeof(aitcam_ipc_ctrl)) ;
  if(!payload) {
    return -ENOMEM ;
  }
  
  *(aitcam_ipc_ctrl *)payload = *ctrl ;
	data->command 	= IPC_V4L2_CTRL | (dir << 31 );
	data->phy_addr	= COM_DMA(h_dma);
	data->size			= sizeof(aitcam_ipc_ctrl);
	data->seq			= cmd_seq_cnt++;
	data->flag		= CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	if (err ) {
	  ret = -EINVAL ;
	  goto quit ;
	}

	if(data->result) {
	  pr_info("ipc_streamon result:%lu\n", data->result);
	  ret = -EINVAL ; 
	  goto quit ;
	}
	
	if( dir == IPC_V4L2_GET ) {
	  ctrl->val =  data->phy_addr ; 
	}
	
quit:
	COM_PAYLOAD_FREE(sizeof(aitcam_ipc_ctrl)) ;
  	
	return ret ;	        
}

//extern int register_recv_intr_callback( void (*ipc_intr_recv_callback)(unsigned char int_src) );
static  unsigned int aitcam_ipc_frame_done(void *slot)
{
  
  struct cpu_share_mem_slot *r_slot = (struct cpu_share_mem_slot *)slot ;
  if(! r_slot ) {
    pr_info("#unknow slot isr\n");
    return 0;  
  }
  pr_debug("#frame_done(dev_id:%d),slot : 0x%08x,streamid : %d,%d\n", r_slot->dev_id,(u32)r_slot,(int)r_slot->send_parm[0],(int)r_slot->recv_parm[0] );
  if(r_slot->dev_id==CPU_COMM_ID_V4L2) {
    switch (r_slot->command) 
    {
      case IPC_V4L2_ISR:
      MMPF_Video_SignalFrameDone((MMP_UBYTE)r_slot->send_parm[0]) ;  
      break ;
    }
  }
  return 0;
}

ISP_CMD_STATUS ISP_IF_APTUNING_Para(EZ_MODE_IQ_DATA_BUF *iq)
{
  CPU_COMM_ERR err =0 ;
	struct cpu_comm_transfer_data __data,*data=&__data ;
  
  EZ_MODE_IQ_DATA_BUF *iq_ipc = (EZ_MODE_IQ_DATA_BUF *)m_iq_ipc_virt_addr ;
  char *iq_data_ipc = (char *)m_iq_ipc_virt_addr   ;
  char *iq_data     = (char *)iq  ;
  if(!iq_ipc || !iq ) {
    return ISP_CMD_STATUS_FAIL ;    
  }  
  /*
  pr_info("----iq : %x,iq->type:%d,%d,buf_size:%x,buf_ptr:%x\n",(unsigned long)iq,iq->cmd_type,iq->data_length,iq->buf_ptr_size,(unsigned long)iq->buf_ptr);
  pr_info("----iq_data_ipc : %x\n",(unsigned long)iq_data_ipc);
  */
  if (iq->data_length  != (iq->buf_ptr_size + EZ_IQ_HEADER_SIZE) ) {
    pr_info("[EZIQ],err : data_length : %d ,buf_ptr_size :%d\r\n",iq->data_length,iq->buf_ptr_size );
  }
  
  //*iq_ipc = *iq ;
  memcpy( iq_data_ipc, iq_data,iq->data_length );
	data->command 	= IPC_IQ_TUNING ;
	data->phy_addr	= COM_DMA(m_iq_ipc_phy_addr);
	data->size			= 4096 ;
	data->seq			  = cmd_seq_cnt++;
	data->flag		  = CPUCOMM_FLAG_CMDSND; 
	COM_SEND();
	if (err ) {
	  return -EINVAL ;
	}
	if(data->result) {
	  pr_info("ISP_IF_APTUNING_Para:%lu\n", data->result);
	  return -EINVAL ; 
	}
	
#if 0
  pr_info(" cmd_type     : %d\n",iq->cmd_type    );
	pr_info(" data_length  : %d\n",iq->data_length );
	pr_info(" sysmode      : %d\n",iq->sysmode     );
	pr_info(" page         : %d\n",iq->page        );
	pr_info(" type         : %d\n",iq->type        );
	pr_info(" index        : %d\n",iq->index       );
	pr_info(" buf_ptr_size : %d\n",iq->buf_ptr_size);
#endif
	
	if(iq->cmd_type==1) {
	  memcpy( iq_data, iq_data_ipc,iq->data_length );
	}
	
	return 0;	          
}
EXPORT_SYMBOL(ISP_IF_APTUNING_Para);

int aitcam_ipc_register(int debug_en)
{
#if 1 //OSD_MEM_EN
  struct _cpu_rtos_mem_info *mem ;
#endif
	void *payload ;
	dma_addr_t h_dma ;
  
	CPU_COMM_ERR ret;
	ret = CpuComm_RegisterEntry(CPU_COMM_ID_V4L2, CPU_COMM_TYPE_DATA);
	if(ret==CPU_COMM_ERR_NONE) {
		cpucmm_id = CPU_COMM_ID_V4L2;
		COM_PAYLOAD_ALLOC( sizeof(MMPF_VIDBUF_QUEUE) * AITCAM_NUM_CONTEXTS ) ;
		if(!payload) {
		  cpucmm_id =  (CPU_COMM_ID)-1 ;   
		  return -ENOMEM ; 
		}
		
		ret = CpuComm_RegisterISRService( CPU_COMM_ID_V4L2,aitcam_ipc_frame_done);
		pr_info("#Enable V4L2 ISR Service ret:%d\n",ret);
    MMPF_VIDBUF_SetQueueBase(payload,(unsigned long)h_dma);  
    m_ipc_virt_addr = payload ;
    m_ipc_phy_addr  = h_dma   ;
    
#if OSD_MEM_EN
  		COM_PAYLOAD_ALLOC( sizeof(struct ait_fdframe_config_ipc )*MAX_OSD_INDEX + sizeof(struct ait_rtc_config_ipc) + sizeof(struct ait_osd_config_ipc )*2) ;
  		if(!payload) {
  		  COM_PAYLOAD_FREE(sizeof(MMPF_VIDBUF_QUEUE) * AITCAM_NUM_CONTEXTS ) ;
  		  cpucmm_id =  (CPU_COMM_ID)-1 ;   
  		  return -ENOMEM ; 
  		}
      m_osd_ipc_virt_addr = payload ;
      m_osd_ipc_phy_addr  = (unsigned long)(h_dma);    
      mem = (struct _cpu_rtos_mem_info *)&AITC_BASE_CPU_SAHRE->CPU_SAHRE_REG[48] ;
      mem->osd_base2rtos = m_osd_ipc_phy_addr ;
#else
      mem = get_cpub_rtos_mem();
      m_osd_ipc_phy_addr = mem->osd_base2rtos ;
      m_osd_ipc_virt_addr = (void *)AIT_RAM_P2V(m_osd_ipc_phy_addr) ;    
#endif  

#if IQ_TUNING_EN
  		COM_PAYLOAD_ALLOC( 4096 ) ;
  		m_iq_ipc_virt_addr = payload ;
  		m_iq_ipc_phy_addr  = (unsigned long)(h_dma);
#endif

	}	
	else {
	  cpucmm_id = (CPU_COMM_ID)-1 ;  
	}
	debug = debug_en ;
	return ret;
}

int aitcam_ipc_unregister(void)
{
  if(cpucmm_id != ( (CPU_COMM_ID)-1)) {
    COM_PAYLOAD_FREE_GLOBAL(sizeof(MMPF_VIDBUF_QUEUE) * AITCAM_NUM_CONTEXTS   ,m_ipc_virt_addr , m_ipc_phy_addr ) ;
#if OSD_MEM_EN
      COM_PAYLOAD_FREE_GLOBAL(sizeof(struct ait_fdframe_config_ipc )*MAX_OSD_INDEX + sizeof(struct ait_rtc_config_ipc) + sizeof(struct ait_osd_config_ipc )*2
              ,m_osd_ipc_virt_addr , m_osd_ipc_phy_addr ) ;
#endif    
#if IQ_TUNING_EN
    if(m_iq_ipc_virt_addr) {
      COM_PAYLOAD_FREE_GLOBAL(4096  ,m_iq_ipc_virt_addr , m_iq_ipc_phy_addr ) ;
    }
#endif
	  CpuComm_UnregisterEntry(cpucmm_id);
  }
	return 0;
}

