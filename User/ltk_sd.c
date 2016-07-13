/**
  ******************************************************************************
  * �ļ�: ltk_sd.c
  * ����: LeiTek (leitek.taobao.com)
  * �汾: V1.0.0
  * ����: SPI SD �м��������SD����SD�����ݶ�д�ȹ���
  ******************************************************************************
  *
  *                  ��Ȩ���� (C), LeiTek (leitek.taobao.com)
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
  * ����: �õ����� sd_card_information �ĵ�ַ
  * ����: ��
  * ����: ���� sd_card_information �ĵ�ַ
  */
sd_card_info_t *ltk_spi_sd_get_card_info(void)
{
    return &sd_card_information;
}

/**
  * ����: �ȴ� SPI ���߾���
  * ����: ��
  * ����: SPI ����״̬
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
  * ����: �� SD ����������
  * ����: cmd: ����
  *       argument: �������
  *       response_type: ������Ӧ����
  *       *response: ������Ӧֵ
  * ����: ������Ӧֵ�ĵ� 0 λ
  */
uint8_t ltk_spi_sd_send_command(uint8_t cmd, uint32_t argument,  
                                sd_response_t response_type, uint8_t *response)
{
    int32_t i = 0;
    uint8_t crc = 0x01;
    int8_t response_length = 0;
    uint8_t tmp;
    uint8_t frame[6];
    
    /* ��������� ACMD<n> */
    if (cmd & 0x80)
    {  
        cmd &= 0x7F;
        /* �ڷ������� ACMD<n>��֮ǰ������ CMD55 */
        ltk_spi_sd_send_command(SD_CMD_APP_CMD, 0, R1, response);
        /* ���������Ӧ���� */
        if(response[0] > 0x01)
        {
            /* ��ѡ�� SD */
            LTK_SPI_SD_CS_HIGH();
            ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
            return response[0];
        }
    }
    
    /* ѡ�� SD */
    LTK_SPI_SD_CS_LOW();
    while(!ltk_spi_sd_cs_ready());
    
    /* ���ô���� CRC */
    if(cmd == SD_CMD_GO_IDLE_STATE)
        crc = 0x95;
    
    if(cmd == SD_CMD_SEND_IF_COND)
        crc = 0x87;
    
    /* �ȴ����λ */
    /* ����(cmd)��ʽ:
       cmd[7:6] : 01
       cmd[5:0] : command */

    frame[0] = ((cmd & 0x3F) | 0x40);       /* byte 1: ���� */

    frame[1] = (uint8_t)(argument >> 24);   /* byte 2: ������� */

    frame[2] = (uint8_t)(argument >> 16);   /* byte 3: ������� */

    frame[3] = (uint8_t)(argument >> 8);    /* byte 4: ������� */

    frame[4] = (uint8_t)(argument);         /* byte 5: ������� */

    frame[5] = (uint8_t)(crc);              /* byte 6: CRC */

    for (i = 0; i < 6; i++)
    {
        ltk_spi_send_byte(SPI1, frame[i]);  /* �������� */
    }
    /* ��ͬ�����������Ӧ�ĳ��Ȳ�ͬ�������뿴 SD �ֲ� */
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
    
    /* �ȴ� R1 �� start bit��start bit Ϊ 0 ʱ����ʾ�����Ѿ����� */
    i = 0xFF;
    do
    {
        tmp = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }while ((tmp & 0x80) && --i);

    response[0] = tmp;
    
    /* ���Ϊ��Ч���� */
    if ((i > 0) && ((response[0] & SD_ILLEGAL_COMMAND) != SD_ILLEGAL_COMMAND))
    {
        /* ���ͳ���Ϊ response_length �� DUMMY ����õ����е�������Ӧ��
           ������Ӧ�ĳ�������Ӧ����� (R1, R3...) ���� */
        i = 1;
        while(i < response_length)
        {
            tmp = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
            response[i] = tmp;
            i++;
        }
        
         /* �������Ϊ R1B����ʾ SD �ڽ��յ�������ڲ�������Ҫ�����ʱ��
            ������SD �������һ������ 0 ����ʾ��ʱ SD ����æµ���������
            Ϊ 0 ʱ����ʾ SD ���Ѿ��������ˣ����ھ���״̬ */
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
  * ����: ͨ�� DMA �� SPI SD �������ݣ�ʹ�� DMA1��channel 3
  * ����: *buff: ��Ҫ���͵����ݻ���ĵ�ַ
  *       byte_to_transfer: �������ݵĳ���
  * ����: ��
  */
void ltk_spi_sd_dma_send(const uint8_t *buff, uint32_t byte_to_transfer)
{
    ltk_dma1_ch3_param_init(buff, byte_to_transfer, DMA_MemoryInc_Enable);

    /* ʹ�� SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    
    /* ʹ�� DMA channel 3 */
    DMA_Cmd(DMA1_Channel3, ENABLE);
     
    /* �ȴ����ͽ��� */
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC3));
    
    /* ���ͽ����󣬹ر� DMA ͨ�� */
    DMA_Cmd(DMA1_Channel3, DISABLE);

    /* �ر� SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
}

/**
  * ����: ͨ�� DMA �� SPI SD �������ݣ�ʹ�� DMA1 ������ͨ�� channel 2 �� 
          channel 3���˴���Ҫ���� DMA ͨ����channel 2 ���ڴ� SPI ���߽���
          ���ݣ��� channel 3 ������ SPI ���߷������ݣ��Ա���� SPI ʱ��
  * ����: *buff: �������ݻ���ĵ�ַ
  *       byte_to_receive: �������ݵĳ���
  * ����: ��
  */
void ltk_spi_sd_dma_receive(uint8_t *buff, uint32_t byte_to_receive)
{
    uint8_t dummy_byte = SD_DUMMY_BYTE;
    ltk_dma1_ch2_param_init(buff, byte_to_receive, DMA_MemoryInc_Enable);
    ltk_dma1_ch3_param_init(&dummy_byte, byte_to_receive, DMA_MemoryInc_Disable);

    /* ʹ�� SPI_MASTER DMA Rx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
    /* ʹ�� SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    
    /* ʹ�� DMA channel 2 */
    DMA_Cmd(DMA1_Channel2, ENABLE);
    /* ʹ�� DMA channel 3 */
    DMA_Cmd(DMA1_Channel3, ENABLE);
     
    /* �ȴ����ͽ��� */
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC2));
    
    /* ��������󣬹ر� channel 2 */
    DMA_Cmd(DMA1_Channel2, DISABLE);
    /* ��������󣬹ر� channel 3 */
    DMA_Cmd(DMA1_Channel3, DISABLE);

    /* �ر� SPI_MASTER DMA Rx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
    /* �ر� SPI_MASTER DMA Tx */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
}

/**
  * ����: ͨ�� DMA �� SPI SD ��ȡĳһ block ������
  * ����: *buff: �������ݻ���ĵ�ַ
  *       byte_to_receive: �������ݵĳ���
  * ����: ��ȡ�Ƿ�ɹ���־
  */
uint8_t ltk_spi_sd_read_data(uint8_t *buff, uint32_t byte_to_receive)
{
    uint16_t expire_count = 0xFFFF;
    uint8_t status, ret = 1;
    LTK_SPI_SD_CS_LOW();
    /* �ȴ� SPI SD ���� IDLE ״̬���������κδ��� */
    do
    {
        status = ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }while(status != 0xFE && --expire_count);
    /* SPI SD ��Ӧ�쳣 */
    if(status != 0xFE)
    {
        ret = 0;
    }
    else /* SPI SD ��Ӧ���� */
    {
        /* ���� DMA �������� */
        ltk_spi_sd_dma_receive(buff, byte_to_receive);
        ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
        ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
    }
    LTK_SPI_SD_CS_HIGH();
    return ret;
}

/**
  * ����: ��ȡ�������߶�� sector ������
  * ����: *buff: �������ݻ���ĵ�ַ
  *       sector: ��Ҫ��ȡ�ĵ�һ�� sector
  *       sector_cnt: ��Ҫ��ȡ�� sector ������
  * ����: ��ȡ�Ƿ�ɹ���־
  */
uint8_t ltk_spi_sd_read_block(uint32_t sector, uint32_t sector_cnt, uint8_t *buff)
{
    uint8_t sd_response[5];
    uint32_t tmp_sect_cnt = sector_cnt;

    if(sector_cnt > 1) /* ��ȡ��� sector, �������� READ_MULTIPLE_BLOCK */
        ltk_spi_sd_send_command(SD_CMD_READ_MULT_BLOCK, sector, R1, sd_response);
    else /* ��ȡ���� sector, �������� SD_CMD_READ_SINGLE_BLOCK */
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
            /* ��ȡ��� sector, ������ɣ��������� STOP_TRANSMISSION */
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
  * ����: ͨ�� DMA �� SPI SD д������
  * ����: *buff: ��Ҫд�����ݻ���ĵ�ַ
  *       byte_to_write: д�����ݵĳ���
  * ����: ��ȡ�Ƿ�ɹ���־
  */
uint8_t ltk_spi_sd_write_data(const uint8_t *buff, uint32_t byte_to_write, uint8_t token)
{
    uint8_t resp, ret = 1;
    LTK_SPI_SD_CS_LOW();
    while(!ltk_spi_sd_cs_ready()); /* �ȴ� CS �ŵ�ƽ�ȶ� */
    ltk_spi_send_byte(SPI1, token); /* transmit data token */
    if(token != 0xFD) /* Is data token */  
    {
        /* ͨ�� DMA �� SPI SD д������ */
        ltk_spi_sd_dma_send(buff, byte_to_write);
        ltk_spi_send_byte(SPI1, 0xFF);        /* CRC (Dummy) */
        ltk_spi_send_byte(SPI1, 0xFF);        /* CRC (Dummy) */
        resp = ltk_spi_send_byte(SPI1, 0xFF); /* ������Ӧ */
        if ((resp & 0x1F) != 0x05)            /* �����Ӧ���󣬷��ش������ */
            ret = 0;
    }
    while(ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE) == 0);
    LTK_SPI_SD_CS_HIGH();
    return ret;
}

/**
  * ����: д�뵥�����߶�� sector ������
  * ����: *buff: д�����ݻ���ĵ�ַ
  *       sector: ��Ҫд��ĵ�һ�� sector
  *       sector_cnt: ��Ҫд��� sector ������
  * ����: ��ȡ�Ƿ�ɹ���־
  */
uint8_t ltk_spi_sd_write_block(const uint8_t *buff, uint32_t sector, uint32_t sector_cnt)
{
    uint8_t sd_response[5];

    if(sector_cnt == 1) /* д��һ�� sector */
    {   
        /* �������� WRITE_BLOCK */
        if((ltk_spi_sd_send_command(SD_CMD_WRITE_SINGLE_BLOCK, sector, R1, sd_response) == 0) && 
             ltk_spi_sd_write_data(buff, SD_BLOCK_SIZE, 0xFE))
            sector_cnt = 0;
    }
    else /* д���� sector */
    {              
        if(sd_card_information.sd_ct & SD_SDC)
            ltk_spi_sd_send_command(SD_ACMD_APP_SET_WR_BLK_ERASE_COUNT, sector_cnt, R1, sd_response);
        /* �������� WRITE_MULTIPLE_BLOCK */
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
  * ����: ���� SD ��
  * ����: ��
  * ����: SD ������
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
        
        /* Ϊ�˼����Ը��ã��� SPI SD ��ʼ��ʱ��SPI ʱ������Ϊ���� */
        SPI_BaudRateConfig(SPI1, LTK_SPI_MIN_SPEED);    

        /* SPI SD CS ���� */
        LTK_SPI_SD_CS_HIGH();

        ltk_systick_delay(100);

        /* ���ٵȴ�74��ʱ�����ڣ����ܺͿ�ͨ�ţ������ڲ�״̬�Ĵ����ڴ��ڼ�ᱻ��ʼ�� */
        for (i = 0; i < 10; i++)
        {
            /* ���� dummy byte 0xFF */
            ltk_spi_send_byte(SPI1, SD_DUMMY_BYTE);
        }
        /* ���Է��� CMD0 ���ʹ����� SPI ģʽ */
        i = 20;
        do
        {
            ltk_spi_sd_send_command(SD_CMD_GO_IDLE_STATE, 0, R1, sd_response);
        }while(sd_response[0] != SD_IN_IDLE_STATE && --i);

        /* ��� SD ������ SPI ģʽ */
        if(sd_response[0] == SD_IN_IDLE_STATE)
        {
            /* ���� CMD8 ������֧�ָ������Ϊ V2.0 ����߰汾
               ����Ϊ V1.X�汾�����߲���SD�� */
            ltk_spi_sd_send_command(SD_CMD_SEND_IF_COND, CMD8_ECHO, R7, sd_response); 
            /* ֧�� CMD8 ���Ϊ V2.0 ���߸��߰汾 */
            if(sd_response[0] == SD_IN_IDLE_STATE)
            {
                /* CMD8 �Ļش��Ƿ���ȷ�����͵Ĳ���Ϊ 0x1AA���ش�ҲӦ���� 0x1AA */
                if((sd_response[3] == ((CMD8_ECHO >> 8) & 0xFF)) && 
                   (sd_response[4] == ((CMD8_ECHO >> 0) & 0xFF)))
                {
                    /* ���� ACMD41 ���ֱ��SD������ */
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
                            /* ��� OCR �� CCS λ����ȷ�� SD ���� */
                            sd_card_information.sd_ct = (sd_response[1] & 0x40) ? SD_SDHC : SD_SDSC;
                        }
                        else
                        {
                            ltk_printf("SD in power down status\n"); 
                        }
                    }
                }
            }
            else /* ��֧�� CMD8 ���Ϊ V1.X �汾�����߲���SD�� */
            {
                /* ���� ACMD41 ���ֱ��SD������ */
                retry_times = 0xFFF;
                do
                {
                    ltk_spi_sd_send_command(SD_ACMD_APP_SEND_OP_COND, 0, R1, sd_response);
                }while(sd_response[0] && --retry_times);
                
                if(retry_times > 0)
                {
                    sd_card_information.sd_stat &= ~SD_STATUS_NOT_INIT;
                    /* SD ������Ϊ 1.X */
                    sd_card_information.sd_ct = SD_VER_1;
                }
            }
        }
        
        ltk_spi_sd_send_command(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, R1, sd_response);

        LTK_SPI_SD_CS_HIGH();
    }
    /* ��ʼ����ɣ�SPI �������óɸ���ģʽ */
    SPI_BaudRateConfig(SPI1, LTK_SPI_DEFAULT_SPEED); 
    
    /* SD û�ҵ�������û�в��� */
    if(sd_card_information.sd_ct == SD_UNKNOWN)
    {
        ltk_printf("Unknown SD card or SD card not present!!!!!!!!!!!!!!!!!\n");
    }
    else  /* ʶ�� SD ������ӡ�� SD ������ */
    {
        ltk_printf("SD card type\t\t\t0x%X\n", (int)sd_card_information.sd_ct);
    }
    
    return sd_card_information.sd_ct;
}

/**
  * ����: ��ʼ���ṹ�� sd_card_info_t
  * ����: ��
  * ����: ��
  */
void ltk_spi_sd_param_init(void)
{
    sd_card_information.sd_ct = SD_UNKNOWN;
    sd_card_information.sd_stat = SD_STATUS_NOT_INIT;
}

/**
  * ����: ��ʼ�� SD ��
  * ����: ��
  * ����: ��
  */
void ltk_spi_sd_init(void)
{
    ltk_spi_init();
    ltk_spi_sd_param_init();
    
    /* ʹ�� DMA1 ��ʱ�ӣ�SPI1 DMA ����ʹ�� DMA1 */
    ltk_dma1_rcc_init();
}
    
/****************************** leitek.taobao.com *****************************/
