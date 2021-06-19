#!/usr/bin/ruby -Ku
#

require "open3"
require "benchmark"
require "timeout"
require "optparse"
require "pp"
require "fileutils"
require_relative "../../../TopologyEnv/extensible/config"


def runCmdBasic(cmd,sign,configpath)
	STDOUT.puts "Running #{sign} with command <#{cmd}> at <#{Dir.pwd}>..."
	logpath=configpath+"/logs"
	begin
		output="#{cmd}\n"+`#{cmd}`
		retval=$?
	rescue Exception => e  
  		puts "Raised "+e.message  
  		puts "With backtrace"+e.backtrace.inspect  
  	end 
	if(!Dir.exists?(logpath))
		Dir.mkdir(logpath);
	end	
	File.write("#{logpath}/#{sign}.log",output)

	if(retval)
		STDOUT.puts "Return value is "+retval.inspect
		return retval.success?
	end
	return false
end
def runCmd(cmd,sign,configpath)
	STDOUT.puts "Running #{sign} with command <#{cmd}> at <#{Dir.pwd}>..."
	logpath=configpath+"/logs"
	if(!Dir.exists?(logpath))
		Dir.mkdir(logpath);
	end	
	logfile=File.open("#{logpath}/#{sign}.log","w")
	logfile.write(cmd+"\n")
	retval=nil
	begin
		Open3.popen2e(cmd){ |streamin,streamouterr,thr|
				streamin.close
				streamouterr.each_char{ |c|
					#Open the log file for each new line, since otherwise changes cannot be seen from other processes. 
					STDOUT.print  c
					logfile.write(c);
					if(c=='\n' || c=='\r')
						STDOUT.flush
						logfile.flush;
					end
				} #end of err stream	
				retval=thr.value					
			} #end of popen
	rescue Exception => e  
  		puts "Raised "+e.message  
  		puts "With backtrace"+e.backtrace.inspect  
  	end 
	logfile.close();
	if(retval)
		STDOUT.puts "Return value is "+retval.inspect
		return retval.success?
	end
	return false
end

def setenv(varname,value)
	ENV[varname]=value;
end



def runCmdWithTime(cmd,sign,configpath,limit)
	STDOUT.puts "Running #{sign} with command <#{cmd}> at <#{Dir.pwd}> and timeout<#{limit}>..."
	logpath=configpath+"/logs"
	if(!Dir.exists?(logpath))
		Dir.mkdir(logpath);
	end	
	logfile=File.open("#{logpath}/#{sign}.log","w")
	logfile.write(cmd+"\n")
	retval=nil
	runtime=nil
	begin

		bench=Thread.new{
			runtime=Benchmark.measure(){ 
			#Timeout::timeout(limit){
				Open3.popen2e(cmd){ |streamin,streamouterr,thr|
					streamin.close
					streamouterr.each_char{ |c|
						#Open the log file for each new line, since otherwise changes cannot be seen from other processes. 
						STDOUT.print  c
						logfile.write(c);
						if(c=='\n' || c=='\r')
							STDOUT.flush
							logfile.flush;
						end
					} #end of err stream	
					retval=thr.value					
				} #end of popen
			#} #End of timeout
			}#end of benchmark
		}

		if(limit>0)
			if(!bench.join(limit))
				bench.kill
				bench.join
				throw Timeout::TimeoutError
			end
		else
			bench.join
		end
		rescue Exception => e  
  		puts "Raised "+e.message  
  		puts "With backtrace"+e.backtrace.inspect  
  	end 
	logfile.close();
	if(retval)
		STDOUT.puts "Return value is "+retval.inspect
		return runtime.total,retval.success?
	end
	return -1,false
end
	 			
#Get this dirty call out of scripts, in case one day we want to do it properly
def getFolderSize(folderpath)

  
	size=`du -c #{folderpath}|tail -n 1| head -c -7`

	return size
end
