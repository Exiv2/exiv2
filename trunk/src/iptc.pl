#!/usr/bin/perl -s -w

use XML::Simple qw(:strict);
#use Data::Dumper;

my $data = XMLin(undef, 'KeyAttr' => {'record' => 'number'}, 'ForceArray' => ['record'], 'SuppressEmpty' => '');
#print Dumper($data);
#exit 0;

#print dataset ids
foreach $key (sort keys %{$data->{record}}) {
    PrintIds( $data->{record}->{$key} );
}

print "\n";

#print dataset arrays
foreach $key (sort keys %{$data->{record}}) {
    PrintRecord( $data->{record}->{$key} );
}

sub PrintIds
{
    my $record = $_[0];

    foreach $dataset (@{$record->{dataset}}) {
        print '        static const uint16 ' . 
             $dataset->{name} . ' ' x (22-length($dataset->{name})) 
             . ' = ' . $dataset->{number} . ";\n";
    }
}

sub PrintRecord
{
    my $record = $_[0];
    print '    static const DataSet ' . $record->{type}. "Record[] = {\n";

    foreach $dataset (@{$record->{dataset}}) {
        print '        DataSet(' . 
             'IptcDataSets::' . $dataset->{name} . ', "' . 
             $dataset->{name} . '", "' . 
             $dataset->{description} . '", ' . 
             $dataset->{mandatory} . ', ' . 
             $dataset->{repeatable} . ', ' . 
             $dataset->{minbytes} . ', ' . 
             $dataset->{maxbytes} . ', Exiv2::' . 
             $dataset->{type} . ', ' . 
             'IptcDataSets::' . $record->{type} . ', "' . 
             $dataset->{photoshop} . "\"),\n";
    }

    #end of record marker
    print "        DataSet(0xffff, \"(Invalid)\", \"(Invalid)\", false, false, 0, 0, Exiv2::unsignedShort, IptcDataSets::$record->{type}, \"\")\n";
    print "    };\n\n";
}


