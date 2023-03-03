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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/soundcard.h>
#include <sys/ioctl.h>

#define SAMPLES_PER_PACKET 320

int main(int argc, char *argv[])
{
    int fd = open("/dev/dsp", O_WRONLY);
    if (fd < 0)
    {
        perror("Opening /dev/dsp failed. Maybe try 'modprobe snd-pcm-oss' ?");
        return 1;
    }

    const int sample_rate = 8000, channels = 1, sample_bits = 16;

    ioctl(fd, SOUND_PCM_WRITE_BITS, &sample_bits);
    ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &channels);
    ioctl(fd, SOUND_PCM_WRITE_RATE, &sample_rate);

    codec2_init();

    short buf[SAMPLES_PER_PACKET];

    for (int i = 0; i < coded_data_len; i += 7)
    {
        codec2_decode(buf, (unsigned char *)&coded_data[i]);
        write(fd, buf, sizeof(short) * SAMPLES_PER_PACKET);
    }

    return 0;
}
