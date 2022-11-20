#include <Arduino.h>
#include "sdcard.hpp"

#define USE_SDMMC 1
#define WT32_SC01_PLUS 1
// https://www.waveshare.com/wiki/SD_Storage_Board
#if USE_SDMMC && (WT32_SC01_PLUS == 0) // esp32-s3-devkit
#define SD_CMD	1
#define SD_CLK  2
#define SD_DAT0 4
#define SD_DAT1 5
#define SD_DAT2 6
#define SD_DAT3 7
#elif (WT32_SC01_PLUS == 0) // sd spi + sd mmc controller which share pins
#define SD_MOSI	 SD_CMD
#define SD_MISO SD_DAT0
// #define SD_DAT1 5  // WP
// #define SD_DAT2 6  // HD
#define SD_CS SD_DAT3
#else // optional pins selection i have on WT32-SC01-PLUS
#define SD_MISO	 38
#define SD_CLK  39
#define SD_MOSI 40
// #define SD_DAT1 5  // WP
// #define SD_DAT2 6  // HD
#define SD_CS 41
#endif

using namespace esptinyusb;
SDCard2USB dev;


void setup()
{
	Serial.begin(115200);
#if USE_SDMMC
	dev.initPins(SD_CMD, SD_CLK, SD_DAT0, SD_DAT1, SD_DAT2, SD_DAT3); // sd_mmc
#else
	dev.initPins(SD_MOSI, SD_CLK, SD_MISO, -1, -1, SD_CS); // sd spi
#endif
	if(dev.partition("/sdcard", USE_SDMMC))
	{
		delay(1000);
		dev.begin();
		
		Serial.println("sd card detcted");
		printf("sd card detected\n");
		sdmmc_card_t card = dev.card();

        sdmmc_card_print_info(stdout, &card);
        if(card.is_mmc)
            printf("its MMC card\n");
        if(card.is_sdio)
            printf("its SDIO card\n");

        if(card.is_mem)
            printf("its MEM card\n");
        if(card.is_ddr)
            printf("its DDR card\n");

	} else
	{
		while(1){
			printf("sd card not detected\n");
			delay(500);
		}
	}
	
}

void loop()
{
	delay(1000);
}