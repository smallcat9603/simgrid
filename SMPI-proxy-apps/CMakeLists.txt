cmake_minimum_required(VERSION 2.8.8)

project(SMPI-proxy-apps)

option(build_mode "Either SMPI or ModelChecker")

INCLUDE(CTest)
ENABLE_TESTING()

set(CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE 30720000)
set(CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE 307200)

if (${build_mode} STREQUAL "SMPI")
  message(STATUS "Build mode: SMPI")

  # MeteoFrance benchmarks 
#  ADD_TEST(MeteoFrance_download  bin/MeteoFrance_getcode.sh)
#  ADD_TEST(MeteoFrance_GCR       bin/MeteoFrance_GCR.sh)
#  ADD_TEST(MeteoFrance_Halo      bin/MeteoFrance_Halo.sh)
#  ADD_TEST(MeteoFrance_transpose bin/MeteoFrance_transpose.sh)

  # ECP benchmarks 
  ADD_TEST(ECP_Tycho2  bin/ECP_Tycho2.sh)
  ADD_TEST(ECP_SNAP  bin/ECP_SNAP.sh)
  ADD_TEST(ECP_Comb  bin/ECP_Comb.sh)
  ADD_TEST(ECP_ProfugusMC  bin/ECP_ProfugusMC.sh)
  ADD_TEST(ECP_Chatterbug  bin/ECP_Chatterbug.sh)
  ADD_TEST(ECP_Picsar  bin/ECP_Picsar.sh)
  ADD_TEST(ECP_Nekbone  bin/ECP_Nekbone.sh)
  ADD_TEST(ECP_XSBench  bin/ECP_XSBench.sh)
  ADD_TEST(ECP_MiniGhost  bin/ECP_MiniGhost.sh)
  ADD_TEST(ECP_MiniGMG  bin/ECP_MiniGMG.sh)
  ADD_TEST(ECP_MiniMD  bin/ECP_MiniMD.sh)
  ADD_TEST(ECP_SNbone  bin/ECP_SNbone.sh)
  ADD_TEST(FFTW       bin/FFTW_PreExec.sh)
  ADD_TEST(ECP_VPFFT  bin/ECP_VPFFT.sh)
  ADD_TEST(ECP_TeaLeaf    bin/ECP_TeaLeaf.sh)
  ADD_TEST(ECP_miniAero    bin/ECP_miniAero.sh)
  ADD_TEST(ECP_kripke    bin/ECP_kripke.sh)
  ADD_TEST(ECP_CLAMR    bin/ECP_CLAMR.sh)
  ADD_TEST(ECP_miniTri    bin/ECP_miniTri.sh)
  ADD_TEST(ECP_ExaMiniMD    bin/ECP_ExaMiniMD.sh)
  ADD_TEST(ECP_CabanaMD    bin/ECP_CabanaMD.sh)
  ADD_TEST(ECP_CoMD      bin/ECP_CoMD.sh)
  ADD_TEST(ECP_AMG       bin/ECP_AMG.sh)
  ADD_TEST(ECP_sw4lite       bin/ECP_sw4lite.sh)
  ADD_TEST(ECP_quicksilver     bin/ECP_quicksilver.sh)
  ADD_TEST(ECP_SimpleMOC     bin/ECP_SimpleMOC.sh)
  ADD_TEST(ECP_PENNANT     bin/ECP_PENNANT.sh)
  ADD_TEST(ECP_SWFFT     bin/ECP_SWFFT.sh)
  ADD_TEST(ECP_EBMS     bin/ECP_EBMS.sh)
  ADD_TEST(ECP_CoSP2    bin/ECP_CoSP2.sh)
  ADD_TEST(ECP_MiniXyce   bin/ECP_MiniXyce.sh)
  ADD_TEST(ECP_HPGMG   bin/ECP_HPGMG.sh)
  ADD_TEST(ECP_MiniVite   bin/ECP_MiniVite.sh)
  ADD_TEST(ECP_PETSC   bin/ECP_PETSC.sh)
  ADD_TEST(ECP_Ember   bin/ECP_Ember.sh)


  # CodeVault benchmarks
  ADD_TEST(CodeVault_PreExec  bin/CodeVault_PreExec.sh)
  ADD_TEST(CodeVault_MCM  bin/CodeVault_MCM.sh)
  ADD_TEST(CodeVault_DynSparse      bin/CodeVault_DynSparse.sh)
  ADD_TEST(CodeVault_parallelio      bin/CodeVault_parallelio.sh)
  ADD_TEST(CodeVault_read2shmem      bin/CodeVault_read2shmem.sh)
  ADD_TEST(CodeVault_bhtree_mpi      bin/CodeVault_bhtree_mpi.sh)

  # Coral benchamrks 
  ADD_TEST(Coral_Lulesh    bin/Coral_Lulesh.sh)
  ADD_TEST(Coral_AMG2013    bin/Coral_AMG2013.sh)
  ADD_TEST(Coral_HACC_IO    bin/Coral_HACC_IO.sh)
  ADD_TEST(Coral_KMI_HASH    bin/Coral_KMI_HASH.sh)
  ADD_TEST(Coral_IOR    bin/Coral_IOR.sh)
  ADD_TEST(Coral_UMT2013    bin/Coral_UMT2013.sh)
  ADD_TEST(Coral_Qbox    bin/Coral_Qbox.sh)
  ADD_TEST(Coral_LSMS    bin/Coral_LSMS.sh)
  ADD_TEST(Coral_MCB    bin/Coral_MCB.sh)
  ADD_TEST(Coral_CLOMP    bin/Coral_CLOMP.sh)

  # Trinity-Nersc benchmarks
  ADD_TEST(Trinity_stream  bin/Trinity_stream.sh)
  ADD_TEST(Trinity_MDTest  bin/Trinity_MDTest.sh)
  ADD_TEST(Trinity_SMB_mpiHeader     bin/Trinity_SMB_mpiHeader.sh)
  ADD_TEST(Trinity_SMB_msgrate     bin/Trinity_SMB_msgrate.sh)
  ADD_TEST(Trinity_ZiaTest bin/Trinity_ZiaTest.sh)
  ADD_TEST(Trinity_MPIMemu bin/Trinity_MPIMemu.sh)
  ADD_TEST(Trinity_OMB_MPI_pt2pt bin/Trinity_OMB_MPI_pt2pt.sh)
  ADD_TEST(Trinity_OMB_MPI_one-sided bin/Trinity_OMB_MPI_one-sided.sh)
  ADD_TEST(Trinity_OMB_MPI_collective bin/Trinity_OMB_MPI_collective.sh)
  ADD_TEST(Trinity_GTC     bin/Trinity_GTC.sh)
  ADD_TEST(Trinity_MiniFE  bin/Trinity_MiniFE.sh)
  ADD_TEST(Trinity_MiniDFT  bin/Trinity_MiniDFT.sh)
  ADD_TEST(Trinity_MILC bin/Trinity_MILC.sh)
  ADD_TEST(Trinity_psnap bin/Trinity_psnap.sh)

  # Mantevo benchmarks 
  ADD_TEST(Mantevo_MiniSMAC2D bin/Mantevo_MiniSMAC2D.sh)
  ADD_TEST(Mantevo_MiniFE     bin/Mantevo_MiniFE.sh)
  ADD_TEST(Mantevo_HPCCG      bin/Mantevo_HPCCG.sh)
  ADD_TEST(Mantevo_MiniAMR    bin/Mantevo_MiniAMR.sh)
  ADD_TEST(Mantevo_CloverLeaf    bin/Mantevo_CloverLeaf.sh)
  ADD_TEST(Mantevo_CloverLeaf3D    bin/Mantevo_CloverLeaf3D.sh)

  # HPL benchmark
  ADD_TEST(HPL_vanilla bin/HPL_vanilla.sh)
  ADD_TEST(HPL_optimized bin/HPL_optimized.sh)

  # Others
  ADD_TEST(Others_mpi-test-suite bin/Others_mpi-test-suite.sh)
  ADD_TEST(Others_NPB bin/Others_NPB.sh)
elseif (${build_mode} STREQUAL "ModelChecker")
  message(STATUS "Build mode: ModelChecker")

else()
  message(FATAL_ERROR "Invalid build mode: ${build_mode}. Either SMPI or ModelCheck")
endif() # SMPI
