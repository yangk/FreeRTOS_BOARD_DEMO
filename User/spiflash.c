#include <stm32f10x_lib.h>
#include "spiflash.h"
#include "stm32f10x_flash.h"


/* Private define ------------------------------------------------------------*/
#define WRITE      0x02  /* Write to Memory instruction */
#define WRSR       0x01  /* Write Status Register instruction */
#define WREN       0x06  /* Write enable instruction */

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define SE         0x20  /* Sector Erase instruction */
#define BE         0xD8  /* block  Erase instruction */
#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte 0xEE
/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sys_spi_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, 	ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

	/*!< Configure sFLASH_SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*!< Configure sFLASH_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*!< Configure sFLASH_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*!< Configure sFLASH_CS_PIN pin: sFLASH Card CS pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	flash_cs_high();
	/*!< SPI configuration */
    SPI_InitStructure.SPI_Direction 		= SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode 				= SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize 			= SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL 				= SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA 				= SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS 				= SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit 			= SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial		= 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}
/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
uint8_t flash_send_byte(uint8_t byte)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	//while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, byte);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	//while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}
/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WriteEnable()
{
	/* Select the FLASH: Chip Select low */
	flash_cs_low();
	/* Send "Write Enable" instruction */
	flash_send_byte(WREN);
	/* Deselect the FLASH: Chip Select high */
	flash_cs_high();
}
/*******************************************************************************
* Function Name  : flash_wait_idle
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void flash_wait_idle()
{
	unsigned char FLASH_Status ;
	do
	{
		/* Select the FLASH: Chip Select low */
		flash_cs_low();
		/* Send "Read Status Register" instruction */
		flash_send_byte(RDSR);
		FLASH_Status = flash_send_byte(Dummy_Byte);
		/* Deselect the FLASH: Chip Select high */
		flash_cs_high();
	}
	while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */
}
/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_PageWrite(int WriteAddr,unsigned char Buffer[] , int NumByteToWrite)
{
	int i,_WriteAddr;
	/* Enable the write access to the FLASH */
	SPI_FLASH_WriteEnable();
	flash_wait_idle();
	/* Select the FLASH: Chip Select low */
	flash_cs_low();
	/* Send "Write to Memory " instruction */
	flash_send_byte(WRITE);
	_WriteAddr = WriteAddr & 0x7FFFFF;
	/* Send WriteAddr high nibble address byte to write to */
	flash_send_byte((_WriteAddr & 0xFF0000) >> 16);
	/* Send WriteAddr medium nibble address byte to write to */
	flash_send_byte((_WriteAddr & 0xFF00) >> 8);
	/* Send WriteAddr low nibble address byte to write to */
	flash_send_byte(_WriteAddr & 0xFF);

	/* while there is data to be written on the FLASH */
	for( i = 0x00 ; i < NumByteToWrite ; i++ )
	{
		/* Send the current byte */
		flash_send_byte(Buffer[i]);
	}
	/* Deselect the FLASH: Chip Select high */
	flash_cs_high();
	/* Wait the end of Flash writing */
	flash_wait_idle();
}
void SPI_FLASH_Write(int WriteAddr,unsigned char buffer[],int NumByteToWrite)
{
	int k, index = 0x00;
	while(NumByteToWrite > 0x00)
	{
		k = WriteAddr & 0xFF;
		k = 0x100 - k;
		if( k > NumByteToWrite)   k = NumByteToWrite;
		SPI_FLASH_PageWrite(WriteAddr, &buffer[index], k);
		NumByteToWrite -= k;
		index += k;
		WriteAddr += k;
	}
}
void SPI_FLASH_SectorErase(int SectorAddr)
{
	int _SectorAddr  ;
	/* Send write enable instruction */
	SPI_FLASH_WriteEnable();
	flash_wait_idle();
	/* Select the FLASH: Chip Select low */
	flash_cs_low();
	/* Send Sector Erase instruction */
	flash_send_byte(SE);
	_SectorAddr = SectorAddr & 0x7FFFFF;
	/* Send SectorAddr high nibble address byte */
	flash_send_byte((_SectorAddr & 0xFF0000) >> 16);
	/* Send SectorAddr medium nibble address byte */
	flash_send_byte((_SectorAddr & 0xFF00) >> 8);
	/* Send SectorAddr low nibble address byte */
	flash_send_byte(_SectorAddr & 0xFF);
	/* Deselect the FLASH: Chip Select high */
	flash_cs_high();
	/* Wait the end of Flash writing */
	flash_wait_idle();
}
 
/*******************************************************************************
* Function Name  : SPI_FLASH_BufferRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_ChipRead(int ReadAddr,unsigned char buffer[],int NumByteToRead)
{
	int _ReadAddr,i;
	/* Select the FLASH: Chip Select low */

	flash_cs_low();
	/* Send "Read from Memory " instruction */
	flash_send_byte(READ);
	_ReadAddr = ReadAddr & 0x7FFFFF;
	/* Send ReadAddr high nibble address byte to read from */
	flash_send_byte((_ReadAddr & 0xFF0000) >> 16);
	/* Send ReadAddr medium nibble address byte to read from */
	flash_send_byte((_ReadAddr& 0xFF00) >> 8);
	/* Send ReadAddr low nibble address byte to read from */
	flash_send_byte(_ReadAddr & 0xFF);
	for( i = 0x00 ; i < NumByteToRead ; i++)/* while there is data to be read */
	{
		buffer[i] = flash_send_byte(Dummy_Byte);
	}
	flash_cs_high();
}
void SPI_FLASH_Read(int ReadAddr,unsigned char buffer[],int NumByteToRead)
{
	int i = 0x00,k;
	while(NumByteToRead > 0x000) 
	{
		k = NumByteToRead;
		SPI_FLASH_ChipRead(ReadAddr,&buffer[i],k);
		i += k;
		ReadAddr += k;
		NumByteToRead -= k;
	}  
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
unsigned int SPI_FLASH_ReadID(int addr)
{
	unsigned int  Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

	/* Select the FLASH: Chip Select low */
	flash_cs_low();
	/* Send "RDID " instruction */
	flash_send_byte(RDID);
	/* Read a byte from the FLASH */
	Temp0 = flash_send_byte(Dummy_Byte);
	/* Read a byte from the FLASH */
	Temp1 = flash_send_byte(Dummy_Byte);
	/* Read a byte from the FLASH */
	Temp2 = flash_send_byte(Dummy_Byte);
	/* Deselect the FLASH: Chip Select high */
	flash_cs_high();
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
	return Temp;
}
static int _FLASH_TOTAL_SZ  = 0x800000;
int flash_write(int addr,unsigned char buffer[],int len)
{ 
  if(addr + len > _FLASH_TOTAL_SZ)
  {
    return(-3);
  }
  SPI_FLASH_Write(addr,buffer,len);
  return(len);
}

int flash_read(int addr,unsigned char buffer[],int len)
{
  if(addr + len > _FLASH_TOTAL_SZ)
  {
    return(-2);
  }
  SPI_FLASH_Read(addr,buffer,len);
  return(len);
}
 
 
