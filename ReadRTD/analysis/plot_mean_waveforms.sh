#!/usr/bin/env bash

run_id=104
work_dir="./_build.d/calo_mean_waveforms.d"

channel_ids=$(find ${work_dir}/ -type d  | cut -d/ -f4 | cut -d- -f2)

calo_rows="0 1 11 12"

declare -i side=1
declare -i column=0

for channel_id in ${channel_ids}; do
    echo >&2 "[info] Processing channel ID = ${channel_id}..."
    min_group=0
    max_group=5
    gpscript="./analysis/mean_waveform.gp"
    if [ -d ${work_dir}/calo_channel-${channel_id} ]; then
	gnuplot -c ${gpscript} ${run_id} "${work_dir}" "${channel_id}" ${min_group} ${max_group} 
    fi
done

find ${work_dir} -name "run-${run_id}*.pdf"

exit 0

# end
