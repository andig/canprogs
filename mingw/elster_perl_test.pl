
use strict;
use warnings;
use elster_perl;

my $a = elster_perl::getname("000a");
print "name: $a\n"; # name: DATUM

$a = elster_perl::gettype("000a");
print "type: $a\n"; # type: 12

elster_perl::set_can232();
elster_perl::setdev("COM4");

$a = elster_perl::setvalue("680 180 000a 150a");
print "value: $a\n"; # value: 1

$a = elster_perl::getvalue("680 180 000a");
print "value: $a\n"; # value: 150a

$a = elster_perl::getstring("680 180 000a");
print "value: $a\n"; # value: 21.10.

# $a = elster_perl::toggle_trace();
# print "value: $a\n"; # value: true

$a = elster_perl::setsniffedframe("180 000a");
print "value: $a\n"; # value: 1

$a = elster_perl::setbits("680 180 000a 0201");
print "value: $a\n"; # value: 1

$a = elster_perl::getstring("680 180 000a");
print "value: $a\n"; # value: 23.11.

$a = elster_perl::clrbits("680 180 000a 0201");
print "value: $a\n"; # value: 1

$a = elster_perl::getstring("680 180 000a");
print "value: $a\n"; # value: 21.10.

$a = elster_perl::setstring("680 180 000a 18.12.");
print "value: $a\n"; # value: 1

$a = elster_perl::getstring("680 180 000a");
print "value: $a\n"; # value: 18.12.

$a = elster_perl::getsniffedvalue("180 000a");
print "value: $a\n"; # value: 1