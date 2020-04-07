set grid
set xlabel "Time (ns)"
set ylabel "Signal amplitude (mV)"
set key out

run_id=1*ARG1
## work_dir=sprintf("%s", ARG2)
## ch_id=sprintf("%s", ARG3)
work_dir=ARG2
ch_id_str=ARG3
group_min=ARG4+0
group_max=ARG5+0
ngroups=group_max-group_min+1
print "Run ID     = ", run_id
print "Work dir   = '", work_dir, "'"
print "Channel ID = '", ch_id_str, "'"
print "Group min  = ", group_min
print "Group max  = ", group_max

set title sprintf("Run %d -- Mean calo waveform for channel [%s]", run_id, ch_id_str)

array mwffiles[ngroups]
array grpids[ngroups]
index=0
do for [igroup = 1:ngroups] {
    group_id = igroup - 1
    _stmp = sprintf('%s/calo_channel-%s/mean_waveform_group-%d.data',work_dir,ch_id_str,group_id)
    syscommand = sprintf("grep '@mean_waveform.nevents=' %s | cut -d= -f2", _stmp)
    mwf_nevents = system(syscommand)
    if (mwf_nevents > 0) {
	index = index + 1
	mwffiles[index] = _stmp
	grpids[index] = group_id
    }
}
max_index=index
## plot for [igroup=1:ngroups] \
##      mwffiles[igroup] \
##      title sprintf("Group #%d", igroup - 1) with lines
##pause -1 "Hit [Enter]..."

set terminal push
set terminal pdfcairo
set output sprintf("%s/run-%d_calo_mean_waveforms_channel-%s.pdf", work_dir, run_id, ch_id_str)
plot for [index=1:max_index] \
     mwffiles[index] \
     title sprintf("Group #%d", grpids[index]) with lines
##replot
set output
set terminal pop

# end





