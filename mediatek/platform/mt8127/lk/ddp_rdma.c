#include <platform/mt_typedefs.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_rdma.h>
#include <platform/ddp_matrix_para.h>
#include <printf.h>

#ifndef ASSERT
#define ASSERT(expr) do { if(!(expr)) printf("ASSERT error func: %s, line: %d\n", __func__, __LINE__);} while (0)
#endif

#define DISP_INDEX_OFFSET 0xa000
#define RDMA_COLOR_BASE 20

int RDMAStart(unsigned idx) {
    ASSERT(idx <= 2);

    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0x3F);
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_ENGINE_EN, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 1);

    return 0;
}

int RDMAStop(unsigned idx) {
    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_ENGINE_EN, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 0);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS, 0);
    return 0;
}

int RDMAReset(unsigned idx) {
    unsigned int delay_cnt=0;
    //static unsigned int cnt=0;
    
    // printf("[DDP] RDMAReset called %d \n", cnt++);

    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_SOFT_RESET, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 1);
    while((DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON)&0x700)==0x100)
    {
        delay_cnt++;
        if(delay_cnt>10000)
        {
            printf("[DDP] error, RDMAReset(%d) timeout, stage 1! DISP_REG_RDMA_GLOBAL_CON=0x%x \n", idx, DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON));
            break;
        }
    }
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_SOFT_RESET, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 0);
    // printf("[DDP] start reset! \n");
    while((DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON)&0x700)!=0x100)
    {
        delay_cnt++;
        if(delay_cnt>10000)
        {
            printf("[DDP] error, RDMAReset(%d) timeout, stage 2! DISP_REG_RDMA_GLOBAL_CON=0x%x \n", idx, DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON));          
            break;
        }
    }   
    // printf("[DDP] end reset! \n");

#if 0    
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON     , 0x00);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0     , 0x00);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_1     , 0x00);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_CON         , 0x00);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_START_ADDR , 0x00);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_SRC_PITCH     , 0x00);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_GMC_SETTING_1 , 0x20);     ///TODO: need check
    //DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_FIFO_CON , 0x80f00008);        ///TODO: need check
#endif

    return 0;
}

unsigned int gUltraLevel = 4;
unsigned int gEnableUltra = 0;
int RDMAConfig(unsigned idx,
                    enum RDMA_MODE mode,
                    enum RDMA_INPUT_FORMAT inFormat, 
                    unsigned address, 
                    enum RDMA_OUTPUT_FORMAT outputFormat, 
                    unsigned pitch,
                    unsigned width, 
                    unsigned height, 
                    BOOL isByteSwap, // input setting
                    BOOL isRGBSwap)  // ourput setting
{
    unsigned int bpp = 2;
    unsigned int rgb_swap = 0;
    unsigned int input_is_yuv = 0;
    unsigned int output_is_yuv = 0;
    // Calculate fifo settings
    unsigned int fifo_pseudo_length = 256; //HW fifo SRAM: 240(89), 256(71/72/82/92), 512(ROME)
    unsigned int fifo_threashold;
    // Calculate ultra settings
    unsigned int fps = 60;
    unsigned int blank_overhead = 115;	  // it is 1.15, need to divide 100 later
    unsigned int rdma_fifo_width = 16;	  // in unit of byte
    unsigned int ultra_low_time = 6;      // in unit of us
    unsigned int pre_ultra_low_time = 8;  // in unit of us
    unsigned int pre_ultra_high_time = 9; // in unit of us
    unsigned int consume_levels_per_usec;
    unsigned int ultra_low_level;
    unsigned int pre_ultra_low_level;
    unsigned int pre_ultra_high_level;
    unsigned int ultra_high_ofs;
    unsigned int pre_ultra_low_ofs;
    unsigned int pre_ultra_high_ofs;
    enum RDMA_INPUT_FORMAT inputFormat = inFormat;
    
	if(gUltraLevel==0)
	{
		ultra_low_time = 4;      // in unit of us
		pre_ultra_low_time = 6;  // in unit of us
		pre_ultra_high_time = 7; // in unit of us
	}
	else if(gUltraLevel==1)
	{
		ultra_low_time = 5;      // in unit of us
		pre_ultra_low_time = 7;  // in unit of us
		pre_ultra_high_time = 8; // in unit of us
	}
	else if(gUltraLevel==2)
	{
		ultra_low_time = 6;      // in unit of us
		pre_ultra_low_time = 8;  // in unit of us
		pre_ultra_high_time = 9; // in unit of us
	}
    
    ASSERT(idx <= 2);
    if((width > RDMA_MAX_WIDTH) || (height > RDMA_MAX_HEIGHT))
    {
    	  printf("DDP error, RDMA input overflow, w=%d, h=%d, max_w=%d, max_h=%d\n", width, height, RDMA_MAX_WIDTH, RDMA_MAX_HEIGHT);
    }

    if(width==0 || height==0)
    {
        printf("DDP error, RDMA input error, w=%d, h=%d, pitch=%d\n", width, height, pitch); 
        ASSERT( width > 0);
        ASSERT( height > 0);
    }
    if( mode == RDMA_MODE_MEMORY )
    {
        ASSERT( pitch > 0);   	
        ASSERT( address > 0);
    }

    switch(inputFormat) {
    case RDMA_INPUT_FORMAT_YUYV:
    case RDMA_INPUT_FORMAT_UYVY:
    case RDMA_INPUT_FORMAT_YVYU:
    case RDMA_INPUT_FORMAT_VYUY:
    case RDMA_INPUT_FORMAT_RGB565:
        bpp = 2;
        break;
    case RDMA_INPUT_FORMAT_RGB888:
        bpp = 3;
        break;
    case RDMA_INPUT_FORMAT_ARGB:
        bpp = 4;
        break;

    default:
        printf("DDP error, unknown RDMA input format = %d\n", inputFormat);
        ASSERT(0);
    }
    // OUTPUT_VALID_FIFO_THREASHOLD = min{(DISP_WIDTH+120)*bpp/16, FIFO_PSEUDO_LENGTH}
    fifo_threashold = (width + 120) * bpp / 16;
    fifo_threashold = fifo_threashold > fifo_pseudo_length ? fifo_pseudo_length : fifo_threashold;
    //printf("RDMA: w=%d, h=%d, addr=%x, pitch=%d, mode=%d\n", width, height, address, width*bpp, mode);
    //--------------------------------------------------------
    // calculate ultra/pre-ultra setting
    // to start to issue ultra from fifo having 4us data
    // to stop  to issue ultra until fifo having 6us data
    // to start to issue pre-ultra from fifo having 6us data
    // to stop  to issue pre-ultra until fifo having 7us data
    // the sequence is ultra_low < pre_ultra_low < ultra_high < pre_ultra_high
    //                 4us         6us             6us+1level   7us
    //--------------------------------------------------------
    consume_levels_per_usec = (width*height*fps*bpp/rdma_fifo_width/100)*blank_overhead;

    // /1000000 for ultra_low_time in unit of us
    ultra_low_level      = ultra_low_time      * consume_levels_per_usec / 1000000;
    pre_ultra_low_level  = pre_ultra_low_time  * consume_levels_per_usec / 1000000;
    pre_ultra_high_level = pre_ultra_high_time * consume_levels_per_usec / 1000000;
    pre_ultra_low_ofs = pre_ultra_low_level - ultra_low_level;
    ultra_high_ofs = 1;
    pre_ultra_high_ofs = pre_ultra_high_level - pre_ultra_low_level - 1;
    //printf("RDMA%d: fifo_pseudo_length=%d, fifo_threashold=%d, ultra_low_level=%x, pre_ultra_low_level=%d, pre_ultra_high_level=%d, ultra_high_ofs=%d, pre_ultra_low_ofs=%d, pre_ultra_high_ofs=%d\n", 
    //        idx, fifo_pseudo_length, fifo_threashold, ultra_low_level, pre_ultra_low_level, pre_ultra_high_level, ultra_high_ofs, pre_ultra_low_ofs, pre_ultra_high_ofs);
	if(gUltraLevel==4)  // always ultra
	{
		ultra_low_level = 0x6b;
		pre_ultra_low_ofs = 0xa0;
		ultra_high_ofs = 1;
		pre_ultra_high_ofs = 1;
	}
	else if(gUltraLevel==3)  // always pre-ultra
	{
		ultra_low_level = 0x6b;
		pre_ultra_low_ofs = 0x36;
		ultra_high_ofs = 0x50;
		pre_ultra_high_ofs = 0x14;
	}

    switch(inputFormat) {
    case RDMA_INPUT_FORMAT_YUYV:
    case RDMA_INPUT_FORMAT_UYVY:
    case RDMA_INPUT_FORMAT_YVYU:
    case RDMA_INPUT_FORMAT_VYUY:
        input_is_yuv = 1;
        break;
        
    case RDMA_INPUT_FORMAT_RGB565:
    case RDMA_INPUT_FORMAT_RGB888:
    case RDMA_INPUT_FORMAT_ARGB:
        input_is_yuv = 0;
        break;
        
    default:
    	  printf("DDP error, unknow input format is %d\n", inputFormat);
        ASSERT(0);
    } 
    
    if(outputFormat==RDMA_OUTPUT_FORMAT_ARGB)
    {
        output_is_yuv = 0;
    }
    else
    {
        output_is_yuv = 1;
    }

    if(input_is_yuv==1 && output_is_yuv==0)
    {
    	DISP_REG_SET_FIELD(SIZE_CON_0_FLD_MATRIX_ENABLE, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 1);
    	DISP_REG_SET_FIELD(SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0x6);
	    // set color conversion matrix
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_00, coef_rdma_601_y2r[0][0] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_01, coef_rdma_601_y2r[0][1] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_02, coef_rdma_601_y2r[0][2] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_10, coef_rdma_601_y2r[1][0] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_11, coef_rdma_601_y2r[1][1] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_12, coef_rdma_601_y2r[1][2] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_20, coef_rdma_601_y2r[2][0] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_21, coef_rdma_601_y2r[2][1] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_22, coef_rdma_601_y2r[2][2] );
	
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_PRE_ADD0, coef_rdma_601_y2r[3][0]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_PRE_ADD1, coef_rdma_601_y2r[3][1]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_PRE_ADD2, coef_rdma_601_y2r[3][2]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_POST_ADD0, coef_rdma_601_y2r[4][0]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_POST_ADD1, coef_rdma_601_y2r[4][1]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_POST_ADD2, coef_rdma_601_y2r[4][2]);
    }
    else if(input_is_yuv==0 && output_is_yuv==1)
    {
        DISP_REG_SET_FIELD(SIZE_CON_0_FLD_MATRIX_ENABLE, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 1);
        DISP_REG_SET_FIELD(SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0x2);
	    // set color conversion matrix
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_00, coef_rdma_601_r2y[0][0] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_01, coef_rdma_601_r2y[0][1] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_02, coef_rdma_601_r2y[0][2] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_10, coef_rdma_601_r2y[1][0] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_11, coef_rdma_601_r2y[1][1] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_12, coef_rdma_601_r2y[1][2] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_20, coef_rdma_601_r2y[2][0] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_21, coef_rdma_601_r2y[2][1] );
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_22, coef_rdma_601_r2y[2][2] );
	
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_PRE_ADD0, coef_rdma_601_r2y[3][0]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_PRE_ADD1, coef_rdma_601_r2y[3][1]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_PRE_ADD2, coef_rdma_601_r2y[3][2]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_POST_ADD0, coef_rdma_601_r2y[4][0]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_POST_ADD1, coef_rdma_601_r2y[4][1]);
    	DISP_REG_SET(idx*DISP_INDEX_OFFSET+DISP_REG_RDMA_CF_POST_ADD2, coef_rdma_601_r2y[4][2]);
    } else {
        DISP_REG_SET_FIELD(SIZE_CON_0_FLD_MATRIX_ENABLE, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0);
        DISP_REG_SET_FIELD(SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0);
    }

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_MODE_SEL, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, mode);
    DISP_REG_SET_FIELD(MEM_CON_FLD_MEM_MODE_INPUT_FORMAT, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_CON, inputFormat);
    
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_START_ADDR, address);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_SRC_PITCH, pitch);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0x3F);
    
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_INPUT_BYTE_SWAP, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, isByteSwap);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_FORMAT, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, outputFormat);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, width);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_RGB_SWAP, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, isRGBSwap||rgb_swap);
    DISP_REG_SET_FIELD(SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_1, height);
    

    // RDMA fifo config
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_FIFO_CON , (1<<31)|(fifo_pseudo_length<<16)|fifo_threashold);
    // disp_rdma ultra high setting
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_GMC_SETTING_0, (pre_ultra_high_ofs<<24)|(ultra_high_ofs<<16)|(pre_ultra_low_ofs<<8)|ultra_low_level);
    
  return 0;
}

void RDMAWait(unsigned idx)
{
    // polling interrupt status
    unsigned int delay_cnt = 0;
    while((DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS) & 0x1) != 0x1) 
    {

		delay_cnt++;
		msleep(1);
		if(delay_cnt>100)
		{
			printf("[DDP] error:RDMA%dWait timeout \n", idx);
			break;
		}
    }
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS , 0x0);
}


void Wait_Rdma_Start(unsigned idx)
{
    // polling interrupt status
    while((DISP_REG_GET(DISP_REG_RDMA_INT_STATUS) & 0x2) != 0x2) ;
    DISP_REG_SET(DISP_REG_RDMA_INT_STATUS , 0x0);
}
