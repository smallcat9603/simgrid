#!/usr/bin/ruby -Ku
#


require_relative "benchutils.rb"

def main

  currentpath=Dir.getwd
  STDOUT.puts "Cleaning folder #{currentpath}. I will assume this is a configuration folder.."
  FileUtils.rm_rf Dir.glob("*result*csv")
  FileUtils.rm_rf Dir.glob("results/*csv")
  FileUtils.rm_rf Dir.glob("logs/*")
  FileUtils.rm_rf Dir.glob("platforms/*")

end



main if $0 == __FILE__
