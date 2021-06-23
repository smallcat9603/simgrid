#!/usr/bin/ruby -Ku
# 
#	Generate Simgrid platform configuration files from an edgelist file
#	Fabien Chaix	<fabien_chaix@nii.ac.jp>  
#	Ikki Fujiwara <ikki@nii.ac.jp>
# Yao Hu <huyao@nii.ac.jp> for use of SimGrid v3.2X
# 

OVERHEAD_OUTER = 400 # Inter-rack cabling overhead [cm]
OVERHEAD_INNER = 200 # Intra-rack cabling overhead [cm]

#Default values 
NODEPOWER = 5E12 # 1E9
CABLEBW= 100E9 # 100E6
SWITCHBW= 5E12 # 125E6
SWITCHLAT= 100E-9 # 50E-6
DEFAULTDISTANCE = 500 #Default distance between racks

# Latency
def latency_of(dist)
  len=(dist == 0) ? OVERHEAD_INNER : OVERHEAD_OUTER + dist
  if len <= 300 # [cm]
    #Copper cable
    len*4.4E-9 #HP BladeSystem c-Class 40G QSFP+ to QSFP+ Direct Attach Copper Cable
  else
    # Optical cable 
    len*5.2E-9 #Speed of light in glass= 190000km/sec
  end
end

$help_message = <<-EOM
Usage:
  #{$0} [Options] Input_file Output_file_base

Options:
  --cfg optionName:optionValue
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
require "socket"
require_relative "config"
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
  opt.on("--cfg OPTIONPAIR") do |v|
    option=v.split(":")
    $opts[option[0].to_sym] = option[1];
  end

  opt.parse!(ARGV)
  unless ARGV[0] && ARGV[1]  #&& ARGV[2]
    puts $help_message
    exit(1)
  end

  edgefile=ARGV[0]
  outputbase=ARGV[1]

  generateplatform(edgefile,outputbase,$opts)
end

#**************************************************
#Generate a Simgrid platform from an edge file
#
#There are also many optional parameters (set in opts[:xxxxxx]) :
#nodeperswitch: The number of compute nodes attached to one switch
#corepernode: The number of core per simulated compute node
#networkmodel: The network model used by simgrid. Usually SMPI, but you may try IB for Infiniband or NS3 it set-up.
#collselector: The selector for MPI collectives
#machinepower: The estimated power of the machine running the simulation in Flops/sec. Script tries to make an educated guess based on machine name
#power: The power of one core of simulated compute nodes in Flop/s 
#cablebw: The bandwith of cable in bit/s
#switchbw: The bandwith of switches in bit/s
#switchlat: The latency of switches in seconds
#defaultdistance: The default distance for cables in meters
#unidirectional: If set, the edge file is considered unidirectional. I.e. cable will only transfer data from source to destination in of the edge file. If not set, script consider that edgefiles ending with ".unidir.edges" and "bFalse.edges" are unidirectional files 
#addproperties: Additional properties as a table of string pairs (For in-line parameters, use the following syntax: prop1=prop1val,prop2=prop2val,prop3=prop3val) 
#tracing: If set, tracing is enabled for display on viva/vite/...
#**************************************************

def generateplatform(edgefile,platformbase,opts)

  #===========================================================
  #   Check inputs
  #===========================================================
  
  nodeperswitch=1;
  if opts.has_key?(:nodeperswitch)
    nodeperswitch=opts[:nodeperswitch].to_i;	
  end
  corepernode=1;
  if opts.has_key?(:corepernode)
    corepernode=opts[:corepernode].to_i;	
  end
  if !opts.has_key?(:networkmodel)
    opts[:networkmodel]="SMPI"
  end
  if !opts.has_key?(:collselector)
    opts[:collselector]="mpich"
  end
  if !opts.has_key?(:machinepower)
    # case Socket.gethostname
    # when /calc[0-9]/
    #   opts[:machinepower]=NODEPOWER #50e9
    #   STDOUT.puts "Detected calcXX computer, hence I will assume host has #{opts[:machinepower]} Flops/sec power."
    # else
    #   opts[:machinepower]=NODEPOWER #1e9
    #   STDOUT.puts "Unknown computer, hence using baseline host power #{opts[:machinepower]} Flops/sec"
    # end
    opts[:machinepower]=NODEPOWER #50e9
  end
  if !opts.has_key?(:power)
    opts[:power]=NODEPOWER
  end
  if !opts.has_key?(:cablebw)
    opts[:cablebw]=CABLEBW
  end
  if !opts.has_key?(:switchbw)
    opts[:switchbw]=SWITCHBW
  end
  if !opts.has_key?(:switchlat)
    opts[:switchlat]=SWITCHLAT
  end
  if !opts.has_key?(:defaultdistance)
    opts[:defaultdistance]=DEFAULTDISTANCE
  end
  if !opts.has_key?(:unidirectional) && (edgefile.end_with?(".unidir.edges") || edgefile.end_with?("bFalse.edges"))  
    opts[:unidirectional]=true;
  end
	
  checkfile(edgefile,"edgefile")
  conn = MyMatrix.new.load_edgelist(edgefile,!(opts[:unidirectional]));
  
  #Not sure that the edge file provides the physical distance between racks.. So we say that if a length is below default distance, we force the default distance  
  conn.map{ |d|
    if(d>0 && d< opts[:defaultdistance] )
      d=opts[:defaultdistance]
    end
  }
  STDOUT.puts "Found #{conn.row_size} nodes in file #{edgefile}"
  
  #Generate SimGrid platform
  simgridfile = File.new( "#{platformbase}.xml", 'w'  )

  #Generate the platform configuration element string

  configString=
  "<?xml version='1.0'?>
  <!DOCTYPE platform SYSTEM 'http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd'>
  <platform version='4.1'>
  <config>
  <prop id='maxmin/precision' value='1e-4'/> 
  <prop id='network/model' value='#{opts[:networkmodel]}'/>
  <!--  Negative values enable auto-select... -->
  <prop id='contexts/nthreads' value='1'/>
  <!--  Power of the executing computer in Flop per seconds. Used for extrapolating tasks execution time by SMPI [default is 20000]-->
  <prop id='smpi/host-speed' value='#{opts[:machinepower]}f'/>
  <!--  Display simulated timing at the end of simulation -->
  <prop id='smpi/display-timing' value='1'/>
  <prop id='cpu/optim' value='Lazy'/>
  <prop id='network/optim' value='Lazy'/>
  <prop id='smpi/coll-selector' value='#{opts[:collselector]}'/>
  <prop id='smpi/cpu-threshold' value='0.00000001'/>"

  if(opts[:addproperties])
    if(opts[:addproperties].class=="String")
      props=opts[:addproperties].split(",");
      opts[:addproperties]=Array.new
      props.each{ |pair| opts[:addproperties].push(pair.split("="))}
    end
    opts[:addproperties].each{ |e| e.each{ |propname,propvalue|
        configString+='<prop id='#{propname}' value='#{propvalue}' />\n'
      }  
    }
  end

  if opts[:tracing]
    configString+="
  <prop id='tracing' value='yes'/> 
  <prop id='tracing/smpi' value='yes'/> 
  <prop id='tracing/smpi/computing' value='yes'/>"
  end

  configString+="
  </config>
  <AS  id='AS0'  routing='Dijkstra'>
  </AS>
  </platform>"

  connectedSwitches=[]
  doc = Document.new(configString)

  platformas= doc.root.elements[2];

  #STDERR.puts doc.root.elements[1].inspect
  
  #platformas.add_element(Rexml::Comment("List of switches and associated hosts"))
  #Generate one switch and associated nodes/links
  for i in 0...conn.row_size
    #Check whether this node is completely  disconnected. If yes, discard it with warning

    haslink=false
    conn.each_link{ |li,lj,dist|
      if li == i || lj == i
        haslink=true
        break
      end
    }

    if !haslink
      STDERR.puts "WARNING: Could not find links onward node #{i}, could be your topology does not include all nodes in [0,#{conn.row_size}[ (which is OK), or could be that the input topology is disconnected (which is probably NOT OK)! This node will not be included in platform."
      next
    end

    connectedSwitches.push(i)
    
    for j in 1..nodeperswitch
      platformas.add_element "host", {
        "id" => "n#{i*nodeperswitch+j}",
        "speed" => "#{opts[:power]}f",
        "core" => corepernode
      }	
      platformas.add_element "link", {
        "id" => "linkn#{i*nodeperswitch+j}s#{i}",
        "bandwidth" => "#{opts[:cablebw]}Bps",
        "latency" => "#{latency_of(0)}s"
      }	
    end

    platformas.add_element "link", {
      "id" => "ls#{i}",
      "bandwidth" => "#{opts[:switchbw]}Bps",
      "latency" => "#{opts[:switchlat]}s"
    }	

    platformas.add_element "router", {
      "id" => "s#{i}"
    }
    #Generate  routes -Step 1-
  end  
    
  #Generate inter-switch links
  conn.each_link{ |i,j,dist|
    platformas.add_element "link", {
      "id" => "links#{i}s#{j}",
      "bandwidth" => "#{opts[:cablebw]}Bps",
      "latency" => "#{latency_of(dist)}s"
    }

  }
 
  #Generate  routes
  #For each physical link, we nee to traverse:
  #-> A Simgrid link representing the physical link with id linksxxsyy or linknxxsyy
  #-> A Simgrid link representing the limitations of the physical router/switch with id lsxxx
  #Since we need to cross the link emulating physical switch only once (we chose on input), we need non symmetrical routes.
  #And naturally, unidirectionnal topoloies are created only by not setting up the reciprocal route (from yy to xx)

  connectedSwitches.each{ |i|
    #for switch to node
    for j in 1..nodeperswitch
      route=platformas.add_element "route", {
        "src" => "n#{i*nodeperswitch+j}",
        "dst" => "s#{i}",
        "symmetrical" => "NO"
      }
      route.add_element "link_ctn", { "id" => "linkn#{i*nodeperswitch+j}s#{i}"}
      route.add_element "link_ctn", { "id" => "ls#{i}"}
      route2=platformas.add_element "route", {
        "src" => "s#{i}",
        "dst" => "n#{i*nodeperswitch+j}",
        "symmetrical" => "NO"
      }
      route2.add_element "link_ctn", { "id" => "linkn#{i*nodeperswitch+j}s#{i}"}
    end

  }
    
  #for inter-switches	
  conn.each_link{ |i,j,dist|
    route=platformas.add_element "route", {
      "src" => "s#{i}",
      "dst" => "s#{j}",
      "symmetrical" => "NO"
    }
    route.add_element "link_ctn", { "id" => "links#{i}s#{j}"}
    route.add_element "link_ctn", { "id" => "ls#{j}"}

    if !opts[:unidirectional]
      route2=platformas.add_element "route", {
        "src" => "s#{j}",
        "dst" => "s#{i}",
        "symmetrical" => "NO"
      }
      route2.add_element "link_ctn", { "id" => "links#{i}s#{j}"}
      route2.add_element "link_ctn", { "id" => "ls#{i}"}
    end

  }

  doc.write(simgridfile,1,false);	

  #Generate dummy (S)MPI platform file
  File.open("#{platformbase}.txt", 'w')  { |file| 
    connectedSwitches.each{ |i|
      for j in 1..nodeperswitch
        file.write("n#{i*nodeperswitch+j}:#{corepernode}\n")
      end 
    }
  }

end
  
main if $0 == __FILE__
