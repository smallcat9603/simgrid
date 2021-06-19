#!/usr/bin/ruby -Ku
#


require_relative "benchutils.rb"

def main
   STDOUT.puts "Clearing the temporary folder of all simulations" 
   FileUtils.rm_r Dir.glob("/tmp/mpienv*")
end



main if $0 == __FILE__
