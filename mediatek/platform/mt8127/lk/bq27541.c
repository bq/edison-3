#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>
#include <platform/bq27541.h>
#include <printf.h>

/**********************************************************
  *
  *   [I2C Slave Setting]
  *
  *********************************************************/
#define bq27541_SLAVE_ADDR_WRITE   0xAA
#define bq27541_SLAVE_ADDR_READ    0xAB

/**********************************************************
  *
  *   [Global Variable]
  *
  *********************************************************/

/**********************************************************
  *
  *   [I2C Function For Read/Write bq27541]
  *
  *********************************************************/
#ifndef BQ27541_BUSNUM
#define BQ27541_BUSNUM 1
#endif  
	
kal_uint16 bq27541_i2c_read16(kal_uint16 addr)
{
    u8 rxBuf[8] = {0};
    u8 lens;
    U32 ret_code = 0;
    u16 data;
    mt_i2c i2c = {0};

    i2c.id = BQ27541_BUSNUM;
    i2c.addr = bq27541_SLAVE_ADDR_WRITE>>1;
    i2c.mode = ST_MODE;
    i2c.speed = 100;
    i2c.dma_en = 0;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        rxBuf[0] = (addr >> 8) & 0xFF;
        lens = 1;
    }
    else // 16 bit : noraml mode
    {
        rxBuf[1] = ( addr >> 8 ) & 0xFF;
        rxBuf[0] = addr & 0xFF;     
        lens = 2;
    }

    ret_code = i2c_write(&i2c, rxBuf, lens);
    if (ret_code != I2C_OK)
        return ret_code;

    lens = 2;
    ret_code = i2c_read(&i2c, rxBuf, lens);
    if (ret_code != I2C_OK)
    {
        return ret_code;
    }
    
    data = (rxBuf[1] << 8) | (rxBuf[0]); //LSB fisrt
    
    return data;
    
}



kal_uint16 bq27541_get_temperture(void)
{
    kal_uint16 ret=0;

    ret=bq27541_i2c_read16((kal_uint16)(BQ27541_CMD_Temperature));

    ret = (ret/10) - 273;
    return ret;
}

