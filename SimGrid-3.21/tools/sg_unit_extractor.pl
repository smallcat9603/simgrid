#! /usr/bin/env perl

# Copyright (c) 2005-2018. The SimGrid Team. All rights reserved.

# This program is free software; you can redistribute it and/or modify it
# under the terms of the license (GNU LGPL) which comes with this package.

use strict;
use Getopt::Long qw(GetOptions);

my $progname="sg_unit_extractor";
# Get the args

sub usage($) {
    my $ret;
    print "USAGE: $progname [--root=part/to/cut] [--outdir=where/to/generate/files] infile [infile+]\n\n";
    print "This program is in charge of extracting the unit tests out of the SimGrid source code.\n";
    print "See http://simgrid.gforge.inria.fr/doc/latest/inside_tests.html for more details.\n";
    exit $ret;
}

my $outdir=undef;
my $root;
my $help;

Getopt::Long::config('permute','no_getopt_compat', 'no_auto_abbrev');
GetOptions(
        'help|h'                => sub {usage(0)},
        'root=s' =>\$root,
        'outdir=s' =>\$outdir) or usage(1);

usage(1) if (scalar @ARGV == 0);

map {process_one($_)} @ARGV;

sub process_one($) {
    my $infile = shift;
    my $outfile;

    $infile =~ s|src/|| unless (-e $infile);

    $outfile =  $infile;
    $outfile =~ s/\.c$/_unit.c/;
    $outfile =~ s/\.cpp$/_unit.cpp/;
    $outfile =~ s|.*/([^/]*)$|$1| if $outfile =~ m|/|;
    $outfile = "$outdir$outfile";

    print "$progname: processing $infile (generating $outfile)...\n";

    # Get the unit data
    my ($unit_source,$suite_name,$suite_title)=("","","");
    my (%tests); # to detect multiple definition
    my (@tests); # actual content

    open IN, "$infile" || die "$progname: Cannot open input file '$infile': $!\n";
    $infile =~ s|$root|| if defined($root);

    my $takeit=0;
    my $line=0;
    my $beginline=0;
    while (<IN>) {
	$line++;
	if (m/ifdef +SIMGRID_TEST/) {
	    $beginline = $line;
	    $takeit = 1;
	    next;
	}
	if (m/endif.*SIMGRID_TEST/) {
	    $takeit = 0;
	    next
	}
	
	if (m/XBT_TEST_SUITE\(\w*"([^"]*)"\w*, *(.*?)\);/) { #" {
	    die "$progname: Multiple suites in the same file ($infile) are not supported yet\n" if length($suite_name);
	    ($suite_name,$suite_title)=($1,$2);
	    die "$progname: Empty suite name in $infile" unless length($suite_name);
	    die "$progname: Empty suite title in $infile" unless length($suite_title);
	    next;
        } elsif (m/XBT_TEST_SUITE/) {
	    die "$progname: Parse error: This line seem to be a test suite declaration, but failed to parse it\n$_\n";
	}

        if (m/XBT_TEST_UNIT\(\w*"([^"]*)"\w*,([^,]*),(.*?)\)/) { #"{
	    die "$progname: multiply defined unit in file $infile: $1\n" if (defined($tests{$1}));

	    my @t=($1,$2,$3);
	    push @tests,\@t;
	    $tests{$1} = 1;
	} elsif (m/XBT_TEST_UNIT/) {
	    die "$progname: Parse error: This line seem to be a test unit, but failed to parse it\n$_\n";
	}
        $unit_source .= $_ if $takeit;
    }
    close IN || die "$progname: cannot close input file '$infile': $!\n";


    if ($takeit) {
	die "$progname: end of file reached in SIMGRID_TEST block.\n".
	  "You should end each of the with a line matching: /endif.*SIMGRID_TEST/\n".
	  "Example:\n".
	  "#endif /* SIMGRID_TEST */\n"
    }

    die "$progname: no suite defined in $infile\n" unless (length($suite_name));

    # Write the test

    my ($GENERATED)=("/*******************************/\n".
	             "/* GENERATED FILE, DO NOT EDIT */\n".
                     "/*******************************/\n\n");
    $beginline+=2;
    open OUT,">$outfile" || die "$progname: Cannot open output file '$outfile': $!\n";
    print OUT $GENERATED;
    print OUT "#include <stdio.h>\n";
    print OUT "#include \"xbt.h\"\n";
    print OUT $GENERATED;
    print OUT "#line $beginline \"$infile\" \n";
    print OUT "$unit_source";
    print OUT $GENERATED;
    close OUT || die "$progname: Cannot close output file '$outfile': $!\n";

    # write the main skeleton if needed
    if (! -e "${outdir}simgrid_units_main.c") {
	open OUT,">${outdir}simgrid_units_main.c" || die "$progname: Cannot open main file '${outdir}simgrid_units_main.c': $!\n";
	print OUT $GENERATED;
	print OUT "#include <stdio.h>\n\n";
	print OUT "#include \"xbt.h\"\n\n";
	print OUT "extern xbt_test_unit_t _xbt_current_unit;\n\n";
	print OUT "#define STRLEN 1024\n";
	print OUT "/* SGU: BEGIN PROTOTYPES */\n";
	print OUT "/* SGU: END PROTOTYPES */\n\n";
	print OUT $GENERATED;
	#  print OUT "# 93 \"sg_unit_extractor.pl\"\n";
	print OUT <<EOF;
int main(int argc, char *argv[]) {
  xbt_test_suite_t suite;
  char selection[STRLEN];
  int verbosity = 0;
  int i;
  int res;

  /* SGU: BEGIN SUITES DECLARATION */
  /* SGU: END SUITES DECLARATION */

  xbt_init(&argc,argv);

  /* Search for the tests to do */
    selection[0]='\\0';
    for (i=1;i<argc;i++) {
      if (!strncmp(argv[i],\"--tests=\",strlen(\"--tests=\"))) {
        char *p=strchr(argv[i],'=')+1;
        if (selection[0] != '\\0')
          strncat(selection, \",\", STRLEN - 1 - strlen(selection));
        strncat(selection, p, STRLEN - 1 - strlen(selection));
      } else if (!strcmp(argv[i], \"--verbose\")) {
        verbosity++;
      } else if (!strcmp(argv[i], \"--dump-only\")||
                 !strcmp(argv[i], \"--dump\")) {
        xbt_test_dump(selection);
        return 0;
      } else if (!strcmp(argv[i], \"--help\")) {
	  printf(
	      "Usage: testall [--help] [--tests=selection] [--dump-only]\\n\\n"
	      "--help: display this help\\n"
	      "--verbose: print the name for each running test\\n"
	      "--dump-only: don't run the tests, but display some debuging info about the tests\\n"
	      "--tests=selection: Use argument to select which suites/units/tests to run\\n"
	      "                   --tests can be used more than once, and selection may be a comma\\n"
	      "                   separated list of directives.\\n\\n"
	      "Directives are of the form:\\n"
	      "   [-]suitename[:unitname]\\n\\n"
	      "If the first char is a '-', the directive disables its argument instead of enabling it\\n"
	      "suitename/unitname is the set of tests to en/disable. If a unitname is not specified,\\n"
	      "it applies on any unit.\\n\\n"
	      "By default, everything is enabled.\\n\\n"
	      "'all' as suite name apply to all suites.\\n\\n"
	      "Example 1: \\"-toto,+toto:tutu\\"\\n"
	      "  disables the whole toto testsuite (any unit in it),\\n"
	      "  then reenables the tutu unit of the toto test suite.\\n\\n"
	      "Example 2: \\"-all,+toto\\"\\n"
	      "  Run nothing but the toto suite.\\n");
	  return 0;
      } else {
        printf("testall: Unknown option: %s\\n",argv[i]);
        return 1;
      }
    }
  /* Got all my tests to do */

  res = xbt_test_run(selection, verbosity);
  xbt_test_exit();
  return res;
}
EOF
	print OUT $GENERATED;
	close OUT || die "$progname: Cannot close main file '${outdir}simgrid_units_main.c': $!\n";
    }

   print "  Suite $suite_name: $suite_title (".(scalar @tests)." tests)\n";
   map {
       my ($name,$func,$title) = @{$_};
       print "    unit $name: func=$func; title=$title\n";
   } @tests;

   #while (my $t = shift @tests) {

   # add this suite to the main
   my $newmain="";
   open IN,"${outdir}simgrid_units_main.c" || die "$progname: Cannot open main file '${outdir}simgrid_units_main.c': $!\n";
    # search prototypes
       while (<IN>) {
	   $newmain .= $_;
	   #    print "Look for proto: $_";
	   last if /SGU: BEGIN PROTOTYPES/;
       }

       # search my prototype
       while (<IN>) {
	   #    print "Seek protos: $_";
	   last if  (/SGU: END PROTOTYPES/ || /SGU: BEGIN FILE $infile/);
	   $newmain .= $_;
       }
       if (/SGU: BEGIN FILE $infile/) { # found an old section for this file. Kill it
	   while (<IN>) {
	       last if /SGU: END FILE/;
	   }
	   $_ = <IN>; # pass extra blank line
	   chomp;
	   die "this line should be blank ($_). Did you edit the file?" if /\W/;
       }
       my ($old_)=($_);
       # add my section
       $newmain .= "  /* SGU: BEGIN FILE $infile */\n";
       map {
	   my ($name,$func,$title) = @{$_};
	   $newmain .=  "    void $func(void);\n"
       } @tests;

       $newmain .= "  /* SGU: END FILE */\n\n";
       if ($old_ =~ /SGU: BEGIN FILE/ || $old_ =~ /SGU: END PROTOTYPES/) {
	   $newmain .= $old_;
       }

       # pass remaining prototypes, search declarations
       while (<IN>) {
	   $newmain .= $_ unless /SGU: END PROTOTYPES/;
	   last if /SGU: BEGIN SUITES DECLARATION/;
       }

       ### Done with prototypes. And now, the actual code

       # search my prototype
       while (<IN>) {
	   last if  (/SGU: END SUITES DECLARATION/ || /SGU: BEGIN FILE $infile/);
	   $newmain .= $_;
       }
       if (/SGU: BEGIN FILE $infile/) { # found an old section for this file. Kill it
	   while (<IN>) {
	       last if /SGU: END FILE/;
	   }
	   $_ = <IN>; # pass extra blank line
	   chomp;
	   die "this line should be blank ($_). Did you edit the file?" if /\W/;
       }
       my ($old_)=($_);
       # add my section
       $newmain .= "    /* SGU: BEGIN FILE $infile */\n";
       $newmain .= "      suite = xbt_test_suite_by_name(\"$suite_name\",$suite_title);\n";
       map {
	   my ($name,$func,$title) = @{$_};
	   $newmain .=  "      xbt_test_suite_push(suite, \"$name\", $func, $title);\n";
       } @tests;

       $newmain .= "    /* SGU: END FILE */\n\n";
       if ($old_ =~ /SGU: BEGIN FILE/ || $old_ =~ /SGU: END SUITES DECLARATION/) {
	   $newmain .= $old_;
       }

       # pass the remaining
       while (<IN>) {
	   $newmain .= $_;
       }
       close IN || die "$progname: Cannot close main file '${outdir}simgrid_units_main.c': $!\n";

       # write it back to main
       open OUT,">${outdir}simgrid_units_main.c" || die "$progname: Cannot open main file '${outdir}simgrid_units_main.c': $!\n";
       print OUT $newmain;
       close OUT || die "$progname: Cannot close main file '${outdir}simgrid_units_main.c': $!\n";
} # end if process_one($)

0;
