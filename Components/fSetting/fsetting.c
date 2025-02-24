#include "fsetting.h"

#define FLASH_ADDR 0x080E0000U  // Sector 11
#define FLASH_SIZE 0x00020000U  // 128k bytes
#define BLOCK_SIZE 0x80U        // 128  bytes
#define FLASH_SECT 11U          // Start at 896k bytes

/// n: block number; 0 ~ 1023
/// data: 0 ~ 126 bytes; -1: ?used
#define BLOCK_ADDR(n) (FLASH_ADDR + (n) * BLOCK_SIZE + 1U)
#define BLOCK_NUMS (FLASH_SIZE / BLOCK_SIZE)  // 1024 blocks
#define WRITE(addr, data) HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, data)
#define ADDR_PTR(offset) BLOCK_ADDR(fOffset + offset)
#define BLOCK_LOCK() WRITE((uint32_t)ADDR_PTR(0) - 1, 0x00)

static uint16_t fOffset = 0;
static void fset_seek(void);
static void fset_erase_all(void);

void fset_save(void *data, int8_t len) {
  if (data == NULL || len < 1) return;

  fset_seek(), HAL_FLASH_Unlock();
  while (len--) WRITE((ADDR_PTR(0) + len), ((uint8_t *)data)[len]);
  BLOCK_LOCK(), HAL_FLASH_Lock();
}

void fset_read(void **ptr) {
  fset_seek();

  /* Get Setting Buffer */
  if (fOffset)  // -1 表示上一个区块
    *ptr = (void *)ADDR_PTR(-1);
  else  // 0 表示当前区块
    *ptr = (void *)ADDR_PTR(0);
}

static void fset_seek(void) {
  do {  // (ADDR_PTR(0) - 1): 0xFF: used, 0x00: free
    if (fOffset >= BLOCK_NUMS) break;
    if (*(uint8_t *)(ADDR_PTR(0) - 1) == 0xFF) return;
  } while (++fOffset < BLOCK_NUMS);

  // if all blocks used
  fset_erase_all(), fOffset = 0;
}

static void fset_erase_all(void) {
  static uint32_t error = 0;
  static FLASH_EraseInitTypeDef erase = {0};

  erase.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase.Sector = FLASH_SECT;
  erase.NbSectors = 1;  // len: Number of sectors
  erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&erase, &error);
  HAL_FLASH_Lock();
}
