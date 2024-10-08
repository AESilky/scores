/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/
/*

This file should be tailored to match the hardware design.

There should be one element of the spi[] array for each hardware SPI used.

There should be one element of the sd_cards[] array for each SD card slot.
The name is should correspond to the FatFs "logical drive" identifier.
(See http://elm-chan.org/fsw/ff/doc/filename.html#vol)
The rest of the constants will depend on the type of
socket, which SPI it is driven by, and how it is wired.

*/
/*
Modified by AES for the scores configuration, 2023.
*/
#include "../../../system_defs.h"

#include <string.h>
//
#include "my_debug.h"
//
#include "hw_config.h"
//
#include "ff.h" /* Obtains integer types */
//
#include "diskio.h" /* Declarations of disk functions */

void spi_dma_isr();

// Hardware Configuration of SPI "object"
// Note: multiple SD cards can be driven by one SPI if they use different slave selects.
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = SPI_DISP_SDC_DEVICE,      // SPI component
        .miso_gpio = SPI_DISP_SDC_MISO,      // GPIO number (not pin number)
        .mosi_gpio = SPI_DISP_SDC_MOSI,
        .sck_gpio = SPI_DISP_SDC_SCK,
        .set_drive_strength = true,
        .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,

        //.baud_rate = 1000 * 1000, // Very conservative. Probably fine given the small amount of data being used.
        .baud_rate = 2200 * 1000,   // 2.2Mhz. The limitation here is SPI slew rate.

        .dma_isr = spi_dma_isr,
        .initialized = false,
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",                 // Name used to mount device
        .spi = &spis[0],                // Pointer to the SPI driving this card
        .ss_gpio = SPI_SDC_CS,          // The SPI slave select GPIO for this SD card
        .set_drive_strength = false,    // Say 'false' because GPIO is setup at the system level
        .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
        // .card_detect_gpio = 22,
        // .card_detected_true = -1,    // What the GPIO read returns when a card is in
        //                              // Use -1 if there is no card detect
        .use_card_detect = false,       // True if using card detect

        // State variables:
        .m_Status = STA_NOINIT
    }
};

void spi_dma_isr() { spi_irq_handler(&spis[0]); }

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */
