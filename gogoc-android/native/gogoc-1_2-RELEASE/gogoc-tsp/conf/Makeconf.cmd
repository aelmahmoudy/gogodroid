@REM = '#####################################################################
@ECHO OFF
REM # $Id: Makeconf.cmd,v 1.1 2009/11/20 16:53:12 jasminko Exp $
REM #
REM #  For license information refer to CLIENT-LICENSE.TXT
REM #
REM # #######################################################################
REM #
SET IN_DIR=%~1
SET OUT_DIR=%~2
SET TEMPLATE=%~3
SET IF_V6V4=%~4
SET IF_V6UDPV4=%~5
SET IF_V4V6=%~6
PATH=%PATH%;c:\perl\bin
perl -x -S %0 %*
SET STATUS=%errorlevel%
IF %STATUS% == 9009 ECHO You do not have Perl in your PATH.
GOTO endofperl
@REM ';     # Perl script starting...
#!perl
#
  use strict;

  #
  # Get environment variables
  #
  my $in_dir=$ENV{'IN_DIR'};
  my $out_dir=$ENV{'OUT_DIR'};
  my $template=$ENV{'TEMPLATE'};
  my $if_v6v4=$ENV{'IF_V6V4'};
  my $if_v6udpv4=$ENV{'IF_V6UDPV4'};
  my $if_v4v6=$ENV{'IF_V4V6'};
  my @content;
  my $line;

  #  
  # Open input file, and read contents.
  #
  open( CONFIN, "$in_dir/gogoc.conf.in" ) or die "Unable to open $in_dir/gogoc.conf.in";
  @content=<CONFIN>;
  close( CONFIN );
  
  #
  # Open output file and perform substitution.
  #
  open( CONFOUT, ">$out_dir/gogoc.conf.sample" ) or die "Unable to open $out_dir/gogoc.conf.sample";

  foreach $line (@content)
  {
    chop($line);
    $line =~ s+\@ifname_v4v6\@+$if_v4v6+;
    $line =~ s+\@ifname_v6udpv4\@+$if_v6udpv4+;
    $line =~ s+\@ifname_v6v4\@+$if_v6v4+;
    $line =~ s+\@conf_template\@+$template+;
    print CONFOUT "$line\n";
  }

  close( CONFOUT );  


__END__

:endofperl
exit %STATUS%
