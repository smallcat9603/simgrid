file(REMOVE_RECURSE
  "bin/colorize"
  "bin/simgrid_update_xml"
  "bin/smpicc"
  "bin/smpicxx"
  "bin/smpif90"
  "bin/smpiff"
  "bin/smpirun"
  "examples/smpi/replay/actions0.txt"
  "examples/smpi/replay/actions1.txt"
  "examples/smpi/replay/actions_allgatherv.txt"
  "examples/smpi/replay/actions_allreduce.txt"
  "examples/smpi/replay/actions_alltoall.txt"
  "examples/smpi/replay/actions_alltoallv.txt"
  "examples/smpi/replay/actions_barrier.txt"
  "examples/smpi/replay/actions_bcast.txt"
  "examples/smpi/replay/actions_gather.txt"
  "examples/smpi/replay/actions_reducescatter.txt"
  "examples/smpi/replay/actions_waitall.txt"
  "examples/smpi/replay/actions_with_isend.txt"
  "examples/smpi/replay_multiple/README"
  "examples/smpi/replay_multiple/description_file"
  "examples/smpi/replay_multiple/smpi_replay.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace0.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace1.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace10.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace11.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace12.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace13.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace14.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace15.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace16.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace17.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace18.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace19.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace2.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace20.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace21.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace22.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace23.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace24.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace25.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace26.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace27.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace28.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace29.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace3.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace30.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace31.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace4.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace5.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace6.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace7.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace8.txt"
  "examples/smpi/replay_multiple/ti_traces_32_1/ti_trace9.txt"
  "examples/smpi/replay_multiple_manual_deploy/compute_only.txt"
  "examples/smpi/replay_multiple_manual_deploy/compute_only/actions0.txt"
  "examples/smpi/replay_multiple_manual_deploy/compute_only/actions1.txt"
  "examples/smpi/replay_multiple_manual_deploy/empty.txt"
  "examples/smpi/replay_multiple_manual_deploy/empty/actions0.txt"
  "examples/smpi/replay_multiple_manual_deploy/empty/actions1.txt"
  "examples/smpi/replay_multiple_manual_deploy/mixed.txt"
  "examples/smpi/replay_multiple_manual_deploy/mixed/actions0.txt"
  "examples/smpi/replay_multiple_manual_deploy/mixed/actions1.txt"
  "examples/smpi/replay_multiple_manual_deploy/replay_multiple_manual.tesh"
  "examples/smpi/replay_multiple_manual_deploy/workload_compute"
  "examples/smpi/replay_multiple_manual_deploy/workload_compute_consecutive"
  "examples/smpi/replay_multiple_manual_deploy/workload_compute_consecutive2"
  "examples/smpi/replay_multiple_manual_deploy/workload_compute_simple"
  "examples/smpi/replay_multiple_manual_deploy/workload_empty1"
  "examples/smpi/replay_multiple_manual_deploy/workload_empty2"
  "examples/smpi/replay_multiple_manual_deploy/workload_empty2_same_resources"
  "examples/smpi/replay_multiple_manual_deploy/workload_empty2_same_time"
  "examples/smpi/replay_multiple_manual_deploy/workload_empty2_same_time_and_resources"
  "examples/smpi/replay_multiple_manual_deploy/workload_mixed1"
  "examples/smpi/replay_multiple_manual_deploy/workload_mixed2"
  "examples/smpi/replay_multiple_manual_deploy/workload_mixed2_same_resources"
  "examples/smpi/replay_multiple_manual_deploy/workload_mixed2_same_time"
  "examples/smpi/replay_multiple_manual_deploy/workload_mixed2_same_time_and_resources"
  "examples/smpi/replay_multiple_manual_deploy/workload_nojob"
  "examples/smpi/tracing/smpi_traced.trace"
  "include/simgrid/config.h"
  "include/simgrid/version.h"
  "include/smpi/mpif.h"
  "src/internal_config.h"
  "teshsuite/smpi/hostfile"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/hardlinks.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
