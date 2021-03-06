/*

Copyright (C) 2011,2012   Michael Dirska, DL1BFF (dl1bff@mdx.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <asf.h>
#include <board.h>
#include <conf_board.h>

#include "gpio.h"
#include "power_clocks_lib.h"

#include "FreeRTOS.h"




static const gpio_map_t lcd_gpio_map =
{
	{ AVR32_PIN_PA02, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// RES
	{ AVR32_PIN_PB12, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // E
	{ AVR32_PIN_PB13, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// CS1
	{ AVR32_PIN_PB14, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // CS2
	{ AVR32_PIN_PB21, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// D/I
	{ AVR32_PIN_PB22, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // R/W	
	{ AVR32_PIN_PB24, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// D0
	{ AVR32_PIN_PB25, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // D1	
	{ AVR32_PIN_PB26, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// D2
	{ AVR32_PIN_PB27, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // D3	
	{ AVR32_PIN_PB28, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// D4
	{ AVR32_PIN_PB29, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // D5	
	{ AVR32_PIN_PB30, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },  // D6
	{ AVR32_PIN_PB31, GPIO_DIR_OUTPUT | GPIO_INIT_LOW }   // D7
};

static const gpio_map_t lcd_pwm_gpio_map =
{
	{ AVR32_PWM_6_PIN, AVR32_PWM_6_FUNCTION },  // LCD_Backlight
	{ AVR32_PWM_0_PIN, AVR32_PWM_0_FUNCTION }    // LCD_PWM_Kontrast
};


static const gpio_map_t switch_gpio_map =
{
	{ AVR32_PIN_PA28, GPIO_DIR_INPUT | GPIO_PULL_UP },	// PTT
	{ AVR32_PIN_PA18, GPIO_DIR_INPUT | GPIO_PULL_UP },	// SW1
	{ AVR32_PIN_PA19, GPIO_DIR_INPUT | GPIO_PULL_UP },  // SW2 
	{ AVR32_PIN_PA20, GPIO_DIR_INPUT                },	// SW3 has external pull up
	{ AVR32_PIN_PA21, GPIO_DIR_INPUT                },  // SW4 special analog input
	{ AVR32_PIN_PA22, GPIO_DIR_INPUT | GPIO_PULL_UP },	// SW5
	{ AVR32_PIN_PA23, GPIO_DIR_INPUT | GPIO_PULL_UP }   // SW6
		
};


static const gpio_map_t adc_gpio_map =
{
	{ AVR32_ADC_AD_0_PIN, AVR32_ADC_AD_0_FUNCTION }  // SW4, PA21	
};

static const gpio_map_t	ambe_pin_gpio_map =
{
	{ AVR32_PIN_PB20, GPIO_DIR_OUTPUT | GPIO_INIT_LOW },	// RESETN
	{ AVR32_PIN_PA07, GPIO_DIR_INPUT  }						// EPR
};

static const gpio_map_t	ambe_spi_gpio_map =
{
	{ AVR32_SPI0_NPCS_1_0_PIN, AVR32_SPI0_NPCS_1_0_FUNCTION },  // PA08
	{ AVR32_SPI0_NPCS_0_0_PIN, AVR32_SPI0_NPCS_0_0_FUNCTION },  // PA10
	{ AVR32_SPI0_MISO_0_0_PIN, AVR32_SPI0_MISO_0_0_FUNCTION },  // PA11	
	{ AVR32_SPI0_MOSI_0_0_PIN, AVR32_SPI0_MOSI_0_0_FUNCTION },  // PA12
	{ AVR32_SPI0_SCK_0_0_PIN,  AVR32_SPI0_SCK_0_0_FUNCTION  }   // PA13
};

static const gpio_map_t i2c_config_gpio_map =
{
	{ AVR32_TWI_SDA_0_0_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH | GPIO_OPEN_DRAIN	},  // PA29
	{ AVR32_TWI_SCL_0_0_PIN, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH | GPIO_OPEN_DRAIN	}   // PA30	
};

static const gpio_map_t i2c_gpio_map =
{
	{ AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION	},  // PA29
	{ AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION	}   // PA30	
};


static const gpio_map_t ssc_gpio_map =
{
	{ AVR32_SSC_TX_FRAME_SYNC_0_PIN, AVR32_SSC_TX_FRAME_SYNC_0_FUNCTION }, // PA14
	{ AVR32_SSC_TX_CLOCK_0_PIN, AVR32_SSC_TX_CLOCK_0_FUNCTION },		// PA15
	{ AVR32_SSC_TX_DATA_0_PIN,  AVR32_SSC_TX_DATA_0_FUNCTION  },		// PA16	
	{ AVR32_SSC_RX_DATA_0_PIN,  AVR32_SSC_RX_DATA_0_FUNCTION }			// PA17			
};


static const gpio_map_t	eth_pin_gpio_map =
{
	{ AVR32_PIN_PA03, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH }	// INT/POWERDOWN
	
};

static const gpio_map_t eth_module_gpio_map = {
	{ AVR32_MACB_MDC_0_PIN, AVR32_MACB_MDC_0_FUNCTION },
	{ AVR32_MACB_MDIO_0_PIN, AVR32_MACB_MDIO_0_FUNCTION },
	{ AVR32_MACB_RXD_0_PIN, AVR32_MACB_RXD_0_FUNCTION },
	{ AVR32_MACB_RXD_1_PIN, AVR32_MACB_RXD_1_FUNCTION },
	{ AVR32_MACB_TXD_0_PIN, AVR32_MACB_TXD_0_FUNCTION },	
	{ AVR32_MACB_TXD_1_PIN, AVR32_MACB_TXD_1_FUNCTION },
	{ AVR32_MACB_TX_EN_0_PIN, AVR32_MACB_TX_EN_0_FUNCTION },	
	{ AVR32_MACB_RX_ER_0_PIN, AVR32_MACB_RX_ER_0_FUNCTION },	
	{ AVR32_MACB_RX_DV_0_PIN, AVR32_MACB_RX_DV_0_FUNCTION },	
	{ AVR32_MACB_TX_CLK_0_PIN, AVR32_MACB_TX_CLK_0_FUNCTION }
};


static const gpio_map_t usart_gpio_map = {
	{ AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION },
	{ AVR32_USART0_TXD_0_0_PIN, AVR32_USART0_TXD_0_0_FUNCTION },
	{ AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION },
	{ AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION }					
};


static const gpio_map_t usb_pin_gpio_map = {
	{ AVR32_PIN_PB17, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH },  // VBOF pin
	{ AVR32_PIN_PA09, GPIO_DIR_INPUT | GPIO_PULL_UP },  // OverCurrent pin
	{ AVR32_PIN_PB16, GPIO_DIR_INPUT | GPIO_PULL_UP }   // ID pin
};

static const gpio_map_t usb_module_gpio_map = {
	// { AVR32_USBB_USB_VBOF_0_1_PIN, AVR32_USBB_USB_VBOF_0_1_FUNCTION },  // VBOF pin	
	{ AVR32_USBB_USB_ID_0_1_PIN, AVR32_USBB_USB_ID_0_1_FUNCTION }  // ID pin
			
};


static const gpio_map_t sdcard_gpio_map = {
	{ AVR32_USART3_RXD_0_0_PIN, AVR32_USART3_RXD_0_0_FUNCTION },
	{ AVR32_USART3_TXD_0_0_PIN, AVR32_USART3_TXD_0_0_FUNCTION },
	{ AVR32_USART3_CLK_0_PIN, AVR32_USART3_CLK_0_FUNCTION }	
};

static const gpio_map_t sdcard_pin_gpio_map = {
	{ AVR32_PIN_PA04, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH }  // CS pin
};


void board_init(void)
{
	
	/*
	pcl_freq_param_t freq = { configCPU_CLOCK_HZ, configPBA_CLOCK_HZ, FOSC0, OSC0_STARTUP };
		
	pcl_configure_clocks ( &freq );
	*/
	
	// first change to OSC0 (12MHz)
	pm_enable_osc0_crystal(& AVR32_PM, FOSC0);            // Enable the Osc0 in crystal mode
	pm_enable_clk0(& AVR32_PM, OSC0_STARTUP);                  // Crystal startup time 
	pm_switch_to_clock(& AVR32_PM, AVR32_PM_MCSEL_OSC0);  // Then switch main clock to Osc0
	
	
	
	pm_enable_osc1_ext_clock(& AVR32_PM);  // ocs1 is external clock
	pm_enable_clk1(& AVR32_PM, OSC1_STARTUP);
	
	pm_pll_setup(&AVR32_PM
	 , 0   // pll
	 , 3 // mul
	  , 0 // div  ->  f_vfo = 16.384 MHz * 8 = 131.072 MHz
	 , 1   // osc
	  , 16  // lockcount
	  );
	  
	pm_pll_set_option(&AVR32_PM
	  , 0 // pll
	  , 1 // pll_freq  (f_vfo range 80MHz - 180 MHz)
	  , 1 // pll_div2  (f_pll1 = f_vfo / 2)
	  , 0 // pll_wbwdisable
	  );
	
	pm_pll_enable(&AVR32_PM, 0);
		
	pm_wait_for_pll0_locked(&AVR32_PM);
	
	pm_cksel(&AVR32_PM
	  , 1, 1 // PBA  (CPU / 4) = 16.384 MHz
	  , 0, 0 // PBB  65.536 MHz
	  , 0, 0 // HSB	 = CPU 65.536 MHz
	  );
	
	flashc_set_wait_state(1);  // one wait state if CPU clock > 33 MHz
	
	pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCCTRL_MCSEL_PLL0); // switch to PLL0
	
	
	// --------------------------------------
	
	// USB clock
	
	// Use 12MHz from OSC0 and generate 96 MHz
	pm_pll_setup(&AVR32_PM, 1,  // pll.
		  7,   // mul.
		  1,   // div.
		  0,   // osc.
		  16); // lockcount.

	pm_pll_set_option(&AVR32_PM, 1, // pll.
		  1,  // pll_freq: choose the range 80-180MHz.
		  1,  // pll_div2.
		  0); // pll_wbwdisable.

	// start PLL1 and wait forl lock
	pm_pll_enable(&AVR32_PM, 1);

	// Wait for PLL1 locked.
	pm_wait_for_pll1_locked(&AVR32_PM);

	pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_USBB,  // gc.
				1,  // osc_or_pll: use Osc (if 0) or PLL (if 1).
				1,  // pll_osc: select Osc0/PLL0 or Osc1/PLL1.
				0,  // diven.
				0); // div.
	pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_USBB);
	
	// --------------------------------------
	
	// LCD display
	
	gpio_enable_gpio( lcd_gpio_map, sizeof( lcd_gpio_map ) / sizeof( lcd_gpio_map[0] ) );
	
	int i;
	
	for (i=0; i < (sizeof( lcd_gpio_map ) / sizeof( lcd_gpio_map[0] )); i++)
	{
		gpio_configure_pin( lcd_gpio_map[i].pin, lcd_gpio_map[i].function);
	}
	
	
	gpio_enable_module( lcd_pwm_gpio_map, sizeof( lcd_pwm_gpio_map ) / sizeof( lcd_pwm_gpio_map[0] ) );
	
	
	// Backlight
	AVR32_PWM.channel[6].CMR.cpre = 3;
	AVR32_PWM.channel[6].cprd = 1000;
	AVR32_PWM.channel[6].cdty = 500;
	
	AVR32_PWM.ENA.chid6 = 1;
	
	// contrast
	AVR32_PWM.channel[0].CMR.cpre = 3;
	AVR32_PWM.channel[0].cprd = 1000;
	AVR32_PWM.channel[0].cdty = 520;
	
	AVR32_PWM.ENA.chid0 = 1;
	
	
	

	// switches

	gpio_enable_gpio( switch_gpio_map, sizeof( switch_gpio_map ) / sizeof( switch_gpio_map[0] ) );
		
	for (i=0; i < (sizeof( switch_gpio_map ) / sizeof( switch_gpio_map[0] )); i++)
	{
		gpio_configure_pin( switch_gpio_map[i].pin, switch_gpio_map[i].function);
	}
	
	// ADC
	
	gpio_enable_module( adc_gpio_map, sizeof( adc_gpio_map ) / sizeof( adc_gpio_map[0] ) );
	
	AVR32_ADC.mr = 0x0F031E20;     // SHTIM = 15,  STARTUP = 3, PRESCAL = 30, SLEEP Mode
	AVR32_ADC.cher = 1; // enable ADC0
	AVR32_ADC.cr = 2; // start conversion
	
	// AMBE interface
	
	gpio_enable_gpio( ambe_pin_gpio_map, sizeof( ambe_pin_gpio_map ) / sizeof( ambe_pin_gpio_map[0] ) );
		
	for (i=0; i < (sizeof( ambe_pin_gpio_map ) / sizeof( ambe_pin_gpio_map[0] )); i++)
	{
		gpio_configure_pin( ambe_pin_gpio_map[i].pin, ambe_pin_gpio_map[i].function);
	}
	
	gpio_enable_module( ambe_spi_gpio_map, sizeof( ambe_spi_gpio_map ) / sizeof( ambe_spi_gpio_map[0] ) );
	
	
	
	
	
	// I2C
	
	gpio_enable_gpio( i2c_config_gpio_map, sizeof( i2c_config_gpio_map ) / sizeof( i2c_config_gpio_map[0] ) );
	
	for (i=0; i < (sizeof( i2c_config_gpio_map ) / sizeof( i2c_config_gpio_map[0] )); i++)
	{
		gpio_configure_pin( i2c_config_gpio_map[i].pin, i2c_config_gpio_map[i].function);
	}
	
	gpio_enable_module( i2c_gpio_map, sizeof( i2c_gpio_map ) / sizeof( i2c_gpio_map[0] ) );
	
	AVR32_TWI.cwgr = 0x00006060;  // CKDIV = 0, CLDIV = CHDIV = 96  -> less than 100kHz
	
	
	// SSC
	
	gpio_enable_module( ssc_gpio_map, sizeof( ssc_gpio_map ) / sizeof( ssc_gpio_map[0] ) );
	
	AVR32_SSC.cmr = 32;  //  32 bit period, 8kHz sample rate  -> 256kHz SSC clock  
	
	AVR32_SSC.tcmr = 0x0F010504;           // 32 bits per frame, STTDLY=1, 
	                    // start = rising edge on TX_FRAME_SYNC, Continuous Transmit Clock
						
	AVR32_SSC.tfmr = 0x0020008F;		// frame sync = Positive Pulse, frame sync length = 1,
	                                        // 1 data word per frame, MSB first, 16 bits per data word
	
	AVR32_SSC.rcmr = 0x0F010121;           // 32 bits per frame, STTDLY=1,
					  // start = TX start ,  clock = TX Clock, 
					  // CKI -> sample bit on rising edge of clock
					  
	AVR32_SSC.rfmr = 0x0000008F;		// frame sync = Positive Pulse, frame sync length = 1,
	                                        // 1 data word per frame, MSB first, 16 bits per data word
											
											
	// Ethernet
	
	gpio_enable_gpio( eth_pin_gpio_map, sizeof( eth_pin_gpio_map ) / sizeof( eth_pin_gpio_map[0] ) );
	
	for (i=0; i < (sizeof( eth_pin_gpio_map ) / sizeof( eth_pin_gpio_map[0] )); i++)
	{
		gpio_configure_pin( eth_pin_gpio_map[i].pin, eth_pin_gpio_map[i].function);
	}


	gpio_enable_module( eth_module_gpio_map, sizeof( eth_module_gpio_map ) / sizeof( eth_module_gpio_map[0] ) );
	
	// USART
	
	gpio_enable_module( usart_gpio_map, sizeof( usart_gpio_map ) / sizeof( usart_gpio_map[0] ) );
	
	
	// USB
	
	gpio_enable_gpio( usb_pin_gpio_map, sizeof( usb_pin_gpio_map ) / sizeof( usb_pin_gpio_map[0] ) );
	
	for (i=0; i < (sizeof( usb_pin_gpio_map ) / sizeof( usb_pin_gpio_map[0] )); i++)
	{
		gpio_configure_pin( usb_pin_gpio_map[i].pin, usb_pin_gpio_map[i].function);
	}

	
	gpio_enable_module( usb_module_gpio_map, sizeof( usb_module_gpio_map ) / sizeof( usb_module_gpio_map[0] ) );
	
	// AVR32_USBB.usbcon = 0x03003000; // UIDE, VBUSPO, OTGPADE
	// AVR32_USBB.usbcon = 0x0300B000; // UIDE, USBE, VBUSPO, OTGPADE
	// AVR32_USBB.usbcon = 0x03006000; // default + VBUSPO
	// AVR32_USBB.udcon = 0;
	
	
	// SD card interface 
	
	gpio_enable_module( sdcard_gpio_map, sizeof( sdcard_gpio_map ) / sizeof( sdcard_gpio_map[0] ) );
	
	gpio_enable_gpio( sdcard_pin_gpio_map, sizeof( sdcard_pin_gpio_map ) / sizeof( sdcard_pin_gpio_map[0] ) );
	
	for (i=0; i < (sizeof( sdcard_pin_gpio_map ) / sizeof( sdcard_pin_gpio_map[0] )); i++)
	{
		gpio_configure_pin( sdcard_pin_gpio_map[i].pin, sdcard_pin_gpio_map[i].function);
	}
	
}
