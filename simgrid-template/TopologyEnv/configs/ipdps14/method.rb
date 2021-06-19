#!/usr/bin/ruby -Ku
#
# 	Control the complete flow for allocating extensible multiple topologies to yearly sets of cabinets
#  config file contains the configuration for requested topologies and cabinets sizing 
#  Syntax details are given in the sample config file 


require "optparse"
require "pp"
require_relative "../../../TopologyEnv/extensible/config"
require_relative "../../../MpiEnv/config/common/benchutils"


#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
#Global configuration
#
#
#
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


MaxProcesses=1;

	
NTries=20;


class TopologyInfo
	
	attr_accessor :platformbase, :topologyname, :topologyclufile, :platformfile, :hostfile, :data

	def initialize(topologyname)
		@topologyname=topologyname
		@simulatedtime=Array.new
	end	
end

StageSizes=[32,64,96,144,196,256,324,400,480,576,692,784,900,1024]
StageLayouts=[[2,4],[4,4],[4,6],[6,6],[6,8],[8,8],[9,9],[10,10],[10,12],[12,12],[12,14],[14,14],[15,15],[16,16]]

#Step 0 is not really necessary, just used for generating simulations small enough for simgrid not to freeze
Dimensions6=[[32,"2-2-2-2-2"],[64,"2-2-2-2-2-2"],[96,"2-2-3-2-2-2"],[144,"2-2-3-3-2-2"],[192,"2-2-4-3-2-2"],[256,"2-2-4-4-2-2"],[324,"3-3-3-3-2-2"],[400,"2-2-5-5-2-2"],[480,"2-2-5-6-2-2"],[576,"2-2-6-6-2-2"],[692,"2-2-7-6-2-2"],[784,"2-2-7-7-2-2"],[900,"3-3-5-5-2-2"],[1024,"2-2-8-8-2-2"] ]
Dimensions5=[[32,"2-2-2-2-2"],[64,"2-2-4-2-2"],[96,"2-3-4-2-2"],[144,"2-3-6-2-2"],[192,"2-4-6-2-2"],[256,"4-4-4-2-2"],[324,"3-3-9-2-2"],[400,"4-5-5-2-2"],[480,"4-5-6-2-2"],[576,"4-6-6-2-2"],[692,"4-7-6-2-2"],[784,"4-7-7-2-2"],[900,"9-5-5-2-2"],[1024,"4-8-8-2-2"] ]
Dimensions4=[[32,"2-4-2-2"],[64,"4-4-2-2"],[96,"6-4-2-2"],[144,"6-6-2-2"],[192,"8-6-2-2"],[256,"8-8-2-2"],[324,"9-9-2-2"],[400,"10-10-2-2"],[480,"10-12-2-2"],[576,"12-12-2-2"],[692,"14-12-2-2"],[784,"14-14-2-2"],[900,"15-15-2-2"],[1024,"16-16-2-2"] ]

DimensionsDragonfly=[[32,"4-2"],[64,"8-4"],[96,"8-4"],[144,"8-4"],[192,"8-4"],[256,"8-4"],[324,"12-6"],[400,"12-6"],[480,"12-6"],[576,"12-6"],[692,"12-6"],[784,"12-6"],[900,"16-8"],[1024,"16-8"] ]


DragonflyName="df"
AllFamilies=["torus6","mesh6","torus5","mesh5","torus4","mesh4","torus",DragonflyName,"rr-6","ring"]
#AllFamilies=["torus6","ring"]
#"hc"

def getTopologyName(family, i)

	
			case family
			when "torus3"
				return "torus"
			when "torus4"
				return "torus-"+Dimensions4[i][1]
			when "torus5"
				return "torus-"+Dimensions5[i][1]
			when "torus6"
				return "torus-"+Dimensions6[i][1]
			when "mesh3"
				return "mesh"
			when "mesh4"
				return "mesh-"+Dimensions4[i][1]
			when "mesh5"
				return "mesh-"+Dimensions5[i][1]
			when "mesh6"
				return "mesh-"+Dimensions6[i][1]
		        when DragonflyName
                		return "df-"+DimensionsDragonfly[i][1]
			end
			
			family
end


class  MyArray < Array

  def sum
    return self.inject(0){|accum, i| accum + i }
  end

  def mean
    return self.sum / self.length.to_f
  end

  def sample_variance
    m = self.mean
    sum = self.inject(0){|accum, i| accum + (i - m) ** 2 }
    return sum / (self.length - 1).to_f
  end

  def standard_deviation
    return Math.sqrt(self.sample_variance)
  end

end

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
#Misc. functions for running the cabling method
#
#
#
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#Stageset is an array of arrays, which contain the stage index (relative to global sizes), and the used topologies family 
def generateConfigFile(stageset,filename,opts,configpath)


	#Create ad-hoc config file
	cabinets=""
	topologies=""
	stageset.each_with_index{ |arr,index|
		i=arr[0]
		cabinets+="#{StageLayouts[i][0]} #{StageLayouts[i][1]}\n"
		ratio=1.0/(arr.length-1)
		arr[1..-1].each{ |family|
			topologies+="#{getTopologyName(family,i)} #{index} #{ratio}\n"
		}
	}
		
	

	allocation="ALLOCATION "
	if opts[:allocationtime]
		allocation+=" #{opts[:allocationtime]}  sa " 
	else
		allocation+=" 600  sa "
	end
	if opts[:patchpanel]
		allocation+=" --use-patch-panel "
	end
	if opts[:costnorm]
		allocation+=" --cost-norm #{opts[:costnorm]} "
	end
		
	
	content= <<-EOM
CABINETS #{opts[:switchperrack]}
#{cabinets}
TOPOLOGIES
#{topologies}
LAYOUT
CLUSTERING ward png
#{allocation}
 EOM
	
	File.write(filename,content)
	
end


def runConfig(filename, opts,configpath,lastyear)
	# run global.rb
	cmd="./global.rb --clustering layout --search #{opts[:searchtype]} --lastyear #{lastyear}   #{filename}"
	if opts[:clean]
		cmd+=" --clean-allocation"
	end

	if opts[:redundancy]
		cmd+=" --redundancy #{opts[:redundancy]} "
	end

	if opts[:usepatchpanels]
		cmd+=" --use-patch-panels "
	end

	if opts[:optimizecables]
		cmd+=" --optimize-cables "
	end
	
	runCmd(cmd,"global",configpath)	

	#get results of interest

	#resultcode=File.read("_result.rb")
	#eval(resultcode)	
end


def continueAlloc(year,costcoeff,energycoeff,opts,configpath)


	cmd="./sa_qxap 10000 1000 "
	if opts[:allocationtime]
		cmd+= opts[:allocationtime].to_s
	end

	cmd += " #{costcoeff} #{energycoeff} "
	if opts[:redundancy]
		cmd+=" #{opts[:redundancy]} "
	else
		cmd+=" 0 "
	end
	
	cmd+=" alloc_#{year}.in"
	
	runCmd(cmd,"alloc_#{year}",configpath)	

end

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
#Misc. functions for running  benches with simgrid 
#
#
#
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

def runBenches(nrouters, topologyInfo, bench_range, configpath, opts)
   
	#Timeout before the bench simulation is aborted (argh, deadlocks..)
	opts['simulationtimeout']=-1
	niterations=1
	
	FileUtils.rm_rf "_data"
	File.open("benches.dat","w").puts "Topology; \t Bench; \tsimulatedtime; \tsimulatedtotalflop; \tsimulatedprocessflop; \t"   

	bench_range.each{ |benchsign|

		bench=benchsign	
		if benchsign.include? "."
			opts['benchclass']=benchsign[benchsign.index(".")+1..-1]
			bench=benchsign[0,benchsign.index(".")]
		end
	
		topologyInfo.each{ |info|
			topoOpts=opts
			#Make the bench program	
			topoOpts[:hostfile]=info.hostfile
			topoOpts[:platformfile]=info.platformfile
			data=MyArray.new
			for iter in 1..niterations 
				sign="#{info.topologyname}_#{benchsign}_#{iter}"
				retval,results=runBench(bench,"simgrid",nrouters,configpath,opts)			
	#results=[simulatedtime,simulatedtotalflop,simulatedprocessflop,tracingtype,execname,maketime,runtime,posttime,tracesize,tracepath]
#		STDOUT.puts results.inspect

                                data=[results[4],results[0],results[1],results[2]]
                                datastr="#{info.topologyname}; \t"+data.join("; \t")
				if(retval!=0)
					STDOUT.puts "Aborting #{info.topologyname}/#{benchsign}..."
					break
				end
				FileUtils.chdir configpath
				STDOUT.puts "Saving data to #{Dir.pwd}/_data"
                               

				File.open("_data","a") do |file| 
					file.write(datastr)
				end				
                                #info.data.push([data])
	                        File.open("benches.dat","a").puts datastr
			end			

			STDOUT.puts "Finished #{info.topologyname}/#{benchsign}"
			
		} #end of topology

		
	}#end of bench	
			
end

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
#Functions for generating figures 
#
#
#
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


def runBenchesFigure(configpath, opts)

	opts[:switchperrack]=4
	stageindex=5
	bench_range=["cg.A","ft.A","graph500"]
	#bench_range=["graph500"]
	opts['graph500class']="tiny"
		
	FileUtils.rm_rf "logs/*.log"	
	FileUtils.rm_rf  "platforms/*"	
	FileUtils.rm_rf  "topologies/*.edges"	
	
	nrouters=StageSizes[stageindex]
	#Generate the topology files for each input topology
	topology_range=AllFamilies.map{ |family| getTopologyName(family,stageindex) }
	topoInfo=Array.new
	topology_range.each{ |topology|
			info=TopologyInfo.new(topology)
			
			info.platformbase=generateTopology(nrouters,topology,configpath,opts)

			Dir.chdir(configpath);

			#Generate simgrid platform file
			info.topologyclufile="topologies/#{info.platformbase}.edges"
			info.platformfile="platforms/#{info.platformbase}.xml"
			info.hostfile="platforms/#{info.platformbase}.txt"
			if( !File.exists?(info.platformfile))	
				Dir.chdir("platforms");
				runCmd("../../../../MpiEnv/config/common/generate_platform.rb --nodeperswitch 1 ../#{info.topologyclufile}  #{info.platformbase}","platform#{topology}__#{nrouters}",configpath);
				Dir.chdir("../");				
			end			
			topoInfo.push(info)
	}	

	runBenches(nrouters,topoInfo,bench_range,configpath,opts)

end


class TopologySet
	attr_accessor :name, :families, :stageindex, :resultcodes, :misc

	def initialize(name)
		@name=name
		@families=Array.new
		@resultcodes=Array.new
	end	
	
	def getresult(resultname,resultindex)
		ff=resultcodes[resultindex]+";\nresults[\"#{resultname}\"];"
		#STDOUT.puts "Evaluating <<#{ff}>>"
		eval(ff)		
	end
end

def 	runImmediateMappingFigure(configpath,opts)

	opts[:switchperrack]=4
	opts[:costnorm]=1
	opts[:searchtype]="cost"
	opts[:clean]=true
	opts[:redundancycoeff]=0;
	opts[:allocationtime]=600;
	
	#myFamilies=["torus6","torus5","torus4","torus","rr-6","ring"]
	myFamilies=["torus6","torus5","torus4","torus",DragonflyName,"rr-6","ring"]
	#myFamilies=["torus6","mesh6","torus5","mesh5","torus4","mesh4","torus","rr-6","ring"]

	#stageindex_range=1..2
	stageindex_range=0..13

	#Single topologies
	topologySets=Array.new
	myFamilies.each{|t| 
		topologySets.push(TopologySet.new(t)); 
		topologySets.last.families.push(t);
	}
	
	#Combinations of increasing count
	#for i in 1..AllFamilies.length
	#	topologySets.push(TopologySet.new("agg#{i}")); 
	#	topologySets.last.families=myFamilies[0...i];
	#end

	topologySets.push(TopologySet.new("optimized")); 
	topologySets.last.families=myFamilies

	#Compute the total cost of each set, for each stage index 	
	
	topologySets.each{ |set|
	
		set.misc=Array.new
		for stageindex in stageindex_range
			
			configfilename="#{configpath}/configurations/config_imm_#{stageindex}_#{set.name}"
			resultfilename="#{configpath}/results/_result_imm_#{stageindex}_#{set.name}.rb"
			if(! File.exists?(resultfilename))
				#Stageset is an array of arrays, which contain the stage index (relative to global sizes), and the used topologies family 
				FileUtils.chdir "../../../TopologyEnv/extensible"
				generateConfigFile(	[[stageindex]+set.families],configfilename,opts, configpath)	
				runConfig(configfilename, opts, configpath,0)
				FileUtils.mv("_result.rb",resultfilename)
				FileUtils.chdir configpath
			end	
							
			set.resultcodes.push(File.read(resultfilename))
                  val=[set.getresult("totalCost",set.resultcodes.length-1),
                       set.getresult("totalLength",set.resultcodes.length-1)]

			 STDOUT.puts "ReadT#{set.name}S#{set.resultcodes.length-1}=#{val}"
			set.misc.push(val)
					
		end
	}
	
	#Add the naive sum of costs
	tot=TopologySet.new("total")
	tot.misc=Array.new(stageindex_range.last+1,[0,0])
	for ti in 0...myFamilies.length
		tot.misc.map!.with_index{ |x,si|
		STDOUT.puts "T#{ti}S#{si}=#{x}+#{topologySets[ti].misc.inspect}"
		  [x[0]+topologySets[ti].misc[si][0],x[1]+topologySets[ti].misc[si][1]] 	}
	end
	topologySets.push(tot)

	#save final data 
	datfile=File.open("immediate.dat","w")
	sep=";\t"
	datfile.write("$R_\Omega$"+sep)			
	topologySets.each{ |set| datfile.write("#{set.name}_Cost#{sep}#{set.name}_Length#{sep}")	}
	datfile.write("\n")
	stageindex_range.each_with_index{ |stage,i|
		datfile.write(StageSizes[stage].to_s+sep);			
		topologySets.each{ |set|
			datfile.write(set.misc[i].join(" ;").to_s+sep)			
		}
		datfile.write("\n")
	}
	datfile.close()

			
	
end

 
def 	runMitigateMappingFigure(configpath,opts)

  opts[:switchperrack]=4
  opts[:costnorm]=1
  opts[:searchtype]="cost"
  opts[:clean]=true
  opts[:redundancycoeff]=0;
  opts[:allocationtime]=600;
  

  #Family to use as a function of stage index
  #              0         1        2       3       4         5        6       7        8          9       10      11       12      13
  # StageSizes=[ 32        64      96       144     196       256      324     400      480        576     692     784      900     1024]
  myFamilies=[   ""  , "torus6","torus6","torus6","torus6","torus5","torus5","torus5","torus5","torus4","torus4","torus4","torus4","torus4"]
  #Results from immediate figure
  naive=     [ [0,0] , [66305,8552000],[124171,15792000],[225421,31488000],[302581,45216000], #torus6
               [405632,63792000],[505052,67416000],[643338,114936000],[758133,115728000], #torus5
               [615198,106488000],[709563,111168000],[835763,142400000],[960843,165744000],[1120870,232808000]]#torus4



  myStageSets=[[13],
               [6,13],
               [5,9,13],
               [4,7,10,13],
               [4,6,8,11,13],
               [4,6,8,10,11,13],
               [4,6,8,10,11,12,13]]
  

  topologySets=Array.new
  myStageSets.each{ |stageindex_range|

    topologySets.push(TopologySet.new(stageindex_range.length.to_s));
    topologySets.last.misc=Array.new;

    topologies=Array.new
    stageindex_range.each{|stageIndex| 
      t=myFamilies[stageIndex];
      topologies.push([stageIndex,t]); 
    }
	

	#Compute the total cost of each set, for each stage index 	
	
    
    configfilename="#{configpath}/configurations/config_mit_#{topologySets.last.name}"
    resultfilename="#{configpath}/results/_result_mit_#{topologySets.last.name}.rb"
    if(! File.exists?(resultfilename))
      #Stageset is an array of arrays, which contain the stage index (relative to global sizes), and the used topologies family 
      FileUtils.chdir "../../../TopologyEnv/extensible"
      STDOUT.puts topologies.inspect
      generateConfigFile(topologies,configfilename,opts, configpath)	
      runConfig(configfilename, opts, configpath,stageindex_range.length-1)
      FileUtils.mv("_result.rb",resultfilename)
      FileUtils.chdir configpath
    end	

    
    
    topologySets.last.resultcodes.push(File.read(resultfilename))
    valCost=topologySets.last.getresult("stageCost",topologySets.last.resultcodes.length-1).inject(0){ |sum,x|  sum+x[0]}
    #valCost=topologySets.last.getresult("totalCost",topologySets.last.resultcodes.length-1)
    valLength=topologySets.last.getresult("totalLength",topologySets.last.resultcodes.length-1)
    STDOUT.puts "ReadT#{topologySets.last.name}S#{topologySets.last.resultcodes.length-1}=[#{valCost} #{valLength}]"
    topologySets.last.misc=[valCost,valLength]
    
	
  }
	

  #Add the naive sum of costs
  #tot=TopologySet.new("total")
  #tot.misc=Array.new(stageindex_range.last+1,0)
  #for ti in 0...myFamilies.length
  #	tot.misc.map!.with_index{ |x,si|
  #	STDOUT.puts "T#{ti}S#{si}=#{x}+#{topologySets[ti].misc.inspect}"
  #	  x+topologySets[ti].misc[si]	}
  #end
  #topologySets.push(tot)

  #save final data 
  datfile=File.open("mitigate.dat","w")
  sep=";\t"
  datfile.write("\"n-stages\"#{sep}\"Total cost\"#{sep}\"Agg. length\"")			
 # topologySets.each{ |myset| datfile.write(myset.name+sep)	}
  datfile.write("\n")
  myStageSets.each_with_index{ |stage,i|
    datfile.write( (i+1).to_s+sep);			
    #Compute naive cost and length
    na=[0,0]
    stage.each{  |stageindex|
      na=[na[0]+naive[stageindex][0],na[1]+naive[stageindex][1]]
    }
    datfile.write(topologySets[i].misc.join(" ;").to_s+sep+na.join(" ;").to_s+sep)
    datfile.write("\n")
  }
  datfile.close()
  
  
end



 
def 	runGroupsMappingFigure(configpath,opts)

  opts[:switchperrack]=4
  opts[:costnorm]=1
  opts[:searchtype]="cost"
  opts[:clean]=true
  opts[:redundancycoeff]=0;
  opts[:allocationtime]=600;
  

  #Generate configuration
  topologiesAll=[[0,"torus6","torus5","torus4",DragonflyName,"rr-6"],
              [1,"torus5","torus4",DragonflyName,"rr-6"],
              [2,"torus5","torus4",DragonflyName,"rr-6"]]
  topologiesOneByOne=[[0,"torus6"],[0,"torus5"],[0,"torus4"],[0,DragonflyName],[0,"rr-6"],
              [1,"torus5"],[1,"torus4"],[1,DragonflyName],[1,"rr-6"],
              [2,"torus5"],[2,"torus4"],[2,DragonflyName],[2,"rr-6"]]

  
  topologySets=Array.new


 #Compute the cost without cable optimization
  topologySets.push(TopologySet.new("NoOpt"));
  configfilename="#{configpath}/configurations/config_group_noopt"
  resultfilename="#{configpath}/results/_result_group_noopt.rb"
  if(! File.exists?(resultfilename))
    #Stageset is an array of arrays, which contain the stage index (relative to global sizes), and the used topologies family 
    FileUtils.chdir "../../../TopologyEnv/extensible"
    opts[:usepatchpanels]=false;
    opts[:optimizecables]=false;
    generateConfigFile(topologiesAll,configfilename,opts, configpath)	
    runConfig(configfilename, opts, configpath,2)
    FileUtils.mv("_result.rb",resultfilename)
    FileUtils.chdir configpath
  end	
  topologySets.last.resultcodes.push(File.read(resultfilename))
  val=topologySets.last.getresult("stageCost",topologySets.last.resultcodes.length-1).inject(0){ |sum,x| sum+x[0]}
  STDOUT.puts "ReadT#{topologySets.last.name}S#{topologySets.last.resultcodes.length-1}=#{val}"
  topologySets.last.misc=val

  return 
 
  #Compute the cost with cable optimization
  topologySets.push(TopologySet.new("NoOpt"));
  configfilename="#{configpath}/configurations/config_group_opt"
  resultfilename="#{configpath}/results/_result_group_opt.rb"
  if(! File.exists?(resultfilename))
    #Stageset is an array of arrays, which contain the stage index (relative to global sizes), and the used topologies family 
    FileUtils.chdir "../../../TopologyEnv/extensible"
    opts[:usepatchpanels]=false;
    opts[:optimizecables]=true;
    generateConfigFile(topologiesAll,configfilename,opts, configpath)	
    runConfig(configfilename, opts, configpath,2)
    FileUtils.mv("_result.rb",resultfilename)
    FileUtils.chdir configpath
  end	
  topologySets.last.resultcodes.push(File.read(resultfilename))
  val=topologySets.last.getresult("stageCost",topologySets.last.resultcodes.length-1).inject(0){ |sum,x| sum+x[0]}
  STDOUT.puts "ReadT#{topologySets.last.name}S#{topologySets.last.resultcodes.length-1}=#{val}"
  topologySets.last.misc=val
 	


  #Compute the cost for naive (i.e. adding the cost of each individual topology) 
  topologySets.push(TopologySet.new("Naive"));
  topologySets.last.misc=0;
  topologiesOneByOne.each{ |tSet|
    configfilename="#{configpath}/configurations/config_group_onebyone#{tSet.join("_").to_s}"
    resultfilename="#{configpath}/results/_result_group_onebyone#{tSet.join("_").to_s}.rb"
    if(! File.exists?(resultfilename))
      #Stageset is an array of arrays, which contain the stage index (relative to global sizes), and the used topologies family 
      FileUtils.chdir "../../../TopologyEnv/extensible"
      opts[:usepatchpanels]=false;
      opts[:optimizecables]=false;
      generateConfigFile([tSet],configfilename,opts, configpath)	
      runConfig(configfilename, opts, configpath,0)
      FileUtils.mv("_result.rb",resultfilename)
      FileUtils.chdir configpath
    end	
    topologySets.last.resultcodes.push(File.read(resultfilename))
    val=topologySets.last.getresult("stageCost",topologySets.last.resultcodes.length-1).inject(0){ |sum,x| sum+x[0]}
    STDOUT.puts "ReadT#{topologySets.last.name}S#{topologySets.last.resultcodes.length-1}=#{val}"
    topologySets.last.misc+=val
  }

 
  #save final data 
  datfile=File.open("groups.dat","w")
  sep=";\t"
  datfile.write("\"n-stages\"#{sep}\"Total cost\"")			
 # topologySets.each{ |myset| datfile.write(myset.name+sep)	}
  datfile.write("\n")
  myStageSets.each_with_index{ |stage,i|
    datfile.write( (i+1).to_s+sep);			
    datfile.write(topologySets[i].misc.to_s+sep)			
    datfile.write("\n")
  }
  datfile.close()
  
  
end


def runPatchworkMappingFigure(configpath,opts)

  opts[:switchperrack]=4
  opts[:costnorm]=1
  opts[:searchtype]="cost"
  opts[:clean]=true
  opts[:redundancycoeff]=0;
  opts[:allocationtime]=600;
	stageindex=5
	bench_range=["cg.A","ft.A","graph500"]
	#bench_range=["graph500"]
	opts['graph500class']="tiny"
    myFamilies=["df","torus5"]
    defectRates=[0,0.05,0.1,0.2]
	FileUtils.rm_rf "logs/*.log"	
	FileUtils.rm_rf  "platforms/*"	
	FileUtils.rm_rf  "topologies/*.edges"	
	
	nrouters=StageSizes[stageindex]

  #First generate the set of edges for the patchowrk
  configfilename="#{configpath}/configurations/config_patchwork"
  resultfilename="#{configpath}/results/_result_patchwork.rb"
  if(! File.exists?(resultfilename))
    FileUtils.chdir "../../../TopologyEnv/extensible"
    generateConfigFile(	[[stageindex]+myFamilies],configfilename,opts, configpath)	
    runConfig(configfilename, opts, configpath,0)
    FileUtils.mv("_result.rb",resultfilename)
    FileUtils.mv("all_1.edges","#{configpath}/topologies/patchwork_base.edges")
    FileUtils.chdir configpath
  end	
  

  #Generate the patchwork topologies
   topology_range=myFamilies
    defectRates.each{ |rate|
      FileUtils.chdir "#{configpath}/topologies"
      runCmd("../../../extensible/lay2fabien.rb #{StageSizes[stageindex]}r#{opts[:switchperrack]} fb2-6-#{rate.to_s}@patchwork_base.edges","laypatch#{rate}",configpath);      
      topology_range.push("fb2-6-#{rate.to_s}@patchwork_base")
    }
    FileUtils.chdir configpath
  

  
	#Generate the topology files for each input topology
	topoInfo=Array.new
	topology_range.each{ |topology|

			info=TopologyInfo.new(topology)
			
			info.platformbase="#{StageSizes[stageindex]}r#{opts[:switchperrack]}+#{getTopologyName(topology,stageindex)}"

			Dir.chdir(configpath);

			#Generate simgrid platform file
			info.topologyclufile="topologies/#{info.platformbase}.edges"
			info.platformfile="platforms/#{info.platformbase}.xml"
			info.hostfile="platforms/#{info.platformbase}.txt"
			if( !File.exists?(info.platformfile))	
				Dir.chdir("platforms");
				runCmd("../../../../MpiEnv/config/common/generate_platform.rb --nodeperswitch 1 ../#{info.topologyclufile}  #{info.platformbase}","platform#{topology}__#{stageindex}",configpath);
				Dir.chdir("../");				
			end			
			topoInfo.push(info)
	}	
	

	runBenches(nrouters,topoInfo,bench_range,configpath,opts)


end
