#!/usr/bin/ruby -Ku
#

require_relative "method"


def main

	thispath=Dir.pwd
	opts=Hash.new;
	
	runImmediateMappingFigure(thispath,opts)

	runMitigateMappingFigure(thispath,opts)
    
#        runBenchesFigure(thispath, opts)

 # runGroupsMappingFigure(thispath,opts)
  
#  runPatchworkMappingFigure(thispath,opts);
  
	return 
	
	
	runStageMappingFigure(thispath,opts)

	runVarMappingFigure(thispath,opts)


end


main if $0 == __FILE__
