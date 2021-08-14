#compdef mt32-pi-ctl
_mt32-pi-ctl() {
	typeset -A opt_args
	local context state line
	
	_arguments -s -S \
		"--help[Display help.]" \
		"-v[Be verbose about what is going on.]" \
		"--verbose[Be verbose about what is going on.]" \
		"-b+[Switch MT-32 romset.]:romset:->romsets" \
		"--romset[Switch MT-32 romset.]:romset:->romsets" \
		"-p+[The ALSA MIDI client and port address to output to.]:midi-output:->midiports" \
		"-r[Reboot the Pi. Will block for a few secs to give it time.]" \
		"--reboot[Reboot the Pi. Will block for a few secs to give it time.]" \
		"-m[Switch mt32-pi to MT-32 mode.]" \
		"--mt32[Switch mt32-pi to MT-32 mode.]" \
		"-g[Switch mt32-pi to FluidSynth mode.]" \
		"--fluidsynth[Switch mt32-pi to FluidSynth mode.]" \
		"-s+[Set FluidSynth SoundFont. (number)]:SoundFont number (0-255):" \
		"--soundfont[Set FluidSynth SoundFont. (number)]:SoundFont number (0-255):" \
		"-S+[Enable/disable MT-32 reversed stereo. (0, 1)]:1 or 0:->rstereo" \
		"--mt32-rstereo[Enable/disable MT-32 reversed stereo. (0, 1)]:1 or 0:->rstereo" \
		"--mt32-reset[Send an MT-32 reset SysEx message]" \
		"--gm-reset[Send a GM reset SysEx message]" \
		"--gs-reset[Send a GS reset SysEx message]" \
		"--mt32-txt[Send an MT-32 text display SysEx]:some text:" \
		"-t+[Send an MT-32 text display SysEx]:some text:" \
		"--sc55-txt[Send an SC-55 text display SysEx]:some text:" \
		"-T+[Send an SC-55 text display SysEx]:some text:" \
		"--sc55-btxt[Display a string on the screen as a Bitmap. (SC-55)]:8 characters:" \
		"-X+[Display a string on the screen as a Bitmap. (SC-55)]:8 characters:" \
		"--sc55-bmp[Display a 16x16 1bpp BMP on the screen. (SC-55 SysEx)]:file:_files" \
		"-P+[Display a 16x16 1bpp BMP on the screen. (SC-55 SysEx)]:file:_files" \
		"-N[Reverse color. Use with '-P/--sc55-bmp' or '-X/--sc55-btxt'.]" \
		"--negative[Reverse color. Use with '-P/--sc55-bmp' or '-X/--sc55-btxt'.]" \
		"-M[Send a list of custom MIDI bytes (in hexadecimal).]:Hexadecimal Bytes:" \
		"--midi[Send a list of custom MIDI bytes (in hexadecimal).]:Hexadecimal Bytes:" \
		"-Y[Send the contents of a SYX-file.]:file:_files" \
		"--syx[Send the contents of a SYX-file.]:file:_files" \
		&& return 0

	case $state in
		(midiports)
			if type aplaymidi > /dev/null 2>&1; then
				local midiports=$(aplaymidi -l | awk -F '[[:space:]][[:space:]]+' 'NR>1 { gsub(/^[ \t]+/,""); gsub(/:/,"\\:"); print $1":"$3 }')
				midiports=(${(ps:\n:)${midiports}})
				_describe 'Available MIDI outputs' midiports && return 0
			else
				return 0
			fi
			;;
		(romsets)
			local romlist=('old:Old MT-32 (ROM v1.xx)' 'new:New MT-32 (ROM v2.xx)' 'cm32l:CM-32L')
			_describe 'romset' romlist && return 0
			;;
		(rstereo)
			local desclist=('0:Disable reversed stereo' '1:Enable reversed stereo')
			_describe 'reversed stereo' desclist && return 0
			;;
	esac

	return 1
}

_mt32-pi-ctl
