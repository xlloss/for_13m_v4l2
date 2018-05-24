/*
 * * Alpha Imaging Tech. v 0.1
 *
 * This file contains definitions of enums and structs used by the camera
 * driver.
 *
 * Copyright (C) 201x AIT Co., Ltd.
 * xxx, <xxx@a-i-t.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version
 */


#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>

#include <mach/ait_if.h>
#include <mach/ait_cpu_sharemem.h>
#include <mach/cpucomm/cpucomm_if.h>
#include "includes_fw.h"
#include "reg_retina.h"

#include "mmp_reg_jpeg.h"
#include "mmp_reg_h264enc.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_mci.h"
#include "mmpf_mci.h"

#include "mmpf_vidbuf.h"
#include "mmpf_jpeg.h"
#include "mmpf_videnc.h"
#include "mmpf_vif.h"
#include "mmpf_scaler.h"
#include "hdr_cfg.h"
#include "mmpf_sensor.h"
#include "mmpf_system.h"
#include "isp_if.h"
#include "mmpf_graphics.h"
#include "mmpf_rawproc.h"
#include "os_wrap.h"

#include "ait_cam_common.h"
#include "ait_cam_ctls.h"
#include "ait_cam_v4l2.h"
//#include "ait_iq_video.h"
#if (STATISTIC_EN == 1)
#include "stream_stat.h"
#endif

#define AUTO_DETECT_RTOS_MEM          (1)

unsigned long  V4L2_BUF_SIZE = (13*1024*1024);


#if AITCAM_IPC_EN
#include "ait_cam_ipc.h"
unsigned long  RESV_FOR_RTOS_SIZE  = (38*1024*1024) ;

#else
unsigned long RESV_FOR_RTOS_SIZE = (0) ;
#endif

#define AIT_CAM_NAME        "ait-cam"
#define AIT_CAM_VER         "v1.3.6"
#ifndef AIT_DRAM_SIZE
#define AIT_DRAM_SIZE (24*1024*1024) // for lgt(12MB now) <-> ait(24MB) codebase different
#endif

__u32 isp_data_from_rtos[7] ;

int ipc_debug = 0 ;
module_param(ipc_debug, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ipc_debug, "ipc debug");

int cpffserver = 1 ;
module_param(cpffserver, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(cpffserver, "cp ffserver to tmp");


int debug_ts = 0 ;
module_param(debug_ts, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_ts, "Print out ts");


int fdfr_osd = 0 ;
module_param(fdfr_osd, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(fdfr_osd, "Force Display FDFR OSD");

int force_rdo = 1;
module_param(force_rdo, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(force_rdo, "Force RDO en");

/*
Change Mode decision when moving scene
*/
int force_md = 1 ;
module_param(force_md, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(force_md, "Force Mode Decision");

int force_tnr = 0;
module_param(force_tnr, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(force_tnr, "Force bitrate");
/*
0 : CLOCK_REALTIME
1 : CLOCK_MONOTONIC ( jiffies )
*/
int clockbase = 0 ;
module_param(clockbase, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(clockbase, "Timestamp(0:CLOCK_REALTIME,1:CLOCK_MONOTONIC");

int resv_sram_base = AIT_SRAM_PHYS_BASE;
module_param(resv_sram_base, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(resv_sram_base, "Reserve SRAM memory base address");

#if (AITCAM_REALTIME_MODE)
int resv_sram_size = 0x2dc00;
#else
int resv_sram_size = AIT_SRAM_SIZE + 0x400;
#endif
module_param(resv_sram_size, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(resv_sram_size, "Reserve SRAM memory size");

int resv_dram_base = 0;
module_param(resv_dram_base, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(resv_dram_base, "Reserve DRAM memory base address");

int resv_dram_size = AIT_DRAM_SIZE;
module_param(resv_dram_size, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(resv_dram_size, "Reserve DRAM memory size");

int debug_level = 1;
module_param(debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_level, "\nLevels:\n"
                 " [1] Basic System/Memory Info.\n"
                 " [2] V4L2 IOCTLs.\n"
                 " [3] PLL/DRAM Setting\n"
                 " [4] H264 Timing");

#if (SUPPORT_DUAL_SNR_PRV)
int dual_snr_en = 1;
module_param(dual_snr_en, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dual_snr_en, "Support dual sensor");
#endif

extern MMP_BOOL gbHDR_en;
#if (AIT_ISP_VIDEO_IQ == 1)
short cust_viq_nr_map[VIQ_NR_STEPS];
int cust_viq_nr_num;
module_param_array(cust_viq_nr_map, short, &cust_viq_nr_num, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(viq_nrstep, "IQ NR steps");

#if (LOW_LIGHT_BR_DEC == 1)
int rc_dec_ratio = 3;
module_param(rc_dec_ratio, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rc_dec_ratio, "5566");
#endif
#endif

#if (AIT_FOV_CROP_EN == 1)
const int fov_crop_base = 100;

int fov_crop_ratio = 0;
module_param(fov_crop_ratio, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(fov_crop_ratio, "Default FoV crop ratio (%)");
#endif

#if (AITCAM_SCHED_RT == 1)
int h264enc_rt_prio = 80;
module_param(h264enc_rt_prio, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(h264enc_rt_prio, "H264Enc RT priority 1 ~ MAX_RT_PRIO-1");
#endif

int cpu_ipc = 0 ;
module_param(cpu_ipc, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(cpu_ipc, "CPU IPC enable");

#ifdef AITCAM_DBG_MSG
void * enc_hdl;
struct timespec enc_start;
#endif
#define AIC_SRC_RAW                 0x06

#if (STATISTIC_EN == 1)
char             stat_en = 1;
struct stat_window      stat_win[AITCAM_NUM_CONTEXTS];
#endif
static int aitcam_ffservertotmp(unsigned int addr);
//static void aitcam_reset_hw_modules(void) ;
//static irqreturn_t aitcam_irq_handler (int irq_id, void *dev_private);
#if AITCAM_IPC_EN==0
static int aitcam_set_irq(struct aitcam_dev *dev)
{
      int ret = 0 ;
      ret = request_irq(AIC_SRC_VIF, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_VI", dev);
 
  
      ret = request_irq(AIC_SRC_ISP, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_ISP", dev);
      /*ret = request_irq(AIC_SRC_GRA, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_GRA", dev);
      if (ret) {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_gra;
      }*/
      
      
      ret = request_irq(AIC_SRC_RAW, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_RAW", dev);
      ret = request_irq(AIC_SRC_IBC, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_IBC", dev);
      ret = request_irq(AIC_SRC_JPG, &aitcam_irq_handler, /*IRQF_SHARED*/IRQF_DISABLED,
                        "AITCAM_JPEG", dev);
      ret = request_irq(AIC_SRC_H264ENC, &aitcam_irq_handler, /*IRQF_SHARED*/IRQF_DISABLED,
                        "AITCAM_H264", dev);
      return 0 ;
}
#endif


MMPF_OS_TIME_UNIT   aitcam_clockbase(void)
{
    return (clockbase == 0) ? MMPF_OS_TIME_UNIT_MS : MMPF_OS_TIME_UNIT_JIFFIES;
}

#if RESV_SRAM_JPEG == 0
sgfdsfdsfsdf

#endif


void aitcam_mem_init (struct aitcam_dev *dev)
{
    int resv_dram_size_t = resv_dram_size ;
    
    dev->dev_mem.base_addr[MEM_SRAM] = resv_sram_base;
    dev->dev_mem.end_addr[MEM_SRAM] = resv_sram_base + resv_sram_size;
    
    dev->dev_mem.base_addr[MEM_DRAM] = (resv_dram_base) ? resv_dram_base : ((num_physpages << PAGE_SHIFT) + CONFIG_PHYS_OFFSET);
    
    if(cpu_ipc){
      
      //if( resv_dram_size >= 64*1024*1024) {
      //  RESV_FOR_RTOS_SIZE = 60*1024*1024 ;
      //}
      RESV_FOR_RTOS_SIZE = resv_dram_size - V4L2_BUF_SIZE ;
      
      dev->dev_mem.base_addr[MEM_DRAM] += RESV_FOR_RTOS_SIZE ;
      if(resv_dram_size_t > RESV_FOR_RTOS_SIZE) {
        resv_dram_size_t -= RESV_FOR_RTOS_SIZE ;
      }
      else {
        resv_dram_size_t = 0 ;
        printk(KERN_ERR "   fatal err:resv dram size is not enough !\n" );
      }
    } 
    else {
    //#if AITCAM_IPC_EN  
    //    dev->dev_mem.base_addr[MEM_DRAM] += RESV_FOR_RTOS_SIZE_DEMO ;
    //#endif
    }
     
    dev->dev_mem.end_addr[MEM_DRAM] = dev->dev_mem.base_addr[MEM_DRAM] + resv_dram_size_t;
    dev->dev_mem.allocated = MMP_TRUE;
    if(debug_level > 2) {  
      printk(KERN_ERR "   resv_sram_base =  x%x,  resv_sram_size = x%x\n", resv_sram_base, resv_sram_size);
      printk(KERN_ERR "   resv_dram_base =  x%x,  resv_dram_size = x%x\n", resv_dram_base, resv_dram_size);
      printk(KERN_ERR "   dram_start =  x%x,  dram_end = x%x\n", dev->dev_mem.base_addr[MEM_DRAM], dev->dev_mem.end_addr[MEM_DRAM]);
    }
    dev->mem_ptr[MEM_SRAM] = dev->dev_mem.base_addr[MEM_SRAM];
    dev->mem_ptr[MEM_DRAM] = dev->dev_mem.base_addr[MEM_DRAM];

    #if (RESV_DRAM_V1 == 1)
    // allocation for hardware reserve memory used when dynamic multi-streaming
    dev->mem_hwresv_ptr[MEM_SRAM] = dev->mem_ptr[MEM_SRAM];
    dev->mem_hwresv_ptr[MEM_DRAM] = dev->mem_ptr[MEM_DRAM];
    if(cpu_ipc) {
      
    }
    else {
      dev->mem_ptr[MEM_SRAM] += RESV_SRAM_HW;
      dev->mem_ptr[MEM_DRAM] += RESV_DRAM_HW;
    }
    dev->mem_hwresv_end[MEM_SRAM] = dev->mem_ptr[MEM_SRAM];
    dev->mem_hwresv_end[MEM_DRAM] = dev->mem_ptr[MEM_DRAM];
    #if 0
    printk(KERN_ERR "   hw resv_sram x%x - x%x\n", dev->mem_hwresv_ptr[MEM_SRAM],
           dev->mem_hwresv_end[MEM_SRAM]);
    printk(KERN_ERR "   hw resv_dram x%x - x%x\n", dev->mem_hwresv_ptr[MEM_DRAM],
           dev->mem_hwresv_end[MEM_DRAM]);
    #endif
    #endif
    memset(dev->mem_regions, 0, sizeof(dev->mem_regions));

    return;
}

int aitcam_mem_getinfo (struct aitcam_dev *dev, enum ait_mem_idx mem_idx,
                        struct ait_mem_regions *available)
{
    if (dev->mem_regions[mem_idx].allocated)
    {
        *available = dev->mem_regions[mem_idx];
    }
    else
    {
        MMP_ULONG *curr, *end;
        #if (USING_FIXED_MEM_POOL)
        __u32 format, /*pipe,*/ baseaddr;
        #endif

        #if (RESV_DRAM_V1 == 1)
        if (mem_idx < AITCAM_NUM_CONTEXTS)   // context indep. mem
        {
            curr = dev->mem_ptr;
            end = dev->dev_mem.end_addr;
        }
        else
        {
            curr = dev->mem_hwresv_ptr;
            end = dev->mem_hwresv_end;
        }
        #else
        curr = dev->mem_ptr;
        end = dev->dev_mem.end_addr;
        #endif

        if ((curr[MEM_DRAM] > end[MEM_DRAM]) || (curr[MEM_SRAM] > end[MEM_SRAM]))
        {
            dbg_printf(0, "mem_getinfo fail @ x%x/x%x\n", curr[MEM_DRAM],
                       curr[MEM_SRAM]);
            return -ENOMEM;
        }

        #if (USING_FIXED_MEM_POOL)
        if (mem_idx < AITCAM_NUM_CONTEXTS)   // context indep. mem
        {
            //format = aitcam_get_format_by_ctx_id(dev, mem_idx);
            baseaddr = aitcam_get_baseaddr_by_ctx_id(dev, mem_idx);
            #if AITCAM_IPC_EN
            available->base_addr[MEM_DRAM] = ( dev->dev_mem.base_addr[MEM_DRAM] + baseaddr );  
            available->end_addr[MEM_DRAM] = available->base_addr[MEM_DRAM] + aitcam_get_maxpoolsize_by_ctx_id(dev, mem_idx) ;
            #else
            available->base_addr[MEM_DRAM] = ( dev->mem_hwresv_end[MEM_DRAM] + baseaddr    ); 
            #endif
        }
        else
        {
            available->base_addr[MEM_DRAM] = curr[MEM_DRAM];
        }
        #else
        available->base_addr[MEM_DRAM] = curr[MEM_DRAM];
        #endif
        #if AITCAM_IPC_EN==0
        available->end_addr[MEM_DRAM] = end[MEM_DRAM];
        #endif
        
        available->base_addr[MEM_SRAM] = curr[MEM_SRAM];
        available->end_addr[MEM_SRAM] = end[MEM_SRAM];

        available->allocated = 0;

        //dbg_printf(0,"===== aitcam_mem_getinfo, DRAM:%x - %x ======\n",available->base_addr[MEM_DRAM],available->end_addr[MEM_DRAM]);

    }

    return 0;
}

int aitcam_mem_alloc (struct aitcam_dev *dev, enum ait_mem_idx mem_idx,
                      struct ait_mem_regions *alloc)
{
    //dbg_printf(0, "############ MEM ALLOC ID: %d ############\n", mem_idx);
    if (dev->mem_regions[mem_idx].allocated)
    {
        struct ait_mem_regions *src = &(dev->mem_regions[mem_idx]);
        int mem_t;

        for (mem_t = 0; mem_t < MEM_MAX; mem_t++)
        {
            if ((alloc->base_addr[mem_t] < src->base_addr[mem_t])
                    || (alloc->end_addr[mem_t] > src->end_addr[mem_t])
                    || (alloc->end_addr[mem_t] < alloc->base_addr[mem_t]))
            {
                dbg_printf(0, "mem re-fail  x%x - x%x\n"
                           "       from  x%x - x%x\n",
                           alloc->base_addr[mem_t], alloc->end_addr[mem_t],
                           src->base_addr[mem_t], src->end_addr[mem_t]);
                return -ENOMEM;
            }
        }
    }
    else
    {
        int mem_t;
        MMP_ULONG *curr, *end;
        #if (USING_FIXED_MEM_POOL)
        __u32 max_pool_size, mem_pool_end;
        #endif

        #if (RESV_DRAM_V1 == 1)
        if (mem_idx < AITCAM_NUM_CONTEXTS)   // context indep. mem
        {
            #if 1
            /* only for context memory. */
            if (alloc->end_addr[MEM_DRAM] <
                    (alloc->base_addr[MEM_DRAM] + alloc->max_buf_size[MEM_DRAM]))
            {
                if (debug_level)
                {
                    printk(KERN_ERR " DRAM resv : x%x, x%x -> new: x%x\n",
                           alloc->base_addr[MEM_DRAM], alloc->end_addr[MEM_DRAM],
                           alloc->base_addr[MEM_DRAM] + alloc->max_buf_size[MEM_DRAM]);
                }
                alloc->end_addr[MEM_DRAM] = alloc->base_addr[MEM_DRAM] + alloc->max_buf_size[MEM_DRAM];
            }
            if (alloc->end_addr[MEM_SRAM] <
                    (alloc->base_addr[MEM_SRAM] + alloc->max_buf_size[MEM_SRAM]))
            {
                if (debug_level)
                {
                    printk(KERN_ERR " SRAM resv : x%x, x%x -> new: x%x\n",
                           alloc->base_addr[MEM_SRAM], alloc->end_addr[MEM_SRAM],
                           alloc->base_addr[MEM_SRAM] + alloc->max_buf_size[MEM_SRAM]);
                }
                alloc->end_addr[MEM_SRAM] = alloc->base_addr[MEM_SRAM] + alloc->max_buf_size[MEM_SRAM];
            }
            #endif

            #if (USING_FIXED_MEM_POOL)
            curr = alloc->base_addr;
            #else
            curr = dev->mem_ptr;
            #endif

            end = dev->dev_mem.end_addr;

            #if (USING_FIXED_MEM_POOL)
            max_pool_size = aitcam_get_maxpoolsize_by_ctx_id(dev, mem_idx);
            mem_pool_end = (alloc->base_addr[MEM_DRAM] + max_pool_size);  

            if (alloc->end_addr[MEM_DRAM] > mem_pool_end)
            {
                dbg_printf(0, " alloc DRAM pool x%x - x%x from %x - x%x fail\n", alloc->base_addr[MEM_DRAM], alloc->end_addr[MEM_DRAM],
                                                                                                                curr[MEM_DRAM], mem_pool_end);
                return -ENOMEM;
            }
            #endif

        }
        else
        {
            curr = dev->mem_hwresv_ptr;
            end = dev->mem_hwresv_end;
        }
        #else
        curr = dev->mem_ptr;
        end = dev->dev_mem.end_addr;
        #endif

        for (mem_t = 0; mem_t < MEM_MAX; mem_t++)
        {
            if ((alloc->base_addr[mem_t] != curr[mem_t])
                    || (alloc->end_addr[mem_t] > end[mem_t]))
            {
                dbg_printf(0, " alloc x%x - x%x from x%x - x%x fail\n",
                           alloc->base_addr[mem_t], alloc->end_addr[mem_t],
                           curr[mem_t], end[mem_t]);
                return -ENOMEM;
            }
        }

        #if (USING_FIXED_MEM_POOL)
        if (mem_idx < AITCAM_NUM_CONTEXTS)   // context indep. mem
        {
            //dbg_printf(0, "############ alloc finish DRAM:%x SRAM:%x ###########\n", alloc->end_addr[MEM_DRAM], alloc->end_addr[MEM_SRAM]);
        }
        else
        {
            curr[MEM_DRAM] = alloc->end_addr[MEM_DRAM];
            curr[MEM_SRAM] = alloc->end_addr[MEM_SRAM];
            //dbg_printf(0, "############ alloc finish DRAM:%x SRAM:%x ###########\n", curr[MEM_DRAM], curr[MEM_SRAM]);
        }
        #else
        curr[MEM_DRAM] = alloc->end_addr[MEM_DRAM];
        curr[MEM_SRAM] = alloc->end_addr[MEM_SRAM];
        //dbg_printf(0, "############ alloc finish DRAM:%x SRAM:%x ###########\n", curr[MEM_DRAM], curr[MEM_SRAM]);
        #endif

        dev->mem_regions[mem_idx] = *alloc;
        dev->mem_regions[mem_idx].allocated = 1;
    }

    return 0;
}


int aitcam_mem_release (struct aitcam_dev *dev, enum ait_mem_idx mem_idx)
{
    //dbg_printf(0, "### MEM FREE ID: %d ###\n", mem_idx);
    if (dev->mem_regions[mem_idx].allocated == 0)
    {
        dbg_printf(0, "mem have released already!!!!!\n");
        return -ENOMEM;
    }
    else if(mem_idx >= AITCAM_NUM_CONTEXTS)
    {
        dbg_printf(0, "memory can't be released!!!!!\n");
        return -ENOMEM;
    }
    else
    {
        #if (USING_FIXED_MEM_POOL)
        //        MMP_ULONG *curr, *end;
        //        __u32 format;

        //        format = aitcam_get_format_by_ctx_id(dev, mem_idx);
        //        if(format == V4L2_PIX_FMT_H264){
        //            curr = dev->mem_ptr;
        //            end = dev->dev_mem.end_addr;

        //            curr[MEM_DRAM] = dev->mem_regions[mem_idx].base_addr[MEM_DRAM];
        //            curr[MEM_SRAM] = dev->mem_regions[mem_idx].base_addr[MEM_SRAM];

        //            dbg_printf(0,"### Free finish DRAM:%x SRAM:%x ###\n",curr[MEM_DRAM],curr[MEM_SRAM]);
        //        }
        //	else{
        //dbg_printf(0,"### Free finish DRAM:%x SRAM:%x ###\n",dev->mem_regions[mem_idx].base_addr[MEM_DRAM],dev->mem_regions[mem_idx].base_addr[MEM_SRAM]);
        //	}
        #else
        MMP_ULONG *curr, *end;

        curr = dev->mem_ptr;
        end = dev->dev_mem.end_addr;

        curr[MEM_DRAM] = dev->mem_regions[mem_idx].base_addr[MEM_DRAM];
        curr[MEM_SRAM] = dev->mem_regions[mem_idx].base_addr[MEM_SRAM];

        //dbg_printf(0,"### Free finish DRAM:%x SRAM:%x ###\n",curr[MEM_DRAM],curr[MEM_SRAM]);
        #endif

        dev->mem_regions[mem_idx].base_addr[MEM_DRAM] = 0;
        dev->mem_regions[mem_idx].end_addr[MEM_DRAM] = 0;

        dev->mem_regions[mem_idx].base_addr[MEM_SRAM] = 0;
        dev->mem_regions[mem_idx].end_addr[MEM_SRAM] = 0;

        dev->mem_regions[mem_idx].allocated = 0;

    }

    return 0;
}


int aitcam_get_fov_crop_info (MMP_ULONG *base, MMP_ULONG *ratio)
{
    #if (AIT_FOV_CROP_EN == 1)
    *base   = (MMP_ULONG)fov_crop_base;
    *ratio  = (MMP_ULONG)fov_crop_ratio;
    #else
    *base   = 1;
    *ratio  = 0;
    #endif

    return 0;
}

void aitcam_queue_wakeup(MMPF_VIDBUF_QUEUE *pVbq)
{
    if(pVbq->non_blocking)
    {
        wake_up(&pVbq->ready_waitq);
    }
    else
    {
        up(&pVbq->vbq_lock);
        //printk(KERN_ERR "[up]:free(%d),ready(%d)\n",MMPF_VIDBUF_GetDepth(pVbq, VIDBUF_FREE_QUEUE),MMPF_VIDBUF_GetDepth(pVbq, VIDBUF_READY_QUEUE));
    }
}

static void aitcam_isp_ir_check (void *priv)
{
    struct aitcam_dev *dev = (struct aitcam_dev *)priv;

    dev->isp_ir_refresh = 1;
}

#if (AITCAM_SCHED_RT == 1) && (AITCAM_IPC_EN==0)
static void aitcam_setup_task(void *arg)
{
    struct sched_param param;

    #if (CUSTOMER != CVN)  // remove it to resolve kworker problem that CPU usage is too high
    if (h264enc_rt_prio && (h264enc_rt_prio < MAX_RT_PRIO))
    {
        param.sched_priority = h264enc_rt_prio;

        sched_setscheduler(current, SCHED_FIFO, &param);
    }
    else
    #endif
    {
        param.sched_priority = 0;

        sched_setscheduler(current, SCHED_NORMAL, &param);
    }
}
#endif

static int aitcam_open(struct file *file)
{
    struct aitcam_dev *dev = video_drvdata(file);
    struct aitcam_ctx *ctx = NULL;
    int ret = 0;
    //AITPS_MCI pMCI = AITC_BASE_MCI;

    #if (HDR_SUPPORT == 1)
    int i;
    #endif
    #if (AIT_OSD_EN == 1)
    int win = 0;
    #endif
    #if (HDR_SUPPORT == 1)

    MMP_RAW_STORE_BUF rawbuf;
    MMP_ULONG         ulCurAddr;
    MMP_ULONG         ulStoreW, ulStoreH;
    #if (HDR_3M == 1)
    //MMP_USHORT        usSensorW=2312, usSensorH=1520;
    #else
    //MMP_USHORT        usSensorW=1928, usSensorH=1092;
    #endif
    MMP_ULONG         ulRawBufSize = 0;
    //MMP_ULONG         pllfreq;

    #endif

    MMP_BOOL skipSnrPwrOn = MMP_FALSE ;
#if AITCAM_IPC_EN
    skipSnrPwrOn = MMP_TRUE ;
#endif    

    if (debug_level >= 2)
    {
        dbg_printf(0, "aitcam open ...aitcam_dev:0x%08x\n", (u32)dev);
    }
    dev->num_opening++; /// It is guarded by aitcam_mutex in vfd

    ctx = kzalloc(sizeof * ctx, GFP_KERNEL);
    if (!ctx)
    {
        printk(KERN_ERR "Not enough memory\n");
        ret = -ENOMEM;
        goto err_alloc;
    }

    ///fh to video_device(dev node), must call in open()
    ///not use v4l2_fh_open, implements stuffs here
    ///1. init fh, from video_device
    ///2. filp->private_data = fh;
    ///3. fh->ctrl_handler = ctx->ctrl_handler(latter)
    v4l2_fh_init(&ctx->fh, video_devdata(file));
    file->private_data = &ctx->fh;
    v4l2_fh_add(&ctx->fh);

    ctx->dev = dev;

    ctx->num = 0;
    ctx->img_pipe = -1 ;
    ctx->output_format = 0 ;
	// def : S_CTRL will return error
	ctx->ctx_flags = CTX_STOP_V4L2_RET_ERR ;
    
    while (dev->ctx[ctx->num])
    {
        ctx->num++;
        if (ctx->num >= AITCAM_NUM_CONTEXTS)
        {
            printk(KERN_ERR "Too many open contexts\n");
            ret = -EBUSY;
            goto err_no_ctx;
        }
    }
    dev->ctx[ctx->num] = ctx;

    ctx->cam_vbq = MMPF_VIDBUF_GetHandle(ctx->num);

    //    /// Init videobuf2 queue for CAPTURE
    //	q = &ctx->vq_dst;
    //	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    //	q->drv_priv = &ctx->fh;
    //	if (s5p_mfc_get_node_type(file) == MFCNODE_ENCODER) {
    //		q->io_modes = VB2_MMAP | VB2_USERPTR;
    //		q->ops = aitcam_get_queue_ops();
    //	} else {
    //		ret = -ENOENT;
    //		goto err_queue_init;
    //	}
    //	q->mem_ops = (struct vb2_mem_ops *)&vb2_dma_contig_memops;
    //	ret = vb2_queue_init(q);
    //	if (ret) {
    //		printk(KERN_ERR "Failed to initialize videobuf2 queue(capture)\n");
    //		goto err_queue_init;
    //	}
    //	init_waitqueue_head(&ctx->queue);
    #if AITCAM_IPC_EN==0
    if( !cpu_ipc)
    {
      MMPF_SYS_EnableClock(MMPF_SYS_CLK_JPG, MMP_TRUE);
      MMPF_SYS_EnableClock(MMPF_SYS_CLK_GRA, MMP_TRUE);
      MMPF_SYS_EnableClock(MMPF_SYS_CLK_SCALE, MMP_TRUE);
      MMPF_SYS_EnableClock(MMPF_SYS_CLK_H264, MMP_TRUE);
      MMPF_SYS_EnableClock(MMPF_SYS_CLK_ICON, MMP_TRUE);
      MMPF_SYS_EnableClock(MMPF_SYS_CLK_IBC, MMP_TRUE);
    }
    #endif
    
    printk(KERN_ERR "[aitcam_open(%d)],num_opening =  %d,ipc = %d\n", ctx->num,dev->num_opening,cpu_ipc);
    //printk(KERN_ERR "   ctx->num =  %d\n", ctx->num);
    //dbg_printf(0, "%d x %d\n", ctx->img_width, ctx->img_height);

    if (dev->num_opening == 1)
    {
        MMPF_OS_WORK_CTX work;
        MMP_ULONG   dram_addr, sram_addr;
        MMP_ULONG   isp_acc_buf_size = 0, isp_dma_size = 0;
        struct ait_mem_regions      availmem;
        struct ait_camif_ir_ctrl    g_ir;
   
        #if 1 
        dev->orientation=V4L2_CAMERA_AIT_ORTN_NORMAL;
        dev->brightness	=DEV_TYPE_V4L2_RESET_VAL;
        dev->contrast		=DEV_TYPE_V4L2_RESET_VAL;
        dev->hue				=DEV_TYPE_V4L2_RESET_VAL;
        dev->saturation	=DEV_TYPE_V4L2_RESET_VAL;
        dev->gamma			=DEV_TYPE_V4L2_RESET_VAL;
        dev->sharpness	=DEV_TYPE_V4L2_RESET_VAL;
        dev->wdr				=DEV_TYPE_V4L2_RESET_VAL;
        dev->powerfreq	=DEV_TYPE_V4L2_RESET_VAL;
        #endif
        #if 1 // reset night mode relative parameters
        dev->is_night  = DEV_TYPE_V4L2_RESET_VAL ;
        dev->lightcond = DEV_TYPE_V4L2_RESET_VAL ;
        dev->nr_gain	 = DEV_TYPE_V4L2_RESET_VAL ;
        dev->nv_mode   = DEV_TYPE_V4L2_RESET_VAL ;        
        #endif
        #if AITCAM_IPC_EN==0
        if(!cpu_ipc)
        {
          MMPF_SYS_EnableClock(MMPF_SYS_CLK_VIF, MMP_TRUE);
          MMPF_SYS_EnableClock(MMPF_SYS_CLK_BAYER, MMP_TRUE);
          MMPF_SYS_EnableClock(MMPF_SYS_CLK_ISP, MMP_TRUE);
          MMPF_SYS_EnableClock(MMPF_SYS_CLK_GNR, MMP_TRUE);
          aitcam_reset_hw_modules();
        }
        #endif
        
        #if (USING_FIXED_MEM_POOL)
        aitcam_pipereset();
        #endif
        ///initialize reserved mem system
        aitcam_mem_init(dev);
        #if AITCAM_IPC_EN==0
        if(!cpu_ipc) 
        {
          if ((ret = aitcam_mem_getinfo(dev, AIT_MEM_IDX_ISP, &availmem)))
          {
              dbg_printf(0, "isp mem get fail\n");
              goto err_mem_init;
          }
          dram_addr = ALIGN32(availmem.base_addr[MEM_DRAM]);
          sram_addr = ALIGN32(availmem.base_addr[MEM_SRAM]);
  
          dev->SensorTask = create_singlethread_workqueue("Sensor Task");
          work.Exec = aitbh_isp_frame_start;
          work.Task = dev->SensorTask;
          work.Work = &dev->IspFrameStWork;
          MMPF_OS_RegisterWork(MMPF_OS_WORKID_ISP_FRM_ST, &work);
  
          dev->H264EncTask = create_singlethread_workqueue("H264enc Task");
          ///Put init work for encoder tasks at reqbuf
          if (!dev->SensorTask || !dev->H264EncTask)
          {
              ret = -ENOMEM;
              goto err_mem_alloc;
          }
          #if (AITCAM_SCHED_RT == 1)
          INIT_WORK(&dev->SetupTaskWork, (void*)aitcam_setup_task);
          queue_work(dev->H264EncTask, &dev->SetupTaskWork);
          #endif
  
          MMPF_Sensor_GetHWBufferSize(&isp_acc_buf_size, &isp_dma_size);
  
          if (debug_level)
          {
              dbg_printf(0, "isp phys start x%x/x%x\n", dram_addr, sram_addr);
          }
          dram_addr = ((dram_addr + 0xFF) & ~0xFF); //align256
          sram_addr = ((sram_addr + 0xFF) & ~0xFF); //align256
          #if 1
          MMPF_Sensor_AllocateBuffer(AIT_RAM_P2V(sram_addr),
                                     AIT_RAM_P2V(sram_addr + isp_acc_buf_size));
          sram_addr += (isp_dma_size + isp_acc_buf_size);
          #else
          MMPF_Sensor_AllocateBuffer(AIT_RAM_P2V(dram_addr),
                                     AIT_RAM_P2V(sram_addr));
          dram_addr += isp_acc_buf_size;
          sram_addr += isp_dma_size;
          #endif
  
          availmem.end_addr[MEM_DRAM] = dram_addr;
          availmem.end_addr[MEM_SRAM] = sram_addr;
          if ((ret = aitcam_mem_alloc(dev, AIT_MEM_IDX_ISP, &availmem)))
          {
              dbg_printf(0, "isp mem alloc fail\n");
              goto err_mem_alloc;
          }
          #if (AIT_ISP_VIDEO_IQ == 1)
          dev->isp_vidiq_ctx_num = 0;
          #endif
          dev->orientation = V4L2_CAMERA_AIT_ORTN_NORMAL;
          dev->nv_mode = V4L2_CAMERA_AIT_NV_OFF;
  
          #if (HDR_SUPPORT == 1)
          if(gbHDR_en)
          {
              MMP_EnalbeHDR(MMP_TRUE);
              MMPF_Sensor_Initialize(PRM_SENSOR, 1, 11,skipSnrPwrOn); ///move to open, tbd
          }
          else
              MMPF_Sensor_Initialize(PRM_SENSOR, 1, 0,skipSnrPwrOn);
          #elif (SUPPORT_DUAL_SNR_PRV)
          MMPF_Sensor_Initialize(PRM_SENSOR, 1, 0,skipSnrPwrOn); ///move to open, tbd
          if(dual_snr_en)
          {
              MMPF_Sensor_Initialize(SCD_SENSOR, 1, 0,skipSnrPwrOn); ///move to open, tbd
          }
          #else
          MMPF_Sensor_Initialize(PRM_SENSOR, 1, 0,skipSnrPwrOn); ///move to open, tbd
          #endif
  
          ait_param_ctrl(AIT_PARAM_CAM_IR_G_CTRL, &g_ir);
          if (g_ir.ir_on)
          {
              //ISP_IF_F_SetSaturation(ISP_SATURATION_LEVEL_GRAY);
              ISP_IF_F_SetImageEffect(ISP_IMAGE_EFFECT_GREY);  // Grey
          }
  
          if (debug_level)
          {
              dbg_printf(0, "isp phys end x%x/x%x\n", dram_addr, sram_addr);
          }
  
          #if 1
          MMPF_IBC_InitModule(MMPF_IBC_PIPE_0);
          MMPF_IBC_InitModule(MMPF_IBC_PIPE_1);
          MMPF_IBC_InitModule(MMPF_IBC_PIPE_2);
          MMPF_IBC_InitModule(MMPF_IBC_PIPE_3);
          MMPF_IBC_InitModule(MMPF_IBC_PIPE_4);
          MMPF_ICON_InitModule(MMPF_ICO_PIPE_0);
          MMPF_ICON_InitModule(MMPF_ICO_PIPE_1);
          MMPF_ICON_InitModule(MMPF_ICO_PIPE_2);
          MMPF_ICON_InitModule(MMPF_ICO_PIPE_3);
          MMPF_ICON_InitModule(MMPF_ICO_PIPE_4);
          MMPF_Scaler_InitModule(MMPF_SCALER_PATH_0);
          MMPF_Scaler_InitModule(MMPF_SCALER_PATH_1);
          MMPF_Scaler_InitModule(MMPF_SCALER_PATH_2);
          MMPF_Scaler_InitModule(MMPF_SCALER_PATH_3);
          MMPF_Scaler_InitModule(MMPF_SCALER_PATH_4);
          #endif
  
          #if 1 //HARD-CODED SYSTEM TUNING
          //MMPF_SYS_TuneMCIPriority(2);
          MMPF_MCI_TunePriority(MCI_MODE_DMAR_H264);
          #endif
   
          #if 0  // modify IBC0 & IBC1 MCI priority for Dual FHD H.264 streaming
              pMCI->MCI_INIT_DEC_VAL[MCI_SRC_IBC0] = MCI_INIT_WT_MAX;
              pMCI->MCI_NA_INIT_DEC_VAL[MCI_SRC_IBC0] = MCI_NA_INIT_WT_MAX;
              pMCI->MCI_ROW_INIT_DEC_VAL[MCI_SRC_IBC0] = MCI_ROW_INIT_WT_MAX;
              pMCI->MCI_RW_INIT_DEC_VAL[MCI_SRC_IBC0] = MCI_RW_INIT_WT_MAX;
  
              pMCI->MCI_INIT_DEC_VAL[MCI_SRC_IBC1] = MCI_INIT_WT_MAX;
              pMCI->MCI_NA_INIT_DEC_VAL[MCI_SRC_IBC1] = MCI_NA_INIT_WT_MAX;
              pMCI->MCI_ROW_INIT_DEC_VAL[MCI_SRC_IBC1] = MCI_ROW_INIT_WT_MAX;
              pMCI->MCI_RW_INIT_DEC_VAL[MCI_SRC_IBC1] = MCI_RW_INIT_WT_MAX;
          #endif
  
          #if (HDR_SUPPORT == 1)
          if(gbHDR_en)
          {
  
              if ((ret = aitcam_mem_getinfo(dev, AIT_MEM_IDX_RAW, &availmem)))
              {
                  dbg_printf(0, "!!!!!!!  mem_getinfo error\n");
              }
              ulCurAddr = ALIGN32(availmem.base_addr[MEM_DRAM]);
              rawbuf.ulRawBufCnt = 2;//m_VidRecdConfigs.ulRawStoreBufCnt = VR_MIN_RAWSTORE_BUFFER_NUM;
              MMPF_VIF_GetGrabResolution(MMPF_Sensor_GetVIFPad(PRM_SENSOR), &ulStoreW, &ulStoreH);
              if (gsHdrCfg.ubRawStoreBitMode == HDR_BITMODE_10BIT)
                  ulRawBufSize = (2 * /*2312 1928*/ulStoreW * /*1518 1090*/ulStoreH * 4 / 3) + 256;
              else
                  ulRawBufSize = (2 * /*2312 1928*/ulStoreW * /*1518 1090*/ulStoreH) + 256;
  
              for (i = 0; i < rawbuf.ulRawBufCnt; i++)
              {
                  rawbuf.ulRawBufAddr[i] = ALIGN256(ulCurAddr);
                  MMPF_RAWPROC_SetRawStoreBuffer(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_RAW_STORE_PLANE0, i, rawbuf.ulRawBufAddr[i]);
                  ulCurAddr = rawbuf.ulRawBufAddr[i] + ulRawBufSize;
                  dbg_printf(0, "@@@ ulCurAddr:%x\n", ulCurAddr);
              }
              availmem.end_addr[MEM_DRAM] = ulCurAddr;
              availmem.end_addr[MEM_SRAM] = sram_addr;
              if ((ret = aitcam_mem_alloc(dev, AIT_MEM_IDX_RAW, &availmem)))
              {
                  dbg_printf(0, "!!!!!!!  mem_alloc error\n");
              }
  
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_S0, MMP_TRUE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_S1, MMP_TRUE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_RAW_F, MMP_TRUE);
              if (rawbuf.ulRawBufCnt > 0)
              {
                  MMPF_RAWPROC_InitStoreBuffer(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_RAW_STORE_PLANE0, rawbuf.ulRawBufAddr[0]);
              }
              //m_bRawPathPreview = MMP_TRUE;
  
              //MMPF_VIF_GetGrabResolution(MMPF_Sensor_GetVIFPad(PRM_SENSOR), &ulStoreW, &ulStoreH);
              MMPF_RAWPROC_SetFetchRange(0, 0, ulStoreH * 2, ulStoreH, ulStoreW * 2);
              MMPF_HDR_InitModule(ulStoreW, ulStoreH);
              MMPF_HDR_SetBufEnd(ALIGN32(ulCurAddr));
              if (gsHdrCfg.ubRawStoreBitMode == HDR_BITMODE_10BIT)
                  MMPF_RAWPROC_EnablePath(MMP_TRUE,
                                          MMP_RAW_IOPATH_ISP2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                          MMP_RAW_COLOR_DEPTH_10);
              else
                  MMPF_RAWPROC_EnablePath(MMP_TRUE,
                                          MMP_RAW_IOPATH_ISP2RAW | MMP_RAW_IOPATH_RAW2ISP,
                                          MMP_RAW_COLOR_DEPTH_8);
  
              MMPF_Scaler_SetInterruptEnable(MMPF_SCALER_PATH_0, MMPF_SCA_EVENT_DBL_FRM_ST, MMP_TRUE);
              MMPF_Scaler_SetInterruptEnable(MMPF_SCALER_PATH_1, MMPF_SCA_EVENT_DBL_FRM_ST, MMP_TRUE);
              MMPF_HDR_InitRawStoreSetting();
              MMPF_RAWPROC_ResetBufIndex(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_RAW_STORE_PLANE0);
              MMPF_RAWPROC_EnableFetchPath(MMP_TRUE);
              MMPF_RAWPROC_EnableStorePath(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_RAW_STORE_PLANE0, /*MMP_RAW_S_SRC_VIF0*/MMP_RAW_S_SRC_ISP_BEFORE_BS, MMP_TRUE);
              if (gsHdrCfg.ubVcStoreMethod == HDR_VC_STORE_2PLANE)
              {
                  MMPF_RAWPROC_ResetBufIndex(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_RAW_STORE_PLANE1);
                  MMPF_RAWPROC_EnableStorePath(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_RAW_STORE_PLANE1, /*MMP_RAW_S_SRC_VIF0*/MMP_RAW_S_SRC_ISP_BEFORE_BS, MMP_TRUE);
              }
              else
              {
                  MMPF_RAWPROC_ResetBufIndex(MMP_RAW_MDL_ID1, MMP_RAW_STORE_PLANE0);
                  MMPF_RAWPROC_EnableStorePath(MMP_RAW_MDL_ID1, MMP_RAW_STORE_PLANE0, MMP_RAW_S_SRC_ISP_BEFORE_BS, MMP_TRUE);
              }
          }
          #endif
        }
        #endif
    }

    
    ret = aitcam_v4l2_ctrls_setup(ctx);
    if (ret)
    {
        printk(KERN_ERR "Failed to setup camera controls\n");
        goto err_ctrls_setup;
    }
    ctx->fh.ctrl_handler = &ctx->ctrl_handler;

    #if (AIT_OSD_EN == 1)
    for (win = 0; win < MAX_OSD_INDEX; win++)
    {
        ctx->osd.win[win].type = AIT_OSD_TYPE_INACTIVATE;
        mutex_init(&(ctx->osd.mlock));
    }
    #endif
    ctx->scale_resol = 60;
    #if (SUPPORT_DUAL_SNR_PRV)
    ctx->snr_src_id = 0;  // enable raw preview
    #else
    ctx->snr_src_id = 0;  // disable raw preview
    #endif

    #if (STATISTIC_EN == 1)
    stat_en = 1;
    if (stat_en == 1)
    {
        stat_window_create(&stat_win[ctx->num], 8);
        stat_window_set_range(&stat_win[ctx->num], 200);
    }
    #endif
    #if AITCAM_IPC_EN
    if(cpu_ipc) {
      #if 0 // move to set format
      if(aitcam_ipc_open(ctx->num)) {
        goto err_mem_alloc ;
      }
      #endif 
    }
    #endif
    ctx->inst_state = AITCAM_STAT_OPEN;

    // init vbq lock for blocking mode
    sema_init( &ctx->cam_vbq->vbq_lock, 0 );
    ctx->cam_vbq->non_blocking = (file->f_flags & O_NONBLOCK) ? 1 : 0 ;
    ctx->loop_recording = (file->f_flags & 0x80000000) ? 1 : 0 ;
      
    printk(KERN_ERR "[aitcam_open]:non_blocking=%d,loop rec:%d...ok\n", ctx->cam_vbq->non_blocking,ctx->loop_recording);

    return ret;

err_mem_alloc:
    #if AITCAM_IPC_EN==0
    if(!cpu_ipc)
    {  
      if (dev->SensorTask)
          destroy_workqueue(dev->SensorTask);
      if (dev->H264EncTask)
          destroy_workqueue(dev->H264EncTask);
    }
    #endif

//err_mem_init:
err_ctrls_setup:
    dev->ctx[ctx->num] = 0;
    aitcam_v4l2_ctrls_delete(ctx);
err_no_ctx:
    v4l2_fh_del(&ctx->fh);
    v4l2_fh_exit(&ctx->fh);
    kfree(ctx);
err_alloc:
    dev->num_opening--;

    return ret;
}

#include "mmp_reg_gbl.h"
#if AITCAM_IPC_EN==0
static void aitcam_reset_hw_modules(void)
{
#if (CHIP == MCR_V2)  			
#if (CUSTOMER == CVN)
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_VIF0, MMP_TRUE);
#else
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_VIF0, MMP_FALSE);
#endif
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_IBC0, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_IBC1, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_IBC2, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_IBC3, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_ICON0, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_ICON1, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_ICON2, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_ICON3, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_SCAL0, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_SCAL1, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_SCAL2, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_SCAL3, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_JPG, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_H264, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_MDL_GRA, MMP_TRUE); 
#endif     
}
#endif

static int aitcam_release(struct file *file)
{
    struct aitcam_ctx *ctx = fh_to_ctx(file->private_data);
    struct aitcam_dev *dev = ctx->dev;

    if (debug_level)
    {
        dbg_printf(0, "aitcam release ...[%d], num_opening = %d\n", ctx->num, dev->num_opening);
    }

    if(dev->num_opening > 0)
    {
        dev->num_opening--;

        #if (HDR_SUPPORT == 1)
        if(gbHDR_en)
        {
            if (gsHdrCfg.bEnable)
            {
                MMPF_HDR_UnInitModule();
            }
        }
        #endif

        if (ctx->inst_state >= AITCAM_STAT_ALLOC)   ///free instance
        {
            if (ctx->inst_state == AITCAM_STAT_RUNNING)
            {
                aitcam_stop_stream(ctx);
            }
            #if AITCAM_IPC_EN==0
            if(!cpu_ipc) {
              switch (ctx->output_format)
              {
                  case V4L2_PIX_FMT_H264:
                  case V4L2_PIX_FMT_MV_AITHREC:
                      MMPF_VIDENC_DeinitInstance((MMP_ULONG)ctx->codec_handle);
                      break;
                  case V4L2_PIX_FMT_MJPEG:
                      if (ctx->codec_handle)
                      {
                          MMPF_JPEG_DeinitInstance(ctx->codec_handle);
                          kfree(ctx->codec_handle);
                      }
                      break;
                  case V4L2_PIX_FMT_GREY:
                  case V4L2_PIX_FMT_YUV420:
                  case V4L2_PIX_FMT_YUYV:
                  case V4L2_PIX_FMT_UYVY:
                  #if AITCAM_IPC_EN
                  case V4L2_PIX_FMT_MPEG:
                  #endif
                      break;
              }
            }
            #endif

        }
        
        if(!cpu_ipc)
        {
          #if AITCAM_IPC_EN==0
          if (dev->num_opening == 0)
          {
                
              MMPF_Sensor_Set3AInterrupt(MMP_FALSE);
  
              #if (AIT_ISP_VIDEO_IQ == 1)
              mutex_unlock(&dev->aitcam_mutex);
              flush_workqueue(dev->SensorTask);
              mutex_lock(&dev->aitcam_mutex);
              #else
              flush_workqueue(dev->SensorTask);
              #endif
  
              MMPF_Sensor_PowerDown(PRM_SENSOR,MMP_TRUE);
              gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
  
              MMPF_VIF_EnableOutput(MMPF_Sensor_GetVIFPad(PRM_SENSOR), MMP_FALSE);
  
              #if (SUPPORT_DUAL_SNR_PRV)
              if(dual_snr_en)
              {
                  MMPF_Sensor_PowerDown(SCD_SENSOR,MMP_TRUE);
                  MMPF_VIF_EnableOutput(MMPF_Sensor_GetVIFPad(SCD_SENSOR), MMP_FALSE);
              }
              #endif
  
              destroy_workqueue(dev->SensorTask);
              destroy_workqueue(dev->H264EncTask);
  
              if (MMPF_JPEG_IsModuleInit())
              {
                  MMPF_JPEG_DeinitModule();
              }
              if (MMPF_VIDENC_IsModuleInit(MMPF_VIDENC_MODULE_H264))
              {
                  MMPF_VIDENC_DeinitModule(MMPF_VIDENC_MODULE_H264);
              }
              
              aitcam_reset_hw_modules();
  
  
              //MMPF_SYS_TuneMCIPriority(1);
              MMPF_MCI_TunePriority(MCI_MODE_DEFAULT);
  
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_VIF, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_BAYER, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_ISP, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_GNR, MMP_FALSE);
              #if (CHIP == VSN_V3)
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_COLR, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_CIDC, MMP_FALSE);
              #endif
  
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_JPG, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_GRA, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_SCALE, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_H264, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_ICON, MMP_FALSE);
              MMPF_SYS_EnableClock(MMPF_SYS_CLK_IBC, MMP_FALSE);
          }
          #endif
        }
        else {
          #if AITCAM_IPC_EN
          //aitcam_reset_hw_modules();
          aitcam_ipc_release(ctx->num,(void *)isp_data_from_rtos);
          
          //if(force_demo_only==2) {
          //  // just test code, should remove in the furture
          //  //dbg_printf(0,"Force2 to power down sensor...\n");
          //  MMPF_SYS_EnableClock(MMPF_SYS_CLK_VIF, MMP_TRUE);
          //  MMPF_Sensor_PowerDown(PRM_SENSOR,MMP_TRUE); 
          //  aitcam_set_irq(dev) ;
          //  cpu_ipc=0;
          //}

          
          #endif
        }
        
        #if (STATISTIC_EN == 1)
        if (stat_en == 1)
        {
            stat_window_destroy(&stat_win[ctx->num]);
        }
        #endif

        dev->ctx[ctx->num] = 0;
        aitcam_v4l2_ctrls_delete(ctx);
        v4l2_fh_del(&ctx->fh);
        v4l2_fh_exit(&ctx->fh);
        kfree(ctx);

    }


    return 0;
}

static unsigned int aitcam_poll(struct file *file,
                                struct poll_table_struct *wait)
{
    struct aitcam_ctx *ctx = fh_to_ctx(file->private_data);
    unsigned int rc = 0;


    if( file->f_flags & O_NONBLOCK )
    {
        if (ctx->inst_state == AITCAM_STAT_FINISH)
        {
            return POLLERR | POLLHUP;
        }

        if (ctx->cam_vbq->ready_vbq.size > 0)
        {
            rc |= (POLLIN | POLLRDNORM);
            return rc;
        }

        poll_wait(file, &(ctx->cam_vbq->ready_waitq), wait);
    }
    else   // blocking mode return ???
    {
        return POLLERR ;
    }
    if (ctx->cam_vbq->ready_vbq.size > 0)
    {
        rc |= (POLLIN | POLLRDNORM);
    }

    return rc;
}

static int aitcam_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long start = vma->vm_start;
    unsigned long size  = vma->vm_end - vma->vm_start;
    struct aitcam_ctx *ctx = fh_to_ctx(file->private_data);
    MMP_ULONG   buf_num, buf_size, buf_addr, phys_page;

    MMPF_Video_GetVidBufQueueInfo(ctx->num, &buf_addr, &buf_size, &buf_num);

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    if (vma->vm_pgoff >= buf_num)
    {
        return -ERANGE;
    }
    phys_page = ((buf_addr + vma->vm_pgoff * buf_size) >> PAGE_SHIFT);

    //ait_err("phys addr : x%x\n", phys_page<<PAGE_SHIFT);

    if (remap_pfn_range(vma, start, phys_page, size, vma->vm_page_prot))
    {
        ait_err("remap_pfn_range failed\n");
        return -EAGAIN;
    }

    vma->vm_flags |= VM_RESERVED; ///prevent swap out

    return 0;
}

#if (AIT_OSD_EN == 1) && (AITCAM_IPC_EN==0)
static void ait_rtc_osd_event (unsigned long data)
{
    struct aitcam_dev *dev = (struct aitcam_dev *)data;

    if (!ait_rtc_IsClockOn(&(dev->rtc.handle)))
    {
        return;
    }

    ait_rtc_UpdateTime(&(dev->rtc.handle), 1);

    dev->rtc.target_time += dev->rtc.nticks;

    dev->rtc.event.expires = dev->rtc.target_time;

    add_timer(&(dev->rtc.event));

    return;
}
#endif // (AIT_OSD_EN == 1)

static long aitcam_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    if (_IOC_TYPE(cmd) == AIT_IOC_MAGIC)
    {
        //int offset;
        struct ait_fdframe_config_ipc *osd_base_fdfr = aitcam_ipc_osd_base();
        struct ait_rtc_config_ipc *rtc_base;
        struct ait_osd_config_ipc *osd_base_rtc, *osd_base_str;

        
        if (_IOC_NR(cmd) > AIT_IOC_MAXNR) return -EFAULT;


        rtc_base = (struct ait_rtc_config_ipc *) (osd_base_fdfr + MAX_OSD_INDEX);
        osd_base_rtc = (struct ait_osd_config_ipc *) (rtc_base + 1);
        osd_base_str = (struct ait_osd_config_ipc *) (osd_base_rtc + 1);

        switch (cmd)
        {              
            case AIT_IOCG_IMAGE_DATA:
            {
              struct ait_vidbuf_info jpg_info,tmp_info ;
              struct ait_vidbuf_info __user *usr_info  = (struct ait_vidbuf_info __user *)arg ;
              int ret;
              if (copy_from_user(&tmp_info, (struct ait_vidbuf_info *)arg, sizeof(struct ait_vidbuf_info))) {
                goto FAULT_IOC ;
              }             
              ret = aitcam_ipc_get_jpg( &jpg_info ) ;
              if( ret < 0 ) {
                goto FAULT_IOC ;
              }
              if( jpg_info.buf_size > tmp_info.buf_size ) {
                pr_info("[JPG] : %d > %d\n",jpg_info.buf_size , tmp_info.buf_size );
                goto FAULT_IOC ;
              }
              put_user( jpg_info.buf_size , &usr_info->buf_size ); 
              put_user( jpg_info.buf_num  , &usr_info->buf_num  );
      				if ( copy_to_user( (void *)usr_info->buf_addr, (void *)jpg_info.buf_addr, jpg_info.buf_size ) ) {
      				  goto FAULT_IOC ;
      				}
            }
            break ;
            case AIT_IOCS_FDFRAME_CFG:
            {
                CPU_LOCK_INIT(); 
                struct aitcam_ctx *ctx = fh_to_ctx(filp->private_data);
                struct ait_fdframe_config cfg ;
                struct ait_fdframe_config_ipc *osd2rtos ;
                
                if (copy_from_user(&cfg, (struct ait_fdframe_config *)arg,
                                   sizeof(struct ait_fdframe_config)))
                {
                    goto FAULT_IOC;
                }
                if (cfg.index >= MAX_OSD_INDEX)
                {
                    goto FAULT_IOC;
                }
                CPU_LOCK();
                osd2rtos = ((struct ait_fdframe_config_ipc *)aitcam_ipc_osd_base()) + cfg.index ;
                osd2rtos->index = cfg.index ;
                osd2rtos->type  = cfg.type  ;
                osd2rtos->pos[0]= cfg.pos[0] ;
                osd2rtos->pos[1]= cfg.pos[1] ;
                osd2rtos->width = cfg.width  ;
                osd2rtos->height = cfg.height  ;
                osd2rtos->TextColorY = cfg.TextColorY  ;
                osd2rtos->TextColorU = cfg.TextColorU  ;
                osd2rtos->TextColorV = cfg.TextColorV  ;
                strcpy(osd2rtos->str,cfg.str );
                CPU_UNLOCK();
                
                
                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 2)
                  pr_info("Face #%d :type:%d (%d,%d) -> (%d,%d),addr:%p\n",cfg.index,cfg.type,cfg.pos[0],cfg.pos[1],cfg.width,cfg.height ,osd2rtos );
                #endif
#if AITCAM_IPC_EN==0
                if (cfg.type == AIT_OSD_TYPE_INACTIVATE)
                {
                    if (ctx->osd.win[cfg.index].type != AIT_OSD_TYPE_INACTIVATE)
                    {
                        mutex_lock(&(ctx->osd.mlock));

                        memset(ctx->osd.win[cfg.index].str, 0, MAX_OSD_STR_LEN);
                        ctx->osd.win[cfg.index].type = cfg.type;
                        ctx->osd.active_num--;

                        mutex_unlock(&(ctx->osd.mlock));
                    }
                }
                else
                {
                    AIT_OSD_HANDLE  *pOsd = &(ctx->osd.win[cfg.index].hdl);

                    mutex_lock(&(ctx->osd.mlock));

                    ait_osd_Init(pOsd);
                    ait_osd_SetPos(pOsd, cfg.pos[AXIS_X], cfg.pos[AXIS_Y]);
                    ait_osd_SetFDWidthHeight(pOsd, cfg.width, cfg.height);
                    if ((cfg.TextColorY < 0x100) && (cfg.TextColorU < 0x100)
                            && (cfg.TextColorV < 0x100))
                    {
                        ait_osd_SetColor(pOsd, cfg.TextColorY, cfg.TextColorU,
                                         cfg.TextColorV);
                    }

                    /* activate or change mode */
                    if (ctx->osd.win[cfg.index].type == AIT_OSD_TYPE_INACTIVATE)
                    {
                        ctx->osd.active_num++;
                    }
                    ctx->osd.win[cfg.index].type = cfg.type;

                    if (cfg.str != NULL)
                    {
                        strncpy(ctx->osd.win[cfg.index].str, cfg.str, MAX_OSD_STR_LEN);
                    }

                    if (ctx->osd.active_num == 1)
                    {
                        ctx->osd.resync_time = 1;
                    }

                    mutex_unlock(&(ctx->osd.mlock));
                }
#endif
            }
            break;
            case AIT_IOCS_RTC:
            {
                struct ait_rtc_config cfg;
                struct ait_rtc_config_ipc *osdRTC2rtos ;
                CPU_LOCK_INIT(); 

                if (copy_from_user(&cfg, (struct ait_rtc_config *)arg,
                                   sizeof(struct ait_rtc_config)))
                {
                    goto FAULT_IOC;
                }

                    if (cfg.bOn)
                    {

                        CPU_LOCK();
                        osdRTC2rtos = rtc_base;
                        osdRTC2rtos->usYear = cfg.usYear ;
                        osdRTC2rtos->usMonth  = cfg.usMonth  ;
                        osdRTC2rtos->usDay= cfg.usDay;
                        osdRTC2rtos->usHour= cfg.usHour;
                        osdRTC2rtos->usMinute = cfg.usMinute  ;
                        osdRTC2rtos->usSecond = cfg.usSecond  ;
                        osdRTC2rtos->bOn = cfg.bOn  ;
                        CPU_UNLOCK();

                        #ifdef AITCAM_DBG_MSG
                        if (debug_level >= 4)
                        {
                            pr_info("datetime = %d-%d-%d, %d:%d:%d\n",cfg.usYear,
                                   cfg.usMonth, cfg.usDay, cfg.usHour, cfg.usMinute, cfg.usSecond);
                        }
                        #endif
                    }
            }
            break;
            case AIT_IOCS_OSD_CFG:
            {
                struct aitcam_ctx *ctx = fh_to_ctx(filp->private_data);
                struct ait_osd_config cfg;
                struct ait_osd_config_ipc *osd2rtosrtc, *osd2rtosstr ;
                CPU_LOCK_INIT(); 

                if (copy_from_user(&cfg, (struct ait_osd_config *)arg,
                                   sizeof(struct ait_osd_config)))
                {
                    goto FAULT_IOC;
                }
                if (cfg.index >= MAX_OSD_INDEX)
                {
                    goto FAULT_IOC;
                }

                    CPU_LOCK();

                    osd2rtosrtc = osd_base_rtc;
                    osd2rtosstr = osd_base_str;

                    if(cfg.type == AIT_OSD_TYPE_RTC) {
                    osd2rtosrtc->index = cfg.index ;
                    osd2rtosrtc->type  = cfg.type  ;
                    osd2rtosrtc->pos[0]= cfg.pos[0];
                    osd2rtosrtc->pos[1]= cfg.pos[1];
                    strcpy(osd2rtosrtc->str,cfg.str );
                    osd2rtosrtc->TextColorY = cfg.TextColorY  ;
                    osd2rtosrtc->TextColorU = cfg.TextColorU  ;
                    osd2rtosrtc->TextColorV = cfg.TextColorV  ;
                    }
                    else if(cfg.type == AIT_OSD_TYPE_CUST_STRING) {
                    osd2rtosstr->index = cfg.index ;
                    osd2rtosstr->type  = cfg.type  ;
                    osd2rtosstr->pos[0]= cfg.pos[0];
                    osd2rtosstr->pos[1]= cfg.pos[1];
                    strcpy(osd2rtosstr->str,cfg.str );
                    osd2rtosstr->TextColorY = cfg.TextColorY  ;
                    osd2rtosstr->TextColorU = cfg.TextColorU  ;
                    osd2rtosstr->TextColorV = cfg.TextColorV  ;
                    }
                    else if(cfg.type == AIT_OSD_TYPE_INACTIVATE) {
                    osd2rtosrtc->index = cfg.index ;
                    osd2rtosrtc->type  = cfg.type  ;

                    osd2rtosstr->index = cfg.index ;
                    osd2rtosstr->type  = cfg.type  ;
                    }
                    CPU_UNLOCK();


                #ifdef aitcam_dbg_msg
                if (debug_level >= 4)
                {
                    if(cfg.type == AIT_OSD_TYPE_CUST_STRING) 
                        pr_info("ait_iocs_osd_cfg cfg: osd[%d], type=%d, pos = (%d,%d), str: %s\n ,colory=x%x, coloru = x%x, colorv = x%x\n", cfg.index, cfg.type,
                           cfg.pos[0], cfg.pos[1], cfg.str,cfg.TextColorY, cfg.TextColorU, cfg.TextColorV);
                    if(cfg.type == AIT_OSD_TYPE_RTC) 
                        pr_info("ait_iocs_osd_cfg cfg: osd[%d], type=%d, pos = (%d,%d), str:(%d,%d)\n ,colory=x%x, coloru = x%x, colorv = x%x\n", cfg.index, cfg.type,
                           cfg.pos[0], cfg.pos[1], cfg.str[0], cfg.str[1],cfg.TextColorY, cfg.TextColorU, cfg.TextColorV);
                }
                #endif


            }
            break;
			case AIT_IOCG_OSD_INFO:
			{
				AIT_RTC_TIME ex_time;
				struct ait_osd_info inf;
				char ex_str[MAX_OSD_STR_LEN];
                CPU_LOCK_INIT(); 

				ait_osd_GetFontPixelSize(&inf.font_h_pix, &inf.font_v_pix);

                ait_rtc_SetClock(&ex_time, 2012, 12, 21, 12, 21, 12, MMP_TRUE);
                ait_rtc_ConvertDateTime2String(&ex_time, ex_str);

				inf.datetime_h_pix = strlen(ex_str) * inf.font_h_pix;
				inf.datetime_v_pix = inf.font_v_pix;

				#ifdef AITCAM_DBG_MSG
				if (debug_level >= 4)
				{
					printk(KERN_ERR "AIT_IOCG_OSD_INFO: font= %d x %d, date/time = %d x %d\n", inf.font_h_pix, inf.font_v_pix,
						   inf.datetime_h_pix, inf.datetime_v_pix);
				}
				#endif

				if (copy_to_user((struct ait_osd_info *)arg, &inf,
								 sizeof(struct ait_osd_info)))
				{
					goto FAULT_IOC;
				}
			}
			break;
            default:
                goto FAULT_IOC;
        }

        return 0;

    FAULT_IOC:
        return -EFAULT;
    }
    else
    {
        return video_ioctl2(filp, cmd, arg);
    }
}

static const struct v4l2_file_operations aitcam_fops =
{
    .owner  = THIS_MODULE,
    .open   = aitcam_open,
    .release = aitcam_release,
    .poll   = aitcam_poll,
    .unlocked_ioctl = aitcam_ioctl,//video_ioctl2,
    .mmap   = aitcam_mmap,
};

void MMPF_VIF_ISR(void);
void MMPF_ISP_ISR(void);
//void MMPF_GRA_ISR(void);
void MMPF_IBC_ISR(void);
void MMPF_H264ENC_ISR(void);
//void MMPF_DMA_ISR(void);
#if AITCAM_IPC_EN==0
static irqreturn_t aitcam_irq_handler (int irq_id, void *dev_private)
{
   switch (irq_id)
    {
        /*case AIC_SRC_DMA:
            MMPF_DMA_ISR();
            break;*/
        case AIC_SRC_VIF:
            MMPF_VIF_ISR();
            break;
        case AIC_SRC_ISP:
            MMPF_ISP_ISR();
            break;
        case AIC_SRC_RAW:
            MMPF_RAWPROC_ISR();
            break;
        /*case AIC_SRC_GRA:
            MMPF_GRA_ISR();
            break;*/
        case AIC_SRC_IBC:
            MMPF_IBC_ISR();
            break;
        case AIC_SRC_JPG:
        {
            AITPS_JPG   pJPG = AITC_BASE_JPG;
            MMP_USHORT  intsrc;

            intsrc = pJPG->JPG_INT_CPU_SR & pJPG->JPG_INT_CPU_EN;
            if (intsrc & JPG_INT_ENC_DONE)
            {
                MMPF_JPEG_ISR();
            }
            else
            {
                return IRQ_NONE;
            }
        }
        break;
        case AIC_SRC_H264ENC:
        {
            AITPS_H264ENC       pH264ENC = AITC_BASE_H264ENC;
            MMP_USHORT          status;

            status = pH264ENC->H264ENC_INT_CPU_EN & pH264ENC->H264ENC_INT_CPU_SR;

            if ((status & ENC_CUR_BUF_OVWR_INT) || (status & SLICE_DONE) || (status & FRAME_ENC_DONE_CPU_INT_EN))
            {

                #ifdef AITCAM_DBG_MSG
                if (debug_level >= 4)
                {
                    static unsigned int cnt = 0;
                    unsigned int delta;
                    struct timespec ct;

                    if ((cnt++ & 0x1F) < 4)
                    {
                        getnstimeofday(&ct);

                        delta = ct.tv_nsec + ((enc_start.tv_sec != ct.tv_sec) ? NSEC_PER_SEC : 0)
                                - enc_start.tv_nsec;

                        printk(KERN_ERR "[%d]:%d;\n", (unsigned int)enc_hdl, delta / 1000000);
                    }
                }
                #endif
                MMPF_H264ENC_ISR();
            }
            else
            {
                return IRQ_NONE;
            }
        }
        break;
        default:
            return IRQ_NONE;
    }
    return IRQ_HANDLED;
}
#endif

#if 1
static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

int file_read(struct file* file,  unsigned char* data, unsigned int size) {
  unsigned long long offset = 0;
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);
	set_fs(oldfs);
	return ret;
}

int file_write(struct file* file, unsigned char* data, unsigned int size) {
  unsigned long long offset = 0;
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

static unsigned long get_ih_item(int item,unsigned char *ih)
{
    unsigned char *ptr = (unsigned char *)(ih + item*4) ;
    unsigned long val ;
    val = ( ptr[0] << 24 ) | ( ptr[1] << 16 ) | (ptr[2] << 8) | ptr[3] ;
    return val ;
}

static unsigned long set_ih_item(int item,unsigned char *ih, unsigned long val)
{
    unsigned long *ptr = (unsigned long *)(ih + item*4) ;
    *ptr = val;
    return 0;
}

static int aitcam_ffservertotmp(unsigned int addr)
{
#define IH_MAGIC 0x27051956
  
#define DST_FILE "/tmp/ffserver"
#define MAX_READ_SIZE (1024*1024)
  struct file *f_dst ;//= file_open( DST_FILE, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP);
  unsigned char *buf = 0 ;
  int rd_size = 0;
  if (get_ih_item(0,(unsigned char *)addr)==IH_MAGIC ) {
    pr_info("Embedded ffserver...\n");
  }
  else {
    return -1 ;  
  }
  rd_size =  get_ih_item(3,(unsigned char *)addr) ;
  if( rd_size > MAX_READ_SIZE ) {
    return -1 ;
  }
  
  f_dst = file_open( DST_FILE, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP);
  if(!f_dst) {
    pr_info("cannot open %s\n",DST_FILE );    
  }
  else {
    buf = kmalloc( rd_size , GFP_KERNEL);
  }
  if(  f_dst ) {
      memcpy(buf, (unsigned char *)(addr + 64) , rd_size );
      
      if(rd_size > 0)  {
        file_write( f_dst , buf , rd_size) ;
      }
  }
  
  if(buf) kfree( buf );    
  if(f_dst) file_close( f_dst);
  
  /*
  mark header signature to invalid to avoid load again
  */  
  set_ih_item(0,(unsigned char *)addr,0);  
  return 0 ;    
}
#endif

static int aitcam_probe(struct platform_device *pdev)
{
    struct aitcam_dev *dev;
    struct video_device *vfd;
    struct ait_camif_reg_ctrl camif_reg;
    int ret;
    dev = kzalloc(sizeof * dev, GFP_KERNEL);
    if (!dev)
    {
        dev_err(&pdev->dev, "No memory for ait-cam device\n");
        return -ENOMEM;
    }

    spin_lock_init(&dev->irqlock);

    dev->plat_dev = pdev;
    if (!dev->plat_dev)
    {
        dev_err(&pdev->dev, "No platform data specified\n");
        ret = -ENODEV;
        goto err_dev;
    }
    MMPF_VIDBUF_SetQueueBase(0,0);  
    #if AUTO_DETECT_RTOS_MEM
    {
      struct _cpu_rtos_mem_info *rtos_mem ;
      unsigned int basic_cfg_end_addr = V4L2_BUF_SIZE ;
      
      rtos_mem = get_cpub_rtos_mem();
      if(rtos_mem->reset_base) {
        resv_dram_base = rtos_mem->reset_base ;  
        resv_dram_size = (int)(rtos_mem->v4l2_size_mb + rtos_mem->working_size_mb ) * 1024 *1024 ;
        if( rtos_mem->flag & H264_3RD_STREAM_EN) {
          if( rtos_mem->v4l2_size_mb * 1024 * 1024 > V4L2_BUF_SIZE) {
            basic_cfg_end_addr = aitcam_set_h264_s2_baseaddr(basic_cfg_end_addr);  
          }  
        }
        if( rtos_mem->flag & JPG_CAPTURE_EN ) {
          if( rtos_mem->v4l2_size_mb * 1024 * 1024 > V4L2_BUF_SIZE) {
            aitcam_set_jpg_baseaddr(basic_cfg_end_addr,rtos_mem->v4l2_size_mb * 1024 * 1024 - basic_cfg_end_addr);
          }
        } 
        
        V4L2_BUF_SIZE  = rtos_mem->v4l2_size_mb * 1024 * 1024 ;
        pr_info("rtos_mem->flag:%x\n",rtos_mem->flag);
      }
      if(cpffserver) {
        aitcam_ffservertotmp(AIT_RAM_P2V(rtos_mem->v4l2_base));      
      }
    }
    #endif

 
    #if AITCAM_IPC_EN  
    if(cpu_ipc) {
      int ret ;
      ret = aitcam_ipc_register(ipc_debug);
      if(ret) {
        cpu_ipc = 0 ;
      }
    }  
    #endif  
    #if AITCAM_IPC_EN==0
    if( !cpu_ipc)
    {
      
      
      /*ret = request_irq(AIC_SRC_DMA, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_DMA", dev);
      if (ret) {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_dma;
      }*/
      ret = request_irq(AIC_SRC_VIF, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_VI", dev);
      if (ret)
      {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_vi;
      }
 
  
      ret = request_irq(AIC_SRC_ISP, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_ISP", dev);
      if (ret)
      {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_isp;
      }
      /*ret = request_irq(AIC_SRC_GRA, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_GRA", dev);
      if (ret) {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_gra;
      }*/
      
      
      ret = request_irq(AIC_SRC_RAW, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_RAW", dev);
      if (ret)
      {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_raw;
      }
      ret = request_irq(AIC_SRC_IBC, &aitcam_irq_handler, IRQF_DISABLED,
                        "AITCAM_IBC", dev);
      if (ret)
      {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_ibc;
      }
      ret = request_irq(AIC_SRC_JPG, &aitcam_irq_handler, /*IRQF_SHARED*/IRQF_DISABLED,
                        "AITCAM_JPEG", dev);
      if (ret)
      {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_jpeg;
      }
      ret = request_irq(AIC_SRC_H264ENC, &aitcam_irq_handler, /*IRQF_SHARED*/IRQF_DISABLED,
                        "AITCAM_H264", dev);
      if (ret)
      {
          dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
          goto err_req_irq_h264;
      }
    }
    else {
    }
    #endif
    
    //#if AITCAM_IPC_EN
    //dev->cpu_ipc = cpu_ipc ;
    //#endif
    mutex_init(&dev->aitcam_mutex);

    ret = v4l2_device_register(&pdev->dev, &dev->v4l2_dev);
    if (ret)
        goto err_v4l2_dev_reg;

    vfd = video_device_alloc();
    if (!vfd)
    {
        v4l2_err(&dev->v4l2_dev, "Failed to allocate video device\n");
        ret = -ENOMEM;
        goto err_vdev_alloc;
    }
    vfd->fops	    = &aitcam_fops,
    vfd->ioctl_ops	= aitcam_get_ioctl_ops();
    vfd->release	= video_device_release,
    vfd->lock	    = &dev->aitcam_mutex;
    vfd->v4l2_dev	= &dev->v4l2_dev;
    snprintf(vfd->name, sizeof(vfd->name), "%s", "ait-cam");
    dev->vfd_cam       = vfd;
    ret = video_register_device(vfd, VFL_TYPE_GRABBER, 0);
    if (ret)
    {
        v4l2_err(&dev->v4l2_dev, "Failed to register video device\n");
        video_device_release(vfd);
        goto err_vdev_reg;
    }
    video_set_drvdata(vfd, dev);
    platform_set_drvdata(pdev, dev);

    camif_reg.ir_notify = aitcam_isp_ir_check;
    camif_reg.priv      = dev;
    ait_param_ctrl(AIT_PARAM_CAM_IF_REG, &camif_reg);

    #if (AIT_OSD_EN == 1)
    dev->rtc.nticks = HZ;
    dev->rtc.target_time = jiffies;
    dev->rtc.handle.bOn = MMP_FALSE;
    spin_lock_init(&dev->rtc.time_rdlock);
    init_timer(&(dev->rtc.event));
    #endif

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 3)
    {
        extern void _dump_iomem(MMP_ULONG start, MMP_ULONG len);

        #if (CHIP == VSN_V3)
        _dump_iomem(0xF0005D00, 32);
        _dump_iomem(0xF0006900, 256);
        _dump_iomem(0xF0006E10, 16);
        #endif
    }
    #endif

    #if AITCAM_MULTI_STREAM_EN
    MMPF_Graphics_Init();

    #ifdef AITCAM_DBG_MSG
    if (debug_level >= 2)
    {
        dbg_printf(0, "AITCAM_MULTI_STREAM_EN is ON\n");
    }
    #endif

    #endif

    #if (SUPPORT_DUAL_SNR_PRV)
    if(dual_snr_en)
    {
        #ifdef AITCAM_DBG_MSG
        if (debug_level >= 2)
        {
            dbg_printf(0, "Dual Sensor is ON\n");
        }
        #endif
    }
    #endif
    v4l2_info(&dev->v4l2_dev,
              "encoder registered as /dev/video%d,cpu_ipc:%d,ver:%s\n", vfd->num,cpu_ipc,AIT_CAM_VER);
    v4l2_info(&dev->v4l2_dev,
              "cpub addr:0x%08x,size:0x%08x,resv for v4l2 : 0x%08x\n", resv_dram_base,resv_dram_size,V4L2_BUF_SIZE);

    return 0;

err_vdev_reg:
    video_device_release(dev->vfd_cam);
err_vdev_alloc:
    v4l2_device_unregister(&dev->v4l2_dev);
    
err_v4l2_dev_reg:
    RUN_AT_IPC_OFF(cpu_ipc,free_irq(AIC_SRC_H264ENC, dev); );
#if AITCAM_IPC_EN==0    
err_req_irq_h264:
    RUN_AT_IPC_OFF(cpu_ipc,free_irq(AIC_SRC_JPG, dev); );
err_req_irq_jpeg:
    //    free_irq(AIC_SRC_GRA, dev);
    //err_req_irq_gra:
    RUN_AT_IPC_OFF(cpu_ipc,free_irq(AIC_SRC_RAW, dev); );
err_req_irq_raw:
    RUN_AT_IPC_OFF(cpu_ipc,free_irq(AIC_SRC_IBC, dev); );
err_req_irq_ibc:
    RUN_AT_IPC_OFF(cpu_ipc,free_irq(AIC_SRC_ISP, dev); );
err_req_irq_isp:
    RUN_AT_IPC_OFF(cpu_ipc,free_irq(AIC_SRC_VIF, dev); );
err_req_irq_vi:
    /*    free_irq(AIC_SRC_DMA, dev);
    err_req_irq_dma:*/
#endif
    
err_dev:
    kfree(dev);
    pr_debug("%s-- with error\n", __func__);
    return ret;
}

static int __devexit aitcam_remove(struct platform_device *pdev)
{
    struct aitcam_dev *dev = platform_get_drvdata(pdev);

    v4l2_info(&dev->v4l2_dev, "Removing %s,ipc:%d\n", pdev->name,cpu_ipc);

    #if (AIT_OSD_EN == 1)
    if (dev->rtc.handle.bOn)
    {
        ait_rtc_SetClock (&(dev->rtc.handle), 0, 0, 0,
                          0, 0, 0, MMP_FALSE);
        del_timer_sync(&(dev->rtc.event));
    }
    #endif

    ait_param_ctrl(AIT_PARAM_CAM_IF_FREE, NULL);

    video_unregister_device(dev->vfd_cam);
    v4l2_device_unregister(&dev->v4l2_dev);
    
    
    if(!cpu_ipc)
    {
      #if AITCAM_IPC_EN==0
      free_irq(AIC_SRC_H264ENC, dev);
      free_irq(AIC_SRC_JPG, dev);
      //free_irq(AIC_SRC_GRA, dev);
      free_irq(AIC_SRC_IBC, dev);
      free_irq(AIC_SRC_RAW, dev);
      free_irq(AIC_SRC_ISP, dev);
      free_irq(AIC_SRC_VIF, dev);
      //free_irq(AIC_SRC_DMA, dev);
      #endif
      
    }
    else {
      #if AITCAM_IPC_EN
      aitcam_ipc_unregister();        
      //if(force_demo_only==1) {
      //  // just test code, should remove in the furture
      //  dbg_printf(0,"Force to power down sensor...\n");
      //  MMPF_SYS_EnableClock(MMPF_SYS_CLK_VIF, MMP_TRUE);
      //  MMPF_Sensor_PowerDown(PRM_SENSOR,MMP_TRUE); 
      //}
        
      #endif
      
      
    }
    kfree(dev);

    return 0;
}

static struct platform_driver ait_cam_driver =
{
    .probe	= aitcam_probe,
    .remove	= __devexit_p(aitcam_remove),
    .driver	= {
        .name	= AIT_CAM_NAME,
        .owner	= THIS_MODULE,
    },
};

static char banner[] __initdata = "AIT-Camera V4L2 driver\n";

static int __init aitcam_init(void)
{
    int ret;

    pr_info("%s", banner);
    ret = platform_driver_register(&ait_cam_driver);
    if (ret)
        pr_err("Platform device registration failed.\n");
    return ret;
}

static void __exit aitcam_exit(void)
{
    platform_driver_unregister(&ait_cam_driver);
}

module_init(aitcam_init);
module_exit(aitcam_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("A-I-T");
MODULE_DESCRIPTION("AIT Camera V4L2 driver");
