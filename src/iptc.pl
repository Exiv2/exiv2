#!/usr/bin/perl -s -w

use XML::Simple qw(:strict);
#use Data::Dumper;

my $data = XMLin(undef, 'KeyAttr' => {'record' => 'number'}, 'ForceArray' => ['record'], 'SuppressEmpty' => '');
#print Dumper($data);
#exit 0;

#print records
foreach $key (sort keys %{$data->{record}}) {
    PrintRecord( $data->{record}->{$key} );
}

sub PrintRecord
{
    my $record = $_[0];
    print '    static const DataSet ' . $record->{type}. "Record[] = {\n";

    foreach $dataset (@{$record->{dataset}}) {
        print '        DataSet(' . 
             $dataset->{number} . ', "' . 
             $dataset->{name} . '", ' . 
             $dataset->{mandatory} . ', ' . 
             $dataset->{repeatable} . ', ' . 
             $dataset->{minbytes} . ', ' . 
             $dataset->{maxbytes} . ', Exiv2::' . 
             $dataset->{type} . ', ' . 
             $record->{type} . ', "' . 
             $dataset->{photoshop} . "\"),\n";
    }

    #end of record marker
    print "        DataSet(0xffff, \"Invalid\", false, false, 0, 0, Exiv2::unsignedShort, $record->{type}, \"\")\n";
    print "    };\n\n";
}


