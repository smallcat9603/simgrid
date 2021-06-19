#!/usr/bin/ruby -Ku
#


require_relative "benchutils.rb"

def main
  currentpath=Dir.getwd
  STDOUT.puts "Clearing replay traces from folder #{currentpath}. I will assume this is a configuration folder.."
  FileUtils.rm_rf Dir.glob("#{currentpath}/../../../MpiEnv/var/locks/*")
end



main if $0 == __FILE__
