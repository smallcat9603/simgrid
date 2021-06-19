#!/usr/bin/ruby -Ku
#
# Complete the tracing of a bench with different tracing tools

require "digest"
require "date"
require_relative "common"
require "tmpdir"	

SIMGRIDPATH="../MpiEnv/simgrid/inst/bin/"


NPBPATH="../MpiEnv/bench/NPB3.3.1/NPB3.3-MPI/"
NPBSIMGRIDNOPRIVATEPATH="../MpiEnv/bench/NPB3.3.1/NPB3.3-SMPI/"
HIMENOPATH="../MpiEnv/bench/himeno/"
MMPATH="../MpiEnv/simgrid/Simgrid-git/examples/smpi/MM/"
GRAPH500PATH="../MpiEnv/bench/graph500/graph500/"


#If you add another bench, do not forget to add the path where lock is set (at least if there is exclusion problems with other benchmarks
BENCHLOCKNAME={ "bt"=>"nas",
  "cg"=>"nas",
  "dt"=>"nas",
  "ep"=>"nas",
  "ft"=>"nas",
  "is"=>"nas",
  "lu"=>"nas",
  "mg"=>"nas",
  "sp"=>"nas",
  "himeno"=>"himeno",
  "mm"=>"mm",
  "graph500"=>"graph500"}
LOCKSLEEPTIME=10
DEFAULTBENCHS=["bt","cg","dt","ep","ft","is","lu","mg","sp","himeno","graph500"]

#Tiny is not official istance size!
GRAPH500CLASSES={'tiny'=>[16,16],'toy'=>[26,16],'mini'=>[29,16],'small'=>[32,16],'medium'=>[36,16],'large'=>[39,16],'huge'=>[42,16]}


#****************************************
# Run any bench 
#
#Benches currently supported (and related options):
#
# NAS suite: < Any of these benches require to set variable opts['benchclass']> 
# *bt
# *cg
# *dt  < opts[:dtgraphname] to set the input graph >
# *ep
# *ft
# *is
# *lu
# *mg
# *sp
#
# Himeno's benchmark:
# *himeno < opts[:himenokmax] controls the kmax parameter, depending on cabinet size and  opts[:himenogridsize] the load of bench>
#         < the flop measure is just 1/simulatedTime > 
# 
# Simgrid 2.5 matrix multiplication example:
# *mm  <only supported with simgrid runtype> <opts[:mmblocksize] controls the size of blocks>
#         < the flop measure is just 1/simulatedTime > 
#
# Graph 500 benchmark: 
# *graph500 <only supported with simgrid runtype> 
#      <opts[:benchclass] is defined in {tiny,toy,mini,small,medium,large,huge}, see http://www.graph500.org/specifications>
#      <The algorithm used for BFS can be set by opts[:bfstype] in {simple,custom,replicated,replicated_csc}, simple is the default>
#      <Onesided operations and file saving are not supported> 
#         < the flop measure is expressed in Traversed Edges Per Seconds or TEPS > 
#
#Other options related to benches
# - extrabenchargument : Content is added as-it-is to the bench command
# - extrampiargument : Content is added as-it-is to the mpi framework command
# - replay : Defines whether to try to read ("read") an existing time-independent trace, or save ("save") a time-independent trace for eventual replay, or try to read or otherwise save the trace ("readsave"). If not set, none of these replay-replated actions are done.
# - benchclass: class for current bench, when it applies
# - clean:  (not to use when multiple processes run simultaneously)
# - privatize: (For Simgrid use, variables are normally privatized, and therefore benches are run directly. If this option is defined to no, scalabench is utilized instead)
# - simulationtimeout: (The duration allowed for the simulation before timeout. nil or zero disable timeout.) 
# - extrabencharguments: additional arguments for the bench program 
# - extrampiarguments: additional arguments for the mpi runtime program 
# - platformfile: The (ideally absolute) path to the Simgrid platform (.xml)
# - hostfile: The (ideally absolute) path to the hostfile (.txt)
# - cacheusage: Defines how to use cache: {use,nouse,only}. use by default. nouse forces the simulation to run, only skips simulation if no cache is available. 
# - cleanafter: Delete traces and so on after a correct execution of the bench 
#****************************************

DigestDiscardKeys=[:replay,:clean,:simulationtimeout,:cacheusage,:cleanafter,:maxprocesses,:repeats,:displayname,:resultfile,:verbose,:tracing]
def runBench(benchname,runtype,nprocs,configpath,opts)			

  
  if(opts[:clean])
    #Clear temporary folders, as they otherwise seem to cause issues
    FileUtils.rm_rf "/tmp/openmpi-sessions*"
    FileUtils.rm_rf "#{configpath}/../../#{NPBPATH}/tmp*"
    FileUtils.rm_rf "#{configpath}/../../#{NPBSIMGRIDNOPRIVATEPATH}/tmp*"
    FileUtils.rm_rf "#{configpath}/../../../MpiEnv/var/*"
    FileUtils.rm_rf "#{configpath}/results/*"
  end



  optsdigest=Digest::SHA1.hexdigest(opts
                                      .reject{|key,value| DigestDiscardKeys.include?(key)}
                                      .sort_by{|key, value| key.to_s}
                                      .to_s).to_s
  cachefile="#{configpath}/results/row#{optsdigest}.csv"
  if !opts.has_key?(:cacheusage)
    opts[:cacheusage]="use"
  end 
  if opts[:cacheusage]!="nouse" && File.exists?(cachefile)
    STDOUT.puts "Skipping simulation of #{benchname}_#{nprocs} by reading result from #{cachefile}.."
    results=File.read(cachefile).split(";\t");
    STDOUT.puts "Read #{results.inspect}"
    return 0,results
  end

  if opts[:cacheusage]=="only"
    STDOUT.puts "Skipping simulation of #{benchname}_#{nprocs} because this is cache-only mode" 
    return -50,nil
  end
  

  if !opts.has_key?(:privatize)
    opts[:privatize]="yes"
  end
  simutimeout=-1
  if opts.has_key?(:simulationtimeout)
    simutimeout=opts[:simulationtimeout]
  end
  if !opts.has_key?(:cleanafter)
    opts[:cleanafter]=true
  end  

  case runtype
  when "vampirtrace"
    #VampirTrace set-up
    #Do not stop tracing
    setenv("VT_MAX_FLUSHES","0")
    setenv("VT_BUFFER_SIZE","2000000000")
    setenv("VT_COMPRESSION_BSIZE","2000000000")
    setenv("VT_OTF_BUFFER_SIZE","2000000000")
    setenv("VT_PFORM_LDIR","./tmpZZ/")
  when "scorep"
    #ScoreP  setup
    setenv("SCOREP_TOTAL_MEMORY","40000000000")
    setenv("SCOREP_ENABLE_PROFILING","false");
    setenv("SCOREP_ENABLE_TRACING","true");
    setenv("SCOREP_TRACING_USE_SION","false")
    setenv("SCOREP_TRACING_NLOCATIONS_PER_SION_FILE","1024")
    setenv("SCOREP_TRACING_COMPRESS","false")
    setenv("SCOREP_MPI_MAX_COMMUNICATORS","5000000")
    setenv("SCOREP_MPI_MAX_WINDOWS","5000000")
    setenv("SCOREP_MPI_MAX_ACCESS_EPOCHS","5000000")
    setenv("SCOREP_MPI_MAX_GROUPS","5000000")
    setenv("SCOREP_MPI_ENABLE_GROUPS","coll,rma,p2p")
  when "simgrid"
    
  when "scalatrace"
    
  when "scalabench"
    
  when "none"	

  else
    
    STDERR.puts "I do not know the trace type #{runtype}, aborting!"
    return -1,nil	
  end


  #make the bench
  logpath="#{configpath}/logs"
  benchpath=""
  execname=""
  execarguments="";
  if !opts.has_key?(:extrabencharguments)
    opts[:extrabencharguments]=""
  end
  if !opts.has_key?(:extrampiarguments)
    opts[:extrampiarguments]=""
  end
  retval=nil
  maketime=nil

  #Wait for the make lock...
  getlock(benchname,configpath)

  #Files needed to coppy to the temporary folder (execname will be copied anyway, but other required files -e.g. libraries- need be added) 
  filesToCopy=Array.new 
  begin
    case benchname
    when "mm"
      if runtype!="simgrid"
        STDERR.puts "mm bench is only supported by simgrid, aborting!"
        return -3,nil
      end
      benchpath="#{configpath}/../../#{MMPATH}"
      Dir.chdir(benchpath)
      #Build the testbench                                                                       
      execname="MM_mpi"
      
      #*****************************************************
      #MM has several requirements for fully utilizing nodes
      #1) group < NBGroup
      #2) nprocs= rowSize * colSize * NBGroup
      #3) k % BlockSize = 0
      #4) k % (sizeRow*BlockSize) = 0
      #5) k % (sizeCol*BlockSize) = 0
      #6) k >= sizeRow*BlockSize
      #7) k >= sizeCol*BlockSize
      #
      #Argument       Variable    Default
      #   -M I    --> m           [1024]  (x dimension)
      #   -N I    --> n           [1024]  (y dimension)
      #   -K I    --> k           [1024]
      #   -B I    --> BlockSize   [64]
      #   -G I    --> NBGroups    [1]
      #   -g I    --> group       [0]
      #   -r I    --> sizeRow     [2]
      #   -c I    --> sizeCol     [nprocs/2]
      #
      #As long as nprocs is a power of two, only (7) requires effort.
      #Simplifying it gives 2*k=nprocs*BlockSize
      #In order to keep BlockSize  stable, we get k=32*nprocs 
      #*****************************************************

      if !opts.has_key?(:mmblocksize)
        opts[:mmblocksize]=64
      end
      
      kparam=nprocs.to_i*opts[:mmblocksize]/2
      execarguments=" -K #{kparam} -B #{opts[:mmblocksize]}"
      maketime,success=runCmdWithTime("make","make_MM",configpath,simutimeout)
    when "himeno"
      benchpath="#{configpath}/../../#{HIMENOPATH}"
      Dir.chdir(benchpath)
      writeBenchConfig("himeno",runtype,configpath,"../../","#{benchpath}/config/make.def")
      #Parametrize the compilation
      if !opts.has_key?(:himenogridsize)
        opts[:himenogridsize]="XS"
        STDERR.puts "No himeno grid size (opts[:himenogridsize])  was given, choosing XS.."
      end
      kmax=opts[:switchperrack].to_i
      if opts.has_key?(:himenokmax)
        kmax=opts[:himenokmax].to_i
      end
      imax=Math.sqrt(nprocs.to_i/kmax).to_i                      
      if(imax*imax*kmax!=nprocs.to_i)
        STDERR.puts "himeno bench requires that grid size be configured differently. Currently, opts[:himenokmax] parameter gives #{imax}x#{imax}x#{kmax}!=#{nprocs}, which is illegal. Please set opts[:himenokmax] so that nprocs/opts[:himenokmax] is a square integer!"
        return -5,nil
      end
      #Build the testbench                                                                                 
      execname="#{benchname.downcase}"
      maketime,success=runCmdWithTime("./paramset.sh #{opts[:himenogridsize]} #{imax} #{imax} #{kmax};make clean;make #{benchname}","make#{execname}#{nprocs}",configpath,simutimeout)
      benchpath+="/bin/"				
    when "graph500"
      if !opts.has_key?(:bfstype)
        opts[:bfstype]="simple"
      end
      if !GRAPH500CLASSES[ opts[:benchclass] ]
        STDERR.puts "graph500 bench does not have a class <#{opts[:benchclass]} >, aborting!"
        return -7,nil
      end
      execarguments=" #{GRAPH500CLASSES[opts[:benchclass]].join(" ")} "
      benchpath="#{configpath}/../../#{GRAPH500PATH}"
      Dir.chdir(benchpath)
      writeBenchConfig(benchname.downcase,runtype,configpath,"../../","#{benchpath}/mpi/make.def")
      #Build the testbench                                                                                 
      execname="graph500_mpi_#{opts[:bfstype]}"
      maketime,success=runCmdWithTime("make","make#{execname}#{nprocs}",configpath,simutimeout)
      benchpath+="/mpi/"
    else
      #assume we have a NAS benchmark		

      if runtype=="simgrid" && !opts[:privatize] 
        benchpath="#{configpath}/../../#{NPBSIMGRIDNOPRIVATEPATH}"
      else
        benchpath="#{configpath}/../../#{NPBPATH}"
      end
      Dir.chdir(benchpath)
      writeBenchConfig(benchname.downcase,runtype,configpath,"../../../../","#{benchpath}/config/make.def")
      #Build the testbench 
      if benchname.downcase == "dt"
        execname="#{benchname.downcase}.#{opts[:benchclass]}.x"
        if opts.has_key?(:dtgraphname)
          execarguments=opts[:dtgraphname]
        else
          execarguments="WH"
        end
      else
        execname="#{benchname.downcase}.#{opts[:benchclass]}.#{nprocs}"
      end
      maketime,success=runCmdWithTime("make #{benchname} CLASS=#{opts[:benchclass]} NPROCS=#{nprocs}","make#{execname}",configpath,simutimeout)
      benchpath+="/bin/"
    end

    if(!success)
      STDERR.puts "Making #{execname} failed, aborting!"
      return -8,nil
    end
    
    filesToCopy.push("#{benchpath}/#{execname}")

  rescue 
    #Something went wrong during make, abort
   return -20,nil  
 ensure
    #And do not forget to erase this damn lock file!
    freelock(benchname,configpath)
 end

  



  #Run the testbench in a temporary folder, assume that trace files will be written in this folder
  tracepath=Dir.mktmpdir("mpienv_"+execname);
  Dir.chdir(benchpath)
  FileUtils.cp_r(filesToCopy,tracepath)
  Dir.chdir(tracepath)



  case runtype
  when "vampirtrace"
    if(!Dir.exists?("tmpZZ"))
      Dir.mkdir("tmpZZ")
    end
  when "scorep"
  when "simgrid"
  when "scalatrace"
  when "scalabench"
  when "none"	
  else
    STDERR.puts "I do not know the trace type #{runtype}, aborting!"
    return -11,nil	
  end

  STDERR.puts "#{DateTime.now}: Starting #{execname} #{execarguments}"
  runtime=nil
  #depending on the replay parameter, save or read an existing replay file if possible
  replaykey="#{execname}_#{(opts[:extrabencharguments]+execarguments).strip.gsub(/\s*/,"-")}_#{nprocs}"
  replayfile="#{configpath}/../../../MpiEnv/var/traces/#{replaykey}.txt"
  if opts[:replay]=="readScalatrace"
    replayfile="#{configpath}/../../../MpiEnv/var/traces/#{replaykey}.scalatrace"
  end

   if opts[:replay]=="dryrun" 
    return 0,[replaykey,tracepath]
  end

 
  if runtype=="simgrid" 
    if ( opts[:replay]=="readScalatrace" || opts[:replay]=="read"  || opts[:replay]=="readsave" ) && File.exists?(replayfile)
      STDOUT.puts "Replaying run#{execname}_#{nprocs} from <#{replayfile}>.."
      replayprogram=nil
      replaympioptions=""
      if opts[:replay]=="readScalatrace" 
        #Anyway replay program asks for a relative path (by prepending ./), so add something to make our absolute path OK
        replayfile="../../#{replayfile}"
        replayprogram="#{configpath}/../../trace/ScalaTrace-TI/replay/replay"
      else
        replayprogram="#{configpath}/../../#{SIMGRIDPATH}/../../Simgrid-git/examples/smpi/smpi_replay"
        replaympioptions=" -ext smpi_replay "
        FileUtils.symlink("#{replayfile}_files","#{tracepath}/#{replaykey}.txt_files")
        FileUtils.cp("#{replayfile}",tracepath)
      end

      #Make sure that the traces are not currently being copied
      getlock(benchname,configpath);freelock(benchname,configpath)
      runtime,success=runCmdWithTime("#{configpath}/../../../MpiEnv/#{SIMGRIDPATH}/smpirun #{replaympioptions}  -np #{nprocs} #{opts[:extrampiarguments]} --cfg=smpi/privatize_global_variables:yes  -platform #{configpath}/#{opts[:platformfile]} -hostfile #{configpath}/#{opts[:hostfile]} #{replayprogram}  #{replayfile}", "run#{execname}_#{nprocs}_#{optsdigest}",configpath,simutimeout)			
    else
      #Request to save time-independent trace file for replay
      if ( opts[:replay]=="save"  || opts[:replay]=="readsave" )
        opts[:extrampiarguments]+=" -trace-ti -trace-file #{replaykey}.txt "
      end
      runtime,success=runCmdWithTime("#{configpath}/../../../MpiEnv/#{SIMGRIDPATH}/smpirun -np #{nprocs} #{opts[:extrampiarguments]} --cfg=smpi/privatize_global_variables:#{opts[:privatize]} -platform #{configpath}/#{opts[:platformfile]} -hostfile #{configpath}/#{opts[:hostfile]}  #{tracepath}/#{execname} #{opts[:extrabencharguments]} #{execarguments}", "run#{execname}_#{nprocs}_#{optsdigest}",configpath,simutimeout)			
      #Copy replay files
      if success && ( opts[:replay]=="save"  || opts[:replay]=="readsave" ) && 
        getlock(benchname,configpath)
        begin
          if !File.exists?("#{configpath}/../../../MpiEnv/var/traces/#{replaykey}.txt")
            STDOUT.puts "Copying replay trace #{tracepath}/#{replaykey}.txt*  --> #{configpath}/../../../MpiEnv/var/traces/"
            FileUtils.cp_r  Dir.glob("/#{tracepath}/#{replaykey}.txt*"),"#{configpath}/../../../MpiEnv/var/traces/"
          end
        ensure
          freelock(benchname,configpath)
        end
      end
    end
  else	
    runtime,success=runCmdWithTime("#{configpath}/../../../MpiEnv/ompi/bin/mpirun --mca opal_set_max_sys_limits 1 -np #{nprocs} #{opts[:extrampiarguments]}  #{tracepath}/#{execname}  #{opts[:extrabencharguments]} #{execarguments}","run#{execname}_#{nprocs}_#{optsdigest}",configpath,simutimeout)
  end

  if(!success)
    STDERR.puts "Running #{execname} failed, aborting!"
    return -3,nil
  end

  #Scan for simulated time and flops
  simulatedtime=nil	
  simulatedprocessflop=nil	
  simulatedtotalflop=nil	
  logfile="#{configpath}/logs/run#{execname}_#{nprocs}_#{optsdigest}.log"
  case benchname
  when "mm"
    File.readlines(logfile).each{ |line|
      res=line.scan(/Simulated time:\s+([0-9eE\-\.]+)\s+seconds\./)
      if(!res.empty?)
        simulatedtime=res[0][0].to_f
        simulatedtotalflop=1/res[0][0].to_f
        simulatedprocessflop=simulatedtotalflop/nprocs.to_f
        next
      end
    }#End of one file line
  when "himeno"
    File.readlines(logfile).each{ |line|
      res=line.scan(/Simulated time:\s+([0-9eE\-\.]+)\s+seconds\./)
      if(!res.empty?)
        simulatedtime=res[0][0].to_f
        simulatedtotalflop=1/res[0][0].to_f
        simulatedprocessflop=simulatedtotalflop/nprocs.to_f
        next
      end
    }#End of one file line
  when "graph500"
    File.readlines(logfile).each{ |line|
      res=line.scan(/mean_time:\s+([0-9eE\-\.]+)\s*/)
      if(!res.empty?)
        simulatedtime=res[0][0].to_f
        next
      end
      #Note that graph500 gives Traversed Edges Per Seconds (TEPS) as a measure of performance
      res=line.scan(/harmonic_mean_TEPS:\s+([0-9eE\-\.]+)\s*/)
      if(!res.empty?)
        simulatedtotalflop=res[0][0].to_f
        simulatedprocessflop=simulatedtotalflop/nprocs.to_f
        next
      end
    }#End of one file line
  else
    #Assume a NAS benchmark
    File.readlines(logfile).each{ |line|
      res=line.scan(/Simulated time:\s+([0-9eE\-\.]+)\s+seconds\./)
      if res.empty?
        res=line.scan(/Time\s+in\s+seconds\s+=\s+([0-9eE\-\.]+)\s+/)
      end
      if(!res.empty?)
        simulatedtime=res[0][0].to_f
        if simulatedtotalflop == nil
          simulatedtotalflop=1.0/simulatedtime
          simulatedprocessflop=simulatedtotalflop/nprocs.to_f
        end
        next
      end

      res=line.scan(/Mop\/s total     =\s+([0-9eE\-\.]+)\s+/)
      if(!res.empty?)
        simulatedtotalflop=res[0][0].to_f
        next
      end
      res=line.scan(/Mop\/s\/process   =\s+([0-9eE\-\.]+)\s+/)
      if(!res.empty?)
        simulatedprocessflop=res[0][0].to_f
        next
      end
    }#End of one file line
  end





  if(!simulatedtime)
    return -4,nil
  end

  STDOUT.puts "Simulated time is #{simulatedtime} and aggregated flop is #{simulatedtotalflop}"



  posttime=nil
  case runtype
  when "vampirtrace"
    posttime,success=runCmdWithTime("vtunify #{execname}","vtunify#{execname}",configpath,simutimeout)  
  when "scorep"
    #Compress traces with gzip, for fair comparison with scalatrace
    posttime,success=runCmdWithTime("tar -czf sco.tar.gz score*","postscore#{execname}",configpath,simutimeout)
  else
    success=true
    posttime=0
  end
  if(!success)
    STDERR.puts "Running post process failed, aborting!"
    return -4,nil
  end
  
  
  tracesize=getFolderSize(tracepath)

  Dir.chdir("..")


  if opts[:cleanafter]
    FileUtils.rm_rf tracepath
  end

  results=[simulatedtime, simulatedtotalflop,simulatedprocessflop,runtype,execname,maketime,runtime,posttime,tracesize,replaykey,tracepath]

  #Save results to a cache file
  if !File.exists?(cachefile)
    File.open(cachefile, 'w')  { |file| file.write(results.join(";\t")); }
  end

  return 0,results

end


#****************************************
# Wait and take the bench lock
#
#****************************************
$benchlocks=Hash.new
$benchlocksbuildmutex=Mutex.new

def getlock(benchname,configpath)
  waitcount=0
  lockbase=BENCHLOCKNAME[benchname]
  if(!lockbase)
    lockbase=benchname;
  end
 
  $benchlocksbuildmutex.synchronize{
    if !$benchlocks.has_key?(lockbase)
      $benchlocks[lockbase]=Mutex.new
    end
  }
  
  STDOUT.puts "Try to get lock from config:#{configpath}, benchname:#{benchname}.." 
  while !$benchlocks[lockbase].try_lock()
    if waitcount==0
      STDOUT.puts "Waiting for make lock  of #{benchname} at #{DateTime.now}.."
    end
    sleep LOCKSLEEPTIME;
    waitcount=waitcount+1
    if waitcount>600/LOCKSLEEPTIME
      STDERR.puts "Benchmark #{benchname} is waiting for lock for now #{LOCKSLEEPTIME*waitcount}. Maybe someone did not clear the locks??"
    end
    if waitcount>1200/LOCKSLEEPTIME
      return false  
    end
  end

  STDOUT.puts "Got make lock  of #{benchname} at #{DateTime.now}.."

#  lockfile="#{configpath}/../../../MpiEnv/var/locks/#{lockbase}.makelock"
#  STDOUT.puts "Try to get lock from config:#{configpath}, benchname:#{benchname} => #{lockfile}.." 
#  while File.exists?(lockfile)
#    if waitcount==0
#      STDOUT.puts "Waiting for make lock  of #{benchname} at #{DateTime.now}.."
#    end
#    sleep LOCKSLEEPTIME;
#    waitcount=waitcount+1
#    if waitcount>600/LOCKSLEEPTIME
#      STDERR.puts "Benchmark #{benchname} is waiting for lock for now #{LOCKSLEEPTIME*waitcount}. Maybe someone did not clear the locks??"
#    end
#    if waitcount>1200/LOCKSLEEPTIME
#      return false  
#    end
#  end
#
#  #Take the lock (and give some information for debug)
#  File.open(lockfile, 'w')  { |file| 
#    file.write("Starting make of #{benchname} at #{DateTime.now} \n")
#    file.write("PID:#{Process.pid}\n")
#    file.write("PPID:#{Process.ppid}\n")
#  }

  return true
end


#****************************************
# Free the bench lock
#
#****************************************
def freelock(benchname,configpath)
  lockbase=BENCHLOCKNAME[benchname]
  if(!lockbase)
    lockbase=benchname;
  end

  $benchlocks[lockbase].unlock()

#  lockfile="#{configpath}/../../../MpiEnv/var/locks/#{lockbase}.makelock"
#  FileUtils.rm_f lockfile
end

#****************************************
# Write the bench configuration file
#
#****************************************
def writeBenchConfig(benchname,runtype,configpath,prefix,target)

  in1file="#{configpath}/../common/make_#{runtype}.in1"
  in2file="#{configpath}/../common/make_#{BENCHLOCKNAME[benchname]}.in2"

  if !File.exists?(in1file) ||  !File.exists?(in2file) 
    STDERR.puts "Could not find bench make configuration files <#{in1file}>  <#{in2file}>, I quit ! "
    exit
  end

  FileUtils.rm_f target
  file=File.open(target,"w")
  begin
    file.puts("MPIENVPREFIX=${PWD}/#{prefix}")
    file.write(File.read(in1file));
    file.write(File.read(in2file));
  ensure
    file.close
  end

  STDOUT.puts "Wrote configuration file <#{target}>  with  <#{configpath}> and  <#{prefix}>"

end

#****************************************
# Run a bench produced by Scalabench
#
#****************************************
def runScalabench(sign,nprocs,tracepath,configpath,opts)			

  simutimeout=0
  if opts.has_key?([:simulationtimeout])
    simutimeout=opts[:simulationtimeout]
  end

  smpipath="#{configpath}/../../../MpiEnv/simgrid/inst/bin"
  benchpath="#{tracepath}_2";			
  makefileContent= <<-EOM
SIMGRIDPATH=#{configpath}/../../../MpiEnv/simgrid/inst
OMPIPATH=#{configpath}/../../../MpiEnv/ompi
PLATFORMPATH=#{configpath}
#PLATFORMPATH=./

all: main.c main.h util.c util.h
\t${SIMGRIDPATH}/bin//smpicc -g -lsimgrid -lgfortran -DUSE_SMPI -I${SIMGRIDPATH}/include/ -L${SIMGRIDPATH}/lib   ./main.c ./util.c -o bench

clean:
\trm -r bench benchompi tmp* log_*.txt .gdbinit

run: all
\t${SIMGRIDPATH}/bin/smpirun -np #{nprocs} -platform ${PLATFORMPATH}/#{opts[:platformfile]} -hostfile ${PLATFORMPATH}/#{opts[:hostfile]}  ./bench

gdb: all
\t${SIMGRIDPATH}/../smpigdb -np #{nprocs} -platform ${PLATFORMPATH}/#{opts[:platformfile]} -hostfile ${PLATFORMPATH}/#{opts[:hostfile]}   ./bench

allompi: main.c main.h util.c util.h
\t${OMPIPATH}/bin/mpicc -g -lgfortran -lmpi -I${OMPIPATH}/include/ -L${OMPIPATH}/lib   ./main.c ./util.c -o benchompi

runompi: allompi
\t${OMPIPATH}/bin/mpirun -np #{nprocs} --mca opal_set_max_sys_limits 1  ./benchompi
EOM

  if(!Dir.exists?(benchpath))
    Dir.mkdir(benchpath)
  end
  Dir.chdir(benchpath)

  File.write("Makefile",makefileContent);

  if(opts[:clean])
    runCmd("make",sign,configpath);
  end		


  #Generate the bench sources from traces  
  if( !runCmd("#{configpath}/../../../MpiEnv/trace/ScalaTraceV2/c_genVNII/generator #{tracepath}/trace_#{nprocs}/0 ","genbench_#{sign}",configpath))
    return -1,nil;
  end

  #Compile the bench binaries
  if( !runCmd("make","make#{sign}",configpath))
    return -2,nil
  end

  #Run the bench program
  #Compile the bench binaries
  if( !runCmdWithTime("make run","run#{sign}",configpath,simutimeout))
    return -3,nil
  end

  
  logfile="#{configpath}/logs/run#{sign}.log"
  timing=nil	
  File.readlines(logfile).each{ |line|
    res=line.scan(/\[smpi_kernel\/INFO\] Simulation time:\s+([0-9eE\-\.]+)\s+seconds/)
    if(!res.empty?)
      timing=res[0][0].to_f
    end
  }

  if(!timing)
    return -4,nil
  end

  STDOUT.puts "Timing is #{timing}"
  return 0,[timing]
end


#****************************************
# Generate topology files based on Ikki's tools
# Options
# -switchperrack number of switches/nodes/routers per rack
# -layoutedges the base name (no extenstion,no path) to alternative layout edge list inthe configpath's topology folder 
#****************************************
def generateTopology(nswitch,topology, configPath, opts)
  ikkipath="#{configPath}/../../../TopologyEnv/ikki"
  laycommand ="#{ikkipath}/lay3.rb"
  if(! File.exists?("#{ikkipath}/sa_qap"))
    Dir.chdir(ikkipath)
    runCmd("make","compile_saqap",configPath)
  end

  #ikkipath="#{configPath}/../../../TopologyEnv/extensible"
  #laycommand ="#{ikkipath}/lay2fabien.rb"
  #if(! File.exists?("#{ikkipath}/sa_qxap"))
  #	Dir.chdir(ikkipath)
  #	runCmd("make","compile_saqxap",configPath)
  #end

  
  Dir.chdir(configPath)		
  if( !Dir.exists?("topologies"))
    Dir.mkdir("topologies")
  end				
  Dir.chdir("topologies");


  switchperrack=1
  if opts.has_key?(:switchperrack)
    switchperrack=opts[:switchperrack]
  end

  clustersuffix="clu"	
  if opts.has_key?(:layoutedges)
    FileUtils.cp(opts[:layoutedges],".")
    platformbase=File.basename(opts[:layoutedges],".edges")
    clustersuffix="ward-#{switchperrack}"
  else		
    platformbase="#{nswitch}r#{switchperrack}+#{topology}"
    if opts.has_key?(:seed)
      platformbase+="__#{opts[:seed]}"
    end
  end		
  
  
  
  #Generate topology file
  topologyedgefile="#{configPath}/topologies/#{platformbase}.edges"						
  topologyclufile="#{configPath}/topologies/#{platformbase}.#{clustersuffix}.edges"		
  

  if( !File.exists?(topologyclufile))	
    if !opts.has_key?(:layoutedges)
      #Build layout of topology
      laycommand+=" -c  #{nswitch}r#{switchperrack} #{topology}" 
      laycommand+="--seed #{opts[:seed]}" if  opts[:seed]
      
      if(!runCmd( laycommand, "lay#{topology}__#{nswitch}",configPath))
        return "";
      end
    end

	
    if( opts.has_key?(:layoutedges) || opts.has_key?(:clustering) )
      
      if(!runCmd( "R --slave -f #{ikkipath}/clust.R --args #{topologyedgefile} ward #{switchperrack} png","cluster#{topology}_#{nswitch}",configPath))
        return "";
      end
    end
    
    
    allocationcommand="#{ikkipath}/alloc4.rb --edge #{topologyedgefile} #{topologyclufile}  ca sa","alloc#{topology}__#{nswitch}"
    
    #Allocate clusters generated by layout
    #allocationcommand="#{ikkipath}/alloc4.rb  #{topologyedgefile} "
    #if !opts[:layoutedges]
    #	allocationcommand+=" --layoutclustering "	
    #end
    #allocationcommand+=" --seed #{opts[:seed]} #{CostCoeff} #{EnergyCoeff} #{RedundancyCoeff} #{AllocationLimit} clean"
    

    if(!runCmdBasic("#{ikkipath}/alloc4.rb --edge #{topologyedgefile} #{topologyclufile}  ca sa","alloc#{topology}__#{nswitch}",configPath))
      return "";
    end	
    return ""			
  end			
  
  Dir.chdir("../");				
  return platformbase
end


#****************************************
# Read a bench configuration from a (file) line 
#
#****************************************
def readBenchConfig(line)

  tokens=line.chomp!.split(" ")
  benchname=""
  if(tokens.empty? || tokens[0]=="#")
    return "","",nil
  end

  benchname=tokens[0];
  nprocs=tokens[1].to_i;
  opts=Hash.new
  #STDOUT.puts "Read tokens #{tokens.inspect}" 
  if(tokens.size>=3)
    tokens[2..-1].each{ |pair|
      key,value=pair.split('=')
      #STDOUT.puts "Read #{key}=#{value}"  
      opts[key]=value;			
    }
  end
  STDOUT.puts "Read #{benchname},#{nprocs},#{opts.inspect} from <#{line}>"
  return benchname,nprocs,opts
end

