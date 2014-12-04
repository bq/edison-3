#include <platform/hdmi_drv.h>
#include <platform/hdmi_reg.h>


#define HDMIDRV_BASE  (0x14015000)
#define HDMISYS_BASE  (0x14000000)
#define HDMIPLL_BASE  (0x10209000)  //pll
#define HDMICKGEN_BASE  (0x10000000)
#define HDMIPAD_BASE  (0x10005000)


void internal_hdmi_read(unsigned int u4Reg, unsigned int *p4Data)
{
	*p4Data = (*(volatile unsigned int*)(u4Reg));
}

void internal_hdmi_write(unsigned int u4Reg, unsigned int u4data)
{
	*(volatile unsigned int*)(u4Reg) = (u4data); 
}

//////////////////////////////////////////////
unsigned int hdmi_drv_read(unsigned short u2Reg)
{
    unsigned int u4Data;
    internal_hdmi_read(HDMIDRV_BASE+u2Reg, &u4Data);
 	return u4Data;
}

void hdmi_drv_write(unsigned short u2Reg, unsigned int u4Data)
{
     internal_hdmi_write(HDMIDRV_BASE+u2Reg, u4Data);
 }
/////////////////////////////////////////////////
unsigned int hdmi_sys_read(unsigned short u2Reg)
{
    unsigned int u4Data;
    internal_hdmi_read(HDMISYS_BASE+u2Reg, &u4Data);
 	return u4Data;
}

void hdmi_sys_write(unsigned short u2Reg, unsigned int u4Data)
{
     internal_hdmi_write(HDMISYS_BASE+u2Reg, u4Data);
}
/////////////////////////////////////////////////////
unsigned int hdmi_hdmitopck_read(unsigned short u2Reg)
{
    unsigned int u4Data;
    internal_hdmi_read(HDMICKGEN_BASE+u2Reg, &u4Data);
 	return u4Data;
}

void hdmi_hdmitopck_write(unsigned short u2Reg, unsigned int u4Data)
{
     internal_hdmi_write(HDMICKGEN_BASE+u2Reg, u4Data);

}
///////////////////////////////////////////////////////
unsigned int hdmi_pll_read(unsigned short u2Reg)
{
    unsigned int u4Data;
    internal_hdmi_read(HDMIPLL_BASE+u2Reg, &u4Data);
 	return u4Data;
}

void hdmi_pll_write(unsigned short u2Reg, unsigned int u4Data)
{
     internal_hdmi_write(HDMIPLL_BASE+u2Reg, u4Data);
}
////////////////////////////////////////////////////////////
#define vWriteHdmiANA(dAddr, dVal)  (*((volatile unsigned int *)(HDMI_ANALOG_BASE + dAddr)) = (dVal))
#define dReadHdmiANA(dAddr)         (*((volatile unsigned int *)(HDMI_ANALOG_BASE + dAddr)))
#define vWriteHdmiANAMsk(dAddr, dVal, dMsk) (vWriteHdmiANA((dAddr), (dReadHdmiANA(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
/////////////////////////////////////////////////////////////////////////////////////////

#define vWriteByteHdmiGRL(dAddr, dVal)  (hdmi_drv_write(dAddr, dVal))
#define bReadByteHdmiGRL(bAddr)         (hdmi_drv_read(bAddr))
#define vWriteHdmiGRLMsk(dAddr, dVal, dMsk) (vWriteByteHdmiGRL((dAddr), (bReadByteHdmiGRL(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteHdmiSYS(dAddr, dVal)  (hdmi_sys_write(dAddr, dVal))
#define dReadHdmiSYS(dAddr)         (hdmi_sys_read(dAddr))
#define vWriteHdmiSYSMsk(dAddr, dVal, dMsk) (vWriteHdmiSYS((dAddr), (dReadHdmiSYS(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteHdmiTOPCK(dAddr, dVal)  (hdmi_hdmitopck_write(dAddr, dVal))
#define dReadHdmiTOPCK(dAddr)         (hdmi_hdmitopck_read(dAddr))
#define vWriteHdmiTOPCKMsk(dAddr, dVal, dMsk) (vWriteHdmiTOPCK((dAddr), (dReadHdmiTOPCK(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteIoPll(dAddr, dVal)  (hdmi_pll_write(dAddr, dVal))
#define dReadIoPll(dAddr)         (hdmi_pll_read(dAddr))
#define vWriteIoPllMsk(dAddr, dVal, dMsk) vWriteIoPll((dAddr), (dReadIoPll(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


void vSetHDMITxPLL(unsigned char bResIndex, unsigned char bdeepmode)
{
  unsigned char u4Feq=0;
  unsigned int v4value1=0;
  unsigned int v4value2=0;
  
  
  if((bResIndex==HDMI_VIDEO_720x480p_60Hz)||(bResIndex==HDMI_VIDEO_720x576p_50Hz))
   u4Feq = 0; //27M
  else if((bResIndex==HDMI_VIDEO_1920x1080p_60Hz)||(bResIndex==HDMI_VIDEO_1920x1080p_50Hz)
  	      ||(bResIndex==HDMI_VIDEO_1280x720p3d_60Hz)||(bResIndex==HDMI_VIDEO_1280x720p3d_50Hz)
  	      ||(bResIndex==HDMI_VIDEO_1920x1080i3d_60Hz)||(bResIndex==HDMI_VIDEO_1920x1080i3d_50Hz)
  	      ||(bResIndex==HDMI_VIDEO_1920x1080p3d_24Hz)||(bResIndex==HDMI_VIDEO_1920x1080p3d_23Hz))
   u4Feq = 2; //148M
  else
   u4Feq = 1; //74M
		
  vWriteIoPllMsk(HDMI_CON6, (0x3<<RG_HTPLL_PREDIV), RG_HTPLL_PREDIV_MASK);
  vWriteIoPllMsk(HDMI_CON6, (0x3<<RG_HTPLL_POSDIV), RG_HTPLL_POSDIV_MASK);
  vWriteIoPllMsk(HDMI_CON6, (0x1<<RG_HTPLL_IC), RG_HTPLL_IC_MASK);
  vWriteIoPllMsk(HDMI_CON6, (0x1<<RG_HTPLL_IR), RG_HTPLL_IR_MASK);
  vWriteIoPllMsk(HDMI_CON2, ((POSDIV[u4Feq][bdeepmode-1])<<RG_HDMITX_TX_POSDIV), RG_HDMITX_TX_POSDIV_MASK);
  vWriteIoPllMsk(HDMI_CON6, ((FBKSEL[u4Feq][bdeepmode-1])<<RG_HTPLL_FBKSEL), RG_HTPLL_FBKSEL_MASK);
  vWriteIoPllMsk(HDMI_CON6, ((FBKDIV[u4Feq][bdeepmode-1])<<RG_HTPLL_FBKDIV), RG_HTPLL_FBKDIV_MASK);
  vWriteIoPllMsk(HDMI_CON7, ((DIVEN[u4Feq][bdeepmode-1])<<RG_HTPLL_DIVEN), RG_HTPLL_DIVEN_MASK);
  vWriteIoPllMsk(HDMI_CON6, ((HTPLLBP[u4Feq][bdeepmode-1])<<RG_HTPLL_BP), RG_HTPLL_BP_MASK);
  vWriteIoPllMsk(HDMI_CON6, ((HTPLLBC[u4Feq][bdeepmode-1])<<RG_HTPLL_BC), RG_HTPLL_BC_MASK);
  vWriteIoPllMsk(HDMI_CON6, ((HTPLLBR[u4Feq][bdeepmode-1])<<RG_HTPLL_BR), RG_HTPLL_BR_MASK);


  v4value1 = (*(volatile unsigned int*)(0x10206170))>>26;
  v4value2 = (*(volatile unsigned int*)(0x102061c0))&0x003fffff;
  if(v4value1==0) v4value1 = 0x1c;
  if(v4value2==0) v4value2 = 0x1c1c1c;

  if((u4Feq==2)&&(bdeepmode!=HDMI_NO_DEEP_COLOR))
  {
   vWriteIoPllMsk(HDMI_CON1, RG_HDMITX_PRED_IMP, RG_HDMITX_PRED_IMP);
   vWriteIoPllMsk(HDMI_CON1, (0x6<<RG_HDMITX_PRED_IBIAS), RG_HDMITX_PRED_IBIAS_MASK);
   vWriteIoPllMsk(HDMI_CON0, (0xf<<RG_HDMITX_EN_IMP), RG_HDMITX_EN_IMP_MASK);
   vWriteIoPllMsk(HDMI_CON1, (v4value1<<RG_HDMITX_DRV_IMP), RG_HDMITX_DRV_IMP_MASK);
   vWriteIoPllMsk(HDMI_CON4, v4value2, RG_HDMITX_RESERVE_MASK);
   vWriteIoPllMsk(HDMI_CON0, (0x1b<<RG_HDMITX_DRV_IBIAS), RG_HDMITX_DRV_IBIAS_MASK);
  }
  else
  {
   vWriteIoPllMsk(HDMI_CON1, 0, RG_HDMITX_PRED_IMP);
   vWriteIoPllMsk(HDMI_CON1, (0x3<<RG_HDMITX_PRED_IBIAS), RG_HDMITX_PRED_IBIAS_MASK);
   vWriteIoPllMsk(HDMI_CON0, (0x0<<RG_HDMITX_EN_IMP), RG_HDMITX_EN_IMP_MASK);
   vWriteIoPllMsk(HDMI_CON1, (v4value1<<RG_HDMITX_DRV_IMP), RG_HDMITX_DRV_IMP_MASK);
   vWriteIoPllMsk(HDMI_CON4, v4value2, RG_HDMITX_RESERVE_MASK);
   vWriteIoPllMsk(HDMI_CON0, (0xa<<RG_HDMITX_DRV_IBIAS), RG_HDMITX_DRV_IBIAS_MASK);
  }

  //power on sequence of hdmi

}

void vTxSignalOnOff(unsigned char bOn)
{
 
  if(bOn)
  {
	  vWriteIoPllMsk(HDMI_CON7, RG_HTPLL_AUTOK_EN, RG_HTPLL_AUTOK_EN);
	  vWriteIoPllMsk(HDMI_CON6, 0, RG_HTPLL_RLH_EN);
	  vWriteIoPllMsk(HDMI_CON6, (0x3<<RG_HTPLL_POSDIV), RG_HTPLL_POSDIV_MASK);
	  vWriteIoPllMsk(HDMI_CON2, RG_HDMITX_EN_MBIAS, RG_HDMITX_EN_MBIAS);
	  udelay(3);
	  vWriteIoPllMsk(HDMI_CON6, RG_HTPLL_EN, RG_HTPLL_EN);
	  vWriteIoPllMsk(HDMI_CON2, RG_HDMITX_EN_TX_CKLDO, RG_HDMITX_EN_TX_CKLDO);
	  vWriteIoPllMsk(HDMI_CON0, (0xf<<RG_HDMITX_EN_SLDO), RG_HDMITX_EN_SLDO_MASK);
	  udelay(200);
	  vWriteIoPllMsk(HDMI_CON2, RG_HDMITX_MBIAS_LPF_EN, RG_HDMITX_MBIAS_LPF_EN);
	  vWriteIoPllMsk(HDMI_CON2, RG_HDMITX_EN_TX_POSDIV, RG_HDMITX_EN_TX_POSDIV);
	  vWriteIoPllMsk(HDMI_CON0, (0xf<<RG_HDMITX_EN_SER), RG_HDMITX_EN_SER_MASK);
	  vWriteIoPllMsk(HDMI_CON0, (0xf<<RG_HDMITX_EN_PRED), RG_HDMITX_EN_PRED_MASK);
	  vWriteIoPllMsk(HDMI_CON0, (0xf<<RG_HDMITX_EN_DRV), RG_HDMITX_EN_DRV_MASK);
	  udelay(3);
  }
  else
  {
	  vWriteIoPllMsk(HDMI_CON0, 0, RG_HDMITX_EN_DRV_MASK);
	  vWriteIoPllMsk(HDMI_CON0, 0, RG_HDMITX_EN_PRED_MASK);
	  vWriteIoPllMsk(HDMI_CON0, 0, RG_HDMITX_EN_SER_MASK);
	  vWriteIoPllMsk(HDMI_CON2, 0, RG_HDMITX_EN_TX_POSDIV);
	  vWriteIoPllMsk(HDMI_CON2, 0, RG_HDMITX_MBIAS_LPF_EN);
	  udelay(200);
	  vWriteIoPllMsk(HDMI_CON0, 0, RG_HDMITX_EN_SLDO_MASK);
	  vWriteIoPllMsk(HDMI_CON2, 0, RG_HDMITX_EN_TX_CKLDO);
	  vWriteIoPllMsk(HDMI_CON6, 0, RG_HTPLL_EN);
	  udelay(3);
	  vWriteIoPllMsk(HDMI_CON2, 0, RG_HDMITX_EN_MBIAS);
	  vWriteIoPllMsk(HDMI_CON6, 0, RG_HTPLL_POSDIV_MASK);
	  vWriteIoPllMsk(HDMI_CON6, 0, RG_HTPLL_RLH_EN);
	  vWriteIoPllMsk(HDMI_CON7, 0, RG_HTPLL_AUTOK_EN);
	  udelay(3);
  }
}

void vConfigHdmiSYS(unsigned char bResIndex)
{
  unsigned char u4Feq=0;
  
  if((bResIndex==HDMI_VIDEO_720x480p_60Hz)||(bResIndex==HDMI_VIDEO_720x576p_50Hz))
   u4Feq = 0; //27M
  else if((bResIndex==HDMI_VIDEO_1920x1080p_60Hz)||(bResIndex==HDMI_VIDEO_1920x1080p_50Hz)
  	      ||(bResIndex==HDMI_VIDEO_1280x720p3d_60Hz)||(bResIndex==HDMI_VIDEO_1280x720p3d_50Hz)
  	      ||(bResIndex==HDMI_VIDEO_1920x1080i3d_60Hz)||(bResIndex==HDMI_VIDEO_1920x1080i3d_50Hz)
  	      ||(bResIndex==HDMI_VIDEO_1920x1080p3d_24Hz)||(bResIndex==HDMI_VIDEO_1920x1080p3d_23Hz))
   u4Feq = 2; //148M
  else
   u4Feq = 1; //74M	
   
  vWriteHdmiTOPCKMsk(HDMICLK_CFG_4, 0, CLK_DPI1_SEL);
  vWriteHdmiTOPCKMsk(HDMICLK_CFG_5, 0, CLK_HDMIPLL_SEL);
 
  vWriteHdmiSYSMsk(HDMI_SYS_CFG110,ENABLE_DPICLK|ENABLE_IDCLK|ENABLE_PLLCLK, DISABLE_DPICLK|DISABLE_IDCLK|DISABLE_PLLCLK); 
  
  vWriteHdmiSYSMsk(HDMI_SYS_CFG110,ENABLE_SPDIFCLK|ENABLE_BCLK, DISABLE_SPDIFCLK|DISABLE_BCLK); 
  
  vWriteHdmiSYSMsk(HDMI_SYS_CFG20, 0, HDMI_OUT_FIFO_EN| MHL_MODE_ON);  
  udelay(100);
  vWriteHdmiSYSMsk(HDMI_SYS_CFG20, HDMI_OUT_FIFO_EN, HDMI_OUT_FIFO_EN| MHL_MODE_ON);   

  if(u4Feq==2)
   vWriteHdmiTOPCKMsk(HDMICLK_CFG_4, DPI1_H_CK, CLK_DPI1_SEL);
  else if(u4Feq==1)
   vWriteHdmiTOPCKMsk(HDMICLK_CFG_4, DPI1_D2, CLK_DPI1_SEL);
  else if(u4Feq==0)
   vWriteHdmiTOPCKMsk(HDMICLK_CFG_4, DPI1_D4, CLK_DPI1_SEL);

  vWriteHdmiTOPCKMsk(HDMICLK_CFG_5, HDMIPLL_CTS, CLK_HDMIPLL_SEL);
  //vWriteHdmiTOPCKMsk(HDMICLK_CFG_9, AD_APLL_CK, CLK_APLL_SEL);
}


void vEnableDeepColor(unsigned char ui1Mode)
{
  unsigned int u4Data;
  
  if(ui1Mode == HDMI_DEEP_COLOR_10_BIT)
  {
  	u4Data = COLOR_10BIT_MODE;
  }
  else if(ui1Mode == HDMI_DEEP_COLOR_12_BIT)
  {
  	u4Data = COLOR_12BIT_MODE;
  }	
  else if(ui1Mode == HDMI_DEEP_COLOR_16_BIT)
  {
  	u4Data = COLOR_16BIT_MODE;
  }	
  else 
  {
  	u4Data = COLOR_8BIT_MODE;
  }	
 	
  if(u4Data == COLOR_8BIT_MODE)	
  {
  	vWriteHdmiSYSMsk(HDMI_SYS_CFG20, u4Data, DEEP_COLOR_MODE_MASK| DEEP_COLOR_EN);	
  }
  else
  {
  	vWriteHdmiSYSMsk(HDMI_SYS_CFG20, u4Data|DEEP_COLOR_EN, DEEP_COLOR_MODE_MASK| DEEP_COLOR_EN);		
  }		

}

void vEnableHdmiMode(unsigned char bOn)
{ 
 unsigned char bData;
 if(bOn==1)
  {
    bData=bReadByteHdmiGRL(GRL_CFG1);
    bData &= ~CFG1_DVI;//enable HDMI mode
    vWriteByteHdmiGRL(GRL_CFG1,bData);
  }
  else
  {
    bData=bReadByteHdmiGRL(GRL_CFG1);
    bData |= CFG1_DVI;//disable HDMI mode
    vWriteByteHdmiGRL(GRL_CFG1,bData);
  }
  
}

void vEnableNCTSAutoWrite(void)
{
  unsigned char bData;
  bData=bReadByteHdmiGRL(GRL_DIVN);
  bData |= NCTS_WRI_ANYTIME;//enabel N-CTS can be written in any time
  vWriteByteHdmiGRL(GRL_DIVN,bData);
	
}	

void vResetHDMI(unsigned char bRst)
{
  if(bRst)
  {
    vWriteHdmiSYSMsk(HDMI_SYS_CFG1C,HDMI_RST,HDMI_RST);
  }
  else
  {
	vWriteHdmiSYSMsk(HDMI_SYS_CFG1C,0,HDMI_RST);
	vWriteHdmiGRLMsk(GRL_CFG3,0,CFG3_CONTROL_PACKET_DELAY);//Designer suggest adjust Control packet deliver time
	vWriteHdmiSYSMsk(HDMI_SYS_CFG1C,ANLG_ON,ANLG_ON);
  }
}

void vEnableNotice(unsigned char bOn)
{
  unsigned char bData;
  if(bOn == 1)
  {
     bData=bReadByteHdmiGRL(GRL_CFG2);
     bData |= 0x40; //temp. solve 720p issue. to avoid audio packet jitter problem
     vWriteByteHdmiGRL(GRL_CFG2, bData);	
  }	
  else
  {
     bData=bReadByteHdmiGRL(GRL_CFG2);
     bData &= ~0x40; 
     vWriteByteHdmiGRL(GRL_CFG2, bData);	
  }
}	

void vHwNCTSOnOff(unsigned char bHwNctsOn)
{
  unsigned char bData;	
  bData=bReadByteHdmiGRL(GRL_CTS_CTRL);
  
  if(bHwNctsOn == 1)
  bData &= ~CTS_CTRL_SOFT;
  else
  bData |= CTS_CTRL_SOFT;
  
  vWriteByteHdmiGRL(GRL_CTS_CTRL, bData);
   
}   

void vSetChannelSwap(unsigned char u1SwapBit)
{
  vWriteHdmiGRLMsk(GRL_CH_SWAP, u1SwapBit, 0xff);
}

void vEnableIecTxRaw(void)
{
  unsigned char bData;
  bData=bReadByteHdmiGRL(GRL_MIX_CTRL);
  bData |= MIX_CTRL_FLAT; 
  vWriteByteHdmiGRL(GRL_MIX_CTRL, bData);
}  

void vSetHdmiI2SDataFmt(unsigned char bFmt) 
{
  unsigned char bData;
  bData=bReadByteHdmiGRL(GRL_CFG0);
  bData &=~0x33;
  switch(bFmt)
  {
    case RJT_24BIT:
      bData |= (CFG0_I2S_MODE_RTJ|CFG0_I2S_MODE_24Bit);
    break;
    
    case RJT_16BIT:
      bData |= (CFG0_I2S_MODE_RTJ|CFG0_I2S_MODE_16Bit);
    break;
    
    case LJT_24BIT:
      bData |= (CFG0_I2S_MODE_LTJ|CFG0_I2S_MODE_24Bit);
    break;
    
    case LJT_16BIT:
      bData |= (CFG0_I2S_MODE_LTJ|CFG0_I2S_MODE_16Bit);
    break;
    
    case I2S_24BIT:
      bData |= (CFG0_I2S_MODE_I2S|CFG0_I2S_MODE_24Bit);
    break;
    
    case I2S_16BIT:
      bData |= (CFG0_I2S_MODE_I2S|CFG0_I2S_MODE_16Bit);
    break;	
  	
  }	
 

  vWriteByteHdmiGRL(GRL_CFG0, bData);
}  

void vAOUT_BNUM_SEL(unsigned char  bBitNum)
{
  vWriteByteHdmiGRL(GRL_AOUT_BNUM_SEL, bBitNum);
	
}	

void vSetHdmiHighBitrate(unsigned char fgHighBitRate)
{
  unsigned char bData;
  if(fgHighBitRate ==1)
  {
  	bData=bReadByteHdmiGRL(GRL_AOUT_BNUM_SEL);
    bData |= HIGH_BIT_RATE_PACKET_ALIGN;
    vWriteByteHdmiGRL(GRL_AOUT_BNUM_SEL, bData);
     udelay(100);//1ms
    bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
    bData |= HIGH_BIT_RATE;
    vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);
  }
  else
  {
  	bData=bReadByteHdmiGRL(GRL_AOUT_BNUM_SEL);
    bData &= ~HIGH_BIT_RATE_PACKET_ALIGN;
    vWriteByteHdmiGRL(GRL_AOUT_BNUM_SEL, bData);
    
  	bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
    bData &= ~HIGH_BIT_RATE;
    vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);
  }	
	
	
}	

void vDSTNormalDouble(unsigned char fgEnable) 
{
  unsigned char bData;	
  if(fgEnable)	
  {
    bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
    bData |=DST_NORMAL_DOUBLE;
    vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);
  }
  else
  {
    bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
    bData&=~DST_NORMAL_DOUBLE;
    vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);	
  }	
   
} 

void vEnableDSTConfig(unsigned char fgEnable) 
{
  unsigned char bData;	
  if(fgEnable)	
  {
    bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
    bData |=SACD_DST;
    vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);
  }
  else
  {
    bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
    bData&=~SACD_DST;
    vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);	
  }	
   
} 

void vDisableDsdConfig(void) 
{
  unsigned char bData;	
  	
  bData=bReadByteHdmiGRL(GRL_AUDIO_CFG);
  bData&=~SACD_SEL;
  vWriteByteHdmiGRL(GRL_AUDIO_CFG, bData);
   
} 

void vSetHdmiI2SChNum(unsigned char bChNum, unsigned char bChMapping)
{
    unsigned char bData, bData1, bData2, bData3;
    
    if(bChNum==2)//I2S 2ch
    {
      bData = 0x04;//2ch data
      bData1 = 0x50;//data0
      

    }
    else if((bChNum==3)||(bChNum==4))//I2S 2ch
    {
      if((bChNum==4)&&(bChMapping == 0x08))
      {
        bData = 0x14;//4ch data
       
      }
      else
      {
      bData = 0x0c;//4ch data
      }
      bData1 = 0x50;//data0
      

    }
    else if((bChNum == 6)||(bChNum == 5))//I2S 5.1ch
    {
     if((bChNum==6)&&(bChMapping == 0x0E))
     {
       bData = 0x3C;//6.0 ch data
       bData1 = 0x50;//data0	
     }	
     else
     {
       bData = 0x1C;//5.1ch data, 5/0ch
       bData1 = 0x50;//data0
     } 
    

    }
    else if(bChNum == 8)//I2S 5.1ch
    {
      bData = 0x3C;//7.1ch data
      bData1 = 0x50;//data0
    }	
    else if(bChNum == 7)//I2S 6.1ch
    {
      bData = 0x3C;//6.1ch data
      bData1 = 0x50;//data0
    }	
    else
    {
      bData = 0x04;//2ch data
      bData1 = 0x50;//data0	
    }	 
  
    bData2=0xc6;
    bData3=0xfa;
  
    vWriteByteHdmiGRL(GRL_CH_SW0, bData1);
    vWriteByteHdmiGRL(GRL_CH_SW1, bData2);
    vWriteByteHdmiGRL(GRL_CH_SW2, bData3);
    vWriteByteHdmiGRL(GRL_I2S_UV, bData);
    
    //vDisableDsdConfig(); 
	
}

void vSetHdmiIecI2s(unsigned char bIn)
{
  unsigned char bData;	

  {
     bData=bReadByteHdmiGRL(GRL_CFG1);
     if(bData&CFG1_SPDIF)
     {
        bData &= ~CFG1_SPDIF;
        vWriteByteHdmiGRL(GRL_CFG1, bData);
      }
      bData=bReadByteHdmiGRL(GRL_CFG1);
  }	
}	

void vSetHDMISRCOff(void)
{
  unsigned char bData;
  
  bData=bReadByteHdmiGRL(GRL_MIX_CTRL);
  bData &= ~MIX_CTRL_SRC_EN;
  vWriteByteHdmiGRL(GRL_MIX_CTRL, bData);
  bData = 0x00;
  vWriteByteHdmiGRL(GRL_SHIFT_L1, bData);
}

void vSetHDMIFS(unsigned char bFs, unsigned char fgAclInv)
{
  
  unsigned char bData;
  
  bData=bReadByteHdmiGRL(GRL_CFG5);
  bData &= CFG5_CD_RATIO_MASK;
  bData |= bFs;
  vWriteByteHdmiGRL(GRL_CFG5, bData);

   if(fgAclInv ==1)
  {
    bData=bReadByteHdmiGRL(GRL_CFG2);
    bData |= 0x80; 
    vWriteByteHdmiGRL(GRL_CFG2, bData);	
  }	
  else
  {
    bData=bReadByteHdmiGRL(GRL_CFG2);
    bData &= ~0x80; 
    vWriteByteHdmiGRL(GRL_CFG2, bData);	
  }	

}

void vHalHDMI_NCTS(unsigned char bAudioFreq, unsigned char bPix, unsigned char bDeepMode)
{
  unsigned char bTemp, bData, bData1[NCTS_BYTES];
  unsigned int u4Temp, u4NTemp=0;
  
   	
  bData=0;
  vWriteByteHdmiGRL(GRL_NCTS, bData);//YT suggest 3 dummy N-CTS
  vWriteByteHdmiGRL(GRL_NCTS, bData);
  vWriteByteHdmiGRL(GRL_NCTS, bData);

  for(bTemp=0; bTemp<NCTS_BYTES; bTemp++)
  {
    bData1[bTemp] = 0;	
  }
  	
  if(bDeepMode == HDMI_NO_DEEP_COLOR)
  {
    for(bTemp=0; bTemp<NCTS_BYTES; bTemp++)
    {

      if((bAudioFreq < 7) && (bPix < 9))

      bData1[bTemp]= HDMI_NCTS[bAudioFreq][bPix][bTemp];
    }
    
    u4NTemp = (bData1[4]<<16)|(bData1[5]<<8)|(bData1[6]);//N
    u4Temp = (bData1[0]<<24)|(bData1[1]<<16)|(bData1[2]<<8)|(bData1[3]);//CTS
    
  }
  else
  {
  	for(bTemp=0; bTemp<NCTS_BYTES; bTemp++)
    {
		if((bAudioFreq < 7) && (bPix < 9)) 

      bData1[bTemp] = HDMI_NCTS[bAudioFreq][bPix][bTemp];
    }
    
    u4NTemp = (bData1[4]<<16)|(bData1[5]<<8)|(bData1[6]);//N
    u4Temp = (bData1[0]<<24)|(bData1[1]<<16)|(bData1[2]<<8)|(bData1[3]);
    
    if(bDeepMode == HDMI_DEEP_COLOR_10_BIT)
    {
      u4Temp = (u4Temp >> 2)*5;// (*5/4) 
    }
    else if(bDeepMode == HDMI_DEEP_COLOR_12_BIT)
    {
      u4Temp = (u4Temp >> 1)*3;// (*3/2) 
    }
    else if(bDeepMode == HDMI_DEEP_COLOR_16_BIT)
    {
      u4Temp = (u4Temp <<1);// (*2) 
    }
    
    bData1[0]= (u4Temp >> 24)& 0xff;
    bData1[1]= (u4Temp >> 16)& 0xff;
    bData1[2]= (u4Temp >> 8)& 0xff;
    bData1[3]= (u4Temp)& 0xff;
    	
  }
  for(bTemp=0; bTemp<NCTS_BYTES; bTemp++)
  {
      bData = bData1[bTemp];
      vWriteByteHdmiGRL( GRL_NCTS, bData);
  }

}

void vHDMI_NCTS(unsigned char bHDMIFsFreq, unsigned char bResolution, unsigned char bdeepmode)
{
  unsigned char  bPix;
  
  
  vWriteHdmiGRLMsk(DUMMY_304, AUDIO_I2S_NCTS_SEL_64, AUDIO_I2S_NCTS_SEL);
  	
  switch(bResolution)
  {
    case HDMI_VIDEO_720x480p_60Hz:
    case HDMI_VIDEO_720x576p_50Hz:
    default:
      bPix = 0;
      break;

    case HDMI_VIDEO_1280x720p_60Hz: //74.175M pixel clock
    case HDMI_VIDEO_1920x1080i_60Hz:
    case HDMI_VIDEO_1920x1080p_23Hz:
      bPix = 2;
      break;
	  
    case HDMI_VIDEO_1280x720p_50Hz: //74.25M pixel clock
    case HDMI_VIDEO_1920x1080i_50Hz:
    case HDMI_VIDEO_1920x1080p_24Hz:
      bPix = 3;
      break;
    case HDMI_VIDEO_1920x1080p_60Hz: //148.35M pixel clock
    case HDMI_VIDEO_1280x720p3d_60Hz:
    case HDMI_VIDEO_1920x1080i3d_60Hz:
	case HDMI_VIDEO_1920x1080p3d_23Hz:	
      bPix = 4;
      break;
    case HDMI_VIDEO_1920x1080p_50Hz: //148.50M pixel clock
    case HDMI_VIDEO_1280x720p3d_50Hz:
    case HDMI_VIDEO_1920x1080i3d_50Hz:
	case HDMI_VIDEO_1920x1080p3d_24Hz:
      bPix = 5;
      break;	  
  }

    vHalHDMI_NCTS(bHDMIFsFreq, bPix, bdeepmode);

}

void vReEnableSRC(void)
{
  unsigned char bData;	
  
  bData=bReadByteHdmiGRL( GRL_MIX_CTRL);
  if(bData & MIX_CTRL_SRC_EN)
  {
    bData &= ~MIX_CTRL_SRC_EN;
    vWriteByteHdmiGRL( GRL_MIX_CTRL, bData);
    udelay(255);
    bData |= MIX_CTRL_SRC_EN;
    vWriteByteHdmiGRL(GRL_MIX_CTRL, bData);
  }
  
}

void vHwSet_Hdmi_I2S_C_Status (unsigned char *prLChData, unsigned char *prRChData)
{
  unsigned char bData;
  
  bData = prLChData[0];

  vWriteByteHdmiGRL(GRL_I2S_C_STA0, bData);   
  vWriteByteHdmiGRL(GRL_L_STATUS_0, bData);
  
  bData = prRChData[0];

  vWriteByteHdmiGRL(GRL_R_STATUS_0, bData);
  
  bData= prLChData[1];
  vWriteByteHdmiGRL( GRL_I2S_C_STA1, bData);   
  vWriteByteHdmiGRL(GRL_L_STATUS_1, bData);
  bData= prRChData[1];
  vWriteByteHdmiGRL(GRL_R_STATUS_1, bData);
  
  bData= prLChData[2];
  vWriteByteHdmiGRL(GRL_I2S_C_STA2, bData);    
  vWriteByteHdmiGRL(GRL_L_STATUS_2, bData);
  bData= prRChData[2];
  vWriteByteHdmiGRL(GRL_R_STATUS_2, bData);

  bData= prLChData[3];
  vWriteByteHdmiGRL(GRL_I2S_C_STA3, bData);   
  vWriteByteHdmiGRL(GRL_L_STATUS_3, bData);
  bData= prRChData[3];
  vWriteByteHdmiGRL(GRL_R_STATUS_3, bData);

  bData=prLChData[4];
  vWriteByteHdmiGRL( GRL_I2S_C_STA4, bData);   
  vWriteByteHdmiGRL(GRL_L_STATUS_4, bData);
  bData=prRChData[4];
  vWriteByteHdmiGRL(GRL_R_STATUS_4, bData);
  
  for(bData =0; bData < 19; bData++)
  {
    vWriteByteHdmiGRL(GRL_L_STATUS_5+bData*4, 0);
    vWriteByteHdmiGRL(GRL_R_STATUS_5+bData*4, 0);
  	
  }	
}

void vHDMI_I2S_C_Status(void)
{
  unsigned char bData = 0;
  unsigned char bhdmi_RCh_status[5];
  unsigned char bhdmi_LCh_status[5];
  
  bhdmi_LCh_status[0]= 0;
  bhdmi_LCh_status[1]= 0;
  bhdmi_LCh_status[2]= 2;
  bhdmi_RCh_status[0]= 0;
  bhdmi_RCh_status[1]= 0;
  bhdmi_RCh_status[2]= 2;


  bhdmi_LCh_status[0]&= ~0x02;
  bhdmi_RCh_status[0]&= ~0x02;

  bData = 0;
 
  bhdmi_LCh_status[3] = bData;   
  bhdmi_RCh_status[3] = bData;  

  bData= 0;

  bData |= ((~( bhdmi_LCh_status[3] & 0x0f))<<4);

  bhdmi_LCh_status[4]= bData;
  bhdmi_RCh_status[4]= bData;

  vHwSet_Hdmi_I2S_C_Status (&bhdmi_LCh_status[0], &bhdmi_RCh_status[0]);

}

void vHalSendAudioInfoFrame(unsigned char bData1,unsigned char bData2,unsigned char bData4,unsigned char bData5)
{
  unsigned char bAUDIO_CHSUM;
  unsigned char bData=0;
 
  vWriteHdmiGRLMsk(GRL_CTRL, 0, CTRL_AUDIO_EN);
  vWriteByteHdmiGRL(GRL_INFOFRM_VER, AUDIO_VERS);
  vWriteByteHdmiGRL(GRL_INFOFRM_TYPE, AUDIO_TYPE);
  vWriteByteHdmiGRL(GRL_INFOFRM_LNG, AUDIO_LEN);
  
  bAUDIO_CHSUM = AUDIO_TYPE + AUDIO_VERS + AUDIO_LEN;

  bAUDIO_CHSUM += bData1;
  bAUDIO_CHSUM += bData2;  
  bAUDIO_CHSUM += bData4;
  bAUDIO_CHSUM += bData5;

  bAUDIO_CHSUM = 0x100 - bAUDIO_CHSUM;
  vWriteByteHdmiGRL(GRL_IFM_PORT, bAUDIO_CHSUM);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData1);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData2);//bData2
  vWriteByteHdmiGRL(GRL_IFM_PORT, 0);//bData3
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData4);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData5);

  for(bData=0; bData<5; bData++)
  {
    vWriteByteHdmiGRL(GRL_IFM_PORT, 0);
  }
  bData=bReadByteHdmiGRL( GRL_CTRL);
  bData |= CTRL_AUDIO_EN;
  vWriteByteHdmiGRL(GRL_CTRL, bData);

}

void vSendAudioInfoFrame(void)
{
 
 vHalSendAudioInfoFrame(1,0,0,0);

}

void vHalSendAVIInfoFrame(unsigned char *pr_bData)
{
  unsigned char bAVI_CHSUM;
  unsigned char bData1=0, bData2=0, bData3=0, bData4=0, bData5=0;
  unsigned char bData;
  
  bData1= *pr_bData;
  bData2= *(pr_bData+1);
  bData3= *(pr_bData+2);
  bData4= *(pr_bData+3);
  bData5= *(pr_bData+4);
  
  vWriteHdmiGRLMsk(GRL_CTRL, 0, CTRL_AVI_EN);
  vWriteByteHdmiGRL(GRL_INFOFRM_VER, AVI_VERS);
  vWriteByteHdmiGRL(GRL_INFOFRM_TYPE, AVI_TYPE);
  vWriteByteHdmiGRL(GRL_INFOFRM_LNG, AVI_LEN);
  
  bAVI_CHSUM = AVI_TYPE + AVI_VERS + AVI_LEN;

  bAVI_CHSUM += bData1;
  bAVI_CHSUM += bData2;
  bAVI_CHSUM += bData3; 
  bAVI_CHSUM += bData4;
  bAVI_CHSUM += bData5;
  bAVI_CHSUM = 0x100 - bAVI_CHSUM;
  vWriteByteHdmiGRL(GRL_IFM_PORT, bAVI_CHSUM);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData1);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData2);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData3);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData4);
  vWriteByteHdmiGRL(GRL_IFM_PORT, bData5);
  
  for(bData2=0; bData2<8; bData2++)
  {
    vWriteByteHdmiGRL(GRL_IFM_PORT, 0);
  }
  bData=bReadByteHdmiGRL(GRL_CTRL);
  bData |= CTRL_AVI_EN;
  vWriteByteHdmiGRL(GRL_CTRL, bData);
}

void vSendAVIInfoFrame(unsigned char ui1resindex, unsigned char ui1colorspace)
{
  unsigned char AviInfoFm[5];
  if(ui1colorspace == HDMI_YCBCR_444)
  {
    AviInfoFm[0]=0x40;
  }
  else if(ui1colorspace == HDMI_YCBCR_422)
  {
    AviInfoFm[0]=0x20;
  }
  else
  {
    AviInfoFm[0]=0x00;
  }
  
  AviInfoFm[0] |= 0x10; //A0=1, Active format (R0~R3) inf valid
  
  AviInfoFm[1]=0x0;//bData2
  
  
  if((ui1resindex==HDMI_VIDEO_720x480p_60Hz)||(ui1resindex==HDMI_VIDEO_720x576p_50Hz))
  {
    AviInfoFm[1] |= AV_INFO_SD_ITU601;
  }
  else
  {
    AviInfoFm[1] |= AV_INFO_HD_ITU709;
  }
  
  AviInfoFm[1] |= 0x20;
  AviInfoFm[1] |= 0x08;
  AviInfoFm[2] =0; //bData3
  AviInfoFm[2] |= 0x04; //limit Range
  AviInfoFm[3] = HDMI_VIDEO_ID_CODE[ui1resindex];//bData4
  
  if((AviInfoFm[1] & AV_INFO_16_9_OUTPUT) && ((ui1resindex==HDMI_VIDEO_720x480p_60Hz)||(ui1resindex==HDMI_VIDEO_720x576p_50Hz)))
  {
    AviInfoFm[3] = AviInfoFm[3]+1;
  }
  
    AviInfoFm[4]  = 0x00;
  
  vHalSendAVIInfoFrame(&AviInfoFm[0]);

}



void vSend_AVUNMUTE(void) 
{
  unsigned char bData;

  bData=bReadByteHdmiGRL(GRL_CFG4);
  bData |= CFG4_AV_UNMUTE_EN;//disable original mute
  bData &= ~CFG4_AV_UNMUTE_SET; //disable 
  
  vWriteByteHdmiGRL(GRL_CFG4, bData);
  udelay(30);
  
  bData &= ~CFG4_AV_UNMUTE_EN;//disable original mute
  bData |= CFG4_AV_UNMUTE_SET; //disable 
  
  vWriteByteHdmiGRL(GRL_CFG4, bData);

}

void set_hdmi_tmds_driver(unsigned int resolutionmode)
{

	vWriteHdmiTOPCK(INFRA_PDN1, CEC_PDN1);
	vWriteHdmiTOPCK(PREICFG_PDN_CLR, DDC_PDN_CLR);

  	vWriteHdmiSYSMsk(HDMI_SYS_CFG20, HDMI_PCLK_FREE_RUN, HDMI_PCLK_FREE_RUN);
	vWriteHdmiSYSMsk(HDMI_SYS_CFG1C, ANLG_ON|HDMI_ON, ANLG_ON|HDMI_ON);

	vSetHDMITxPLL(resolutionmode, HDMI_NO_DEEP_COLOR);
	vTxSignalOnOff(SV_ON);
	vConfigHdmiSYS(resolutionmode);
	vEnableDeepColor(HDMI_NO_DEEP_COLOR);

	vResetHDMI(1);
	vResetHDMI(0);
	vWriteHdmiGRLMsk(VIDEO_CFG_4, NORMAL_PATH, VIDEO_SOURCE_SEL);
	vEnableNotice(1);
	vEnableHdmiMode(1);
	vEnableNCTSAutoWrite();
    vWriteHdmiGRLMsk(GRL_CFG4,0,CFG_MHL_MODE);

    if((resolutionmode==HDMI_VIDEO_1920x1080i_50Hz)||(resolutionmode==HDMI_VIDEO_1920x1080i_60Hz))
     vWriteHdmiGRLMsk(GRL_CFG2,0,MHL_DE_SEL);
    else
     vWriteHdmiGRLMsk(GRL_CFG2,MHL_DE_SEL,MHL_DE_SEL);

	vHwNCTSOnOff(0);

    vSetChannelSwap(LFE_CC_SWAP);
    vEnableIecTxRaw();
    vSetHdmiI2SDataFmt(HDMI_LJT_24BIT);
    vAOUT_BNUM_SEL(AOUT_24BIT);
    vSetHdmiHighBitrate(0);
    vDSTNormalDouble(0);
    vEnableDSTConfig(0);
    vDisableDsdConfig();
    vSetHdmiI2SChNum(2, 0);
    vSetHdmiIecI2s(SV_I2S);
    vSetHDMISRCOff();
    vSetHDMIFS(CFG5_FS128, 0);

    vHDMI_NCTS(HDMI_FS_44K, resolutionmode,  HDMI_NO_DEEP_COLOR);
	vReEnableSRC();
	vHDMI_I2S_C_Status();
	vSendAudioInfoFrame();
	vHwNCTSOnOff(1);

	vSendAVIInfoFrame(resolutionmode,  HDMI_RGB);

	vSend_AVUNMUTE();
	vWriteByteHdmiGRL(GRL_AUDIO_CFG, 0x0);
}

