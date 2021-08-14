# mt32-pi-ctl bash completion script
_mt32-pi-ctl() {
	local cur prev opts
	compopt +o default
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"
	opts="--help
	--verbose
	--reboot
	--mt32
	--fluidsynth
	--romset
	--soundfont
	--mt32-rstereo
	--mt32-reset
	--gm-reset
	--gs-reset
	--mt32-txt
	--sc55-txt
	--sc55-bmp
	--sc55-btxt
	--negative
	--midi
	--syx"

	if [[ ${cur} == -* || ${COMP_CWORD} -eq 1 ]]; then
		COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
		return 0
	fi
	
	case "${prev}" in
		-b|--romset)
			local romsets="old new cm32l"
			COMPREPLY=( $(compgen -W "${romsets}" -- ${cur}) )
			;;
		-Y|--syx|-P|--sc55-bmp)
			compopt -o default
			;;
		-p)
			COMPREPLY=()
			if command -v aplaymidi > /dev/null 2>&1; then
				local midiports=$(aplaymidi -l | awk 'NR>1 { print $1 }' ORS=' ')
				COMPREPLY=( $(compgen -W "${midiports}" -- ${cur}) )
			fi
	esac
}
complete -F _mt32-pi-ctl mt32-pi-ctl
