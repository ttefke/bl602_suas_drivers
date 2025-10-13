#include <Arduino.h>
#include <SPI.h>

extern "C" {
// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>

// HALs
#include <bl602_glb.h>
#include <bl602_spi.h>
#include <bl_gpio.h>
#include <bl_irq.h>
#include <spi_reg.h>
}

// Porting layer allowing to use the Arduino rfid library on the PineCone
// code inspired from https://github.com/bouffalolab/arduino-bouffalo/

void spiInterrupt([[gnu::unused]] void* ctx) {}

// Configure SPI
void SPIClass::begin() {
  // Configuration array for GPIO pins
  uint8_t gpio_pins[] = {
      6, /* Chip select -> this is a dummy pin, we control the CS pin manually*/
      SCK,  /* Clock       */
      MOSI, /* MOSI        */
      MISO, /* MISO        */
  };

  taskENTER_CRITICAL();  // Lock

  // Ensure MOSI and MISO pins are as in the manual
  GLB_Swap_SPI_0_MOSI_With_MISO(ENABLE);

  // Configure SPI pins
  GLB_GPIO_Func_Init(
      GPIO_FUN_SPI,                             // Function -> SPI
      (GLB_GPIO_Type*)gpio_pins,                // Affected pin(s)
      sizeof(gpio_pins) / sizeof(gpio_pins[0])  // Size of pin(s) aray
  );

  // Initialize as SPI master
  GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_MASTER);

  // Enable interrupt
  SPI_IntMask((SPI_ID_Type)0, SPI_INT_ALL, MASK);
  bl_irq_enable(SPI_IRQn);
  bl_irq_register_with_ctx(SPI_IRQn, (void*)spiInterrupt, this);

  // Disable SPI transfer (for now)
  SPI_Disable((SPI_ID_Type)0, SPI_WORK_MODE_MASTER);
  SPI_Disable((SPI_ID_Type)0, SPI_WORK_MODE_SLAVE);

  // Configure CS pin as output
  bl_gpio_enable_output(SS, /* No pullup/pulldown */ LOW, LOW);

  // Set CS pin to high (active low)
  bl_gpio_output_set(SS, HIGH);

  // Free pin 6 again
  uint8_t pin[] = {6};
  GLB_GPIO_Func_Init(GPIO_FUN_SWGPIO,              // Function -> GPIO
                     (GLB_GPIO_Type*)pin,          // Affected pin(s)
                     sizeof(pin) / sizeof(pin[0])  // Size of pin(s) aray
  );

  taskEXIT_CRITICAL();  // Unlock
}

// Start a transaction
void SPIClass::beginTransaction(SPISettings settings) {
  /* Set SPI frequency if not already set */
  if (this->currentFrequency != settings.clock) {
    this->setFrequency(settings.clock);
  }

  taskENTER_CRITICAL();  // Lock

  /* Set CS pin to low */
  bl_gpio_output_set(SS, LOW);

  /* Set SPI configuration -> see manual p. 120f. */
  SPI_CFG_Type spiConfig;
  spiConfig.deglitchEnable = ENABLE;    // Disable deglitch function -> this is
                                        // important to work around some bugs
  spiConfig.continuousEnable = ENABLE;  // Enable continuous mode
  spiConfig.byteSequence =
      SPI_BYTE_INVERSE_BYTE0_FIRST,  // Set byte sequence order
      spiConfig.bitSequence =
          settings.bitOrder == SPI_MSBFIRST
              ? SPI_BIT_INVERSE_MSB_FIRST
              : SPI_BIT_INVERSE_LSB_FIRST,  // Set bit sequence order
      spiConfig.frameSize =
          SPI_FRAME_SIZE_8;  // Set frame size -> send each byte individually

  // Set clock phase and polarity
  // spiConfig.clkPhaseInv = (settings.dataMode == SPI_MODE0 ||
  // settings.dataMode == SPI_MODE2) ? SPI_CLK_PHASE_INVERSE_0 :
  // SPI_CLK_PHASE_INVERSE_1; spiConfig.clkPolarity = (settings.dataMode ==
  // SPI_MODE0 || settings.dataMode == SPI_MODE1) ? SPI_CLK_POLARITY_LOW :
  // SPI_CLK_POLARITY_HIGH;
  spiConfig.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
  spiConfig.clkPolarity = SPI_CLK_POLARITY_HIGH;

  /* Initialize SPI configuration */
  SPI_Init((SPI_ID_Type)0, &spiConfig);

  /* Disable FIFO queue for DMA  -> see manual p. 124f.*/
  SPI_FifoCfg_Type fifoConfig;
  fifoConfig.txFifoThreshold = 1;
  fifoConfig.rxFifoThreshold = 5;
  fifoConfig.txFifoDmaEnable = DISABLE;
  fifoConfig.rxFifoDmaEnable = DISABLE;
  SPI_FifoConfig((SPI_ID_Type)0, &fifoConfig);

  /* Enable SPI transfer */
  SPI_Enable((SPI_ID_Type)0, SPI_WORK_MODE_MASTER);

  taskEXIT_CRITICAL();  // Unlock
}

void SPIClass::setFrequency(uint32_t frequency) {
  /* Check if frequency is in allowed bounds */
  if (frequency > 40000000) {
    printf("The SPI frequency is too high!\r\n");
    return;
  } else if (frequency < 156250) {
    printf("The SPI frequency is too low!\r\n");
    return;
  }

  taskENTER_CRITICAL();  // Lock

  /* Configure SPI clock */
  SPI_ClockCfg_Type clockConfig;

  /* Set clock divider */
  uint8_t clockDivider = (uint8_t)(40000000 / frequency);
  GLB_Set_SPI_CLK(ENABLE, 0);
  clockConfig.startLen = clockDivider;
  clockConfig.stopLen = clockDivider;
  clockConfig.dataPhase0Len = clockDivider;
  clockConfig.dataPhase1Len = clockDivider;
  clockConfig.intervalLen = clockDivider;

  /* Configure clock */
  SPI_ClockConfig((SPI_ID_Type)0, &clockConfig);

  taskEXIT_CRITICAL();  // Unlock
}

uint8_t SPIClass::transfer(uint8_t data) {
  /* Send eight bits (one byte)*/
  uint8_t result;
  taskENTER_CRITICAL();  // Lock
  SPI_SendRecv_8bits((SPI_ID_Type)0, &data, &result, 1, SPI_TIMEOUT_ENABLE);
  taskEXIT_CRITICAL();  // Unlock
  return result;
}

void SPIClass::endTransaction(void) {
  taskENTER_CRITICAL();  // Lock

  /* Clear SPI ended interrupt */
  uint32_t tmpval;
  tmpval = BL_RD_REG(SPI_BASE, SPI_INT_STS);
  BL_WR_REG(SPI_BASE, SPI_INT_STS, BL_SET_REG_BIT(tmpval, SPI_CR_SPI_END_CLR));

  /* Disable SPI */
  SPI_Disable((SPI_ID_Type)0, SPI_WORK_MODE_MASTER);

  /* Set CS to high again */
  bl_gpio_output_set(SS, HIGH);

  taskEXIT_CRITICAL();  // Unlock
}

/* SPI class to include for the drivers */
SPIClass SPI;