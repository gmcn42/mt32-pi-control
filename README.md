# An mt32-pi control program for DOS

`MT32-PI.EXE` is a DOS control program for the mt32-pi MIDI synthesizer.

## Features
* Supports sending mt32-pi's custom System Exclusive messages for temporary configuration of SoundFonts & Co.
* Can send standard MT-32, GM, and GS reset sequences
* Can send screen text in MT-32 mode and 16x16 1bpp bitmaps in SoundFont mode to the mt32-pi's display
* Custom MIDI messages from the command line
* compiled for 8086 Real Mode, so should work on pretty much any system with an MPU401-compatible interface
* Useful for making game-specific `.BAT` files to correctly set up the synth before the game starts

## Usage Summary
`MT32-PI.EXE` accepts the following UNIX-style parameters:

```
USAGE: MT32-PI.EXE [OPTIONS]
OPTIONS:
  -h/--help: Print this info.
  -v/--verbose: Be verbose about what is going on.
  -r/--reboot: Reboot the Pi. Will block for a few secs to give it time.
  -p/--port [ADDR]: Set the port address of the MPU401 interface. Default: 330.
  -m/--mt32: Switch mt32-pi to MT-32 mode.
  -g/--fluidsynth: Switch mt32-pi to FluidSynth mode.
  -b/--romset [old, new, cm32l]: Switch MT-32 romset.
  -s/--soundfont [NUMBER]: Set FluidSynth SoundFont.
  --mt32-reset: Send an MT-32 reset SysEx message.
  --gm-reset: Send a GM reset SysEx message.
  --gs-reset: Send a GS reset SysEx message.
  -t/--mt32-txt "Some text": Send an MT-32 text display SysEx.
  -T/--sc55-txt "Some text": Send an SC-55 text display SysEx.
  -P/--sc55-bmp FILE.BMP: Display a 16x16 1bpp BMP on the screen. (SC-55 SysEx)
  -N/--negative: Reverse image color. Use with '-P/--sc55-bmp'.
  -M/--midi "C0 01 C0 DE": Send a list of custom MIDI bytes.
```

## Building
The `MAKEFILE` is written for the DOS-version of [Open Watcom C 1.9](https://sourceforge.net/projects/openwatcom/files/open-watcom-1.9/) as it can generate Real Mode executables. The Sourceforge release works perfectly in DosBox.
Make sure you have the environment variables correctly set up. For that, DosBox users will need to run the `AUTOEXEC.BAT` code supplied by the installer.

Then run `wmake` in the source folder and compilation should run. Optionally, if you also have [upx](https://upx.github.io/) installed in your DOS environment, you can run `UPXCOMP.BAT` afterwards to pack the EXE and save a couple KB of executable size.

## Showcase :)
<img src="https://github.com/gmcn42/mt32-pi-control/raw/main/images/mt32pictl_1.jpg" width="480">

<img src="https://github.com/gmcn42/mt32-pi-control/raw/main/images/mt32pictl_2.jpg" width="480">
