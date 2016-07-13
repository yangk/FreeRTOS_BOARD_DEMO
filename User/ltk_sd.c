/**
  ******************************************************************************
  * 文件: ltk_sd.c
  * 作者: LeiTek (leitek.taobao.com)
  * 版本: V1.0.0
  * 描述: SPI SD 中间件，启动SD卡，SD卡数据读写等功能
  ******************************************************************************
  *
  *                  版权所有 (C), LeiTek (leitek.taobao.com)
  *                                www.leitek.com
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "ltk_spi.h"
#include "ltk_sd.h"
#include "ltk_dma.h"
#include "ltk_systick.h"
#include "ltk_semihosting.h"
#include "ltk_gpio.h"
#include "ltk_rcc.h"
#include "ltk_debug.h"

#define CMD8_ECHO   0x1AA

static sd_card_info_t sd_card_information;
  
/**
  * 描述: 得到变量 sd_card_information 的地址
  * 参数: 无
  * 返回: 变量 sd_card_information 的地址
  */
sd_card_info_t *ltk_spi_sd_get_card_info(void)
{
    return &sd_card_information;
}

/**
  * 描述: 等待 SPI 总线就绪
  * 参数: 无
  * 返回: SPI 总线状态
  */
__inline uint8_t ltk_spi_sd_cs_ready(void)
{
    uint16_t i = 0;
    uint16_t tmp = 0;
    do
    {
        tmp = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }while(tmp != 0xFF && tmp != 0x00 && ++i < 0xFFFD);

    if(i >= 0xFFFD)
        return 0;
    return 1;        
}

/**
  * 描述: 向 SD 卡发送命令
  * 参数: cmd: 命令
  *       argument: 命令参数
  *       response_type: 命令响应类型
  *       *response: 命令响应值
  * 返回: 命令响应值的第 0 位
  */
uint8_t ltk_spi_sd_send_command(uint8_t cmd, uint32_t argument,  
                                sd_response_t response_type, uint8_t *response)
{
    int32_t i = 0;
    uint8_t crc = 0x01;
    int8_t response_length = 0;
    uint8_t tmp;
    uint8_t frame[6];
    
    /* 如果是命令 ACMD<n> */
    if (cmd & 0x80)
    {  
        cmd &= 0x7F;
        /* 在发送命令 ACMD<n>，之前，发送 CMD55 */
        ltk_spi_sd_send_command(SD_CMD_APP_CMD, 0, R1, response);
        /* 如果命令响应错误 */
        if(response[0] > 0x01)
        {
            /* 不选中 SD */
            LTK_SPI_SD_CS_HIGH();
            ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
            return response[0];
        }
    }
    
    /* 选中 SD */
    LTK_SPI_SD_CS_LOW();
    while(!ltk_spi_sd_cs_ready());
    
    /* 设置传输的 CRC */
    if(cmd == SD_CMD_GO_IDLE_STATE)
        crc = 0x95;
    
    if(cmd == SD_CMD_SEND_IF_COND)
        crc = 0x87;
    
    /* 先传输高位 */
    /* 命令(cmd)格式:
       cmd[7:6] : 01
       cmd[5:0] : command */

    frame[0] = ((cmd & 0x3F) | 0x40);       /* byte 1: 命令 */

    frame[1] = (uint8_t)(argument >> 24);   /* byte 2: 命令参数 */

    frame[2] = (uint8_t)(argument >> 16);   /* byte 3: 命令参数 */

    frame[3] = (uint8_t)(argument >> 8);    /* byte 4: 命令参数 */

    frame[4] = (uint8_t)(argument);         /* byte 5: 命令参数 */

    frame[5] = (uint8_t)(crc);              /* byte 6: CRC */

    for (i = 0; i < 6; i++)
    {
        ltk_spi_send_byte(SPI1, frame[i]);  /* 发送命令 */
    }
    /* 不同的命令，命令响应的长度不同，具体请看 SD 手册 */
    switch (response_type)
    {
        case R1:
        case R1B:
            response_length = 1;
            break;
        case R2:
            response_length = 2;
            break;
        case R3:
        case R7:
            response_length = 5;
            break;
        default:
            break;
    }
    
    /* 等待 R1 的 start bit，start bit 为 0 时，表示命令已经启动 */
    i = 0xFF;
    do
    {
        tmp = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }while ((tmp & 0x80) && --i);

    response[0] = tmp;
    
    /* 如果为有效命令 */
    if ((i > 0) && ((response[0] & SD_ILLEGAL_COMMAND) != SD_ILLEGAL_COMMAND))
    {
        /* 发送长度为 response_length 的 DUMMY 命令，得到所有的命令响应，
           命令响应的长度有响应的类别 (R1, R3...) 决定 */
        i = 1;
        while(i < response_length)
        {
            tmp = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
            response[i] = tmp;
            i++;
        }
        
         /* 如果类型为 R1B，表示 SD 在接收到命令后，内部还会需要额外的时间
            来处理，SD 卡会输出一连串的 0 来表示此时 SD 正在忙碌，当输出不
            为 0 时，表示 SD 卡已经处理完了，处于就绪状态 */
        if (response_type == R1B)
        {
            do
            {
                tmp = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);;
            }while (tmp == 0x0);

            ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
        }
    }
    
    LTK_SPI_SD_CS_HIGH();
    ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    return response[0];
}

/**
  * 描述: 通过 DMA 向 SPI SD 发送数据，使用 DMA1，channel 3
  * 参数: *buff: 将要发送的数据缓存的地址
  *       byte_to_transfer: 发送数据的长度
  * 返回: 无
  */
void ltk_spi_sd_dma_send(const uint8_t *buff, uint32_t byte_to_transfer)
{
    ltk_dma1_ch3_param_init(buff, byte_to_transfer, DMA_MemoryInc_Enable);

    /* 使能 SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    
    /* 使能 DMA channel 3 */
    DMA_Cmd(DMA1_Channel3, ENABLE);
     
    /* 等待发送结束 */
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC3));
    
    /* 发送结束后，关闭 DMA 通道 */
    DMA_Cmd(DMA1_Channel3, DISABLE);

    /* 关闭 SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
}

/**
  * 描述: 通过 DMA 从 SPI SD 接收数据，使用 DMA1 的两个通道 channel 2 和 
          channel 3，此处需要两个 DMA 通道，channel 2 用于从 SPI 总线接收
          数据，而 channel 3 用于向 SPI 总线发送数据，以便产生 SPI 时钟
  * 参数: *buff: 接收数据缓存的地址
  *       byte_to_receive: 接收数据的长度
  * 返回: 无
  */
void ltk_spi_sd_dma_receive(uint8_t *buff, uint32_t byte_to_receive)
{
    uint8_t dummy_byte = SD_DUMMY_BYTE;
    ltk_dma1_ch2_param_init(buff, byte_to_receive, DMA_MemoryInc_Enable);
    ltk_dma1_ch3_param_init(&dummy_byte, byte_to_receive, DMA_MemoryInc_Disable);

    /* 使能 SPI_MASTER DMA Rx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
    /* 使能 SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    
    /* 使能 DMA channel 2 */
    DMA_Cmd(DMA1_Channel2, ENABLE);
    /* 使能 DMA channel 3 */
    DMA_Cmd(DMA1_Channel3, ENABLE);
     
    /* 等待发送结束 */
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC2));
    
    /* 传输结束后，关闭 channel 2 */
    DMA_Cmd(DMA1_Channel2, DISABLE);
    /* 传输结束后，关闭 channel 3 */
    DMA_Cmd(DMA1_Channel3, DISABLE);

    /* 关闭 SPI_MASTER DMA Rx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
    /* 关闭 SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
}

/**
  * 描述: 通过 DMA 从 SPI SD 读取某一 block 的数据
  * 参数: *buff: 接收数据缓存的地址
  *       byte_to_receive: 接收数据的长度
  * 返回: 读取是否成功标志
  */
uint8_t ltk_spi_sd_read_data(uint8_t *buff, uint32_t byte_to_receive)
{
    uint16_t expire_count = 0xFFFF;
    uint8_t status, ret = 1;
    LTK_SPI_SD_CS_LOW();
    /* 等待 SPI SD 进入 IDLE 状态，并且无任何错误 */
    do
    {
        status = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }while(status != 0xFE && --expire_count);
    /* SPI SD 响应异常 */
    if(status != 0xFE)
    {
        ret = 0;
    }
    else /* SPI SD 响应正常 */
    {
        /* 启动 DMA 接收数据 */
        ltk_spi_sd_dma_receive(buff, byte_to_receive);
        ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
        ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }
    LTK_SPI_SD_CS_HIGH();
    return ret;
}

/**
  * 描述: 读取单个或者多个 sector 的数据
  * 参数: *buff: 接收数据缓存的地址
  *       sector: 需要读取的第一个 sector
  *       sector_cnt: 需要读取的 sector 的数量
  * 返回: 读取是否成功标志
  */
uint8_t ltk_spi_sd_read_block(uint32_t sector, uint32_t sector_cnt, uint8_t *buff)
{
    uint8_t sd_response[5];
    uint32_t tmp_sect_cnt = sector_cnt;

    if(sector_cnt > 1) /* 读取多个 sector, 发送命令 READ_MULTIPLE_BLOCK */
        ltk_spi_sd_send_command(SD_CMD_READ_MULT_BLOCK, sector, R1, sd_response);
    else /* 读取单个 sector, 发送命令 SD_CMD_READ_SINGLE_BLOCK */
        ltk_spi_sd_send_command(SD_CMD_READ_SINGLE_BLOCK, sector, R1, sd_response);
        
    if(sd_response[0] == 0)
    {  
        do 
        {
            if(!ltk_spi_sd_read_data(buff, SD_BLOCK_SIZE)) 
            {
                return 0;
            }
            buff += SD_BLOCK_SIZE;
        } while(--tmp_sect_cnt);
        if(sector_cnt > 1)
        {
            /* 读取多个 sector, 传输完成，发送命令 STOP_TRANSMISSION */
            ltk_spi_sd_send_command(SD_CMD_STOP_TRANSMISSION, 0, R1B, sd_response);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
  * 描述: 通过 DMA 向 SPI SD 写入数据
  * 参数: *buff: 将要写入数据缓存的地址
  *       byte_to_write: 写入数据的长度
  * 返回: 读取是否成功标志
  */
uint8_t ltk_spi_sd_write_data(const uint8_t *buff, uint32_t byte_to_write, uint8_t token)
{
    uint8_t resp, ret = 1;
    LTK_SPI_SD_CS_LOW();
    while(!ltk_spi_sd_cs_ready()); /* 等待 CS 脚电平稳定 */
    ltk_spi_send_byte(SPI1, token); /* transmit data token */
    if(token != 0xFD) /* Is data token */  
    {
        /* 通过 DMA 向 SPI SD 写入数据 */
        ltk_spi_sd_dma_send(buff, byte_to_write);
        ltk_spi_send_byte(SPI1, 0xFF);        /* CRC (Dummy) */
        ltk_spi_send_byte(SPI1, 0xFF);        /* CRC (Dummy) */
        resp = ltk_spi_send_byte(SPI1, 0xFF); /* 数据响应 */
        if ((resp & 0x1F) != 0x05)            /* 如果响应错误，返回错误代码 */
            ret = 0;
    }
    while(ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE) == 0);
    LTK_SPI_SD_CS_HIGH();
    return ret;
}

/**
  * 描述: 写入单个或者多个 sector 的数据
  * 参数: *buff: 写入数据缓存的地址
  *       sector: 需要写入的第一个 sector
  *       sector_cnt: 需要写入的 sector 的数量
  * 返回: 读取是否成功标志
  */
uint8_t ltk_spi_sd_write_block(const uint8_t *buff, uint32_t sector, uint32_t sector_cnt)
{
    uint8_t sd_response[5];

    if(sector_cnt == 1) /* 写入一个 sector */
    {   
        /* 发送命令 WRITE_BLOCK */
        if((ltk_spi_sd_send_command(SD_CMD_WRITE_SINGLE_BLOCK, sector, R1, sd_response) == 0) && 
             ltk_spi_sd_write_data(buff, SD_BLOCK_SIZE, 0xFE))
            sector_cnt = 0;
    }
    else /* 写入多个 sector */
    {              
        if(sd_card_information.sd_ct & SD_SDC)
            ltk_spi_sd_send_command(SD_ACMD_APP_SET_WR_BLK_ERASE_COUNT, sector_cnt, R1, sd_response);
        /* 发送命令 WRITE_MULTIPLE_BLOCK */
        if(ltk_spi_sd_send_command(SD_CMD_WRITE_MULT_BLOCK, sector, R1, sd_response) == 0)
        {
            do 
            {
                if(!ltk_spi_sd_write_data(buff, SD_BLOCK_SIZE, 0xFC)) 
                    break;
                buff += SD_BLOCK_SIZE;
            } while(--sector_cnt);
            while(ltk_spi_sd_write_data(NULL, SD_BLOCK_SIZE, 0xFD) == 0); /* STOP_TRAN token */
        }
    }
    return (!sector_cnt);
}

/**
  * 描述: 启动 SD 卡
  * 参数: 无
  * 返回: SD 卡类型
  */
sd_card_type_t ltk_spi_sd_start(void)
{
    uint8_t i = 0;
    uint16_t retry_times = 0;
    uint8_t sd_response[5];
    
    if(sd_card_information.sd_stat & SD_STATUS_NO_DISK)
    {
        sd_card_information.sd_ct = SD_UNKNOWN;
    }
    else
    {
        
        /* 为了兼容性更好，在 SPI SD 初始化时，SPI 时钟设置为低速 */
        SPI_BaudRateConfig(SPI1, LTK_SPI_MIN_SPEED);    

        /* SPI SD CS 拉高 */
        LTK_SPI_SD_CS_HIGH();

        ltk_systick_delay(100);

        /* 至少等待74个时钟周期，才能和卡通信，卡的内部状态寄存器在此期间会被初始化 */
        for (i = 0; i < 10; i++)
        {
            /* 发送 dummy byte 0xFF */
            ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
        }
        /* 尝试发送 CMD0 命令，使其进入 SPI 模式 */
        i = 20;
        do
        {
            ltk_spi_sd_send_command(SD_CMD_GO_IDLE_STATE, 0, R1, sd_response);
        }while(sd_response[0] != SD_IN_IDLE_STATE && --i);

        /* 如果 SD 卡接受 SPI 模式 */
        if(sd_response[0] == SD_IN_IDLE_STATE)
        {
            /* 发送 CMD8 命令，如果支持该命令，则为 V2.0 或更高版本
               否则，为 V1.X版本，或者不是SD卡 */
            ltk_spi_sd_send_command(SD_CMD_SEND_IF_COND, CMD8_ECHO, R7, sd_response); 
            /* 支持 CMD8 命令，为 V2.0 或者更高版本 */
            if(sd_response[0] == SD_IN_IDLE_STATE)
            {
                /* CMD8 的回传是否正确，发送的参数为 0x1AA，回传也应该是 0x1AA */
                if((sd_response[3] == ((CMD8_ECHO >> 8) & 0xFF)) && 
                   (sd_response[4] == ((CMD8_ECHO >> 0) & 0xFF)))
                {
                    /* 发送 ACMD41 命令，直到SD卡就绪 */
                    retry_times = 0xFFF;
                    do
                    {
                        ltk_spi_sd_send_command(SD_ACMD_APP_SEND_OP_COND, 1UL << 30, R1, sd_response);
                    }while(sd_response[0] && --retry_times);

                    if(retry_times > 0)
                    {
                        ltk_spi_sd_send_command(SD_CMD_READ_OCR, 0x0, R3, sd_response);
                        if(sd_response[1] & 0x80)
                        {
                            sd_card_information.sd_stat &= ~SD_STATUS_NOT_INIT;
                            /* 检查 OCR 的 CCS 位，以确定 SD 容量 */
                            sd_card_information.sd_ct = (sd_response[1] & 0x40) ? SD_SDHC : SD_SDSC;
                        }
                        else
                        {
                            ltk_printf("SD in power down status\n"); 
                        }
                    }
                }
            }
            else /* 不支持 CMD8 命令，为 V1.X 版本，或者不是SD卡 */
            {
                /* 发送 ACMD41 命令，直到SD卡就绪 */
                retry_times = 0xFFF;
                do
                {
                    ltk_spi_sd_send_command(SD_ACMD_APP_SEND_OP_COND, 0, R1, sd_response);
                }while(sd_response[0] && --retry_times);
                
                if(retry_times > 0)
                {
                    sd_card_information.sd_stat &= ~SD_STATUS_NOT_INIT;
                    /* SD 卡类型为 1.X */
                    sd_card_information.sd_ct = SD_VER_1;
                }
            }
        }
        
        ltk_spi_sd_send_command(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, R1, sd_response);

        LTK_SPI_SD_CS_HIGH();
    }
    /* 初始化完成，SPI 总线配置成高速模式 */
    SPI_BaudRateConfig(SPI1, LTK_SPI_DEFAULT_SPEED); 
    
    /* SD 没找到或者是没有插入 */
    if(sd_card_information.sd_ct == SD_UNKNOWN)
    {
        ltk_printf("Unknown SD card or SD card not present!!!!!!!!!!!!!!!!!\n");
    }
    else  /* 识别到 SD 卡，打印出 SD 卡类型 */
    {
        ltk_printf("SD card type\t\t\t0x%X\n", (int)sd_card_information.sd_ct);
    }
    
    return sd_card_information.sd_ct;
}

/**
  * 描述: 初始化结构体 sd_card_info_t
  * 参数: 无
  * 返回: 无
  */
void ltk_spi_sd_param_init(void)
{
    sd_card_information.sd_ct = SD_UNKNOWN;
    sd_card_information.sd_stat = SD_STATUS_NOT_INIT;
}

/**
  * 描述: 初始化 SD 卡
  * 参数: 无
  * 返回: 无
  */
void ltk_spi_sd_init(void)
{
    ltk_spi_init();
    ltk_spi_sd_param_init();
    
    /* 使能 DMA1 的时钟，SPI1 DMA 传输使用 DMA1 */
    ltk_dma1_rcc_init();
}
    
/****************************** leitek.taobao.com *****************************/
