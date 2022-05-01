#!/bin/perl/bin/perl.exe

use warnings; 
use strict;
use Data::Dumper;

open(my $input, $ARGV[0] ) or die "Could not open $ARGV[0]";
open(my $output, ">" . $ARGV[1] ) or die "Could not open $ARGV[1]";

my $first = 1;
my $firstline = 1;
my $doxstyle = 0;
my $freestyle = 0;
my $err=0;
my $badline = "";

my $Filename = $ARGV[0];
my $Author = "Probably Klaas Holwerda or Julian Smart";
my $Purpose;
my $Created;
my $Copyright;
my $Licence;
my $Other = "";
my $ModifiedBy = "";

$Filename =~ s{\\}{/}g;
$Filename =~ s{.*/modules/}{};

while( <$input> )
{
    if( m{^ */{20,100}$} )
    {
        # ////// Style header begin / end
        if( ! $first )
        {
            last;
        }
        $first = 0;
    }
    elsif( m{^/\*![ \t]*\\file} )
    {
        # /*! \file Style header begin
        $first = 0;
        $doxstyle = 1;
    }
    elsif( $doxstyle && m{^\*/[ \t]*$} )
    {
        # */ Style header end
        last;
    }
    elsif( $firstline && m{^/\*[ \t]*$} )
    {
        # /* Style header begin
        $first = 0;
        $freestyle = 1;
    }
    elsif( $freestyle && m{^[ \t]*\*/[ \t]*$} )
    {
        # */ Style header end
        last;
    }
    elsif( m{^[ \t]*$} )
    {
        # ignore empty line
    }
    elsif( m{^[ \t]*\*[ \t]*$} )
    {
        # ignore empty line with one *
    }
    elsif( m{^[/\*! \t]*Name:[ \t]*(.*)} )
    {
        # Prefer true filename;
    }
    elsif( m{^[/\*! \t]*(?:Author:|\\author)[ \t]*(.*)} )
    {
        $Author = $1;
    }
    elsif( m{^[/\*! \t]*Author2:[ \t]*(.*)} )
    {
        $Author = $Author . " and " . $1;
    }
    elsif( m{^[/\*! \t]*(?:Purpose:?|\\brief)[ \t]*(.*)} )
    {
        $Purpose = $1;
    }
    elsif( m{^[/\*! \t]*(?:Created:|\\date|\\data)[ \t]*(.*)} )
    {
        if( $1 !~ m{XX/XX/XX} )
        {
            $Created = $1;
        }
    }
    elsif( m{^[/\*! \t]*Copyright:[ \t]*(.*)} )
    {
        my $text = $1;
        $text =~ s{^200([0-9])[ \t]}{200$1-2004 };
        $Copyright = $text;
    }
    elsif( m{^[/\*! \t]*Licence:[ \t]*(.*)} )
    {
        my $text = $1;
        $text =~ s{wxWindows}{wxWidgets};
        $Licence = $text;
    }
    elsif( m{^[/\*! \t]*Modified by:[ \t]*(.*)$} )
    {
        $ModifiedBy = $1;
    }
    elsif( m{^[/\*! \t]*Program:?[ \t]*(.*)} )
    {
        # Ignore (replaced by filename)
    }
    elsif( m{^[/\*! \t]*Last Update:?[ \t]*(.*)} )
    {
        # Ignore (replaced by RCS-ID)
    }
    elsif( m{^[/\*! \t]*(RCS-ID:?)?[ \t]*\$Id.*\$[ \t]*$} )
    {
        # Ignore
    }
    elsif( $doxstyle && m{^[/\*! \t]*(.*)} )
    {
        # Doxygen other text
        $Other = $Other . "\n    " . $1;
    }
    elsif( $firstline )
    {
        # Doesn't look like there is any header at all, so guess one
        $badline = $_;
        last;
    }
    else
    {
        print "##################################################################\n";
        print "$ARGV[0]\n";
        print "?? $_";
        print "##################################################################\n";
        $err = 1;
    }

    if( $firstline && ! m{^[ \t]*$} )
    {
        $firstline = 0;
    }
}

if( $err )
{
    close( $input );
    close( $output );
    exit 1;
}

print $output "/*! \\file $Filename\n";

if( $Purpose )
{
    print $output "    \\brief $Purpose\n";
}

if( $Author )
{
    print $output "    \\author $Author\n";
}

if( $Created )
{
    print $output "    \\date Created $Created\n";
}

if( $Copyright )
{
    print $output "\n    Copyright: $Copyright\n";
}
else
{
    print $output "\n    Copyright: 2001-2004 (C) $Author\n";
}

if( $Licence )
{
    print $output "\n    Licence: $Licence\n";
}
else
{
    print $output "\n    Licence: wxWidgets Licence\n";
}

if( $ModifiedBy )
{
    print $output "\n    Modified by: $ModifiedBy\n";
}

if( $Other )
{
    print $output $Other, "\n";
}

# The $Id: UpdateHeaders.pl,v 1.1 2005/06/01 17:51:19 frm Exp $ has to be obscurified a bit, so that it isn't recognized by CVS/RCS

print $output "\n    RCS-ID: " .'$' . 'Id' . '$' . "\n";

print $output "*/\n";

if( $badline )
{
    print $output "\n";
    print $output $badline;
}

while( <$input> )
{
    print $output $_;
}

close( $input );
close( $output );

exit 0;