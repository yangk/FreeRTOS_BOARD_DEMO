#ifndef _SPIFLASH_H_
#define _SPIFLASH_H_


/* Deselect SPI FLASH: Chip Select pin high */
#define flash_cs_low()       GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define flash_cs_high()      GPIO_SetBits(GPIOA, GPIO_Pin_4)

#define  PAGE_SIZE      	0x100	//bytes
#define  SECTOR_SIZE        0x1000	//bytes
#define  BLOCK_SIZE         0x10000 //bytes

#define  MAX_PAGE_CNT      	0x8000
#define  MAX_SECTOR_CNT		0x800
#define  MAX_BLOCK_CNT		0x80

void sys_spi_init(void);
int flash_read(int addr,unsigned char buffer[],int len);
int flash_write(int addr,unsigned char buffer[],int len);
void SPI_FLASH_SectorErase(int SectorAddr);
#endif
