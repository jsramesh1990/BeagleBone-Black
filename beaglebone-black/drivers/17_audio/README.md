# BeagleBone Black ALSA Driver

## Overview

This project demonstrates the Linux **ALSA PCM driver framework** on
the BeagleBone Black.

The driver demonstrates:

- ALSA sound-card registration
- PCM device creation
- Playback support
- Capture support
- PCM open/close
- Hardware parameter configuration
- PCM prepare
- PCM trigger
- PCM pointer
- ALSA kernel integration

> **Important:** This is an ALSA framework/demo driver. It does not
> directly program the BeagleBone Black AM335x McASP/I2S hardware.

---

## Directory Structure

```text
17_alsa/
├── alsa_driver.c
├── alsa_driver.h
├── makefile
└── README.md
ALSA Architecture
                User Application
                       |
                       v
                ALSA User Space
                       |
                +------+------+
                |             |
              aplay         arecord
                |             |
                +------+------+
                       |
                       v
                 ALSA Kernel
                       |
                       v
                  PCM Core
                       |
                       v
                ALSA Driver
                       |
                       v
              DMA / Audio HW
                       |
                       v
                McASP / I2S
                       |
                       v
                  Codec
                       |
                       v
                  Speaker
ALSA Driver Flow
Device Tree
     |
     v
Platform Device
     |
     v
ALSA Driver probe()
     |
     v
snd_card_new()
     |
     v
snd_pcm_new()
     |
     v
snd_pcm_set_ops()
     |
     v
snd_card_register()
     |
     v
ALSA PCM Device
     |
     +------------------+
     |                  |
     v                  v
 Playback            Capture
     |                  |
     v                  v
 PCM callbacks       PCM callbacks
ALSA Sound Card

The driver creates:

struct snd_card

The sound card contains information about the audio device.

The driver configures:

Driver name
Short name
Long name
PCM device
PCM

PCM means:

Pulse Code Modulation

PCM is the basic digital audio data representation used by ALSA.

Typical audio parameters:

Sample Rate
Channels
Sample Format
Buffer Size
Period Size

Example:

48 kHz
2 channels
16-bit samples
PCM Playback Flow
Application
    |
    v
ALSA Library
    |
    v
PCM Write
    |
    v
ALSA PCM Core
    |
    v
Driver
    |
    v
DMA
    |
    v
McASP / I2S
    |
    v
Audio Codec
    |
    v
Speaker
PCM Capture Flow
Microphone
    |
    v
Audio Codec
    |
    v
McASP / I2S
    |
    v
DMA
    |
    v
ALSA Driver
    |
    v
ALSA PCM Core
    |
    v
ALSA Library
    |
    v
Application
PCM Operations

The driver implements:

struct snd_pcm_ops

Important callbacks:

open()
close()
ioctl()
hw_params()
hw_free()
prepare()
trigger()
pointer()
PCM Open

When an application opens the PCM device:

aplay
  |
  v
ALSA PCM
  |
  v
pcm_open()
  |
  v
driver open()

The driver sets the supported hardware parameters.

Hardware Parameters

The application configures:

Sample Rate
Channels
Format
Buffer Size
Period Size

Example:

Rate     = 48000 Hz
Channels = 2
Format   = S16_LE

The driver receives these values in:

snd_pcm_hw_params
Supported Formats

This demo supports:

S16_LE
S24_LE
S32_LE

The most common format for embedded audio testing is:

S16_LE
Supported Sample Rates

The demo supports:

8000 Hz
16000 Hz
44100 Hz
48000 Hz

Common embedded audio rate:

48000 Hz
Channels

The driver supports:

1 channel
2 channels

Therefore:

1 = Mono
2 = Stereo
Buffer and Period

ALSA PCM uses buffers divided into periods.

              PCM Buffer
+--------------------------------------+
| Period 1 | Period 2 | Period 3 | P4 |
+--------------------------------------+
              ^
              |
          DMA Position

The hardware normally processes one period at a time.

Trigger

The driver receives trigger commands:

START
STOP
PAUSE
RESUME

Flow:

START
  |
  v
Enable DMA
  |
  v
Start Audio Hardware

STOP:

STOP
  |
  v
Stop DMA
  |
  v
Stop Audio Hardware
Pointer

The PCM pointer tells ALSA where the hardware currently is in
the audio buffer.

PCM Buffer
+--------------------------------+
| Audio Data                     |
|          ^                     |
|          |                     |
|       HW Pointer               |
+--------------------------------+

A real driver gets this position from the DMA controller.

This demo returns 0 because it does not control real audio DMA.

ALSA Commands

After loading the driver:

cat /proc/asound/cards

Show PCM devices:

cat /proc/asound/pcm

List playback devices:

aplay -l

List capture devices:

arecord -l
Build

Build the driver:

make

Expected output:

alsa_driver.ko

Check:

ls -l alsa_driver.ko
Load Driver
sudo insmod alsa_driver.ko

Check:

lsmod | grep alsa_driver

Check logs:

dmesg | tail -30
Check ALSA Card
cat /proc/asound/cards

Example:

0 [BBB_ALSA]: BBB_ALSA
             BeagleBone ALSA Demo
Check PCM
cat /proc/asound/pcm

This displays registered PCM devices.

ALSA Playback Test

For a real hardware-backed driver, playback can be tested with:

aplay -D hw:0,0 test.wav

Or:

aplay test.wav

Check supported formats:

aplay --dump-hw-params -D hw:0,0

The demo driver does not implement actual DMA/audio hardware
transfer, so it should not be expected to produce sound.

ALSA Capture Test

For a real capture device:

arecord -D hw:0,0 -f S16_LE -r 48000 -c 2 test.wav

Stop with:

Ctrl+C

Playback the captured file:

aplay test.wav
ALSA Mixer

View mixer controls:

amixer

List controls:

amixer controls

View simple controls:

amixer scontrols

For actual codec hardware, mixer controls may include:

Master Volume
Speaker Volume
Headphone Volume
Mic Gain
ADC Gain
DAC Volume
Mute
Remove Driver
sudo rmmod alsa_driver

Check:

lsmod | grep alsa_driver

Check logs:

dmesg | tail -30
Debugging

Check ALSA cards:

cat /proc/asound/cards

Check PCM devices:

cat /proc/asound/pcm

Check modules:

lsmod | grep snd

Check kernel logs:

dmesg | grep -i alsa

Follow logs:

dmesg -w
ALSA Driver Layer
User Application
       |
       v
ALSA Library
       |
       v
ALSA PCM Core
       |
       v
PCM Driver
       |
       v
DMA Engine
       |
       v
McASP / I2S
       |
       v
Audio Codec
       |
       v
Speaker / Microphone
BeagleBone Black Audio Hardware

A production BeagleBone Black audio solution can involve the
AM335x McASP peripheral.

Conceptually:

                 AM335x
                   |
                   v
                 McASP
                   |
          +--------+--------+
          |                 |
          v                 v
        TX DMA            RX DMA
          |                 |
          v                 v
        Audio             Audio
         TX                RX
          |                 |
          +--------+--------+
                   |
                   v
              Audio Codec
                   |
          +--------+--------+
          |                 |
          v                 v
       Speaker           Microphone
I2S / McASP

Typical audio data flow:

CPU
 |
 v
ALSA
 |
 v
PCM
 |
 v
DMA
 |
 v
McASP
 |
 v
I2S / Audio Interface
 |
 v
Codec

McASP can provide the audio serial interface used to communicate
with an external codec.

DMA Audio Flow

A production driver normally uses DMA:

ALSA Buffer
     |
     v
DMA Buffer
     |
     v
DMA Controller
     |
     v
McASP
     |
     v
Codec

For capture:

Codec
  |
  v
McASP
  |
  v
DMA
  |
  v
ALSA Buffer
  |
  v
Application
Device Tree

A real ALSA/ASoC audio driver commonly obtains hardware information
from Device Tree.

Conceptually:

Device Tree
    |
    +-- McASP
    |
    +-- Codec
    |
    +-- I2C
    |
    +-- DMA
    |
    +-- Audio Card

Typical configuration includes:

compatible
reg
interrupts
dmas
dma-names
clocks
sound-dai
codec
ALSA vs ASoC
ALSA

ALSA is the overall Linux Advanced Linux Sound Architecture.

ALSA
 |
 +-- PCM
 +-- Mixer
 +-- Control
 +-- MIDI
 +-- Sequencer
ASoC

ASoC means:

ALSA System on Chip

It is designed for embedded SoCs.

ASoC separates audio components:

CPU DAI
   |
   v
Codec DAI
   |
   v
Machine Driver
   |
   v
Sound Card
ASoC Architecture
                 ALSA
                  |
                  v
             ASoC Core
                  |
        +---------+---------+
        |         |         |
        v         v         v
      CPU DAI   Codec DAI  Machine
        |         |         |
        +---------+---------+
                  |
                  v
             Sound Card
                  |
                  v
              PCM Device
Production Audio Driver Flow
Device Tree
     |
     v
Platform / ASoC Probe
     |
     v
Register CPU DAI
     |
     v
Register Codec DAI
     |
     v
Machine Driver
     |
     v
Create Sound Card
     |
     v
Create PCM
     |
     v
Configure DMA
     |
     v
Configure McASP/I2S
     |
     v
Configure Codec
     |
     v
ALSA Application
Important ALSA APIs

Sound card:

snd_card_new()
snd_card_register()
snd_card_free()

PCM:

snd_pcm_new()
snd_pcm_set_ops()

PCM parameters:

params_rate()
params_channels()
params_format()
params_buffer_bytes()
params_period_bytes()
Interview Explanation

A good embedded Linux ALSA explanation:

"I use the ALSA/ASoC framework to integrate audio hardware. The
audio driver registers the sound card and PCM interface, handles
PCM open, hardware parameter configuration, prepare and trigger
operations, and uses DMA for efficient audio data movement. On an
SoC such as the BeagleBone Black AM335x, the production audio path
typically involves ALSA PCM, DMA, McASP, an external codec, and
finally the speaker or microphone."

Production Audio Flow
Application
    |
    v
ALSA API
    |
    v
PCM
    |
    v
ASoC
    |
    v
Machine Driver
    |
    +----------+
    |          |
    v          v
CPU DAI     Codec DAI
    |          |
    v          v
  McASP      Codec
    |          |
    +----+-----+
         |
         v
        DMA
         |
         v
   Audio Hardware
Important Interview Topics
ALSA
ASoC
PCM
DAI
CPU DAI
Codec DAI
Machine Driver
I2S
McASP
DMA
PCM buffer
PCM period
Playback
Capture
Mixer controls
aplay
arecord
amixer
Device Tree
Audio clock configuration
Makefile Commands

Build:

make

Clean:

make clean

Load:

make load

Unload:

make unload

Status:

make status

Information:

make info

Logs:

make logs

Test:

make test
Final Audio Flow
              User Application
                     |
                     v
                ALSA Library
                     |
                     v
                 ALSA PCM
                     |
                     v
                  ASoC
                     |
             +-------+-------+
             |               |
             v               v
          CPU DAI        Codec DAI
             |               |
             v               v
           McASP           Codec
             |               |
             +-------+-------+
                     |
                     v
                    DMA
                     |
                     v
              Audio Hardware
                     |
              +------+------+
              |             |
              v             v
           Speaker      Microphone

Important: For a real BeagleBone Black audio implementation, the next step beyond this demo is an ASoC machine/codec driver + McASP + DMA + Device Tree configuration. The above module demonstrates the ALSA PCM framework, not actual audio data transfer.


### Final structure

```text
beaglebone-black/
└── drivers/
    └── 17_alsa/
        ├── alsa_driver.c
        ├── alsa_driver.h
        ├── makefile
        └── README.md

Core flow to remember:

Application
    ↓
ALSA Library
    ↓
ALSA PCM
    ↓
ASoC
    ↓
CPU DAI / McASP
    ↓
DMA
    ↓
Codec
    ↓
Speaker / Microphone
