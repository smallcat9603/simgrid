#!/usr/bin/ruby -Ku
#

require_relative "../common/benchtop"
require_relative "../common/generate_platform"



def main

  
  thispath=Dir.pwd

  FileUtils.rm_rf "logs/*.log"	
  FileUtils.rm_rf "platforms/*"	

  commonOpts=Hash.new
  #General parameters
  commonOpts[:switchperrack]=1    #The number of switches in one rack/cabinet
  commonOpts[:nodeperswitch]=1    #The number of compute nodes attached to one switch
  commonOpts[:corepernode]  =1    #The number of core per simulated compute node
  commonOpts[:runtype]="simgrid"  #The type of program run
  commonOpts[:repeats]=1          #The number of repetitions for each configuration            
  commonOpts[:maxprocesses]=4     #Number of concurrent simulations to run

  #commonOpts[:machinepower]      #The estimated power of the machine running the simulation. Guessed if not defined. 


  #Benches parameters
  commonOpts[:bfstype]="simple"   #Graph500 implementation type
  commonOpts[:himenogridsize]="S" #The size of himeno grid size


  #Simgrid parameters
  commonOpts[:privatize]="yes"
  commonOpts[:replay]="readScalatrace"  #Defines usage of replay in Simgrid {read,save,readsave,none}
  commonOpts[:networkmodel]       #The network model used by simgrid. {SMPI,IB, NS3}
  commonOpts[:collselector]       #The selector for MPI collectives
  #commonOpts[:extrampiarguments]="--cfg=viva/uncategorized:uncat.plist --cfg=viva/categorized:cat.plist"
  #commonOpts[:extrampiarguments]="-trace -trace-viva -trace-resource"
  #commonOpts[:extrampiarguments]="--log=smpi.thres:debug --log=smpi/maxmin.t:debug"
  
  #(Simgrid) platform generation
  commonOpts[:power]=100e9        #The power of one core of simulated compute nodes in Flop/s 
  commonOpts[:cablebw]=40e9       #The bandwidth of cable in bit/s
  commonOpts[:switchbw]=1e16      #The bandwidth of switches in bit/s
  commonOpts[:switchlat]=200e-9   #The latency of switches in seconds
  commonOpts[:defaultdistance]=5  #The default distance for cables in meters
  commonOpts[:addproperties]=[]   #Additional properties as a table of string pairs
  commonOpts[:tracing]=false      #If set, tracing is enabled for display on viva/vite/...






  opts=Hash.new
  opts[:benchname]="graph500"
  opts[:nswitches]=64
  opts[:platformfile]="./test_64.xml"
  opts[:hostfile]="./test_64.txt"
  opts[:benchclass]="tiny"
  opts[:cacheusage]="nouse"
  opts[:cleanafter]=false

  runBench("graph500","scalatrace",64,thispath,commonOpts.merge(opts));

exit

  tableOfOpts=Array.new;
  tableOfOpts.push opts
STDOUT.puts "RRRR#{thispath}"
 runExperiment(commonOpts,tableOfOpts,thispath)
 
exit



  tableOfOpts=Array.new;
  #nswitches_range=["32","64"] #,"256"]
  nswitches_range=["64"]
  nswitches_range.each{ |nswitches|
    
    opts=Hash.new
    opts[:nswitches]=nswitches;
    opts[:resultfile]="result_#{nswitches}"

    #Glob the topologies directory for all edge files
    topology_range=Dir.chdir("#{thispath}/topologies/size_#{nswitches}"){|d| Dir.glob('*.edges').collect!{|s| s.chomp('.edges')}}
    #topology_range=["topology1","topology2"]

    bench_range=BENCHLOCKNAME.keys
    #bench_range=["bench1","bench2"]

    #Generate the topology files for each input topology
    topology_range.each{ |topology|

      opts[:layoutedges]="size_#{nswitches}/#{topology}.edges"
      platformbase="#{topology}"
      #Generate simgrid platform file
      topologyclufile="topologies/#{opts[:layoutedges]}"
      opts[:platformfile]="platforms/#{platformbase}.xml"
      opts[:hostfile]="platforms/#{platformbase}.txt"
      if( !File.exists?(opts[:platformfile]))	
        Dir.chdir("#{thispath}/platforms");
        platformFlags=""
        if( topology.end_with?(".unidir"))
          opts[:unidirectional]=true
        end

        generateplatform("../#{topologyclufile}",platformbase,commonOpts.merge(opts))

        Dir.chdir("../");				
      end			

      
      #Push one hash per bench program	
      bench_range.each{ |bench|        

        opts[:benchname]=bench

        case  bench
        when "graph500"
          opts[:benchclass]="tiny"
        else
          opts[:benchclass]="A"
        end

        tableOfOpts.push(opts.clone);

      }#End of benches loop
    }#End of topologies loop
  }#End of nswitchesw loop

  
  runExperiment(commonOpts,tableOfOpts,thispath)

end


main if $0 == __FILE__
