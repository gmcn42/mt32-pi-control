# mt32-pi-ctl fish completion script

function __fish_list_midi_ports --description 'List available MIDI ports'
    if type -q aplaymidi
        aplaymidi -l | awk 'NR>1 {gsub(/^[[:space:]]+/, "", $1); print $1, $3}' FS='[[:space:]]{2,}' OFS='\t'
    end
end

complete -c mt32-pi-ctl -f
complete -c mt32-pi-ctl -s h -l help -d "Display help."
complete -c mt32-pi-ctl -s v -l verbose -d "Be verbose about what is going on."
complete -c mt32-pi-ctl -s r -l reboot -d "Reboot the Pi. Will block for a few secs to give it time."
complete -c mt32-pi-ctl -s p -x -a "(__fish_list_midi_ports)" -d "The ALSA MIDI client and port address to output to."
complete -c mt32-pi-ctl -s m -l mt32 -d "Switch mt32-pi to MT-32 mode."
complete -c mt32-pi-ctl -s g -l fluidsynth -d "Switch mt32-pi to FluidSynth mode."
complete -c mt32-pi-ctl -s b -l romset -x -a "(echo -e \"old\tOld MT-32 (ROM v1.xx)\nnew\tNew MT-32 (ROM v2.xx)\ncm32l\tCM-32L\n\")" -d "Switch MT-32 romset."
complete -c mt32-pi-ctl -s s -l soundfont -x -d "Set FluidSynth SoundFont. (number)"
complete -c mt32-pi-ctl -s S -l mt32-rstereo -x -d "Enable/disable MT-32 reversed stereo. (0, 1)"
complete -c mt32-pi-ctl -l mt32-reset -d "Send an MT-32 reset SysEx message."
complete -c mt32-pi-ctl -l gm-reset -d "Send a GM reset SysEx message."
complete -c mt32-pi-ctl -l gs-reset -d "Send a GS reset SysEx message."
complete -c mt32-pi-ctl -s t -l mt32-txt -x -d "Send an MT-32 text display SysEx."
complete -c mt32-pi-ctl -s T -l sc55-txt -x -d "Send an SC-55 text display SysEx."
complete -c mt32-pi-ctl -s P -l sc55-bmp -r -d "Display a 16x16 1bpp BMP on the screen. (SC-55 SysEx)"
complete -c mt32-pi-ctl -s X -l sc55-btxt -x -d "Display a string on the screen as a Bitmap. (SC-55)"
complete -c mt32-pi-ctl -s N -l negative -n "__fish_contains_opt -s P sc55-bmp -s X sc55-btxt" -d "Reverse color. Use with '-P/--sc55-bmp' or '-X/--sc55-btxt'."
complete -c mt32-pi-ctl -s M -l midi -x -d "Send a list of custom MIDI bytes."
complete -c mt32-pi-ctl -s Y -l syx -r -d "Send the contents of a SYX-file."
