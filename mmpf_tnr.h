//==============================================================================
//
//  File        : mmpf_tnr.h
//  Description : INCLUDE File for the Firmware TNR control  
//  Author      : Eroy Yang
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_TNR_H_
#define _MMPF_TNR_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "includes_fw.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

/* TNR In/Out Mode */
#define TNR_IN_RT_MODE		    (0x01)
#define TNR_IN_FRM_MODE		    (0x02)
#define TNR_OUT_RT_MODE		    (0x04)
#define TNR_OUT_FRM_MODE	    (0x08) 

#define TNR_FRM_IN_FRM_OUT_MODE (TNR_IN_FRM_MODE | TNR_OUT_FRM_MODE)
#define TNR_RT_IN_FRM_OUT_MODE  (TNR_IN_RT_MODE  | TNR_OUT_FRM_MODE)
#define TNR_FRM_IN_RT_OUT_MODE  (TNR_IN_FRM_MODE | TNR_OUT_RT_MODE)
#define TNR_RT_IN_RT_OUT_MODE   (TNR_IN_RT_MODE  | TNR_OUT_RT_MODE)

/* TNR Filtered Pixel Count */
#define PELS_TFLT_UNFLT         (0)
#define PELS_TFLT_Y_ACT         (1)
#define PELS_TFLT_METER         (2)
#define PELS_ESTB_EDGES         (3)
#define PELS_ESTB_EDGEN         (4)
#define PELS_ESTB_EDGEW         (5)
#define MAX_PELCOUNTER_NUM      (6)

/* Enable TNR Test Mode */
#define TNR_TEST_FLOW_EN        (1)

/* TNR Reference Buffer Num */
#define MAX_TNR_REF_BUF_NUM     (2)

/* TNR Active Status */
#define TNR_NOT_ACTIVE          (0)
#define TNR_BEFORE_ACT_REG      (1)
#define TNR_DURING_ACT_REG      (2)
#define TNR_AFTER_ACT_REG       (3)

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct _MMPF_TNR_INPUT_PARAM
{
  MMP_USHORT img_width;                     
  MMP_USHORT img_height;                              		 																											    

  /* VUI Parameters */
  MMP_USHORT video_full_range_flag;
																																						
  /* Color Adjust */
  MMP_USHORT cadj_en;																																		
  MMP_USHORT cadj_bright_offset;																												
  MMP_USHORT cadj_contrast_adj_lum;																											
  MMP_USHORT cadj_contrast_adj_chr;																								
																				
  /* Temporal Filter */
  MMP_USHORT tflt_en;
  MMP_USHORT tflt_nlevel_intra_0;
  MMP_USHORT tflt_nlevel_inter_0;
  MMP_USHORT tflt_nlevel_intra_1;
  MMP_USHORT tflt_nlevel_inter_1;
  MMP_USHORT tflt_uniform_vary;
  MMP_USHORT tflt_uniform_lobd;
  MMP_USHORT tflt_chroma_en;
  MMP_USHORT tflt_w_idx_lum;
  MMP_USHORT tflt_w_idx_chr;
  MMP_USHORT tflt_local_region_h;
  MMP_USHORT tflt_unflt_ratio_lcl;
  MMP_USHORT tflt_unflt_ratio_gbl;
  MMP_USHORT tflt_unflt_ratio_lobd;
  MMP_USHORT tflt_meter_ratio_lcl;
  MMP_USHORT tflt_meter_ratio_gbl;
  MMP_USHORT tflt_meter_ratio_lobd;
  MMP_USHORT tflt_scene_pic_w_num;
  MMP_USHORT tflt_scene_pic_h_num;
  MMP_USHORT tflt_scene_affm_ratio_0;
  MMP_USHORT tflt_scene_dsnt_ratio_0;
  MMP_USHORT tflt_scene_elct_ratio_0;
  MMP_USHORT tflt_scene_affm_ratio_1;
  MMP_USHORT tflt_scene_dsnt_ratio_1;
  MMP_USHORT tflt_scene_elct_ratio_1;
  MMP_USHORT tflt_scene_affm_ratio_2;
  MMP_USHORT tflt_scene_dsnt_ratio_2;
  MMP_USHORT tflt_scene_elct_ratio_2;
  MMP_USHORT tflt_scene_edge_ratio;
  MMP_USHORT tflt_scene_blur_ratio;
  MMP_USHORT tflt_scene_gblm_ratio;
  MMP_USHORT tflt_merge_rnd_lum;
  MMP_USHORT tflt_merge_rnd_chr;
  MMP_USHORT tflt_merge_thr_lum;
  MMP_USHORT tflt_merge_thr_chr;
  MMP_USHORT tflt_edgea_ratio_cur;
  MMP_USHORT tflt_edgea_ratio_ref;
  MMP_USHORT tflt_motion_ratio_gbl;

  /* Edge Stabilization */
  MMP_USHORT estb_edgel_en;
  MMP_USHORT estb_edgel_chr;
  MMP_USHORT estb_edgel_e_idx;
  MMP_USHORT estb_edgel_e_minus;
  MMP_USHORT estb_edgel_adj_s;
  MMP_USHORT estb_edgel_adj_n;
  MMP_USHORT estb_edgel_adj_w;
  MMP_USHORT estb_edgel_lvl_hi_s;
  MMP_USHORT estb_edgel_lvl_lo_s;
  MMP_USHORT estb_edgel_lvl_hi_n;
  MMP_USHORT estb_edgel_lvl_lo_n;
  MMP_USHORT estb_edgel_lvl_hi_w;
  MMP_USHORT estb_edgel_lvl_lo_w;
  MMP_USHORT estb_edgel_sad_thr_s;
  MMP_USHORT estb_edgel_sad_thr_n;
  MMP_USHORT estb_edgel_sad_thr_w;
  MMP_USHORT estb_edgel_dir_sel_s;
  MMP_USHORT estb_edgel_dir_sel_n;
  MMP_USHORT estb_edgel_dir_sel_w;
  MMP_USHORT estb_edgel_thr_lum_s;
  MMP_USHORT estb_edgel_thr_chr_s;
  MMP_USHORT estb_edgel_thr_lum_n;
  MMP_USHORT estb_edgel_thr_chr_n;
  MMP_USHORT estb_edgel_thr_lum_w;
  MMP_USHORT estb_edgel_thr_chr_w;
  MMP_USHORT estb_edgel_flt_sel;
  MMP_USHORT estb_edgel_e_idx_thr;
  MMP_USHORT estb_edgel_e_one_thr;

  /* Spatial Filter */
  MMP_USHORT sflt_en;
  MMP_USHORT sflt_bubb_flt_en;
  MMP_USHORT sflt_bubb_flt_chr;		
  MMP_USHORT sflt_bubb_flt_sel;
  MMP_USHORT sflt_flat_flt_en;
  MMP_USHORT sflt_flat_flt_chr;		
  MMP_USHORT sflt_flat_flt_sel;
  MMP_USHORT sflt_edge_flt_en;
  MMP_USHORT sflt_edge_flt_chr;		
  MMP_USHORT sflt_edge_flt_tap;
  MMP_USHORT sflt_edge_enh_en;
  MMP_USHORT sflt_edge_enh_chr;
  MMP_USHORT sflt_edge_enh_tap;
  MMP_USHORT sflt_edge_enh_att;
  MMP_USHORT sflt_edge_enh_thr;
  MMP_USHORT sflt_div_rnd_lum;
  MMP_USHORT sflt_div_rnd_chr;

} MMPF_TNR_INPUT_PARAM;

typedef struct _MMPF_TNR_IMG_PARAM
{
    MMP_ULONG   fm_cnt;                           
    MMP_USHORT  pic_lum_w;                          //HW OPR[RW]    0x3006~0x3007
    MMP_USHORT  pic_lum_h;                          //HW OPR[RW]    0x3004~0x3005

    /* VUI parameters */
    MMP_USHORT  video_full_range_flag;              //HW OPR[RW]    0x300B[4]

    /* TFLT : Hardware Cfg */
    MMP_USHORT  tflt_hw_blk_w;                      //固定值 = 9
    MMP_USHORT  tflt_hw_blk_h;                      //固定值 = 3

    /* TFLT : Global Ctrl */
    MMP_USHORT  tflt_en;                		    //HW OPR[RW]    0x3008[1]  
    MMP_USHORT  tflt_nlevel_intra_0;                //用來算[OPR]tflt_intra_thr_0
    MMP_USHORT  tflt_nlevel_inter_0;                //用來算[OPR]tflt_intra_pel_thr_0/tflt_inter_pel_thr_0/tflt_inter_thr_0
    MMP_USHORT  tflt_nlevel_intra_1;                //用來算[OPR]tflt_intra_thr_1
    MMP_USHORT  tflt_nlevel_inter_1;                //用來算[OPR]tflt_intra_pel_thr_1/tflt_inter_pel_thr_1/tflt_inter_thr_1
    MMP_USHORT  tflt_uniform_vary;     		        //HW OPR[RW]    0x3009 
    MMP_USHORT  tflt_uniform_lobd;      		    //HW OPR[RW]    0x300A
    MMP_USHORT  tflt_chroma_en;         		    //HW OPR[RW]    0x300B[1]
    MMP_USHORT  tflt_w_idx_lum;         		    //HW OPR[RW]    0x3022[1:0]
    MMP_USHORT  tflt_w_coe_lum_c[4];			    //HW OPR[RW]    0x3024~0x3025
    MMP_USHORT  tflt_w_coe_lum_r[4];    		    //HW OPR[RW]    0x3026~0x3027
    MMP_USHORT  tflt_w_idx_chr;         		    //HW OPR[RW]    0x3022[5:4]
    MMP_USHORT  tflt_w_coe_chr_c[4];    		    //HW OPR[RW]    0x3028~0x3029
    MMP_USHORT  tflt_w_coe_chr_r[4];    		    //HW OPR[RW]    0x302A~0x302B
    MMP_USHORT  tflt_local_region_h;    		    //HW OPR[RW]    0x3020~0x3021
    MMP_USHORT  tflt_unflt_ratio_lcl;               //用來算[OPR]tflt_unflt_thr_lcl          
    MMP_USHORT  tflt_unflt_ratio_gbl;               //用來算tflt_unflt_thr_gbl
    MMP_USHORT  tflt_unflt_ratio_lobd;              //用來算tflt_unflt_thr_lcl_lobd/tflt_unflt_thr_gbl_lobd?
    MMP_USHORT  tflt_meter_ratio_lcl;               //用來算[OPR]tflt_unflt_thr_lcl/tflt_meter_thr_lcl
    MMP_USHORT  tflt_meter_ratio_gbl;               //用來算tflt_unflt_thr_gbl/tflt_meter_thr_gbl
    MMP_USHORT  tflt_meter_ratio_lobd;              //用來算tflt_meter_thr_lcl_lobd/tflt_meter_thr_gbl_lobd
    MMP_USHORT  tflt_scene_pic_w_num;               //用來算[OPR]tflt_scene_district_w/tflt_scene_num
    MMP_USHORT  tflt_scene_pic_h_num;               //用來算[OPR]tflt_scene_district_h/tflt_scene_num
    MMP_USHORT  tflt_scene_affm_ratio_0;            //用來算[OPR]tflt_scene_affm_thr_0
    MMP_USHORT  tflt_scene_dsnt_ratio_0;            //用來算[OPR]tflt_scene_dsnt_thr_0
    MMP_USHORT  tflt_scene_elct_ratio_0;            //用來算[OPR]tflt_scene_elct_thr_0
    MMP_USHORT  tflt_scene_affm_ratio_1;            //用來算[OPR]tflt_scene_affm_thr_1
    MMP_USHORT  tflt_scene_dsnt_ratio_1;            //用來算[OPR]tflt_scene_dsnt_thr_1
    MMP_USHORT  tflt_scene_elct_ratio_1;            //用來算[OPR]tflt_scene_elct_thr_1
    MMP_USHORT  tflt_scene_affm_ratio_2;            //用來算[OPR]tflt_scene_affm_thr_2
    MMP_USHORT  tflt_scene_dsnt_ratio_2;            //用來算[OPR]tflt_scene_dsnt_thr_2
    MMP_USHORT  tflt_scene_elct_ratio_2;            //用來算[OPR]tflt_scene_elct_thr_2
    MMP_USHORT  tflt_scene_edge_ratio;              //用來算[OPR]tflt_scene_edge_thr
    MMP_USHORT  tflt_scene_blur_ratio;              //用來算tflt_scene_blur_thr
    MMP_USHORT  tflt_scene_gblm_ratio;              //用來算tflt_scene_gblm_thr
    MMP_USHORT  tflt_merge_rnd_lum;  				//HW OPR[RW]    0x3042[2:0]   		
    MMP_USHORT  tflt_merge_rnd_chr;     			//HW OPR[RW]    0x3043[2:0] 
    MMP_USHORT  tflt_merge_thr_lum;     			//HW OPR[RW]    0x3042[7:4] 
    MMP_USHORT  tflt_merge_thr_chr;     			//HW OPR[RW]    0x3043[7:4] 
    MMP_USHORT  tflt_edgea_ratio_cur;   		    //用來算[OPR]tflt_edgea_thr_cur
    MMP_USHORT  tflt_edgea_ratio_ref;   			//用來算[OPR]tflt_edgea_thr_ref
    MMP_USHORT  tflt_motion_ratio_gbl;              //用來算tflt_motion_thr_gbl
    MMP_ULONG   tflt_block_size;                    //可由算式推得值
    MMP_ULONG   tflt_scene_size;                    //可由算式推得值
    MMP_ULONG   tflt_local_size;                    //可由算式推得值
    MMP_ULONG   tflt_frame_size;                    //可由算式推得值

    /* TFLT : Hardware OPR */
    MMP_USHORT  tflt_bypass;            			//HW OPR[RW]    0x3008[1]
    MMP_USHORT  tflt_intra_thr_0;       			//HW OPR[RW]    0x3014~0x3015
    MMP_USHORT  tflt_intra_pel_thr_0;   			//HW OPR[RW]    0x3012
    MMP_USHORT  tflt_inter_thr_0;       			//HW OPR[RW]    0x300C~0x300D
    MMP_USHORT  tflt_inter_pel_thr_0;   			//HW OPR[RW]    0x3010
    MMP_USHORT  tflt_intra_thr_1;       			//HW OPR[RW]    0x3016~0x3017
    MMP_USHORT  tflt_intra_pel_thr_1;   			//HW OPR[RW]    0x3013
    MMP_USHORT  tflt_inter_thr_1;       			//HW OPR[RW]    0x300E~0x300F
    MMP_USHORT  tflt_inter_pel_thr_1;   			//HW OPR[RW]    0x3011
    MMP_ULONG   tflt_intra_cost_sum;    			//HW OPR[RO]    0x3088~0x308B
    MMP_ULONG   tflt_intra_cost_max;    			//HW OPR[RO]    0x3090~0x3091
    MMP_ULONG   tflt_inter_cost_sum;    			//HW OPR[RO]    0x308C~0x308F
    MMP_ULONG   tflt_inter_cost_max;    			//HW OPR[RO]    0x3092~0x3093
    MMP_ULONG   tflt_unflt_thr_lcl;     			//HW OPR[RW]    0x301C~0x301E
    MMP_ULONG   tflt_meter_thr_lcl;     			//HW OPR[RW]    0x3018~0x301A
    MMP_USHORT  tflt_meter_change;      			//HW OPR[RO]    0x3087[0]
    MMP_USHORT  tflt_meter_act_gbl;     			//HW OPR[RW]    0x300B[2]
    MMP_USHORT  tflt_meter_act_lcl;    			    //HW OPR[RW]    0x300B[3]
    MMP_USHORT  tflt_scene_district_w;  			//HW OPR[RW]    0x3030~0x3031
    MMP_USHORT  tflt_scene_district_h;  			//HW OPR[RW]    0x3032~0x3033
    MMP_USHORT  tflt_scene_affm_thr_0;  			//HW OPR[RW]    0x3034~0x3035
    MMP_USHORT  tflt_scene_dsnt_thr_0;  			//HW OPR[RW]    0x3036~0x3037
    MMP_USHORT  tflt_scene_affm_thr_1;  			//HW OPR[RW]    0x3038~0x3039
    MMP_USHORT  tflt_scene_dsnt_thr_1;  			//HW OPR[RW]    0x303A~0x303B
    MMP_USHORT  tflt_scene_affm_thr_2;  			//HW OPR[RW]    0x303C~0x303D
    MMP_USHORT  tflt_scene_dsnt_thr_2;  			//HW OPR[RW]    0x303E~0x303F
    MMP_USHORT  tflt_scene_edge_thr;    			//HW OPR[RW]    0x3040~0x3041
    MMP_USHORT  tflt_scene_act;         			//HW OPR[RW]    0x300B[0]
    MMP_USHORT  tflt_scene_vote_0;                  //HW OPR[RO]    0x3094~0x3095
    MMP_USHORT  tflt_scene_vote_1;                  //HW OPR[RO]    0x3096~0x3097
    MMP_USHORT  tflt_scene_vote_2;                  //HW OPR[RO]    0x3098~0x3099
    MMP_USHORT  tflt_scene_vote_3;                  //HW OPR[RO]    0x309A~0x309B
    MMP_USHORT  tflt_w_buf_reset;       			//HW OPR[RW]    0x3008[6]
    MMP_USHORT  tflt_edgea_thr_cur;     			//HW OPR[RW]    0x302C~0x302D
    MMP_USHORT  tflt_edgea_thr_ref;     			//HW OPR[RW]    0x302E~0x032F
    MMP_ULONG   pelcounter_lcl[MAX_PELCOUNTER_NUM]; //HW OPR[RO]    0x3050~0x3087 
    MMP_ULONG   pelcounter_gbl[MAX_PELCOUNTER_NUM]; //HW OPR[RO]    0x3050~0x3087 

    /* TFLT : Firmware */
    MMP_ULONG   tflt_unflt_thr_gbl;                 //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_unflt_thr_lcl_lobd;            //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_unflt_thr_gbl_lobd;            //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_meter_thr_gbl;                 //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_meter_thr_lcl_lobd;            //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_meter_thr_gbl_lobd;            //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_meter_gbl_only;                //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_scene_for_meter;               //可由算式推得值, 當成判斷條件
    MMP_ULONG   tflt_scene_num;                     //可由算式推得值?
    MMP_ULONG   tflt_scene_elct_thr_0;              //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_scene_elct_thr_1;              //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_scene_elct_thr_2;              //可由算式推得值, 當成判斷條件?
    MMP_ULONG   tflt_scene_blur_thr;                //可由算式推得值, 當成判斷條件
    MMP_ULONG   tflt_scene_gblm_thr;                //可由算式推得值, 當成判斷條件
    MMP_ULONG   tflt_scene_change;      
    MMP_ULONG   tflt_scene_change_type; 
    MMP_ULONG   tflt_motion_thr_gbl;    
    MMP_ULONG   tflt_motion_detect;     

    /* Edge Stabilization : Hardware Cfg */
    MMP_USHORT  estb_hw_edgel_w;                    //固定值 = 7
    MMP_USHORT  estb_hw_edgel_h;                    //固定值 = 7

    /* Edge Stabilization : Global Ctrl */
    MMP_USHORT  estb_edgel_en;          			//HW OPR[RW]    0x3008[3:2]
    MMP_USHORT  estb_edgel_chr;         			//HW OPR[RW]    0x30A0[1]
    MMP_USHORT  estb_edgel_e_idx;       			//HW OPR[RW]    0x30A1[5:0]
    MMP_USHORT  estb_edgel_e_minus;     			//HW OPR[RW]    0x30A2[5:0]
    MMP_USHORT  estb_edgel_adj_s;       			//HW OPR[RW]    0x30A3
    MMP_USHORT  estb_edgel_adj_n;       			//HW OPR[RW]    0x30A4
    MMP_USHORT  estb_edgel_adj_w;       			//HW OPR[RW]    0x30A5 ??
    MMP_USHORT  estb_edgel_lvl_hi_s;    			//HW OPR[RW]    0x30A6~0x30A7
    MMP_USHORT  estb_edgel_lvl_lo_s;    			//HW OPR[RW]    0x30A8~0x30A9
    MMP_USHORT  estb_edgel_lvl_hi_n;    			//HW OPR[RW]    0x30AA~0x30AB
    MMP_USHORT  estb_edgel_lvl_lo_n;    			//HW OPR[RW]    0x30AC~0x30AD
    MMP_USHORT  estb_edgel_lvl_hi_w;    			//HW OPR[RW]    0x30AE~0x30AF
    MMP_USHORT  estb_edgel_lvl_lo_w;    			//HW OPR[RW]    0x30B0~0x30B1
    MMP_USHORT  estb_edgel_sad_thr_s;   			//HW OPR[RW]    0x30B2~0x30B3
    MMP_USHORT  estb_edgel_sad_thr_n;   			//HW OPR[RW]    0x30B4~0x30B5
    MMP_USHORT  estb_edgel_sad_thr_w;   			//HW OPR[RW]    0x30B6~0x30B7
    MMP_USHORT  estb_edgel_dir_sel_s;   			//HW OPR[RW]    0x30B8[1:0]
    MMP_USHORT  estb_edgel_dir_sel_n;   			//HW OPR[RW]    0x30B8[3:2]
    MMP_USHORT  estb_edgel_dir_sel_w;   			//HW OPR[RW]    0x30B8[5:4]
    MMP_USHORT  estb_edgel_thr_lum_s;   			//HW OPR[RW]    0x30B9
    MMP_USHORT  estb_edgel_thr_chr_s;   			//HW OPR[RW]    0x30BC
    MMP_USHORT  estb_edgel_thr_lum_n;   			//HW OPR[RW]    0x30BA
    MMP_USHORT  estb_edgel_thr_chr_n;   			//HW OPR[RW]    0x30BD
    MMP_USHORT  estb_edgel_thr_lum_w;   			//HW OPR[RW]    0x30BB
    MMP_USHORT  estb_edgel_thr_chr_w;   			//HW OPR[RW]    0x30BE   
    MMP_UBYTE   estb_edgel_shp[7][7];   			//HW OPR[RW]    0x30C0~0x30C6   (1bit/1pixel)
    MMP_USHORT  estb_edgel_e_idx_thr;   			//HW OPR[RW]    0x30C8
    MMP_USHORT  estb_edgel_e_one_thr;   			//HW OPR[RW]    0x30C9
    MMP_USHORT  estb_e_buf_reset;       			//HW OPR[RW]    0x30A0[0]

    /* Spatial Filter : Hardware Cfg */    
    MMP_USHORT  sflt_hw_flt_w;                      //固定值 = 7
    MMP_USHORT  sflt_hw_flt_h;                      //固定值 = 7

    /* Spatial Filter : Global Ctrl */
    MMP_USHORT  sflt_en;                			//HW OPR[RW]    0x3008[5]
    MMP_USHORT  sflt_bubb_flt_en;       			//HW OPR[RW]    0x3100[0]
    MMP_USHORT  sflt_bubb_flt_chr;      			//HW OPR[RW]    0x3100[1]
    MMP_UBYTE   sflt_bubb_flt_shp[7][7];			//HW OPR[RW]    0x3108~0x310E   (1bit/1pixel) 
    MMP_UBYTE   sflt_bubb_flt_sft[7][7];			//HW OPR[RW]    0x3110~0x311C   (2bit/1pixel)
    MMP_USHORT  sflt_bubb_flt_div_lum;  			//HW OPR[RW]    0x3104~0x3105
    MMP_USHORT  sflt_bubb_flt_div_chr;  			//HW OPR[RW]    0x3106~0x3107
    MMP_USHORT  sflt_flat_flt_en;       			//HW OPR[RW]    0x3100[5:4]
    MMP_USHORT  sflt_flat_flt_chr;      			//HW OPR[RW]    0x3100[6]              
    MMP_UBYTE   sflt_flat_flt_shp[7][7];			//HW OPR[RW]    0x3124~0x312A   (1bit/1pixel) 
    MMP_UBYTE   sflt_flat_flt_sft[7][7];			//HW OPR[RW]    0x3130~0x313C   (2bit/1pixel)
    MMP_USHORT  sflt_flat_flt_div_lum;  			//HW OPR[RW]    0x3120~0x3121
    MMP_USHORT  sflt_flat_flt_div_chr;  			//HW OPR[RW]    0x3122~0x3123
    MMP_USHORT  sflt_edge_flt_en;       			//HW OPR[RW]    0x3101[1:0]
    MMP_USHORT  sflt_edge_flt_chr;      			//HW OPR[RW]    0x3101[2]
    MMP_USHORT  sflt_edge_flt_tap;      			//HW OPR[RW]    0x3140[3:0]
    MMP_USHORT  sflt_edge_flt_div_lum;  			//HW OPR[RW]    0x3142~0x3143
    MMP_USHORT  sflt_edge_flt_div_chr;  			//HW OPR[RW]    0x3144~0x3145
    MMP_USHORT  sflt_edge_enh_en;       			//HW OPR[RW]    0x3101[5:4]
    MMP_USHORT  sflt_edge_enh_chr;      			//HW OPR[RW]    0x3101[6]
    MMP_USHORT  sflt_edge_enh_tap;      			//HW OPR[RW]    0x3146[3:0]
    MMP_USHORT  sflt_edge_enh_att;                  //在Init_Img()中初始化sflt_edge_enh_div_lum/sflt_edge_enh_div_chr
    MMP_USHORT  sflt_edge_enh_thr;      			//HW OPR[RW]    0x3147
    MMP_USHORT  sflt_edge_enh_div_lum;  			//HW OPR[RW]    0x3148~0x3149
    MMP_USHORT  sflt_edge_enh_div_chr;  			//HW OPR[RW]    0x314A~0x314B
    MMP_USHORT  sflt_div_rnd_lum;       			//HW OPR[RW]    0x314C[2:0]
    MMP_USHORT  sflt_div_rnd_chr;       			//HW OPR[RW]    0x314C[6:4]
    MMP_USHORT  sflt_bypass;            			//HW OPR[RW]    0x3008[5]

} MMPF_TNR_IMG_PARAM;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void        MMPF_TNR_ISR(void);
MMP_ERR     MMPF_TNR_SetInterruptEnable(MMP_BOOL bEnable, MMP_UBYTE ubMask);
MMP_ERR     MMPF_TNR_SetBufAddr(MMP_ULONG ulCurYAddr,      MMP_ULONG ulCurCbrAddr,
                            MMP_ULONG ulRtCurYAddr,    MMP_ULONG ulRtCurCbrAddr, 
						    MMP_ULONG ulRefYAddr,      MMP_ULONG ulRefCbrAddr,
						    MMP_ULONG ulWbufAddr,      MMP_ULONG ulEbufAddr);
MMP_ERR     MMPF_TNR_SetRtPingPongAddr(MMP_ULONG ulYAddr,  MMP_ULONG ulCbrAddr,
                                       MMP_ULONG ulY1Addr, MMP_ULONG ulCbr1Addr, MMP_UBYTE ubMbRowW);
MMP_ERR     MMPF_TNR_SetFrameRes(MMP_USHORT usW, MMP_USHORT usH);
MMP_ERR     MMPF_TNR_SetEngineEnable(MMP_BOOL bTnrEn);
MMP_ERR     MMPF_TNR_SetModuleEnable(MMP_BOOL bTfltEn, MMP_BOOL bEstbEn, MMP_BOOL bSfltEn);
MMP_ERR     MMPF_TNR_SetInOutMode(MMP_UBYTE ubPipe, MMP_UBYTE ubMode);
MMP_ERR     MMPF_TNR_SetOpMode(MMP_UBYTE ubMode);
MMP_UBYTE   MMPF_TNR_GetOpMode(void);
MMP_ERR     MMPF_TNR_SetActivePipe(MMP_UBYTE ubPipe);
MMP_UBYTE   MMPF_TNR_GetActivePipe(void);
MMP_BOOL    MMPF_TNR_IsTnrBusy(void);
MMP_ERR     MMPF_TNR_InitRefFrameBuf(MMP_UBYTE ubIbcPipe, MMP_ULONG ulBaseAddr);
MMP_ERR     MMPF_TNR_SetActiveDuration(MMP_ULONG ulStartFrm, MMP_ULONG ulEndFrm);
MMP_UBYTE   MMPF_TNR_CheckActiveStatus(void);
MMP_ERR     MMPF_TNR_ResetModule(void);

void MMPF_TNR_InitInputParam(void);
void MMPF_TNR_InitImageParam(void);
void MMPF_TNR_PreProcess(void);
void MMPF_TNR_PostProcess(void);
void MMPF_TNR_UpdateIntoOPR(void);
void MMPF_TNR_UpdateFromOPR(void);

void MMPF_TNR_InitTNR(void);
void MMPF_TNR_TriggerTNR(void);	
				 
#endif //_MMPF_TNR_H_