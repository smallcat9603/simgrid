#!/usr/bin/ruby -Ku
# 
#	Generate Simgrid platform configuration files from an edgelist file
#	Fabien Chaix	<fabien_chaix@nii.ac.jp>  
#	Ikki Fujiwara <ikki@nii.ac.jp>
# 

OVERHEAD_OUTER = 400 # Inter-rack cabling overhead [cm]
OVERHEAD_INNER = 200 # Intra-rack cabling overhead [cm]

NODEPOWER = 1E9
CABLEBW= 40E9
SWITCHBW= 1E12
SWITCHLAT=100E-9

# Energy consumption per packet
def latency_of(dist)
	len=(dist == 0) ? OVERHEAD_INNER : OVERHEAD_OUTER + dist
	if len <= 700 # [cm]
		len*1E-11 #Copper cable
	else
		len*0.3E-11 # Optical cable
	end
end

$help_message = <<-EOM
Usage:
  #{$0} [Options] Input_file Output_file_base

Options:

Input file:
  the edge file

Output file:
  the (S)MPI platform basefile (Output_file_base.txt)
  the Simgrid platform description (Output_file_base.xml)

EOM

#' 

require "optparse"
require "time"
require "rexml/document"
require_relative "../../../TopologyEnv/extensible/config"
include REXML

def checkfile(filename,type)
	if !File.size?(filename)
		STDERR.puts "'#{filename}' of type #{type} cannot be found, maybe not in the same directory?"
		exit(1)
	end
end
	
def main
	$opts = Hash.new
	opt = OptionParser.new
	opt.on("--nodeperswitch N_NODE_PER_SWITCH") do |v|
		$opts[:nodeperswitch] = v
	end
	opt.parse!(ARGV)
	unless ARGV[0] && ARGV[1]  #&& ARGV[2]
		puts $help_message
		exit(1)
	end
	
	edgefile=ARGV[0]
	outputbase=ARGV[1]
	
	#===========================================================
	#   Input
	#===========================================================
	
	# Read clustering log file
	if $opts[:nodeperswitch]
		nodeperswitch=$opts[:nodeperswitch].to_i;	
	else
		nodeperswitch=1;
	end
	checkfile(edgefile,"edgefile")
	conn = MyMatrix.new.load_edgelist(edgefile)


	#Generate dummy (S)MPI platform file
	File.open("#{outputbase}.txt", 'w')  { |file| 
		for i in 0...conn.row_size
			for j in 1..nodeperswitch
				file.write("c-#{i*nodeperswitch+j}\n")
			end 
		end
	}
		
	#Generate SimGrid platform
	simgridfile = File.new( "#{outputbase}.xml", 'w'  )
	doc = Document.new('<?xml version="1.0"?>
<!DOCTYPE platform SYSTEM "http://simgrid.gforge.inria.fr/simgrid.dtd">
<platform version="3">
<AS  id="AS0"  routing="Full">
</AS>
</platform>
' )

	platformas= doc.root.elements[1];

	#STDERR.puts doc.root.elements[1].inspect
		
	#Generate one cluster per switch (and associated nodes)
	for i in 0...conn.row_size
		platformas.add_element "cluster", {
			"id" => "switch#{i}", "prefix" => "c-", "suffix" => "",
			"radical" => "#{i*nodeperswitch+1}-#{(i+1)*nodeperswitch}",
#			"router_id" =>  "switch#{i}",
			"power" => NODEPOWER,
			"bw" => CABLEBW,
			"lat" => latency_of(0),
			"bb_bw" => SWITCHBW,
			"bb_lat" => SWITCHLAT
			}	
	end
	
	
	#Generate links
	conn.each_link{ |i,j,dist|
		platformas.add_element "link", {
			"id" => "link#{i}e#{j}",
			"bandwidth" => CABLEBW,
			"latency" => latency_of(dist)
			}

	}

	#Generate  routes (in the sense of simgrid)
	conn.each_link{ |i,j,dist|
		asroute=platformas.add_element "bypassASroute", {
			"src" => "switch#{i}",
			"dst" => "switch#{j}",
         "gw_src" => "c-switch#{i}_router",
         "gw_dst" => "c-switch#{j}_router"}

          
      asroute.add_element "link_ctn", { "id" => "link#{i}e#{j}"}

	}
		
	doc.write(simgridfile,1,false);	

end
			

main if $0 == __FILE__
