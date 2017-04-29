# uSDcard_to_QSPI_flash
STM32F769I-Discovery board - bitmaps from uSD card -> QSPI Flash
Reads bitmaps from the Media directory on your micro SD card and uploads them to the on-board QSPI flash<br>
Bitmaps should be 8888 format not 565<br>
Outputs info about what its doing to UART 1 so open a terminal window to see what its doing<br>
Remember that to write to the QSPI flash, you cannot be in memory mapped mode. To read the bitmaps or whatever back out, put the qspi in memory mapped mode and add 0x90000000 to the address<br>
Connected to UART One of STM32F769I-Discovery
<br>
             SDRAM<br>
----------------------------------<br>
 SDRAM size          : 0x01000000<br>
 MT48LC4M32B2 status : READY<br>
<br>
       QUAD SPI Flash memory<br>
---------------------------------<br>
 Flash size         : 0x04000000<br>
 Erase Sector size  : 0x00001000<br>
 Erase Sectors      : 16384<br>
 Program Page size  : 0x00000100<br>
 Program Pages      : 262144<br>
 MX25L51245G status : OK<br>
<br>
             MicroSD Card<br>
--------------------------------------<br>
 Manufacturer     : SanDisk<br>
 Product name     : SU04G<br>
 Card type        : SDHC/SDXC<br>
 Capacity         : 3965190144 bytes<br>
<br>
 1152122   Media/CLIFF.BMP  <br>
 1152122   Media/DROPS.BMP  <br>
 1152122   Media/LILLY.BMP  <br>
 1152122   Media/MNTN.BMP   <br>
 1152122   Media/ROSE.BMP   <br>
 1152122   Media/SEAL.BMP   <br>
 1152122   Media/TANGS.BMP  <br>
 1152054   Media/SOFA.BMP   <br>
 1536138   Media/WHIT.BMP   <br>
 1152054   Media/SERE.BMP   <br>
 1152054   Media/WOOD.BMP   <br>
<br>
 Loading into SDRAM Media/CLIFF.BMP  <br>
 Erasing flash prior to write<br>
 Writing at 0x00000000 Media/CLIFF.BMP  <br>
 Next available block: 0x0011A000<br>
<br>
 Loading into SDRAM Media/DROPS.BMP  <br>
 Erasing flash prior to write<br>
 Writing at 0x0011A000 Media/DROPS.BMP  <br>
 Next available block: 0x00234000<br>
<br>
 Loading into SDRAM Media/LILLY.BMP  <br>
 Erasing flash prior to write<br>
 Writing at 0x00234000 Media/LILLY.BMP  <br>
 Next available block: 0x0034E000<br>
<br>
 Loading into SDRAM Media/MNTN.BMP   <br>
 Erasing flash prior to write<br>
 Writing at 0x0034E000 Media/MNTN.BMP   <br>
 Next available block: 0x00468000<br>
<br>
et cetera<br>
