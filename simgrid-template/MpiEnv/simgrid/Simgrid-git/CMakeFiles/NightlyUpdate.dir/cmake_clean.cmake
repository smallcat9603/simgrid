file(REMOVE_RECURSE
  "src/context_sysv_config.h"
  "src/internal_config.h"
  "include/smpi/mpif.h"
  "include/simgrid_config.h"
  "bin/smpicc"
  "bin/smpicxx"
  "bin/smpiff"
  "bin/smpif90"
  "bin/smpirun"
  "bin/colorize"
  "bin/simgrid_update_xml"
  "examples/smpi/tracing/smpi_traced.trace"
  "CMakeFiles/NightlyUpdate"
)

# Per-language clean rules from dependency scanning.
foreach(lang)
  include(CMakeFiles/NightlyUpdate.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
