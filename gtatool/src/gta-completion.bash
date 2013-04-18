# Copyright (C) 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

_gta()
{
    local cur commands
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    commands="
	combine
	component-add
	component-compute
	component-convert
	component-extract
	component-merge
	component-reorder
	component-set
	component-split
	compress
	create
	diff
	dimension-add
	dimension-extract
	dimension-flatten
	dimension-merge
	dimension-reorder
	dimension-reverse
	dimension-split
	extract
	fill
	from
	from-csv
	from-datraw
	from-dcmtk
	from-exr
	from-ffmpeg
	from-gdal
	from-jpeg
	from-magick
	from-mat
	from-netcdf
	from-netpbm
	from-pcd
	from-pfs
	from-ply
	from-pvm
	from-rat
	from-raw
	from-sndfile
	from-teem
	gui
	help
	info
	merge
	resize
	set
	stream-extract
	stream-foreach
	stream-grep
	stream-merge
	stream-split
	tag
	to
	to-csv
	to-datraw
	to-exr
	to-gdal
	to-jpeg
	to-magick
	to-mat
	to-netcdf
	to-netpbm
	to-pcd
	to-pfs
	to-ply
	to-pvm
	to-rat
	to-raw
	to-sndfile
	to-teem
	uncompress
	version
    "

    cmd=""
    if [ ${COMP_CWORD} -gt 0 ]; then
	if [[ ${COMP_WORDS[1]} == -* ]]; then
	    cmd=${COMP_WORDS[2]}
	else
	    cmd=${COMP_WORDS[1]}
	fi
    fi
    if [ -z "$cmd" ]; then
	cmd="gta"
    fi

    case "$cmd" in
    combine)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --mode" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-add)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --components --index --value" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-compute)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --expression" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-convert)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --components --normalize" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-extract)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --keep --drop" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-merge)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-reorder)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --indices" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-set)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --indices --value" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    component-split)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --drop" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    compress)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --method" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    create)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimensions --components --value --n" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    diff)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --absolute --force" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-add)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimension" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-extract)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimension --index" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-flatten)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --prepend-coordinates" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-merge)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-reorder)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --indices" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-reverse)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --indices" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    dimension-split)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimension" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    extract)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --low --high" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    fill)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --low --high --value" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    from)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-csv)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --components --delimiter --no-data-value" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-datraw)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-dcmtk)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-exr)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-ffmpeg)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --list-streams --stream" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-gdal)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-jpeg)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-magick)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --force-format" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-mat)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-netcdf)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-netpbm)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-pcd)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-pfs)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-ply)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-pvm)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-rat)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-raw)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimensions --components --endianness" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-sndfile)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    from-teem)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    gui)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    help)
	COMPREPLY=( $(compgen -W "${commands}" -- ${cur}) )
	;;
    info)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --statistics" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    merge)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimension" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    resize)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --dimensions --index --value" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    set)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --index --source" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    stream-extract)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --drop" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    stream-foreach)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --n" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    stream-grep)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    stream-merge)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    stream-split)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    tag)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --get-global --set-global --unset-global --unset-global-all --get-dimension --set-dimension --unset-dimension --unset-dimension-all --get-component --set-component --unset-component --unset-component-all --unset-all" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    to)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-csv)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --delimiter" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-datraw)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-exr)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-gdal)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --format" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-jpeg)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --quality" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-magick)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-mat)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-netcdf)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-netpbm)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-pcd)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-pfs)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-ply)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-pvm)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-rat)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-raw)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --endianness" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-sndfile)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    to-teem)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -- ${cur}) )
	fi
	;;
    uncompress)
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -f -o plusdirs -X '!*.gta' -- ${cur}) )
	fi
	;;
    version)
	;;
    *)
	# we only have "gta"
	if [[ ${cur} == -* ]]; then
	    COMPREPLY=( $(compgen -W "--help --version --verbose --quiet" -- ${cur}) )
	else
	    COMPREPLY=( $(compgen -W "${commands}" -- ${cur}) )
	fi
    esac
    return 0
}

complete -F _gta -o filenames gta
