# Copyright (c) 2025 Tristan Wellman
# Script to grab Lospec palette colors and put them in a vec3 array for glsl.
# https://lospec.com
use HTTP::Tiny;

print "Enter a Lospec URL: ";
my $url = <STDIN>;
chomp($url);

my $req = HTTP::Tiny->new(agent=>"getPaletteGLSL/1.0")->get($url);
die "ERROR:: Failed to fetch $url" unless $req->{success};
my $html = $req->{content};

my ($palette) = $html =~ m{<article\s+class="palette">(.*?)</article>}si;
die "ERROR:: Could not find palette block at $url" unless defined $palette;

my @hexes = ();
while($palette =~ m{<div\s+class="color"\s+style="[^"]*background:\s*#([0-9A-Fa-f]{6})[^"]*"[^>]*>\s*#\1}gi) {
	push @hexes, $1;
}
die "ERROR:: Empty palette" unless @hexes;
my $size = scalar @hexes;
print "Found $size colors!\n";

# print out in glsl form
print "const vec3 palette[$size] = vec3[$size] (\n";
my $i = 0;
foreach $hex (@hexes) {
	my $r = hex(substr($hex,0,2));
	my $g = hex(substr($hex,2,2));
	my $b = hex(substr($hex,4,2));
	print "\tvec3($r,$g,$b)/255.0";
	if($i<$size-1) {
		print ",";
	}
	# Print a new line every 2 colors to save on some space.
	if(($i+1)%2==0||$i==@hex-1) {
		print "\n";
	} else {
		print " ";
	}
	$i++;
}
print ");\n";
