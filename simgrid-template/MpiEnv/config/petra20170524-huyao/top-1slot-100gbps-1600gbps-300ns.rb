#!/usr/bin/ruby -Ku
#

require_relative "../common/benchtop"
require_relative "../common/generate_platform"
require "csv"


def main

  if ARGV.length < 3
    STDOUT.puts "usage: <first configuration index>  <last configuration index> <part index>"
    return -1
  end


  thispath=Dir.pwd

  FileUtils.rm_rf "logs/*.log"	
  FileUtils.rm_rf "platforms/*"	

  commonOpts=Hash.new
  commonOpts[:verbose]=true

  #Configuration sets

  results=Array.new
  conf_range=ARGV[0].to_i..ARGV[1].to_i
  for conf in conf_range 
		
  #General parameters
  commonOpts[:switchperrack]=1    #The number of switches in one rack/cabinet
  commonOpts[:nodeperswitch]=1    #The number of compute nodes attached to one switch
  commonOpts[:replay]="none" #"readsave"  #Defines usage of replay in Simgrid {read,save,readsave,readScalatrace,none}
  commonOpts[:runtype]="simgrid"  #The type of program run
  commonOpts[:repeats]=1          #The number of repetitions for each configuration            
  commonOpts[:maxprocesses]=28    #Number of concurrent simulations to run
  commonOpts[:cleanafter]=true     #Delete traces and so on after a correct execution of the bench 
  #commonOpts[:machinepower]      #The estimated power of the machine running the simulation. Guessed if not defined. 


  #Benches parameters
  commonOpts[:bfstype]="replicated"   #Graph500 implementation type
  commonOpts[:himenogridsize]="S" #The size of himeno grid size
  npbclass="B"
  graph500class="tiny"

  #Simgrid parameters
  commonOpts[:privatize]="yes"
  #commonOpts[:networkmodel]       #The network model used by simgrid. {SMPI,IB, NS3}
  #commonOpts[:extrampiarguments]="--cfg=viva/uncategorized:uncat.plist --cfg=viva/categorized:cat.plist"
  #commonOpts[:extrampiarguments]="-trace -trace-viva -trace-resource"
  #commonOpts[:extrampiarguments]="--log=smpi.thres:debug --log=smpi/maxmin.t:debug"
  
  #(Simgrid) platform generation
  nswitches_range=nil
  case conf
  when 0	
  commonOpts[:corepernode]  =1    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  nswitches_range=["64"]
  when 1
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =8    #The himeno parameter for k (may be necessary for some number of processes)
  nswitches_range=["64"]
  when 2
  commonOpts[:corepernode]  =1    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  nswitches_range=["256"]
  when 3
  commonOpts[:corepernode]  =1    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =8    #The himeno parameter for k (may be necessary for some number of processes)
  nswitches_range=["256"]
  end

  commonOpts[:cacheusage]="use" #Defines how to use cache: {use,nouse,only}
 # bench_range=BENCHLOCKNAME.keys
  bench_range=["mm","bt","cg","dt","ft","is","lu","sp","mg","graph500"]
  #bench_range=["mm","bt","cg","dt","ft","is","lu","sp","mg","graph500"]
  case ARGV[2]
  when "0"
   bench_range=["is"]
   #bench_range=["ft","lu","cg","is"]
  when "1"
   bench_range=["mm","himeno","graph500"]
  when "only"
   commonOpts[:cacheusage]="only"  #Defines how to use cache: {use,nouse,only}
  end

  commonOpts[:power]=1000e9        #The power of one core of simulated compute nodes in Flop/s 
  commonOpts[:cablebw]=1600e9  #huyao170612      #The bandwidth of cable in bit/s
  commonOpts[:switchlat]=300e-9  #huyao170605   #The latency of switches in seconds
  ###commonOpts[:switchlat]=100e-9   #The latency of switches in seconds
  commonOpts[:collselector]="mvapich2"  #The selector for MPI collectives
  commonOpts[:switchbw]=1e16      #The bandwidth of switches in bit/s
  commonOpts[:defaultdistance]=5  #The default distance for cables in meters
  commonOpts[:addproperties]=[]   #Additional properties as a table of string pairs
  commonOpts[:tracing]=false      #If set, tracing is enabled for display on viva/vite/...


  tableOfOpts=Array.new;
  nswitches_range.each{ |nswitches|
    
    opts=Hash.new
    opts[:nswitches]=nswitches;
    opts[:resultfile]="result_#{nswitches}_1slot_100gbps_1600gbps_300ns" #huyao170614

    #Glob the topologies directory for all edge files
    #topology_range=Dir.chdir("#{thispath}/topologies/size_#{nswitches}"){|d| Dir.glob('*.edges').collect!{|s| s.chomp('.edges')}}
    #topology_range.reject!{|x| !x.match("4torus") && !x.match("Ito_d8")}
	#topology_range=["ito_d16_256-bFalse", "random_d17_256-bFalse", "ito_d17_256-bFalse", "reduced_d16_256-bFalse", "random_d16_256-bFalse",  "reduced_d17_256-bFalse" ]
	########topology_range=["3torus-4-4-4", "fullyconnected"]
	#topology_range=["3torus-4-4-4-400Gbps", "3torus-4-4-4-1Tbps", "petra-elec_fullyconnected", "petra-hikari-fullyconnected"]
   	topology_range=["petra-hikari-fullyconnected"]

    #Generate the topology files for each input topology
    topology_range.each{ |topology|
	platformbase="#{conf}_#{topology}"
      opts[:displayname]="#{platformbase}"
      opts[:layoutedges]="size_#{nswitches}/#{topology}.edges"
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
        when "mm"
          #opts[:replay]="none"  #Defines usage of replay in Simgrid {read,save,readsave,readScalatrace,none}
        when "graph500"
          #opts[:replay]="none"  #Defines usage of replay in Simgrid {read,save,readsave,readScalatrace,none}
          opts[:benchclass]=graph500class
        when "ft"
          opts[:benchclass]="A"
        when "is"
          opts[:benchclass]="A"
        else
         # opts[:replay]="readsave"  #Defines usage of replay in Simgrid {read,save,readsave,readScalatrace,none}
          opts[:benchclass]=npbclass
        end

        tableOfOpts.push(opts.clone);

      }#End of benches loop
    }#End of topologies loop
  }#End of nswitchesw loop

  results.concat runExperiment(commonOpts,tableOfOpts,thispath)

  end#End of configurations



  measuresymbol=:runtime
  rowsymbol=:displayname
  columnsymbol=:benchname
  baserowname=/3torus/
  nswitches_range=["64"]
  #nswitches_range=["64","256"]

  tables=Hash.new
  headers=[:name]+results.collect{ |opts| opts[columnsymbol].to_sym}.uniq 
  headerRow=CSV::Row.new(headers,headers.collect{|x| 0 },true)
  STDOUT.puts headerRow.inspect
  nswitches_range.each{ |nswitches|   tables[nswitches]=CSV::Table.new([headerRow])  }
 #,{:headers=>true}
  STDOUT.puts tables.inspect

  #Insert results into appropriate table

  results.each{ |opts|
    if !opts.has_key?(measuresymbol)
      STDOUT.puts "Missing value for #{opts.inspect}"
      next
    end
    found=false
    nswitches=opts[:nswitches]
    STDOUT.puts "nswitches is #{opts[:nswitches]}"
    #Try to find existing row
    tables[nswitches].each{ |row|
      if row[:name].to_s==opts[rowsymbol]
        row[ opts[columnsymbol].to_sym ]=opts[measuresymbol]
        found=true
        break
      end
    }
    #Else, create a new one
    if !found
      row=CSV::Row.new(headers,headers.collect{|x| 0 })
      row[:name]=opts[rowsymbol]
      row[ opts[columnsymbol].to_sym ]=opts[measuresymbol]
      tables[nswitches] << row
    end
  }

  #Normalize data
  tables.each{ |nswitches,table| 
    baserow=Hash.new
    table.each{ |row| 
      if row[:name].to_s.match(baserowname)
        row.each{ |key,value| baserow[key]=value.to_f}
        break;
      end
    }
    if baserow.empty?
      STDERR.puts "Could not find base row  #{baserowname} for size #{nswitches}!"
      break
    else
      STDOUT.puts "Base row is #{baserow.inspect}"
    end
    table.each{ |row| 
      row.each{ |key,value|
        if key==:name
          next
        end
       # STDOUT.puts "<#{key.inspect}>"
        if !baserow.has_key?(key) ||  baserow[key]==0
          STDERR.puts "Could not normalize #{key} since it does not exist to base row #{baserow.inspect}!"
          next
        else
        #          STDERR.puts "Normalize #{key} with base row #{baserow.inspect}"
        end
        row[key]=value.to_f/baserow[key]
        }
    }
  }

  #Save results to CSV files 
  tables.each{ |nswitches,table| 
    FileUtils.rm_rf  "2result_#{nswitches}_1slot_100gbps_1600gbps_300ns.csv" #huyao170614
    sleep(1)
    File.write("2result_#{nswitches}_1slot_100gbps_1600gbps_300ns.csv",table.to_csv({:col_sep=>"\t;"}))  #huyao170614
  }
end


main if $0 == __FILE__
