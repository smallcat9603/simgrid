#!/bin/sh

#
# 2007-02-25 15:36
#

for TEX_FILE in `ls *.tex` ; do

        rm -f ${TEX_FILE}.doc

        nkf -e ${TEX_FILE} | \

	#
	# Delete "\n"
	#
	sed 's/^$/_NN_/g' | \
	sed 's/\(.\)$/\1 /g' | \

	#
	# Ignore comments
	#
        sed 's/^%.*//' | \
        sed 's/[^\\]%.*//' | \

	#
	# Remove tex commands
	#
        sed 's/\\begin.*/_N_/' | \
        sed 's/\\end.*/_N_/' | \
        sed 's/\\centering.*//' | \
        sed 's/\\item/_N_-/' | \
        sed 's/\\qed//' | \
        sed 's/\\hspace.*//' | \
        sed 's/\\vspace.*//' | \
        sed 's/\\includegraphics.*//' | \
        sed 's/{\\bf \([^}]*\)}/\1/' | \
        sed 's/{\\it \([^}]*\)}/\1/' | \
	sed 's/\\label{.*}//g' | \

	#
	# Extract titles
	#
        sed 's/\\section{\([^}]*\)}/_NN_\1:_NN_/'       | \
        sed 's/\\subsection{\([^}]*\)}/_NN_\1:_NN_/'    | \
        sed 's/\\subsubsection{\([^}]*\)}/_NN_\1:_NN_/' | \
        sed 's/\\paragraph{\([^}]*\)}/_NN_\1:_NN_/'     | \

        tr -s '\t' ' ' | \
        tr -d '\n' | \

	#
	# Replace "_N_" to "\n"
	#
        sed 's/_N_/\n/g' | \
        sed 's/_NN_/\n\n/g' | \

	#
	# For subfigure, caption, etc...
	#
	sed 's/\\subfigure\[\([^]]*\)\].*{.*}/(Figure\/Table) \1/'  | \
	sed 's/\\caption{\([^}]*\)}/(Figure\/Table) \1/'  > \
	${TEX_FILE}.doc

done
