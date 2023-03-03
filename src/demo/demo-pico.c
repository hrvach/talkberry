/*
Copyright (c) 2023 Hrvoje Cavrak

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See the file LICENSE included with this distribution for more
  information.
*/

#include "codec2.h"
#include "data.h"
#include "defines.h"

#include <pico/stdlib.h>

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

/* These have to belong to the same slice, refer datasheet for slice table
   Pairs can be written as {2n, 2n+1} for n in [0..14] */
#define AUDIO_PIN 0
#define AUDIO_PIN_LSB 1

#define AUDIO_SAMPLES 320
#define REPETITION_RATE 4

/*
Wiring diagram, GPIO 0 = pin 1, GPIO 1 = pin 2 on RPi PICO.

GPIO 0 ---|/\/\/\|---\------o   Tip
            2.2k     |
                    === 100n
                     |
GPIO 1 --------------/------o   Ring
*/

//////////////////////// EVIL GLOBAL VARIABLES ////////////////////////
uint32_t audio_buffer[REPETITION_RATE * AUDIO_SAMPLES] = {0};
static volatile int write_done = 0;
int dma_channel;

//////////////////////// IRQ HANDLER ROUTINE /////////////////////////
void dma_irq_handler()
{
    write_done = 1;
    dma_hw->ch[dma_channel].al3_read_addr_trig = (uintptr_t)audio_buffer;
    dma_hw->ints0 = (1u << dma_channel);
}

int main(int argc, char *argv[])
{
    short int buffer[640];

    set_sys_clock_khz(131000, true);
    stdio_init_all();
    codec2_init();

    // Configure GPIO pins for PWM
    gpio_set_dir(0, GPIO_OUT);
    gpio_set_dir(1, GPIO_OUT);

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    gpio_set_function(AUDIO_PIN_LSB, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    ///////////////// CONFIGURE PWM ///////////////////
    //
    pwm_config config = pwm_get_default_config();

    // Don't divide, go as fast as possible
    pwm_config_set_clkdiv(&config, 1);

    // Wrap at 4095 provides us with 12 bits of resolution in theory
    pwm_config_set_wrap(&config, 4095);

    // Set opposite polarities and use GPIO 0 and 1 as outputs, to cancel
    // some of the noise
    pwm_set_output_polarity(audio_pin_slice, false, true);

    // Do the actual init
    pwm_init(audio_pin_slice, &config, true);

    ///////////////// CONFIGURE DMA ///////////////////
    //
    dma_channel = dma_claim_unused_channel(true);

    // Setup DMA channel
    dma_channel_config dma_channel_config = dma_channel_get_default_config(dma_channel);

    // We transfer 32 bits - half for channel A, half for channel B
    channel_config_set_transfer_data_size(&dma_channel_config, DMA_SIZE_32);

    // Reading increments (we read from a buffer) but write doesn't (we write to the same PWM address)
    channel_config_set_read_increment(&dma_channel_config, true);
    channel_config_set_write_increment(&dma_channel_config, false);

    // Using DREQ, DMA knows when the PWM block needs more data
    channel_config_set_dreq(&dma_channel_config, DREQ_PWM_WRAP0 + audio_pin_slice);

    dma_channel_configure(dma_channel,                        // DMA channel
                          &dma_channel_config,                // Channel config
                          &pwm_hw->slice[audio_pin_slice].cc, // We write to the compare counter
                          audio_buffer,                       // We read from audio_buffer
                          REPETITION_RATE * AUDIO_SAMPLES,    // Will write number of samples x sample repeat
                          false                               // Don't auto-start immediately
    );

    dma_channel_set_irq0_enabled(dma_channel, true);       // DMA will trigger IRQ0
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler); // Define interrupt handler
    irq_set_enabled(DMA_IRQ_0, true);                      // Set DMA_IRQ_0 as enabled

    dma_channel_start(dma_channel); // Initial start of the DMA loop

    while (true)
    {
        for (int i = 0; i < coded_data_len; i += 7) // For each decoded codec2 packet
        {
            for (int j = 0; j < AUDIO_SAMPLES; j++) // write samples to the audio buffer
            {
                uint32_t sample = (uint32_t)(((int32_t)buffer[j]) + 32768);
                uint32_t low, high;

                high = (sample >> 4) & 0xfff;

                /* Fix channel B in the middle (to 0x7ff) and make its
                   PWM opposing polarity to reduce noise a bit */
                sample = (high << 16) | 0x7ff;

                audio_buffer[4 * j + 0] = sample;
                audio_buffer[4 * j + 1] = sample;
                audio_buffer[4 * j + 2] = sample;
                audio_buffer[4 * j + 3] = sample;
            }

            codec2_decode(buffer, &coded_data[i]); // Decode new packet while DMA is playing

            // After finishing decode, wait until DMA reaches end of audio buffer
            while (!write_done)
                ;

            // Reset barrier for DMA sync (should use a proper synchronization
            // primitive in a real application)
            write_done = 0;
        }
    }
}
