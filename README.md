# An mt32-pi control program for DOS, Amiga, Atari ST, Linux, and Windows

`MT32-PI.EXE`/`MT32-PI.TTP`/`mt32-pi-ctl` is a control program for the [mt32-pi MIDI synthesizer](https://github.com/dwhinham/mt32-pi) available for DOS PCs, Atari ST and Amiga computers as well as modern systems running Linux and Windows.

## Features
* Supports sending mt32-pi's custom System Exclusive messages for temporary configuration of SoundFonts & Co.
* Can send standard MT-32, GM, and GS reset sequences
* Can send screen text in MT-32 mode and 16x16 1bpp bitmaps in SoundFont mode to the mt32-pi's display
* Custom MIDI messages from the command line
* DOS version compiled for 8086 Real Mode, so should work on pretty much any system with an MPU401-compatible interface
* Amiga version is compiled for AmigaOS ≥1.3 and supports both `camd.library` and direct serial port access, so it likewise should work on pretty much any system
* Useful for making game/application-specific start-up scripts to correctly set up the synth

## Usage Summary
`MT32-PI.EXE`/`MT32-PI.TTP`/`mt32-pi-ctl` accept the following UNIX-style parameters:

### Platform-specific options
#### DOS-specific options

```
USAGE: MT32-PI.EXE [OPTIONS]
OPTIONS:
  -p [ADDR]: Set the port address of the MPU401 interface. Default: 330.
```

#### AmigaOS-specific options

```
USAGE: mt32-pi-ctl [OPTIONS]
OPTIONS:
  -S: Use direct serial port access instead of camd.library.
  -l: The camd output location to connect to. (Default: out.0)
```

#### Atari-ST-specific options

```
USAGE: MT32-PI.TTP [OPTIONS]
OPTIONS:
  [No special options as the Atari ST has a built-in MIDI port which is automatically used]
```
This is a CLI/TTP-utility. If you launch it directly from TOS a graphical prompt will let
you input the parameters. Though for more convenience I would recommend a text based shell
like EmuCON or Okami.

Note that quoting in Atari ST shells seems to work a bit different than in other systems (or
maybe I don't get it but the following works, in any case).
Quoted parameters must use single-quotes, be located in front, and the parameter needs to
be within the quotes.

Example: When on other systems you'd write `mt32-pi-ctl -m -t "Hello, World!"`, on Atari ST
it should be `MT32-PI.TTP '-t Hello, World!' -m`.

#### Linux-specific options

```
USAGE: mt32-pi-ctl [OPTIONS]
OPTIONS:
  -p "[CLIENT]:[PORT]" : The ALSA MIDI client and port address to output to (*MANDATORY*).
```
To find out which client/port to use, you can run `aconnect -l` to list available devices.

#### Windows-specific options

```
USAGE: mt32-pi-ctl.exe [OPTIONS]
OPTIONS:
  -p PORT : Set MIDI output port number (*MANDATORY*).
  -l : List available MIDI output ports and exit.
```

### Common options
```
  -h/--help: Print this info.
  -v/--verbose: Be verbose about what is going on.
  -r/--reboot: Reboot the Pi. Will block for a few secs to give it time.
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
  -X/--sc55-btxt "SomeText": Display a string on the screen as a Bitmap using the
                             miniwi 4x8 font. Max. 8 characters (SC-55)
  -N/--negative: Reverse image color. Use with '-P/--sc55-bmp'.
  -M/--midi "C0 01 C0 DE": Send a list of custom MIDI bytes.
```

You may specify multiple options, i.e. `MT32-PI.EXE -m -t "Hello, World!"` will first send the MT-32 mode command and then the screen text.

## Building

### DOS
The `MAKEFILE` is written for the DOS-version of [Open Watcom C 1.9](https://sourceforge.net/projects/openwatcom/files/open-watcom-1.9/) as it can generate Real Mode executables. The Sourceforge release works perfectly in DosBox.
Make sure you have the environment variables correctly set up. For that, DosBox users will need to run the `AUTOEXEC.BAT` code supplied by the installer.

Then run `wmake` in `dos_src/` and compilation should run. Optionally, if you also have [upx](https://upx.github.io/) installed in your DOS environment, you can run `UPXCOMP.BAT` afterwards to pack the EXE and save a couple KB of executable size. `wmake dist` will also compress and then copy the resulting file to `dos_bin/`.

### Amiga
The `Makefile` is written for [bebbo's amiga-gcc](https://github.com/bebbo/amiga-gcc). After you have installed the toolchain, you also need to run `make sdk=camd` in your `amiga-gcc` source directory to install the `camd` library.

After that, you can run `make` and `make dist` in the `amiga-src/` folder. If you have installed amiga-gcc somewhere else than `/opt/amiga`, you will need to adjust the `PREFIX` variable in the `Makefile` accordingly.

## Showcase :)
<img src="https://github.com/gmcn42/mt32-pi-control/raw/main/images/mt32pictl_1.jpg" width="480">

<img src="https://github.com/gmcn42/mt32-pi-control/raw/main/images/mt32pictl_2.jpg" width="480">

The BMP file is in the archive. I made it with GIMP but MS Paint and others should work just fine as long as the file is 16x16px monochrome (1-bit indexed).
