#!/bin/perl

# Copyright (c) 2013-2014. The SimGrid Team.
# All rights reserved.

# This program is free software; you can redistribute it and/or modify it
# under the terms of the license (GNU LGPL) which comes with this package.

# C noncomment spell checker
# For each given source file, print the filename, a colon, and the number
# of misspelled words, then a list of misspelled words.
# Words contained in the file stopwords.txt are not considered spelling errors.
# Copyright 2003, Dan Kegel.  Licensed under GPL.  See the file ../COPYING for details.

sub check_content($) {
	my $content = shift;
	$content =~ tr/*/ /;
	print POUT "$content\n";
}

$TEMPFILE="/tmp/spell.tmp";
$STOPFILE=shift(@ARGV);

open(STOPFILE, $STOPFILE) || die "can't open stopword file $STOPFILE";
while (<STOPFILE>) {
	chomp;
	$stopped{$_}++;
}
close(STOPFILE);

foreach $file (@ARGV) {
	open (FI, $file) || die $file;
	$content = join ("", <FI>);
	close (FI);

	open(POUT, "> $TEMPFILE") || die;
	$content =~ s!//(.+)$! !gm;
	$content =~ s!/\*(.+?)\*/! !gs;
	print POUT $content;
	close(POUT);

	open(PIN, "ispell -d american -l < $TEMPFILE | sort -uf |") || die;
	undef @badwords;
	while (<PIN>) {
		chomp;
		if ($stopped{$_} == 0) {
			push(@badwords, $_);
		}
	}
	close(PIN) || die;

	if (@badwords) {
		print "$file: ".scalar(@badwords)."\n\n";
		print join(" ",@badwords)."\n\n";
	}
}
