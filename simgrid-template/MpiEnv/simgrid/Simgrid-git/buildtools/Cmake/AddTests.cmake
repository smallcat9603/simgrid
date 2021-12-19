
IF(enable_memcheck)
  include(FindValgrind)
ENDIF()

IF(enable_smpi AND NOT WIN32)
  exec_program("chmod a=rwx ${CMAKE_BINARY_DIR}/bin/smpicc" OUTPUT_VARIABLE "OKITOKI")
  exec_program("chmod a=rwx ${CMAKE_BINARY_DIR}/bin/smpicxx" OUTPUT_VARIABLE "OKITOKI")
  exec_program("chmod a=rwx ${CMAKE_BINARY_DIR}/bin/smpiff" OUTPUT_VARIABLE "OKITOKI")
  exec_program("chmod a=rwx ${CMAKE_BINARY_DIR}/bin/smpif90" OUTPUT_VARIABLE "OKITOKI")
  exec_program("chmod a=rwx ${CMAKE_BINARY_DIR}/bin/smpirun" OUTPUT_VARIABLE "OKITOKI")
ENDIF()

### For code coverage
### Set some variables
SET(UPDATE_TYPE "svn")
SET(DROP_METHOD "http")
SET(DROP_SITE "cdash.inria.fr/CDash")
SET(DROP_LOCATION "/submit.php?project=${PROJECT_NAME}")
SET(DROP_SITE_CDASH TRUE)
SET(TRIGGER_SITE "http://cdash.inria.fr/CDash/cgi-bin/Submit-Random-TestingResults.cgi")
#If you use the --read-var-info option Memcheck will run more slowly but may give a more detailed description of any illegal address.

IF(WIN32)
SET(TESH_COMMAND perl.exe ${CMAKE_BINARY_DIR}/bin/tesh)
SET(TESH_OPTION $TESH_OPTION --timeout 50)
ELSE()
SET(TESH_COMMAND perl ${CMAKE_BINARY_DIR}/bin/tesh)
ENDIF()

#some tests may take forever on non futexes systems, using busy_wait with n cores < n workers
# default to posix for these tests if futexes are not supported
IF(NOT HAVE_FUTEX_H)
SET(CONTEXTS_SYNCHRO --cfg contexts/synchro:posix)
ENDIF()

MACRO(ADD_TESH NAME)
  SET(ARGT ${ARGV})
  LIST(REMOVE_AT ARGT 0)
  IF(WIN32)
    STRING(REPLACE "§" "\;" ARGT "${ARGT}")
  ENDIF()
  ADD_TEST(${NAME} ${TESH_COMMAND} ${TESH_OPTION} ${ARGT})
ENDMACRO()

MACRO(ADD_TESH_FACTORIES NAME FACTORIES)
  SET(ARGR ${ARGV})
  LIST(REMOVE_AT ARGR 0) # remove name
  FOREACH(I ${FACTORIES}) # remove all factories
    LIST(REMOVE_AT ARGR 0)
  ENDFOREACH()
  FOREACH(FACTORY ${FACTORIES})
    if ((${FACTORY} STREQUAL "thread") OR
        (${FACTORY} STREQUAL "raw" AND HAVE_RAWCTX) OR
        (${FACTORY} STREQUAL "ucontext" AND CONTEXT_UCONTEXT))
      ADD_TESH("${NAME}-${FACTORY}" "--cfg" "contexts/factory:${FACTORY}" ${ARGR})
    ENDIF()
  ENDFOREACH()
ENDMACRO()

INCLUDE(CTest)
ENABLE_TESTING()

IF(NOT enable_memcheck)
  ## CORE ##
  ### TESH ###
  ADD_TESH(tesh-self-basic                       --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/basic.tesh)
  ADD_TESH(tesh-self-cd                          --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/cd.tesh)
  ADD_TESH(tesh-self-IO-broken-pipe              --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/IO-broken-pipe.tesh)
  ADD_TESH(tesh-self-IO-orders                   --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/IO-orders.tesh)
  ADD_TESH(tesh-self-IO-bigsize                  --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/IO-bigsize.tesh)
  ADD_TESH(tesh-self-set-return                  --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/set-return.tesh)
  ADD_TESH(tesh-self-set-timeout                 --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/set-timeout.tesh)
  ADD_TESH(tesh-self-set-output-ignore           --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/set-output-ignore.tesh)
  ADD_TESH(tesh-self-set-output-sort             --setenv bindir=${CMAKE_BINARY_DIR}/bin --cd "${CMAKE_HOME_DIRECTORY}/tools/tesh" set-output-sort.tesh)
  ADD_TESH(tesh-self-catch-return                --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/catch-return.tesh)
  ADD_TESH(tesh-self-catch-timeout               --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/catch-timeout.tesh)
  ADD_TESH(tesh-self-catch-wrong-output          --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/catch-wrong-output.tesh)
  ADD_TESH(tesh-self-bg-basic                    --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/bg-basic.tesh)
  ADD_TESH(tesh-self-background                  --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/background.tesh)
  IF(NOT WIN32)
    ADD_TESH(tesh-self-set-signal                --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/set-signal.tesh)
    ADD_TESH(tesh-self-bg-set-signal             --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/bg-set-signal.tesh)
    ADD_TESH(tesh-self-catch-signal              --cd "${CMAKE_BINARY_DIR}/bin" ${CMAKE_HOME_DIRECTORY}/tools/tesh/catch-signal.tesh)
  ENDIF()

IF(enable_java)
  IF(WIN32)
    SET(TESH_CLASSPATH "${CMAKE_BINARY_DIR}/teshsuite/java/\;${SIMGRID_FULL_JAR}")
    STRING(REPLACE "\;" "§" TESH_CLASSPATH "${TESH_CLASSPATH}")
  ELSE()
    SET(TESH_CLASSPATH "${CMAKE_BINARY_DIR}/teshsuite/java/:${SIMGRID_FULL_JAR}")
  ENDIF()
  ADD_TESH(tesh-java-sleep-host-off            --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/teshsuite/java ${CMAKE_HOME_DIRECTORY}/teshsuite/java/sleep_host_off/sleep_host_off.tesh)
ENDIF()

  ### GENERIC  ###
  # BEGIN TESH TESTS
  # test for code coverage
  ADD_TEST(test-help                             ${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms/basic_parsing_test --help)
  ADD_TEST(test-help-models                      ${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms/basic_parsing_test --help-models)
  IF(HAVE_TRACING)
    ADD_TEST(test-tracing-help                   ${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms/basic_parsing_test --help-tracing)
    ADD_TESH(graphicator                         --setenv srcdir=${CMAKE_HOME_DIRECTORY} --setenv bindir=${CMAKE_BINARY_DIR}/bin --cd ${CMAKE_HOME_DIRECTORY}/tools/graphicator graphicator.tesh)
  ENDIF()
  # END TESH TESTS

  ### MC ###
  IF(HAVE_MC)
    ADD_TESH(tesh-mc-dwarf                       --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/mc/dwarf --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/mc/dwarf dwarf.tesh)
    ADD_TESH(tesh-mc-dwarf-expression            --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/mc/dwarf_expression --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/mc/dwarf_expression dwarf_expression.tesh)

    ADD_TESH_FACTORIES(mc-bugged1                "ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/mc --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/mc bugged1.tesh)
    ADD_TESH_FACTORIES(mc-bugged2                "ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/mc --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/mc bugged2.tesh)
    IF(CONTEXT_UCONTEXT AND PROCESSOR_x86_64) # liveness model-checking works only on 64bits (for now ...)
      ADD_TESH(mc-bugged1-liveness-ucontext         --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/mc --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/mc bugged1_liveness.tesh)
      ADD_TESH(mc-bugged1-liveness-ucontext-sparse  --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/mc --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/mc bugged1_liveness_sparse.tesh)
      ADD_TESH(mc-bugged1-liveness-visited-ucontext --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/mc --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/mc bugged1_liveness_visited.tesh)
      ADD_TESH(mc-bugged1-liveness-visited-ucontext-sparse --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/mc --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/mc bugged1_liveness_visited_sparse.tesh)
    ENDIF()
  ENDIF()


  ### SIMIX ###
  # BEGIN TESH TESTS
  IF(HAVE_RAWCTX)
    ADD_TESH(tesh-simix-factory-default          --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_raw.tesh)
  ELSEIF(CONTEXT_UCONTEXT)
    ADD_TESH(tesh-simix-factory-default          --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_ucontext.tesh)
  ELSE()
    ADD_TESH(tesh-simix-factory-default          --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_thread.tesh)
  ENDIF()
  ADD_TESH(tesh-simix-factory-thread             --cfg contexts/factory:thread --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_thread.tesh)
  IF(HAVE_RAWCTX)
    ADD_TESH(tesh-simix-factory-raw              --cfg contexts/factory:raw --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_raw.tesh)
  ENDIF()
  IF(CONTEXT_UCONTEXT)
    ADD_TESH(tesh-simix-factory-ucontext         --cfg contexts/factory:ucontext --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/check_defaults --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/check_defaults factory_ucontext.tesh)
  ENDIF()
  # END TESH TESTS
  ADD_TESH_FACTORIES(stack-overflow              "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simix/stack_overflow --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simix/stack_overflow stack_overflow.tesh)
  ###
  ### Declare that we know that some tests are broken
  ###
  IF(release)
    IF(WIN32 OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
      # These tests are known to fail on Windows and Mac OS X
      # (the expected error message is not shown).
      SET_TESTS_PROPERTIES(stack-overflow-thread PROPERTIES WILL_FAIL true)
      IF(CONTEXT_UCONTEXT)
        SET_TESTS_PROPERTIES(stack-overflow-ucontext PROPERTIES WILL_FAIL true)
      ENDIF()
      IF(HAVE_RAWCTX)
        SET_TESTS_PROPERTIES(stack-overflow-raw PROPERTIES WILL_FAIL true)
      ENDIF()
    ENDIF()
  ENDIF()


  ### SURF ###
  # BEGIN TESH TESTS
  ADD_TESH(tesh-surf-lmm                         --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/surf/lmm_usage --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/surf/lmm_usage lmm_usage.tesh)
  ADD_TESH(tesh-surf-maxmin                      --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/surf/maxmin_bench --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/surf/maxmin_bench maxmin_bench.tesh)
  ADD_TESH(tesh-surf-usage                       --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/surf/surf_usage --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/surf/ surf_usage/surf_usage.tesh)
  ADD_TESH(tesh-surf-trace                       --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/surf/trace_usage --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/surf/ trace_usage/trace_usage.tesh)
  # END TESH TESTS


  ### XBT ###
  # BEGIN TESH TESTS
  ADD_TESH(tesh-xbt-log-large                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/log_large --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/log_large log_large_test.tesh)
  ADD_TESH(tesh-xbt-log-parallel                 --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/parallel_log --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/parallel_log parallel_log_crashtest.tesh)
#  ADD_TESH(tesh-xbt-mallocator                  --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/mallocator --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/mallocator mallocator.tesh)
  IF(HAVE_MMALLOC)
    IF(${ARCH_32_BITS})
      ADD_TESH(tesh-xbt-mmalloc-32               --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/mmalloc --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/mmalloc mmalloc_32.tesh)
    ELSE()
      ADD_TESH(tesh-xbt-mmalloc-64               --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/mmalloc --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/mmalloc mmalloc_64.tesh)
    ENDIF()
  ENDIF()
  ADD_TESH(tesh-xbt-parmap                       --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/parmap_test --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/parmap_test parmap_test.tesh)
  IF(enable_debug)
    ADD_TESH(tesh-xbt-log                          --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/log_usage --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/log_usage log_usage.tesh)
  ELSE()
    ADD_TESH(tesh-xbt-log                          --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/log_usage --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/log_usage log_usage_ndebug.tesh)
  ENDIF()
  ADD_TESH(tesh-xbt-graphxml                     --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/graphxml_usage --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/graphxml_usage graphxml_usage.tesh)
  ADD_TESH(tesh-xbt-heap                         --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/heap_bench --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/heap_bench heap_bench.tesh)
  #ADD_TESH(test-xbt-parmap                         --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/xbt/parmap_bench --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/xbt/parmap_bench parmap_bench.tesh)
  # END TESH TESTS


  ## INTERFACES ##
  ### MSG ###
  # BEGIN TESH TESTS
  ADD_TESH_FACTORIES(tesh-msg-get-sender         "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/get_sender --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/get_sender ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/get_sender/get_sender.tesh)
  ADD_TESH_FACTORIES(tesh-msg-host-on-off        "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/host_on_off --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/host_on_off ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/host_on_off/host_on_off.tesh)
  ADD_TESH_FACTORIES(tesh-msg-host-on-off-processes "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/host_on_off_processes --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/host_on_off_processes ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/host_on_off_processes/host_on_off_processes.tesh)
  ADD_TESH_FACTORIES(tesh-msg-pid                "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/pid --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/pid ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/pid/pid.tesh)
  ADD_TESH_FACTORIES(tesh-msg-process            "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/process --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/process ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/process/process.tesh)
  ADD_TESH_FACTORIES(tesh-msg-process-join       "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/process_join --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/process_join ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/process_join/process_join.tesh)
  ADD_TESH_FACTORIES(tesh-msg-storage-basic      "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/storage --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/storage ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/storage/storage_basic.tesh)
  ADD_TESH_FACTORIES(tesh-msg-task-destroy-cancel "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/msg/task_destroy_cancel --cd ${CMAKE_BINARY_DIR}/teshsuite/msg/task_destroy_cancel ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/task_destroy_cancel/task_destroy_cancel.tesh)
  ADD_TESH_FACTORIES(tesh-msg-trace              "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/msg/trace --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/trace ${CMAKE_HOME_DIRECTORY}/teshsuite/msg/trace/trace.tesh)

  ADD_TESH(msg-file                              --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/ --cd ${CMAKE_HOME_DIRECTORY}/examples/ ${CMAKE_HOME_DIRECTORY}/examples/msg/io/io.tesh)
  ADD_TESH(msg-storage                           --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/ --cd ${CMAKE_HOME_DIRECTORY}/examples/ ${CMAKE_HOME_DIRECTORY}/examples/msg/io/storage.tesh)
  ADD_TESH(msg-remote-io                         --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/ --cd ${CMAKE_HOME_DIRECTORY}/examples/ ${CMAKE_HOME_DIRECTORY}/examples/msg/io/remote.tesh)
  ADD_TESH(msg-start-kill-time                   --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/ --cd ${CMAKE_HOME_DIRECTORY}/examples/ ${CMAKE_HOME_DIRECTORY}/examples/msg/start_kill_time/start_kill_time.tesh)
  ADD_TESH(msg-chainsend                         --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/chainsend --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg/chainsend --cd ${CMAKE_HOME_DIRECTORY}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/chainsend/chainsend.tesh)
  ADD_TESH_FACTORIES(msg-sendrecv-CLM03          "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/sendrecv/sendrecv_CLM03.tesh)
  ADD_TESH_FACTORIES(msg-sendrecv-Vegas          "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/sendrecv/sendrecv_Vegas.tesh)
  ADD_TESH_FACTORIES(msg-sendrecv-Reno           "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/sendrecv/sendrecv_Reno.tesh)
  ADD_TESH_FACTORIES(msg-suspend                 "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/suspend/suspend.tesh)
  ADD_TESH_FACTORIES(msg-pmm                     "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/pmm/pmm.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-bypass      "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_bypass.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-kill        "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_kill.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-multicore   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_multicore.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-no-crosstraffic-mailbox   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_mailbox.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-no-crosstraffic           "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-no-crosstraffic-forwarder "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_forwarder.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-no-crosstraffic-failure   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_failure.tesh)
  ADD_TESH_FACTORIES(msg-masterslave             "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-forwarder   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_forwarder_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-failure     "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_failure_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-mailbox     "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_mailbox_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-cpu-ti      "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/masterslave --cd ${CMAKE_HOME_DIRECTORY}/examples/msg masterslave/masterslave_cpu_ti_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-masterslave-vivaldi     "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/masterslave/masterslave_vivaldi.tesh)
  ADD_TESH_FACTORIES(msg-cloud-master-worker-vm      "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/cloud/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg/cloud/ --cd ${CMAKE_BINARY_DIR}/examples/msg/cloud/ ${CMAKE_HOME_DIRECTORY}/examples/msg/cloud/master_worker_vm.tesh)
  ADD_TESH_FACTORIES(msg-cloud-two-tasks-vm      "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/cloud/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg/cloud/ --cd ${CMAKE_BINARY_DIR}/examples/msg/cloud/ ${CMAKE_HOME_DIRECTORY}/examples/msg/cloud/two_tasks_vm.tesh)
  ADD_TESH_FACTORIES(msg-cloud-simple-vm         "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/cloud/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg/cloud/ --cd ${CMAKE_BINARY_DIR}/examples/msg/cloud/ ${CMAKE_HOME_DIRECTORY}/examples/msg/cloud/simple_vm.tesh)
  ADD_TESH_FACTORIES(msg-energy-pstates          "thread" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/energy/e1/pstate.tesh)
  ADD_TESH_FACTORIES(msg-energy-consumption      "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/energy/e2/energy_consumption.tesh)
  ADD_TESH_FACTORIES(msg-energy-concurrent-tasks "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/energy/e3/concurrent_tasks.tesh)
  ADD_TESH_FACTORIES(msg-token-ring              "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_BINARY_DIR}/examples/msg/token_ring ${CMAKE_HOME_DIRECTORY}/examples/msg/token_ring/token_ring.tesh)
  ADD_TESH_FACTORIES(msg-migration               "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/migration/migration.tesh)
  ADD_TESH_FACTORIES(msg-ptask                   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/parallel_task/parallel_task.tesh)
  ADD_TESH_FACTORIES(msg-priority                "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/priority/priority.tesh)
  ADD_TESH_FACTORIES(msg-properties              "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/properties/msg_prop.tesh)
  ADD_TESH_FACTORIES(msg-icomms                  "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg/icomms --cd ${CMAKE_BINARY_DIR}/examples/msg/icomms ${CMAKE_HOME_DIRECTORY}/examples/msg/icomms/peer.tesh)
  ADD_TESH_FACTORIES(msg-actions                 "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/actions --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/actions actions.tesh)
  ADD_TESH_FACTORIES(msg-chord-no-crosstraffic   "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/chord --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/chord chord.tesh)
  ADD_TESH_FACTORIES(msg-chord-no-crosstraffic-parallel "thread;ucontext;raw" --cfg contexts/nthreads:4 ${CONTEXTS_SYNCHRO} --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/chord --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/chord chord.tesh)
  ADD_TESH_FACTORIES(msg-chord                   "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/chord --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/chord chord_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-chord-parallel          "thread;ucontext;raw" --cfg contexts/nthreads:4  ${CONTEXTS_SYNCHRO} --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/chord --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/chord chord_crosstraffic.tesh)
  ADD_TESH_FACTORIES(msg-bittorrent              "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/bittorrent --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/bittorrent bittorrent.tesh)
  ADD_TESH_FACTORIES(msg-bittorrent-parallel     "thread;ucontext;raw" --cfg contexts/nthreads:4  ${CONTEXTS_SYNCHRO} --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/bittorrent --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/bittorrent bittorrent.tesh)
  ADD_TESH_FACTORIES(msg-kademlia                "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/kademlia --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/kademlia kademlia.tesh)
  ADD_TESH_FACTORIES(msg-kademlia-parallel       "thread;ucontext;raw" --cfg contexts/nthreads:4 --cfg contexts/factory:thread ${CONTEXTS_SYNCHRO} --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg/kademlia --cd ${CMAKE_HOME_DIRECTORY}/examples/msg/kademlia kademlia.tesh)
  ADD_TESH_FACTORIES(msg-gtnets-crosstraffic     "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/gtnets-crosstraffic.tesh)
  IF(HAVE_GTNETS)
    ADD_TESH_FACTORIES(msg-gtnets-waxman         "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/gtnets-waxman.tesh)
    ADD_TESH_FACTORIES(msg-gtnets-dogbone        "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/gtnets-dogbone-gtnets.tesh)
    ADD_TESH_FACTORIES(msg-gtnets-onelink        "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/gtnets-onelink-gtnets.tesh)
    ADD_TESH_FACTORIES(msg-gtnets-dogbone-lv08   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/gtnets-dogbone-lv08.tesh)
    ADD_TESH_FACTORIES(msg-gtnets-onelink-lv08   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/gtnets-onelink-lv08.tesh)
    IF(HAVE_TRACING)
      ADD_TESH(msg-tracing-gtnets-waxman         --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/tracing-gtnets-waxman.tesh)
      ADD_TESH(msg-tracing-gtnets-dogbone        --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/tracing-gtnets-dogbone-gtnets.tesh)
      ADD_TESH(msg-tracing-gtnets-onelink        --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/tracing-gtnets-onelink-gtnets.tesh)
      ADD_TESH(msg-tracing-gtnets-dogbone-lv08   --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/tracing-gtnets-dogbone-lv08.tesh)
      ADD_TESH(msg-tracing-gtnets-onelink-lv08   --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/msg --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/gtnets/tracing-gtnets-onelink-lv08.tesh)
    ENDIF()
  ENDIF()
  IF(HAVE_NS3)
    ADD_TESH_FACTORIES(msg-ns3                   "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_BINARY_DIR}/examples/msg ${CMAKE_HOME_DIRECTORY}/examples/msg/ns3/ns3.tesh)
  ENDIF()
  IF(HAVE_TRACING)
    ADD_TESH(tracing-ms                          --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/ms.tesh)
    ADD_TESH(tracing-trace-platform              --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/trace_platform.tesh)
    ADD_TESH(tracing-user-variables              --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/user_variables.tesh)
    ADD_TESH(tracing-link-user-variables         --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/link_user_variables.tesh)
    ADD_TESH(tracing-link-srcdst-user-variables  --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/link_srcdst_user_variables.tesh)
    ADD_TESH(tracing-categories                  --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/categories.tesh)
    ADD_TESH(tracing-process-migration           --setenv bindir=${CMAKE_BINARY_DIR}/examples/msg --cd ${CMAKE_HOME_DIRECTORY}/examples/msg tracing/procmig.tesh)
  ENDIF()
  ADD_TEST(msg-icomms-waitany                      ${CMAKE_BINARY_DIR}/examples/msg/icomms/peer3 ${CMAKE_HOME_DIRECTORY}/examples/platforms/small_platform.xml ${CMAKE_HOME_DIRECTORY}/examples/msg/icomms/deployment_peer05.xml)
  # END TESH TESTS


  ### SIMDAG ###
  # BEGIN TESH TESTS
  # these tests need the assertion mechanism
  # exclude them from memcheck, as they normally die, leaving lots of unfree'd objects
  IF(enable_debug AND NOT enable_memcheck)
    ADD_TESH(tesh-parser-bogus-symmetric         --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms bogus_two_hosts_asymetric.tesh)
    ADD_TESH(tesh-parser-bogus-missing-gw        --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms bogus_missing_gateway.tesh)
    ADD_TESH(tesh-parser-bogus-disk-attachment   --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms bogus_disk_attachment.tesh)

    #These tests fail on Windows as the signal returned is not the same as Unix would send.
    IF(HAVE_TRACING AND WIN32)
      set_property(TEST tesh-parser-bogus-symmetric PROPERTY WILL_FAIL TRUE)
      set_property(TEST tesh-parser-bogus-missing-gw PROPERTY WILL_FAIL TRUE)
      set_property(TEST tesh-parser-bogus-disk-attachment PROPERTY WILL_FAIL TRUE)
    ENDIF()
  ENDIF()
  ADD_TESH(tesh-simdag-bypass                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms basic_parsing_test_bypass.tesh)
  ADD_TESH(tesh-simdag-flatifier                 --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms flatifier.tesh)
  ADD_TESH(tesh-simdag-link                      --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --setenv srcdir=${CMAKE_HOME_DIRECTORY} --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms basic_link_test.tesh)
  ADD_TESH(tesh-simdag-reinit-costs              --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/test_reinit_costs.tesh)
  ADD_TESH(tesh-simdag-parser                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms basic_parsing_test.tesh)
  ADD_TESH(tesh-simdag-parser-sym-full           --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms basic_parsing_test_sym_full.tesh)
  ADD_TESH(tesh-simdag-full-links                --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms get_full_link.tesh)
  ADD_TESH(tesh-simdag-basic0                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic0.tesh)
  ADD_TESH(tesh-simdag-basic1                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic1.tesh)
  ADD_TESH(tesh-simdag-basic2                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic2.tesh)
  ADD_TESH(tesh-simdag-basic3                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic3.tesh)
  ADD_TESH(tesh-simdag-basic4                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic4.tesh)
  ADD_TESH(tesh-simdag-basic5                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic5.tesh)
  ADD_TESH(tesh-simdag-basic6                    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/basic --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/basic basic6.tesh)
  ADD_TESH(tesh-simdag-incomplete                --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/simdag/incomplete --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/incomplete incomplete.tesh)
  ADD_TESH(tesh-simdag-p2p-1                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/p2p/test_latency1.tesh)
  ADD_TESH(tesh-simdag-p2p-2                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/p2p/test_latency2.tesh)
  ADD_TESH(tesh-simdag-p2p-3                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/p2p/test_latency3.tesh)
  ADD_TESH(tesh-simdag-p2p-4                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/p2p/test_latency_bound.tesh)
  ADD_TESH(tesh-simdag-mxn-1                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/mxn/test_intra_all2all.tesh)
  ADD_TESH(tesh-simdag-mxn-2                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/mxn/test_intra_independent_comm.tesh)
  ADD_TESH(tesh-simdag-mxn-3                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/network/mxn/test_intra_scatter.tesh)
  ADD_TESH(tesh-simdag-par-1                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/partask/test_comp_only_seq.tesh)
  ADD_TESH(tesh-simdag-par-2                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/partask/test_comp_only_par.tesh)
  ADD_TESH(tesh-simdag-availability              --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite --cd ${CMAKE_BINARY_DIR}/teshsuite ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/availability/availability_test.tesh)
  ADD_TEST(tesh-simdag-full-links01                ${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms/basic_parsing_test ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms/two_clusters.xml FULL_LINK)
  ADD_TEST(tesh-simdag-full-links02                ${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms/basic_parsing_test ${CMAKE_HOME_DIRECTORY}/teshsuite/simdag/platforms/two_clusters_one_name.xml FULL_LINK)
  ADD_TEST(tesh-simdag-one-link-g5k                ${CMAKE_BINARY_DIR}/teshsuite/simdag/platforms/basic_parsing_test ${CMAKE_HOME_DIRECTORY}/examples/platforms/g5k.xml ONE_LINK)
  # END TESH TESTS

  # BEGIN TESH TESTS
  IF(HAVE_GRAPHVIZ)
    IF(HAVE_TRACING)
      ADD_TESH(simdag-dotload                    --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag/dot --cd ${CMAKE_BINARY_DIR}/examples/simdag/dot ${CMAKE_HOME_DIRECTORY}/examples/simdag/dot/test_simdag_dotload.tesh)
    ELSE()
      ADD_TESH(simdag-dotload                    --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag/dot --cd ${CMAKE_BINARY_DIR}/examples/simdag/dot ${CMAKE_HOME_DIRECTORY}/examples/simdag/dot/test_simdag_dotload_notrace.tesh)
    ENDIF()
  ENDIF()
  ADD_TESH(simdag-simdag                         --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/test_simdag.tesh)
  ADD_TESH(simdag-simdag2                        --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/test_simdag2.tesh)
  ADD_TESH(simdag-seq-access                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/test_simdag_seq_access.tesh)
  ADD_TESH(simdag-typed-tasks                    --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/test_simdag_typed_tasks.tesh)
  IF(HAVE_TRACING)
    ADD_TESH(simdag-fail                         --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag --cd ${CMAKE_HOME_DIRECTORY}/examples/simdag test_simdag_fail.tesh)
  ELSE()
    ADD_TESH(simdag-fail                         --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag --cd ${CMAKE_HOME_DIRECTORY}/examples/simdag test_simdag_fail_notrace.tesh)
  ENDIF()
  ADD_TESH(simdag-avail                          --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag --cd ${CMAKE_HOME_DIRECTORY}/examples/simdag test_simdag_avail.tesh)
  ADD_TESH(simdag-comm-throttling                --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/test_simdag_comm_throttling.tesh)
  IF(HAVE_TRACING)
    ADD_TESH(simdag-dax                          --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag/dax --cd ${CMAKE_HOME_DIRECTORY}/examples/simdag/dax smalldax.tesh)
  ELSE()
    ADD_TESH(simdag-dax                          --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag/dax --cd ${CMAKE_HOME_DIRECTORY}/examples/simdag/dax smalldax_notrace.tesh)
  ENDIF()
  ADD_TESH(simdag-dax-cycle                      --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag/dax --cd ${CMAKE_BINARY_DIR}/examples/simdag/dax ${CMAKE_HOME_DIRECTORY}/examples/simdag/dax/simple_dax_with_cycle.tesh)
  ADD_TESH(simdag-prop                           --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/properties/test_prop.tesh)
  ADD_TESH(simdag-minmin-scheduling              --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag/scheduling --cd ${CMAKE_HOME_DIRECTORY}/examples/simdag/scheduling test_minmin.tesh)
  ADD_TESH(simdag-io                             --setenv bindir=${CMAKE_BINARY_DIR}/examples/simdag/ --setenv srcdir=${CMAKE_HOME_DIRECTORY}/ --cd ${CMAKE_HOME_DIRECTORY}/examples/ ${CMAKE_HOME_DIRECTORY}/examples/simdag/io/io.tesh)
  IF(HAVE_TRACING)
    ADD_TESH(tracing-simdag                      --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/simdag --cd ${CMAKE_BINARY_DIR}/examples/simdag ${CMAKE_HOME_DIRECTORY}/examples/simdag/test_simdag_tracing.tesh)
  ENDIF()
  # END TESH TESTS

  IF(HAVE_TRACING AND WIN32)
    # expect this one to fail on Windows, as generated random colors will be different
    set_property(TEST tracing-simdag PROPERTY WILL_FAIL TRUE)
  ENDIF()
  ### SMPI ###
  IF(enable_smpi)
    # BEGIN TESH TESTS
    # smpi broken usage
    ADD_TESH_FACTORIES(tesh-smpi-broken          "thread"              --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/pingpong --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/pingpong broken_hostfiles.tesh)
    # smpi examples
    ADD_TESH_FACTORIES(tesh-smpi-bcast           "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/bcast --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/bcast bcast.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-reduce          "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/reduce --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/reduce reduce.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-vector          "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/vector --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/vector vector.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-hvector         "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/hvector --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/hvector hvector.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-indexed         "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/indexed --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/indexed indexed.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-struct          "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/struct --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/struct struct.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-pt2pt           "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/pingpong --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/pingpong pt2pt.tesh)
    ADD_TESH_FACTORIES(tesh-smpi-compute         "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/compute --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/compute compute.tesh)
    IF(NOT WIN32)
      ADD_TESH_FACTORIES(tesh-smpi-shared        "thread;ucontext;raw" --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/shared --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/shared shared.tesh)
    ENDIF()
    # https://gforge.inria.fr/tracker/index.php?func=detail&aid=17132&group_id=12&atid=165
    ADD_TESH_FACTORIES(tesh-smpi-bug-17132       "thread" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/bug-17132 --cd ${CMAKE_BINARY_DIR}/teshsuite/bug-17132 ${CMAKE_HOME_DIRECTORY}/teshsuite/bug-17132/bug-17132.tesh)
    IF(enable_debug)
      ADD_TESH_FACTORIES(tesh-smpi-bug-17132-surf-debug "thread" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/bug-17132 --cd ${CMAKE_BINARY_DIR}/teshsuite/bug-17132 ${CMAKE_HOME_DIRECTORY}/teshsuite/bug-17132/bug-17132-surf-debug.tesh)
    ENDIF()
    IF(HAVE_TRACING)
      ADD_TESH(tesh-smpi-replay-ti-tracing       --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/pingpong --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/pingpong TI_output.tesh)
    ENDIF()
    FOREACH (GATHER_COLL default ompi mpich ompi_basic_linear ompi_linear_sync ompi_binomial mvapich2 mvapich2_two_level impi)
      ADD_TESH(tesh-smpi-gather-coll-${GATHER_COLL} --cfg smpi/gather:${GATHER_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/gather --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/gather gather_coll.tesh)
    ENDFOREACH()
    FOREACH (ALLGATHER_COLL default  2dmesh 3dmesh bruck GB loosely_lr
                            NTSLR NTSLR_NB pair rdb  rhv ring SMP_NTS
                            smp_simple spreading_simple ompi mpich ompi_neighborexchange mvapich2 mvapich2_smp impi)
      ADD_TESH(tesh-smpi-allgather-coll-${ALLGATHER_COLL} --cfg smpi/allgather:${ALLGATHER_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/allgather --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/allgather allgather_coll.tesh)
    ENDFOREACH()
    FOREACH (ALLGATHERV_COLL default GB pair ring ompi mpich ompi_neighborexchange ompi_bruck mpich_rdb mpich_ring mvapich2 impi)
      ADD_TESH(tesh-smpi-allgatherv-coll-${ALLGATHERV_COLL} --cfg smpi/allgatherv:${ALLGATHERV_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/allgatherv --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/allgatherv allgatherv_coll.tesh)
    ENDFOREACH()
    FOREACH (ALLREDUCE_COLL default lr rab1 rab2 rab_rdb
                            rdb smp_binomial smp_binomial_pipeline
                            smp_rdb smp_rsag smp_rsag_lr smp_rsag_rab redbcast ompi mpich ompi_ring_segmented mvapich2 mvapich2_rs mvapich2_two_level impi)
      ADD_TESH(tesh-smpi-allreduce-coll-${ALLREDUCE_COLL} --cfg smpi/allreduce:${ALLREDUCE_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/allreduce --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/allreduce allreduce_coll.tesh)
    ENDFOREACH()
    FOREACH (ALLREDUCE_COLL_LARGE ompi_ring_segmented)
      ADD_TESH(tesh-smpi-allreduce-coll-large-${ALLREDUCE_COLL_LARGE} --cfg smpi/allreduce:${ALLREDUCE_COLL_LARGE} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/allreduce --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/allreduce allreduce_coll_large.tesh)
    ENDFOREACH()
    FOREACH (ALLTOALL_COLL 2dmesh 3dmesh pair pair_rma pair_one_barrier pair_light_barrier
                           pair_mpi_barrier rdb ring ring_light_barrier
                           ring_mpi_barrier ring_one_barrier
                           bruck basic_linear ompi mpich mvapich2 mvapich2_scatter_dest, impi)
      ADD_TESH(tesh-smpi-alltoall-coll-${ALLTOALL_COLL} --cfg smpi/alltoall:${ALLTOALL_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/alltoall --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/alltoall alltoall_coll.tesh)
    ENDFOREACH()
    FOREACH (ALLTOALLV_COLL default pair pair_light_barrier pair_mpi_barrier
                            pair_one_barrier  ring ring_light_barrier
                            ring_mpi_barrier ring_one_barrier bruck ompi mpich mvapich2 ompi_basic_linear impi)
      ADD_TESH(tesh-smpi-alltoallv-coll-${ALLTOALLV_COLL} --cfg smpi/alltoallv:${ALLTOALLV_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/alltoallv --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/alltoallv alltoallv_coll.tesh)
    ENDFOREACH()
    FOREACH (BCAST_COLL default arrival_pattern_aware arrival_pattern_aware_wait arrival_scatter
                        binomial_tree flattree flattree_pipeline NTSB NTSL NTSL_Isend scatter_LR_allgather
                        scatter_rdb_allgather SMP_binary SMP_binomial SMP_linear ompi mpich ompi_split_bintree ompi_pipeline mvapich2 mvapich2_intra_node mvapich2_knomial_intra_node impi)
      ADD_TESH(tesh-smpi-bcast-coll-${BCAST_COLL} --cfg smpi/bcast:${BCAST_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/bcast --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/bcast bcast_coll.tesh)
    ENDFOREACH()
    FOREACH (REDUCE_COLL default arrival_pattern_aware binomial flat_tree NTSL scatter_gather ompi mpich ompi_chain ompi_binary ompi_basic_linear ompi_binomial ompi_in_order_binary mvapich2 mvapich2_knomial mvapich2_two_level impi rab)
      ADD_TESH(tesh-smpi-reduce-coll-${REDUCE_COLL} --cfg smpi/reduce:${REDUCE_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/reduce --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/reduce reduce_coll.tesh)
    ENDFOREACH()
    FOREACH (REDUCE_SCATTER_COLL default  ompi mpich ompi_basic_recursivehalving ompi_ring mpich_noncomm mpich_pair mvapich2 mpich_rdb impi)
      ADD_TESH(tesh-smpi-reduce-scatter-coll-${REDUCE_SCATTER_COLL} --cfg smpi/reduce_scatter:${REDUCE_SCATTER_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/reduce --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/reduce reduce_scatter_coll.tesh)
    ENDFOREACH()
    FOREACH (SCATTER_COLL default  ompi mpich ompi_basic_linear ompi_binomial mvapich2 mvapich2_two_level_binomial mvapich2_two_level_direct impi)
      ADD_TESH(tesh-smpi-scatter-coll-${SCATTER_COLL} --cfg smpi/scatter:${SCATTER_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/scatter --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/scatter scatter_coll.tesh)
    ENDFOREACH()
    FOREACH (BARRIER_COLL default  ompi mpich ompi_basic_linear ompi_tree ompi_bruck ompi_recursivedoubling ompi_doublering mvapich2_pair mvapich2 impi)
      ADD_TESH(tesh-smpi-barrier-coll-${BARRIER_COLL} --cfg smpi/barrier:${BARRIER_COLL} --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/barrier --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/barrier barrier_coll.tesh)
    ENDFOREACH()
    # END TESH TESTS
    IF(enable_smpi_MPICH3_testsuite)
      ADD_TEST(test-smpi-mpich3-coll-thread      ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:thread -execarg=--cfg=smpi/privatize_global_variables:yes)
      SET_TESTS_PROPERTIES(test-smpi-mpich3-coll-thread    PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
      IF(CONTEXT_UCONTEXT)
        ADD_TEST(test-smpi-mpich3-coll-ompi-ucontext ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:ucontext -execarg=--cfg=smpi/coll_selector:ompi -execarg=--cfg=smpi/privatize_global_variables:yes -execarg=--cfg=smpi/bcast:binomial_tree)
        SET_TESTS_PROPERTIES(test-smpi-mpich3-coll-ompi-ucontext PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
      ENDIF()
      IF(HAVE_RAWCTX)
        ADD_TEST(test-smpi-mpich3-coll-mpich-raw   ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/coll_selector:mpich -execarg=--cfg=smpi/privatize_global_variables:yes)
      ADD_TEST(test-smpi-mpich3-coll-ompi-raw ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/coll_selector:ompi -execarg=--cfg=smpi/privatize_global_variables:yes -execarg=--cfg=smpi/bcast:binomial_tree)
      ADD_TEST(test-smpi-mpich3-coll-mpich-raw ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/coll_selector:mpich -execarg=--cfg=smpi/privatize_global_variables:yes)
      ADD_TEST(test-smpi-mpich3-coll-mvapich2-raw ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/coll_selector:mvapich2 -execarg=--cfg=smpi/privatize_global_variables:yes)
      ADD_TEST(test-smpi-mpich3-coll-impi-raw ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/coll perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/coll -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/coll_selector:impi -execarg=--cfg=smpi/privatize_global_variables:yes)
        SET_TESTS_PROPERTIES(test-smpi-mpich3-coll-mpich-raw test-smpi-mpich3-coll-ompi-raw test-smpi-mpich3-coll-mpich-raw test-smpi-mpich3-coll-mvapich2-raw test-smpi-mpich3-coll-impi-raw  PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
      ENDIF()
      IF(HAVE_RAWCTX)
        ADD_TEST(test-smpi-mpich3-attr-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/attr perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/attr -tests=testlist -execarg=--cfg=contexts/factory:raw)
        ADD_TEST(test-smpi-mpich3-comm-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/comm perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/comm -tests=testlist -execarg=--cfg=contexts/factory:raw)
        ADD_TEST(test-smpi-mpich3-init-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/init perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/init -tests=testlist -execarg=--cfg=contexts/factory:raw)
        ADD_TEST(test-smpi-mpich3-datatype-raw   ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/datatype perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/datatype -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/privatize_global_variables:yes)
        ADD_TEST(test-smpi-mpich3-group-raw      ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/group perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/group -tests=testlist -execarg=--cfg=contexts/factory:raw)
        ADD_TEST(test-smpi-mpich3-pt2pt-raw      ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/pt2pt perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/pt2pt -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/privatize_global_variables:yes)
        ADD_TEST(test-smpi-mpich3-topo-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/topo perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/topo -tests=testlist -execarg=--cfg=contexts/factory:raw)
        ADD_TEST(test-smpi-mpich3-rma-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/rma perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/rma -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/privatize_global_variables:yes)
        ADD_TEST(test-smpi-mpich3-info-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/info perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/info -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/privatize_global_variables:yes)
        ADD_TEST(test-smpi-mpich3-perf-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/perf perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/perf -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/running_power:-1)
        SET_TESTS_PROPERTIES(test-smpi-mpich3-attr-raw test-smpi-mpich3-comm-raw test-smpi-mpich3-init-raw test-smpi-mpich3-datatype-raw test-smpi-mpich3-group-raw test-smpi-mpich3-pt2pt-raw test-smpi-mpich3-topo-raw test-smpi-mpich3-rma-raw test-smpi-mpich3-info-raw PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
      ENDIF()
      IF(SMPI_FORTRAN)
        ADD_TEST(test-smpi-mpich3-thread-f77     ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/f77/ perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/f77/ -tests=testlist -execarg=--cfg=contexts/stack_size:8000 -execarg=--cfg=smpi/privatize_global_variables:yes)
        SET_TESTS_PROPERTIES(test-smpi-mpich3-thread-f77 PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
        ADD_TEST(test-smpi-mpich3-thread-f90     ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/f90/ perl ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/f90/ -tests=testlist -execarg=--cfg=smpi/privatize_global_variables:yes)
        SET_TESTS_PROPERTIES(test-smpi-mpich3-thread-f90 PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
      ENDIF()
    ENDIF()

    # BEGIN TESH TESTS
    ADD_TESH_FACTORIES(smpi-energy               "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/energy --cd ${CMAKE_BINARY_DIR}/examples/smpi/energy ${CMAKE_HOME_DIRECTORY}/examples/smpi/energy/energy.tesh)
    IF(SMPI_FORTRAN)
      ADD_TESH_FACTORIES(smpi-energy-f77         "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/energy --cd ${CMAKE_BINARY_DIR}/examples/smpi/energy ${CMAKE_HOME_DIRECTORY}/examples/smpi/energy/f77/energy.tesh)
      ADD_TESH_FACTORIES(smpi-energy-f90         "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/energy --cd ${CMAKE_BINARY_DIR}/examples/smpi/energy ${CMAKE_HOME_DIRECTORY}/examples/smpi/energy/f90/energy.tesh)
    ENDIF()
    ADD_TESH_FACTORIES(smpi-msg-masterslave      "thread;ucontext;raw" --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/smpi_msg_masterslave --cd ${CMAKE_BINARY_DIR}/examples/smpi/smpi_msg_masterslave ${CMAKE_HOME_DIRECTORY}/examples/smpi/smpi_msg_masterslave/msg_smpi.tesh)
    IF(NOT HAVE_MC)
      ADD_TESH(smpi-replay-multiple      --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/replay_multiple --cd ${CMAKE_BINARY_DIR}/examples/smpi/replay_multiple ${CMAKE_HOME_DIRECTORY}/examples/smpi/replay_multiple/replay_multiple.tesh)
    ENDIF()
    IF(HAVE_TRACING)
      ADD_TESH(smpi-tracing-ptp                  --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi --cd ${CMAKE_BINARY_DIR}/examples/smpi ${CMAKE_HOME_DIRECTORY}/examples/smpi/tracing/smpi_traced.tesh)
      ADD_TESH(smpi-replay                       --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi --cd ${CMAKE_BINARY_DIR}/examples/smpi ${CMAKE_HOME_DIRECTORY}/examples/smpi/replay/smpi_replay.tesh)
    ENDIF()
    IF(HAVE_MC)
      ADD_TESH(smpi-mc-non-determinism                  --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/mc --cd ${CMAKE_BINARY_DIR}/examples/smpi/mc ${CMAKE_HOME_DIRECTORY}/examples/smpi/mc/non_deterministic.tesh)
      ADD_TESH(smpi-mc-send-determinism                  --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/smpi/mc --cd ${CMAKE_BINARY_DIR}/examples/smpi/mc ${CMAKE_HOME_DIRECTORY}/examples/smpi/mc/send_deterministic.tesh)
    ENDIF()
    # END TESH TESTS
  ENDIF()


  ## BINGINGS ##
  ### LUA ###
  # BEGIN TESH TESTS
  IF(HAVE_LUA)
    ADD_TESH(lua-duplicated-globals              --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/state_cloner duplicated_globals.tesh)
    ADD_TESH(lua-masterslave                     --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/masterslave master_slave.tesh)
    ADD_TESH(lua-mult-matrix                     --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/multi_matrix mult_matrix.tesh)
    ADD_TESH(lua-masterslave-bypass              --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/console master_slave_bypass.tesh)
    ADD_TESH(lua-chord                           --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/chord chord.tesh)
    ADD_TESH(lua-bittorrent                      --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/bittorrent bittorrent.tesh)
    ADD_TESH(lua-kademlia                        --cd ${CMAKE_HOME_DIRECTORY}/examples/lua/kademlia kademlia.tesh)
    SET_TESTS_PROPERTIES(lua-duplicated-globals  PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
    SET_TESTS_PROPERTIES(lua-masterslave         PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
    SET_TESTS_PROPERTIES(lua-mult-matrix         PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
    SET_TESTS_PROPERTIES(lua-masterslave-bypass  PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
    SET_TESTS_PROPERTIES(lua-chord               PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
    SET_TESTS_PROPERTIES(lua-bittorrent          PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
    SET_TESTS_PROPERTIES(lua-kademlia            PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_BINARY_DIR}/examples/lua/?.so")
  ENDIF()
  # END TESH TESTS


  ### JAVA ###
  IF(enable_java)
    IF(WIN32)
      SET(TESH_CLASSPATH "${CMAKE_BINARY_DIR}/examples/java/\;${SIMGRID_FULL_JAR}")
      STRING(REPLACE "\;" "§" TESH_CLASSPATH "${TESH_CLASSPATH}")
    ELSE()
      SET(TESH_CLASSPATH "${CMAKE_BINARY_DIR}/examples/java/:${SIMGRID_FULL_JAR}")
    ENDIF()
    ADD_TESH(java-async                          --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/async/async.tesh)
    ADD_TESH(java-bittorrent                     --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/bittorrent/bittorrent.tesh)
    ADD_TESH(java-bypass                         --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/master_slave_bypass/bypass.tesh)
    ADD_TESH(java-chord                          --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/chord/chord.tesh)
    ADD_TESH(java-cloud                          --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/cloud/cloud.tesh)
    ADD_TESH(java-cloud-migration                --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/cloud/migration/migration.tesh)
    ADD_TESH(java-commTime                       --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/commTime/commtime.tesh)
    ADD_TESH(java-kademlia                       --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/kademlia/kademlia.tesh)
    ADD_TESH(java-kill                           --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/master_slave_kill/kill.tesh)
    ADD_TESH(java-masterslave                    --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/masterslave/masterslave.tesh)
    ADD_TESH(java-migration                      --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/migration/migration.tesh)
    ADD_TESH(java-mutualExclusion                --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/mutualExclusion/mutualexclusion.tesh)
    ADD_TESH(java-pingPong                       --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/pingPong/pingpong.tesh)
    ADD_TESH(java-priority                       --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/priority/priority.tesh)
    ADD_TESH(java-reservation-surf-plugin        --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/reservationSurfPlugin/reservation_surf_plugin.tesh)
    ADD_TESH(java-startKillTime                  --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/startKillTime/startKillTime.tesh)
    ADD_TESH(java-surf-cpu-model                 --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/surfCpuModel/surf_cpu_model.tesh)
    ADD_TESH(java-surf-plugin                    --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/surfPlugin/surf_plugin.tesh)
    ADD_TESH(java-suspend                        --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/suspend/suspend.tesh)
    IF(HAVE_TRACING)
      ADD_TESH(java-tracing                      --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/java --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/java ${CMAKE_HOME_DIRECTORY}/examples/java/tracing/tracingPingPong.tesh)
    ENDIF()
  ENDIF()


  ### SCALA ###
  IF(enable_scala)
    IF(WIN32)
      SET(TESH_CLASSPATH "${CMAKE_BINARY_DIR}/examples/scala/\;${SIMGRID_FULL_JAR}\;${SCALA_JARS}")
      STRING(REPLACE "\;" "§" TESH_CLASSPATH "${TESH_CLASSPATH}")
    ELSE()
      SET(TESH_CLASSPATH "${CMAKE_BINARY_DIR}/examples/scala/:${SIMGRID_FULL_JAR}:${SCALA_JARS}")
    ENDIF()
    ADD_TESH(scala-bypass                        --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/scala --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/scala ${CMAKE_HOME_DIRECTORY}/examples/scala/master_slave_bypass/bypass.tesh)
    ADD_TESH(scala-kill                          --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/scala --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/scala ${CMAKE_HOME_DIRECTORY}/examples/scala/master_slave_kill/kill.tesh)
    ADD_TESH(scala-masterslave                   --setenv srcdir=${CMAKE_HOME_DIRECTORY}/examples/scala --setenv classpath=${TESH_CLASSPATH} --cd ${CMAKE_BINARY_DIR}/examples/scala ${CMAKE_HOME_DIRECTORY}/examples/scala/masterslave/masterslave.tesh)
  ENDIF()

ENDIF()

if(enable_smpi AND enable_smpi_ISP_testsuite)
FOREACH (tesh
    any_src-can-deadlock10
    any_src-can-deadlock4
    any_src-can-deadlock5
    any_src-can-deadlock6
    any_src-waitall-deadlock2
    any_src-waitall-deadlock3
    any_src-waitany-deadlock2
    any_src-waitany-deadlock
    any_src-wait-deadlock
    basic-deadlock-comm_create
    basic-deadlock-comm_dup
    basic-deadlock-comm_split
    basic-deadlock
    bcast-deadlock
    collective-misorder-allreduce
    collective-misorder
    complex-deadlock
    deadlock-config
    finalize-deadlock
    irecv-deadlock
    no-error2
    no-error3-any_src
    no-error3
    no-error
    )
  ADD_TESH(umpire_${tesh}
    --setenv srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/isp/umpire
    --setenv bindir=${CMAKE_BINARY_DIR}/teshsuite/smpi/isp/umpire
    --cd ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/isp/umpire
    ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/isp/umpire/${tesh}.tesh)
ENDFOREACH()
endif()

  ## OTHER ##
ADD_TEST(testall                                 ${CMAKE_BINARY_DIR}/src/testall)

IF(enable_auto_install)
  ADD_TEST(simgrid_install                       make install)
ENDIF()

IF(enable_memcheck)
  INCLUDE(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/memcheck_tests.cmake)
ENDIF()
