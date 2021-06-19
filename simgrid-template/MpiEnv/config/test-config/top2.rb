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
  commonOpts[:maxprocesses]=24    #Number of concurrent simulations to run
  commonOpts[:cleanafter]=true     #Delete traces and so on after a correct execution of the bench 
  #commonOpts[:machinepower]      #The estimated power of the machine running the simulation. Guessed if not defined. 


  #Benches parameters
  commonOpts[:bfstype]="simple"   #Graph500 implementation type
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
  conf_range=nil
  case conf
  when 0	
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["64","random_d6_64-bTrue","ft"],
              ["64","random_d6_64-bTrue","lu"],
              ["64","random_d6_64-bTrue","cg"],
              ["64","random_d6_64-bTrue","is"],
              ["64","random_d6_64-bTrue","mg"],
              ["64","random_d6_64-bTrue","bt"],
              ["64","random_d6_64-bTrue","sp"],
              ["64","random_d6_64-bTrue","dt"],
  			  ["64","3torus-4-4-4","ft"],
              ["64","3torus-4-4-4","lu"],
              ["64","3torus-4-4-4","cg"],
              ["64","3torus-4-4-4","is"],
              ["64","3torus-4-4-4","mg"],
              ["64","3torus-4-4-4","bt"],
              ["64","3torus-4-4-4","sp"],
              ["64","3torus-4-4-4","dt"]]
#              ["64","random_d6_64-bTrue","graph500"]]
  when 1
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","augment_d8_256-bFalse","cg"],
              ["256","4torus-4-4-4-4","is"],
              ["256","augment_d8_256-bTrue","ft"]]
  when 2
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","random_d8_256-bFalse","cg"],#256 deg=8 proc=4,uni
              ["256","augment_d8_256-bFalse","is"],
              ["256","Ito_d8_256-bFalse","is"],
              ["256","augment_d8_256-bFalse","lu"],
              ["256","random_d8_256-bFalse","graph500"],
              ["256","random_d8_256-bFalse","ft"],
              ["256","augment_d8_256-bFalse","ft"],
              ["256","Ito_d8_256-bFalse","ft"],
              ["256","augment_d8_256-bFalse","sp"],
              ["256","Ito_d8_256-bFalse","sp"]]
	#10 simulations
  when 3
  commonOpts[:corepernode]  =8    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =8    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","4torus-4-4-4-4","graph500"], #8cores/switch graph500
              ["256","augment_d8_256-bFalse","graph500"],
              ["256","augment_d8_256-bTrue","graph500"],
              ["256","Ito_d8_256-bFalse","graph500"],
              ["256","random_d8_256-bFalse","graph500"],
              ["256","random_d8_256-bTrue","graph500"],
              ["64","3torus-4-4-4","graph500"],
              ["64","augment_d6_64-bFalse","graph500"],
              ["64","Ito_d6_64-bFalse","graph500"],
              ["64","random_d6_64-bFalse","graph500"],
              ["64","random_d6_64-bTrue","graph500"],
              ["64","reduced_d6_64-bTrue","graph500"]]
	#12 simulations
  when 4
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","augment_d8_256-bFalse","cg"],
              ["256","4torus-4-4-4-4","is"],
              ["256","augment_d8_256-bTrue","ft"]]
  when 5
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","4torus-4-4-4-4","graph500"], #4cores/switch graph500
              ["256","augment_d8_256-bFalse","graph500"],
              ["256","augment_d8_256-bTrue","graph500"],
              ["256","Ito_d8_256-bFalse","graph500"],
              ["256","random_d8_256-bFalse","graph500"],
              ["256","random_d8_256-bTrue","graph500"]]
#              ["64","3torus-4-4-4","graph500"],
#             ["64","augment_d6_64-bFalse","graph500"],
#              ["64","Ito_d6_64-bFalse","graph500"],
#              ["64","random_d6_64-bFalse","graph500"],
#              ["64","random_d6_64-bTrue","graph500"],
#              ["64","reduced_d6_64-bTrue","graph500"]]

  when 6
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","4torus-4-4-4-4","himeno"]] 
  when 7
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","augment_d8_256-bTrue","mm"],
              ["256","random_d8_256-bTrue","mm"]]
  when 8
  commonOpts[:corepernode]  =4    #The number of core per simulated compute node
  commonOpts[:himenokmax]   =4    #The himeno parameter for k (may be necessary for some number of processes)
  conf_range=[["256","augment_d8_256-bTrue","is"],
              ["256","4torus-4-4-4-4","is"], 
              ["256","random_d8_256-bTrue","is"]]
  end

  commonOpts[:cacheusage]="use" #Defines how to use cache: {use,nouse,only}
  case ARGV[2]
  when "0"
  when "1"
  when "only"
   commonOpts[:cacheusage]="only"  #Defines how to use cache: {use,nouse,only}
  end

  commonOpts[:power]=500e9        #The power of one core of simulated compute nodes in Flop/s 
  commonOpts[:cablebw]=100e9       #The bandwidth of cable in bit/s
  commonOpts[:switchlat]=60e-9   #The latency of switches in seconds
  commonOpts[:collselector]="mvapich2"  #The selector for MPI collectives
  commonOpts[:switchbw]=1e16      #The bandwidth of switches in bit/s
  commonOpts[:defaultdistance]=5  #The default distance for cables in meters
  commonOpts[:addproperties]=[]   #Additional properties as a table of string pairs
  commonOpts[:tracing]=false      #If set, tracing is enabled for display on viva/vite/...


  tableOfOpts=Array.new;
  conf_range.each{ |nswitches,topology,bench|
    
       STDOUT.puts "<#{nswitches}> <#{topology}> <#{bench}>"

      opts=Hash.new
      opts[:nswitches]=nswitches;
      opts[:resultfile]="result_#{nswitches}"
      
      #Generate the topology files for each input topology
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

  }#End of conf_range loop

  results.concat runExperiment(commonOpts,tableOfOpts,thispath)

  end#End of configurations



  measuresymbol=:runtime
  rowsymbol=:displayname
  columnsymbol=:benchname
  baserowname=/3torus/
  nswitches_range=["64","256"]

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
    FileUtils.rm_rf  "2result_#{nswitches}.csv" 
    sleep(1)
    File.write("2result_#{nswitches}.csv",table.to_csv({:col_sep=>"\t;"}))
  }
end


main if $0 == __FILE__
