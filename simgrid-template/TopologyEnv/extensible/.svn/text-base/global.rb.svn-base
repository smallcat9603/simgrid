#!/usr/bin/ruby -Ku
#
# 	Control the complete flow for allocating extensible multiple topologies to yearly sets of cabinets
#  config file contains the configuration for requested topologies and cabinets sizing 
#  Syntax details are given in the sample config file 


MaxProcesses=1;

	
NTries=20;

	
	
$help_message = <<-EOM
Usage:
  #{$0} [Options] Config_file

Options:
  --seed <SEED>
    	random seed
  --clean-all,--clean-allocation
      remove temporary files to force complete computation
  --clustering <CLUSTERING_TYPE>
  	   Selects the clustering method amongst:
  	   legacy: Use Ikki script for clustering (does not work on local workstation, but ok on CalcX machines)
	   unstable: Use the BUGGY script with igraph 	
	   layout: Use basic layout-time clustering [default]
  --search SEARCH_TYPE
      Define the type of allocation search:
      none: skip search
      pareto: Try to draw a pareto frontier (exenery,cost) 
      cost: Search once for cost-only optimization  
      energy: Search once for energy-only optimization  
  --lastyear LAST_YEAR
      Define until which year (included), the allocation will take place. First year is 1.
  --redundancy RATIO
      The desired redundancy coefficient [default:0]
  --optimize-cables
      Call the cable optimization program
  --use-patch-panels
      Enable the use of patch-panels
EOM

require "optparse"
require "pp"
require "fileutils"
require_relative "config"


def runAlloc(config,configname,costcoeff,energycoeff, redundancycoeff, limit,lastyear,layoutclustering,usepatchpanels)

        #Remove intermediate allocation results
        FileUtils.rm_rf Dir.glob('alloc_*.in')

	allocationcommand="./alloc4.rb #{config.allocationoptions[2..-1].join(" ")} #{configname}  "
	if layoutclustering
		allocationcommand+=" --layoutclustering "	
	end
  	if usepatchpanels
		allocationcommand+=" --use-patch-panels "	
	end
  

	allocationcommand+=" --seed #{config.seed} #{costcoeff} #{energycoeff} #{redundancycoeff} #{limit} #{config.allocationoptions[0..1].join(" ")} --lastyear #{lastyear} clean"

	STDOUT.puts "Executing allocation command: #{allocationcommand}.."
	
	if(File.exists?(config.allilogfn))
		FileUtils.rm config.allilogfn
	end

	allocationoutput=`#{allocationcommand}`
	if $? !=0 
		File.open(config.allilogfn,"w").write(allocationoutput);
		STDERR.puts "allocation command failed! Output is stored in #{config.allilogfn}."
		exit 3
	end

	File.open(config.allilogfn,"w").write(allocationoutput);

	
	#Skip NNodes and Synth$
	cost=nil;energy=nil;
	allocationoutput.scan(/Synthesis\s+\d+\s+\d+\s+(\d+.\d*)\t\s+(\d+.\d*)\t\s+/).each do |m|	
		cost=m[0].to_f
		energy=m[1].to_f
	end
	STDOUT.puts "Allocation (#{costcoeff},#{energycoeff},#{redundancycoeff}) leads to cost=#{cost} and energy=#{energy}" 
	return cost,energy
end

def runPost(config,configname,limit)


	_command="mincostflow/mincostflow  #{config.postoptions[1..-1].join(" ")} #{configname}  "
	STDOUT.puts "Executing post command: #{_command}.."
	
	if(File.exists?(config.postilogfn))
		FileUtils.rm config.postilogfn
	end

	_output=`#{allocationcommand}`
	if $? !=0 
		File.open(config.postilogfn,"w").write(_output);
		STDERR.puts "post command failed! Output is stored in #{config.postilogfn}."
		exit 3
	end

	File.open(config.postilogfn,"w").write(_output);
	
	#Skip NNodes and Synth$
	cost=nil;energy=nil;
	_output.scan(/Synthesis\s+\d+\s+\d+\s+(\d+.\d*)\t\s+(\d+.\d*)\t\s+/).each do |m|	
		cost=m[0].to_f
		energy=m[1].to_f
	end
	STDOUT.puts "Post (#{config.postoptions[1..-1].join(" ")}) leads to cost=#{cost} and energy=#{energy}" 
	return cost,energy
end



def main
	
	redundancycoeff=0;

	$opts = Hash.new
	opt = OptionParser.new
	opt.on("--seed SEED") do |v|
		$opts[:seed] = v
	end
	opt.on("--clean-all") do |v|
		$opts[:cleanall] = v
		$opts[:cleanallocation] = v
	end
	opt.on("--clean-allocation") do |v|
		$opts[:cleanallocation] = v
	end
	opt.on("--clustering CLUSTERING_TYPE") do |v|
		$opts[:clustering] = v
	end
	opt.on("--search TYPE") do |v|
		$opts[:search] = v
	end
	opt.on("--lastyear LAST_YEAR") do |v|
		$opts[:lastyear] = v
	end
	opt.on("--redundancy RATIO") do |v|
		$opts[:redundancy] = v
		redundancycoeff=v;
	end
	opt.on("--optimize-cables") do |v|
		$opts[:optimizecables] = v
	end
	opt.on("--use-patch-panels") do |v|
		$opts[:usepatchpanels] = v
	end

  	opt.parse!(ARGV)
	if ! $opts[:seed] 
		$opts[:seed]=0;
	end
	layoutclustering=false;
	if !$opts[:clustering] || $opts[:clustering]=='layout'
		layoutclustering=true;
	end

	if ARGV.length < 1
		puts $help_message
		exit(1)
	end
	
	if !File.exist?(ARGV[0])
		STDERR.puts "Configuration file <#{ARGV[0]}>does not exist. "
		exit(0)
	end

	if  $opts[:search]!='none' &&  $opts[:search]!='cost' && $opts[:search]!='energy' && $opts[:search]!='pareto'
		STDERR.puts "You need to select an allocation search method with --search "
		exit(0)
	end

	if  !$opts[:lastyear]
		STDERR.puts "You need to set a last year with --lastyear"
		exit(0)
	end
        
	configname=ARGV[0]

	if $opts[:cleanall] 
		`rm *.log *.edges *.png *.coord`
	end
		
	
	if !File.exist?(configname)
		STDERR.puts "Configuration file <#{configname}> does not exist. "
		exit(0)
	end

	#Read the configuration file
	config=MyConfig.new(configname,$opts[:seed])

	if $opts[:cleanallocation] 
		FileUtils.rm_rf config.allilogfn
	end


	#Compile the allocation binary
	`make`

	threads=[];
	mutex=Mutex.new
	ongoingProcesses=0;
	
	config.topologies.each{ |thistopo|
  		threads << Thread.new(thistopo) { |topo|
			
			mutex.lock
			while	ongoingProcesses>=MaxProcesses
				mutex.unlock
				sleep 1	
				mutex.lock
			end
			ongoingProcesses=ongoingProcesses+1;			
			mutex.unlock
						
			cabinetcount=config.cabinetyears[topo.year].cabinetcount
	  		nnodes=cabinetcount.to_i*config.nodepercabinet.to_i;
	  		layoutcommand="./lay3.rb #{config.layoutoptions.join(" ")} --layout #{config.cabinetyears[topo.year].rows}x#{config.cabinetyears[topo.year].columns} --seed #{config.seed} -c #{nnodes}r#{config.nodepercabinet} #{topo.name}" 
			STDOUT.puts "Executing layout command: #{layoutcommand}.."
			layoutoutput=`#{layoutcommand}`
			if $? !=0 
				File.open(topo.layglogfn,"w").write(layoutoutput);
				STDERR.puts "Layout command failed! Output is stored in #{topo.layglogfn}."
			else

				if !layoutclustering 					
					clust=if $opts[:clustering]=='legacy' 
								'clust_legacy'
							else
								'clust'
							end
					clusteringcommand="R --slave -f ./#{clust}.R --args #{topo.layedgefn} #{config.clusteringoptions[0]} #{config.nodepercabinet} #{config.clusteringoptions[1..-1].join(" ")}"
					STDOUT.puts "Executing clustering command: #{clusteringcommand}.."
					clusteringoutput=`#{clusteringcommand}`
					if $? !=0 
						File.open(topo.cluglogfn,"w").write(clusteringoutput);
						STDERR.puts "clustering command failed! Output is stored in #{topo.cluglogfn}."
					else
	
					end
				
				end #End of layout
				
			end
			sleep 1	
			mutex.lock()
			ongoingProcesses=ongoingProcesses-1			
			mutex.unlock

		}
	}

	threads.each { |aThread|  aThread.join }

	#Compute cluster allocation along the pareto border


	#Clean the pareto frontier data file	
	if File.exists?("pareto.csv")
		File.delete("pareto.csv");
	end
	
	cost=Array.new;
	energy=Array.new;

	if $opts[:search]=='none'
        STDOUT.puts "Skipping allocation search following argument <--search none>"
    end

	#Cost only case	
	if $opts[:search]=='cost' || $opts[:search]=='pareto'
		cost[0],energy[0]=runAlloc(config,configname,1,0, redundancycoeff, ARGV[1],$opts[:lastyear], layoutclustering,$opts[:usepatchpanels])
	end
	
	#Energy only case	
	if $opts[:search]=='energy' || $opts[:search]=='pareto'
		cost[NTries],energy[NTries]=runAlloc(config,configname,0,1,redundancycoeff,ARGV[1],$opts[:lastyear], layoutclustering,$opts[:usepatchpanels])
	end
	
	if $opts[:search]=='pareto'
		for x in 1...NTries	
		
			ratio= x.to_f/NTries.to_f;
			costcoeff=1/(cost[0]*ratio + cost[NTries]* (1-ratio));
			energycoeff=1/(energy[0]*ratio + energy[NTries]*(1-ratio));
			#Since there are integers in the cost function, get large coefficients 		
					
			coeffnorm=1000;
			if energycoeff<costcoeff
				coeffnorm=coeffnorm/energycoeff;
			else
				coeffnorm=coeffnorm/costcoeff;
			end
			costcoeff=costcoeff*coeffnorm;		
			energycoeff=energycoeff*coeffnorm;						
			cost[x],energy[x]=runAlloc(config,configname,costcoeff,energycoeff,redundancycoeff,ARGV[1],$opts[:lastyear],layoutclustering,$opts[:usepatchpanels])
	
		end
	end


	STDOUT.puts "\nAfter allocation\nIndex\tCost\tEnergy\n"
	for x in 0..NTries	
		if(cost[x] != nil)
			STDOUT.puts "#{x}\t#{cost[x]}\t#{energy[x]}\n"
		end
	end

  #Call cable optimizer on the last allocation results (alloc_*.in)
  if $opts[:optimizecables]


	optcommand="mincostflow/mincostflow #{configname}"
	optoutput=`#{allocationcommand}`
    optCode=$?
    File.open("optim.log","w").write(optoutput);
	if optCode !=0 
		STDERR.puts "optimization command failed! Output is stored in optim.log."
		exit 4
	end

	
	cost=nil;energy=nil;
	#optoutput.scan(/Synthesis\s+\d+\s+\d+\s+(\d+.\d*)\t\s+(\d+.\d*)\t\s+/).each do |m|	
#		cost=m[0].to_f
#		energy=m[1].to_f
#	end
	STDOUT.puts "Optimization leads to cost=#{cost} and energy=#{energy}" 
	return cost,energy
    


  end
  

		
		
end							

main if $0 == __FILE__
