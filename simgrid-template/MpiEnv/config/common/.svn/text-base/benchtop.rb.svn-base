#!/usr/bin/ruby -Ku
#

require_relative "../common/benchutils"
require 'thread'

RunningThreads=Array.new
ThreadsPriorityIncrement=5


#****************************************
#Run a list of  simulations, with provided results
#*commonOpts is an hash with paramters common to all simulations
#*tableOpts is an array of hash tables, containing parameters that change depending on simulation
#*configPath is the path to configuration 
#On top of hash key used by  runBenches function, there are a few parameters used in this function:
# - maxprocesses: Number of concurrent simulations to run
# - benchname: Name of the bench, as used by runBench
# - runtype: Program to use to run/trace the program
# - nswitches: number of simulated switches
# - repeats: Number of times each simulation is run
# - displayname: name value for one simulation inthe result file
# - resultfile: the basename of the result file (not including the path and .csv)
# - verbose: Return verbose results 
#When a key is present in both commonOpts and opts, value in opts is prefered
#****************************************

#If true, results in the csv file contain all opts content etc
$verbose=false

def runExperiment(commonOpts, tableOfOpts, configpath)

  tableOfResults=Array.new
  thispath=Dir.pwd
  
  commonOpts[:configpath]=configpath

  FileUtils.chdir(configpath)

  if commonOpts.has_key?(:verbose)
    $verbose=commonOpts[:verbose]
  end

  #Change priority of simulations not to crash the computer
  `/usr/bin/renice +#{ThreadsPriorityIncrement} -p #{Process.pid}`

  #If we need to create some replays, switch a bit the order, to avoid generating many times the replay file
  if commonOpts[:replay]=="save" || commonOpts[:replay]=="readsave" ||  commonOpts[:replay]=="readScalatrace" 
    keys=Hash.new;
    tmp=Array.new;tmp2=Array.new;tmpOthers=Array.new;
    #Split the table content in 3 parts
    tableOfOpts.each{|v|
      thiskey=v[:benchname]
      if v.has_key?(:extrampiarguments)
        thiskey+=v[:extrampiarguments].join(".");
      end
      if v.has_key?(:extrabencharguments)
        thiskey+=v[:extrabencharguments].join(".");
      end
      thiskey+=v[:nswitches].to_s
      if !keys.has_key?(thiskey)
        tmp.push(v)
        keys[thiskey]=1
      else
        if(keys[thiskey]==1)
          tmp2.push(v)
        else
          tmpOthers.push(v)
        end
        keys[thiskey]=keys[thiskey].next
      end 
    }
    
    if commonOpts[:replay]=="readScalatrace"
      tmp.each{ |opts|
        myOpts=commonOpts.merge(opts)
        myOpts[:replay]="dryrun"
        nproc=myOpts[:nswitches].to_i * myOpts[:nodeperswitch].to_i * myOpts[:corepernode].to_i
        retval,results=runBench(myOpts[:benchname],"scalatrace",nproc,myOpts[:configpath],myOpts)        
        replaykey=results[-2]
        replayfile="#{configpath}/../../../MpiEnv/var/traces/#{replaykey}.scalatrace"
        if ! File.exists?(replayfile)
          STDOUT.puts "Generate Scalatrace traces for #{replaykey}..."
          myOpts[:replay]="none"
          retval,results=runBench(myOpts[:benchname],"scalatrace",nproc,myOpts[:configpath],myOpts)
          if retval==0
            FileUtils.cp "#{results[-1]}/trace_#{nproc}/0", replayfile
            STDOUT.puts "Scalatrace #{replaykey}: OK!"
          else
            STDOUT.puts "Scalatrace #{replaykey} returned #{retval},#{results}: KO!"
          end
        end
      }
    end
    tableOpts=tmp+tmp2+tmpOthers
   # STDOUT.puts "Found these keys:\n#{keys.inspect}\n"
   # STDOUT.puts "Which gives this table:\n#{tableOfOpts.inspect}\n"
  end

  runningCount=0;
  
  mutex=Mutex.new
  resultMutex=Mutex.new
 
  tableOfOpts.each_with_index{ |specOpts,simindex|

    #Merge common and specific options
    #Hash are shared amongst threads, so create a local copy for each one
    #We should store clones of all used variables, in order to avoid 'corruption' by the main thread
    tmpOpts=commonOpts.merge(specOpts){ |key,commonValue,specificValue| specificValue} #commonValue+specificValue

    [1..tmpOpts[:repeats]].each{ |repeat| 
      opts=tmpOpts.clone

      #Try to make educated guesses..
      if !opts.has_key?(:himenokmax)
        opts[:himenokmax]=opts[:switchperrack]	
      end
      
      if !opts.has_key?(:repeats)
        !opts[:repeats]=1
      end
      
      if !opts.has_key?(:displayname)
        opts[:displayname]="sim_#{simindex}"
      end

      if !opts.has_key?(:runtype)
        opts[:runtype]="simgrid"
      end

      if !opts.has_key?(:resultfile)
        opts[:resultfile]="result_#{DateTime.now}"
      end

      if !opts.has_key?(:maxprocesses)
        opts[:maxprocesses]=1
      end
      nproc=opts[:nswitches].to_i * opts[:nodeperswitch].to_i * opts[:corepernode].to_i


      sign="#{opts[:displayname]}_#{opts[:benchname]}_#{repeat}"


      opts[:resultpath]="#{configpath}/#{opts[:resultfile]}.csv";

      #Block the main thread when we have enough processes
      mutex.synchronize {runningCount=runningCount.next;}

      newT=Thread.new{
		
        
        
        optsContent="";opts.each_pair{|key,value| optsContent+="#{key}:#{value};\t"}
        returnValue=[]
        begin          
          retval,results=runBench(opts[:benchname],opts[:runtype],nproc,opts[:configpath],opts)
          #sleep(rand()*6);STDOUT.puts "Ran #{sign}!"
          if(retval==0)
            if(results.length<2)
              results[1]=0;
            end
            if(results.length<3)
              results[2]=0;
            end	
            opts[:runtime]=results[0]
            opts[:mops]=results[1]
            opts[:mopsproc]=results[2]
            resultMutex.synchronize { tableOfResults.push opts}
            returnValue=[opts[:resultpath],opts[:displayname],opts[:benchname],results[0],results[1],results[2],optsContent ]
          else
            returnValue=[opts[:resultpath],opts[:displayname],opts[:benchname],"Returned_#{retval}",optsContent ]
          end
        rescue
          returnValue=[opts[:resultpath],opts[:displayname],opts[:benchname],"ERROR",optsContent]
        ensure
          mutex.synchronize{ runningCount=runningCount.pred; }
          
        end
        
        returnValue
      }

      #Try the Ruby native way (but linux does not support this)
      newT.priority=-ThreadsPriorityIncrement;
      RunningThreads.push(newT);
      
      while mutex.synchronize{ runningCount.to_i>=opts[:maxprocesses]} do
        processFinishedThreads(RunningThreads);
        sleep(1);
      end

    }#End of repetitions
  } #End of simulations

  while mutex.synchronize{runningCount.to_i>0} do
    processFinishedThreads(RunningThreads);
    sleep(1);
  end

  return tableOfResults
end


def processFinishedThreads( runningThreads )
  runningThreads.delete_if{|t| 
    #Find terminated threads, to save their value, and remove them definitively
    if !t.alive?
      dataout=t.value
      begin
        f=File.open(dataout[0],"a");
        if $verbose      
          f.write( dataout[1..-1].join(";\t")+"\r\n")
	elsif !dataout[3].to_s.start_with?("Returned") && !dataout[3].to_s.start_with?("ERROR")   
          f.write( dataout[1..5].join(";\t")+"\r\n")
        end
      ensure
        f.close
      end
    end
    !t.alive?
  }
end

at_exit do
  processFinishedThreads(RunningThreads);
  RunningThreads.each{ |t| t.kill }
end

