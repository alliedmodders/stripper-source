#!/usr/bin/perl

use File::Basename;

my ($myself, $path) = fileparse($0);
chdir($path);

require 'helpers.pm';

chdir('..');

open(PDBLOG, '../OUTPUT/pdblog.txt') or die "Could not open pdblog.txt: $!\n";

#Get version info
my ($version);
$version = Build::ProductVersion(Build::PathFormat('product.version'));
$version =~ s/-dev//g;
$version .= '-hg' . Build::HgRevNum('.');

my ($build_type);
$build_type = Build::GetBuildType(Build::PathFormat('buildbot/build_type'));

if ($build_type eq "dev")
{
	$build_type = "buildbot";
}
elsif ($build_type eq "rel")
{
	$build_type = "release";
}

my ($line);
while (<PDBLOG>)
{
	$line = $_;
	$line =~ s/\.pdb/\*/;
	chomp $line;
	Build::Command("symstore add /r /f \"..\\OUTPUT\\$line\" /s \"S:\\stripper\" /t \"Stripper\" /v \"$version\" /c \"$build_type\"");
}

close(PDBLOG);

#Lowercase DLLs.  Sigh.
my (@files);
opendir(DIR, "S:\\stripper");
@files = readdir(DIR);
closedir(DIR);

my ($i, $j, $file, @subdirs);
for ($i = 0; $i <= $#files; $i++)
{
	$file = $files[$i];
	next unless ($file =~ /\.dll$/);
	next unless (-d "S:\\stripper\\$file");
	opendir(DIR, "S:\\stripper\\$file");
	@subdirs = readdir(DIR);
	closedir(DIR);
	for ($j = 0; $j <= $#subdirs; $j++)
	{
		next unless ($subdirs[$j] =~ /[A-Z]/);
		Build::Command("rename S:\\stripper\\$file\\" . $subdirs[$j] . " " . lc($subdirs[$j]));
	}	
}

