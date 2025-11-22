/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright 2008-2013 Code Red Technologies Ltd,
 * Copyright 2013-2025 NXP
 * Generated linker script file for LPC55S69
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.30
 * MCUXpresso IDE v11.8.1 [Build 1197] [2023-10-27] on Oct 6, 2025, 5:13:08 PM
 */

MEMORY
{
  /* Define each memory region */
  PROGRAM_FLASH (rx) : ORIGIN = 0x10000000, LENGTH = 0x3fe00 /* 261632 bytes (alias Flash) */  
  SG_veneer_table (rx) : ORIGIN = 0x1003fe00, LENGTH = 0x200 /* 512 bytes (alias Flash2) */  
  Ram0 (rwx) : ORIGIN = 0x30000000, LENGTH = 0x30000 /* 192K bytes (alias RAM) */  
}

  /* Define a symbol for the top of each memory region */
  __base_PROGRAM_FLASH = 0x10000000  ; /* PROGRAM_FLASH */  
  __base_Flash = 0x10000000 ; /* Flash */  
  __top_PROGRAM_FLASH = 0x10000000 + 0x3fe00 ; /* 261632 bytes */  
  __top_Flash = 0x10000000 + 0x3fe00 ; /* 261632 bytes */  
  __base_SG_veneer_table = 0x1003fe00  ; /* SG_veneer_table */  
  __base_Flash2 = 0x1003fe00 ; /* Flash2 */  
  __top_SG_veneer_table = 0x1003fe00 + 0x200 ; /* 512 bytes */  
  __top_Flash2 = 0x1003fe00 + 0x200 ; /* 512 bytes */  
  __base_Ram0 = 0x30000000  ; /* Ram0 */  
  __base_RAM = 0x30000000 ; /* RAM */  
  __top_Ram0 = 0x30000000 + 0x30000 ; /* 192K bytes */  
  __top_RAM = 0x30000000 + 0x30000 ; /* 192K bytes */  
