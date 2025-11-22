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
  PROGRAM_FLASH (rx) : ORIGIN = 0x40000, LENGTH = 0x60000 /* 384K bytes (alias Flash) */  
  Ram0 (rwx) : ORIGIN = 0x20030000, LENGTH = 0x14000 /* 80K bytes (alias RAM) */  
}

  /* Define a symbol for the top of each memory region */
  __base_PROGRAM_FLASH = 0x40000  ; /* PROGRAM_FLASH */  
  __base_Flash = 0x40000 ; /* Flash */  
  __top_PROGRAM_FLASH = 0x40000 + 0x60000 ; /* 384K bytes */  
  __top_Flash = 0x40000 + 0x60000 ; /* 384K bytes */  
  __base_Ram0 = 0x20030000  ; /* Ram0 */  
  __base_RAM = 0x20030000 ; /* RAM */  
  __top_Ram0 = 0x20030000 + 0x14000 ; /* 80K bytes */  
  __top_RAM = 0x20030000 + 0x14000 ; /* 80K bytes */  
