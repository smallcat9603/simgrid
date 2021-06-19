#!/usr/bin/ruby -Ku
#


require_relative "benchutils.rb"

def main

  currentpath=Dir.getwd
  STDOUT.puts "Clearing make locks from folder #{currentpath}. I will assume this is a configuration folder.."
  BENCHLOCKNAME.each_pair{ |bench,lock|
    lockfile="#{currentpath}/../../../MpiEnv/var/locks/#{lock}.makelock"
    if File.exists?(lockfile)
      FileUtils.rm_rf(lockfile)
      STDOUT.puts "Removed lock for #{bench}."
      else
      STDOUT.puts "#{bench} lock file  not present: #{lockfile}."
    end
  }

end



main if $0 == __FILE__
