# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyFiles, path


URL = "http://dev.exiv2.org/issues/937"


class DarwinCoreXmpMetadataPrint(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-bug937.jpg")

    commands = [
         "$exiv2 -q -pa      -g dwc  $filename",
         "$exiv2 -q -PXkyctl -g Date $filename",
    ]

    stdout = [
        """Xmp.dwc.Record                               XmpText     0  type="Struct"
Xmp.dwc.Record/dwc:institutionID             XmpText    25  Charles Darwin Foundation
Xmp.dwc.Record/dwc:collectionID              XmpText    29  urn:lsid:biocol.org:col:34818
Xmp.dwc.Record/dwc:institutionCode           XmpText     3  CDS
Xmp.dwc.Record/dwc:datasetID                 XmpText     3  MVZ
Xmp.dwc.Record/dwc:collectionCode            XmpText     7  Mammals
Xmp.dwc.Record/dwc:datasetName               XmpText    25  Grinnell Resurvey Mammals
Xmp.dwc.Record/dwc:ownerInstitutionCode      XmpText     3  NPS
Xmp.dwc.Record/dwc:basisOfRecord             XmpText    14  FossilSpecimen
Xmp.dwc.Record/dwc:informationWithheld       XmpText    53  location information not given for endangered species
Xmp.dwc.Record/dwc:dataGeneralizations       XmpText    90  Coordinates generalized from original GPS coordinates to the nearest half degree grid cell
Xmp.dwc.Record/dwc:dynamicProperties         XmpText    45  targusLengthInMeters=0.014; weightInGrams=120
Xmp.dwc.Occurrence                           XmpText     0  type="Struct"
Xmp.dwc.Occurrence/dwc:occurrenceID          XmpText    28  urn:lsid:nhm.ku.edu:Herps:32
Xmp.dwc.Occurrence/dwc:catalogNumber         XmpText     9  2008.1334
Xmp.dwc.Occurrence/dwc:occurrenceDetails     XmpText    50  http://mvzarctos.berkeley.edu/guid/MVZ:Mamm:165861
Xmp.dwc.Occurrence/dwc:recordNumber          XmpText     8  OPP 7101
Xmp.dwc.Occurrence/dwc:recordedBy            XmpText    17  Oliver P. Pearson
Xmp.dwc.Occurrence/dwc:individualID          XmpText    10  U.amer. 44
Xmp.dwc.Occurrence/dwc:individualCount       XmpText     1  1
Xmp.dwc.Occurrence/dwc:sex                   XmpText     6  female
Xmp.dwc.Occurrence/dwc:lifeStage             XmpText     3  egg
Xmp.dwc.Occurrence/dwc:reproductiveCondition XmpText    16  non-reproductive
Xmp.dwc.Occurrence/dwc:behavior              XmpText     8  roosting
Xmp.dwc.Occurrence/dwc:establishmentMeans    XmpText    10  cultivated
Xmp.dwc.Occurrence/dwc:occurrenceStatus      XmpText     7  present
Xmp.dwc.Occurrence/dwc:preparations          XmpText     4  skin
Xmp.dwc.Occurrence/dwc:disposition           XmpText    13  in collection
Xmp.dwc.Occurrence/dwc:otherCatalogNumbers   XmpText    16  FNMH:Mammal:1234
Xmp.dwc.Occurrence/dwc:previousIdentifications XmpText   109  Anthus sp., field ID by G. Iglesias; Anthus correndera, expert ID by C. Cicero 2009-02-12 based on morphology
Xmp.dwc.Occurrence/dwc:associatedMedia       XmpText    71  http://arctos.database.museum/SpecimenImages/UAMObs/Mamm/2/P7291179.JPG
Xmp.dwc.Occurrence/dwc:associatedReferences  XmpText   153  Christopher J. Conroy, Jennifer L. Neuwald. 2008. Phylogeographic study of the California vole, Microtus californicus Journal of Mammalogy, 89(3):755-767
Xmp.dwc.Occurrence/dwc:associatedOccurrences XmpText    56  sibling of FMNH:Mammal:1234; sibling of FMNH:Mammal:1235
Xmp.dwc.Occurrence/dwc:associatedSequences   XmpText    16  GenBank:U34853.1
Xmp.dwc.Occurrence/dwc:associatedTaxa        XmpText    17  host:Quercus alba
Xmp.dwc.Occurrence/dwc:occurrenceRemarks     XmpText    18  found dead on road
Xmp.dwc.Event                                XmpText     0  type="Struct"
Xmp.dwc.Event/dwc:eventID                    XmpText     4  1234
Xmp.dwc.Event/dwc:samplingProtocol           XmpText   134  Penguins from space: faecal stains reveal the location of emperor penguin colonies, http://dx.doi.org/10.1111/j.1466-8238.2009.00467.x
Xmp.dwc.Event/dwc:samplingEffort             XmpText    14  40 trap-nights
Xmp.dwc.Event/dwc:earliestDate               XmpText    25  2012-09-03T00:00:00-06:00
Xmp.dwc.Event/dwc:latestDate                 XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.Event/dwc:eventTime                  XmpText    25  1899-12-30T01:41:33+03:00
Xmp.dwc.Event/dwc:startDayOfYear             XmpText     1  1
Xmp.dwc.Event/dwc:endDayOfYear               XmpText     3  366
Xmp.dwc.Event/dwc:year                       XmpText     4  2008
Xmp.dwc.Event/dwc:month                      XmpText     2  10
Xmp.dwc.Event/dwc:day                        XmpText     2  31
Xmp.dwc.Event/dwc:habitat                    XmpText    11  oak savanna
Xmp.dwc.Event/dwc:verbatimEventDate          XmpText    11  spring 1910
Xmp.dwc.Event/dwc:fieldNumber                XmpText    16  RV Sol  87-03-08
Xmp.dwc.Event/dwc:fieldNotes                 XmpText    42  notes available in Grinnell-Miller Library
Xmp.dwc.Event/dwc:eventRemarks               LangAlt     1  lang="x-default" after the recent rains the river is nearly at flood stage
Xmp.dwc.dctermsLocation                      XmpText     0  type="Struct"
Xmp.dwc.dctermsLocation/dwc:locationID       XmpText     4  1234
Xmp.dwc.dctermsLocation/dwc:higherGeographyID XmpText    11  TGN:1002002
Xmp.dwc.dctermsLocation/dwc:higherGeography  XmpText    24  South America; Argenitna
Xmp.dwc.dctermsLocation/dwc:continent        XmpText    10  Antarctica
Xmp.dwc.dctermsLocation/dwc:waterBody        XmpText    12  Indian Ocean
Xmp.dwc.dctermsLocation/dwc:islandGroup      XmpText    21  Alexander Archipelago
Xmp.dwc.dctermsLocation/dwc:island           XmpText    13  Isla Victoria
Xmp.dwc.dctermsLocation/dwc:country          XmpText     7  Denmark
Xmp.dwc.dctermsLocation/dwc:countryCode      XmpText     2  AR
Xmp.dwc.dctermsLocation/dwc:stateProvince    XmpText     7  Montana
Xmp.dwc.dctermsLocation/dwc:county           XmpText     8  Missoula
Xmp.dwc.dctermsLocation/dwc:municipality     XmpText    10  Holzminden
Xmp.dwc.dctermsLocation/dwc:locality         XmpText    50  25 km NNEoc  Bariloche along National Road no. 237
Xmp.dwc.dctermsLocation/dwc:verbatimLocality XmpText    35  25 km NNE Bariloche por R. Nac. 237
Xmp.dwc.dctermsLocation/dwc:verbatimElevation XmpText     9  100-200 m
Xmp.dwc.dctermsLocation/dwc:minimumElevationInMeters XmpText     3  100
Xmp.dwc.dctermsLocation/dwc:maximumElevationInMeters XmpText     3  200
Xmp.dwc.dctermsLocation/dwc:verbatimDepth    XmpText     9  100-200 m
Xmp.dwc.dctermsLocation/dwc:minimumDepthInMeters XmpText     3  100
Xmp.dwc.dctermsLocation/dwc:maximumDepthInMeters XmpText     3  100
Xmp.dwc.dctermsLocation/dwc:minimumDistanceAboveSurfaceInMeters XmpText    82  1.5 meter sediment core from the bottom of a lake (at depth 20m) at 300m elevation
Xmp.dwc.dctermsLocation/dwc:maximumDistanceAboveSurfaceInMeters XmpText    82  1.5 meter sediment core from the bottom of a lake (at depth 20m) at 300m elevation
Xmp.dwc.dctermsLocation/dwc:locationAccordingTo XmpText    35  Getty Thesaurus of Geographic Names
Xmp.dwc.dctermsLocation/dwc:locationRemarks  XmpText    22  under water since 2005
Xmp.dwc.dctermsLocation/dwc:verbatimCoordinates XmpText    20  41 05 54S 121 05 34W
Xmp.dwc.dctermsLocation/dwc:verbatimLatitude XmpText    12  41 05 54.03S
Xmp.dwc.dctermsLocation/dwc:verbatimLongitude XmpText    10  121 05 34W
Xmp.dwc.dctermsLocation/dwc:verbatimCoordinateSystem XmpText     3  UTM
Xmp.dwc.dctermsLocation/dwc:verbatimSRS      XmpText     5  WGS84
Xmp.dwc.dctermsLocation/dwc:decimalLatitude  XmpText    11  -41.0983423
Xmp.dwc.dctermsLocation/dwc:decimalLongitude XmpText    12  -121.1761111
Xmp.dwc.dctermsLocation/dwc:geodeticDatum    XmpText     5  WGS84
Xmp.dwc.dctermsLocation/dwc:coordinateUncertaintyInMeters XmpText     2  30
Xmp.dwc.dctermsLocation/dwc:coordinatePrecision XmpText     7  0.00001
Xmp.dwc.dctermsLocation/dwc:pointRadiusSpatialFit XmpText     1  1
Xmp.dwc.dctermsLocation/dwc:footprintWKT     XmpText    45  POLYGON ((10 20, 11 20, 11 21, 10 21, 10 20))
Xmp.dwc.dctermsLocation/dwc:footprintSRS     XmpText    12  GCS_WGS_1984
Xmp.dwc.dctermsLocation/dwc:footprintSpatialFit XmpText     1  1
Xmp.dwc.dctermsLocation/dwc:georeferencedBy  XmpText    17  Kristina Yamamoto
Xmp.dwc.dctermsLocation/dwc:georeferenceProtocol XmpText    45  MaNIS/HerpNet/ORNIS Georeferencing Guidelines
Xmp.dwc.dctermsLocation/dwc:georeferenceSources XmpText    35  USGS 1:24,000 Florence Montana Quad
Xmp.dwc.dctermsLocation/dwc:georeferenceVerificationStatus XmpText    21  requires verification
Xmp.dwc.dctermsLocation/dwc:georeferenceRemarks XmpText    35  assumed distance by road (Hwy. 101)
Xmp.dwc.GeologicalContext                    XmpText     0  type="Struct"
Xmp.dwc.GeologicalContext/dwc:geologicalContextID XmpText     4  1234
Xmp.dwc.GeologicalContext/dwc:earliestEonOrLowestEonothem XmpText    11  Phanerozoic
Xmp.dwc.GeologicalContext/dwc:latestEonOrHighestEonothem XmpText    11  Proterozoic
Xmp.dwc.GeologicalContext/dwc:earliestEraOrLowestErathem XmpText     8  Cenozoic
Xmp.dwc.GeologicalContext/dwc:earliestPeriodOrLowestSystem XmpText     7  Neogene
Xmp.dwc.GeologicalContext/dwc:latestPeriodOrHighestSystem XmpText     8  Tertiary
Xmp.dwc.GeologicalContext/dwc:earliestEpochOrLowestSeries XmpText    11  Pleistocene
Xmp.dwc.GeologicalContext/dwc:latestEpochOrHighestSeries XmpText    14  Ibexian Series
Xmp.dwc.GeologicalContext/dwc:earliestAgeOrLowestStage XmpText     8  Atlantic
Xmp.dwc.GeologicalContext/dwc:latestAgeOrHighestStage XmpText     6  Boreal
Xmp.dwc.GeologicalContext/dwc:lowestBiostratigraphicZone XmpText     6  Boreal
Xmp.dwc.GeologicalContext/dwc:highestBiostratigraphicZone XmpText     8  Atlantic
Xmp.dwc.GeologicalContext/dwc:lithostratigraphicTerms XmpText    10  Supergroup
Xmp.dwc.GeologicalContext/dwc:group          XmpText    16  Notch Peak Group
Xmp.dwc.GeologicalContext/dwc:formation      XmpText    20  Notch Peak Formation
Xmp.dwc.GeologicalContext/dwc:member         XmpText    15  Lava Dam Member
Xmp.dwc.GeologicalContext/dwc:bed            XmpText    12  Lava Dam Bed
Xmp.dwc.Identification                       XmpText     0  type="Struct"
Xmp.dwc.Identification/dwc:identificationID  XmpText     4  1234
Xmp.dwc.Identification/dwc:identifiedBy      XmpText    15  James L. Patton
Xmp.dwc.Identification/dwc:dateIdentified    XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.Identification/dwc:identificationReferences XmpText    50  Aves del Noroeste Patagonico. Christie et al. 2004
Xmp.dwc.Identification/dwc:identificationRemarks XmpText   106  Distinguished between Anthus correndera and Anthus hellmayri based on the comparative lengths of the uñas
Xmp.dwc.Identification/dwc:identificationQualifier XmpText    29  aff. agrifolia var. oxyadenia
Xmp.dwc.Identification/dwc:typeStatus        XmpText   101  holotype of Ctenomys sociabilis. Pearson O. P., and M. I. Christie. 1985. Historia Natural, 5(37):388
Xmp.dwc.Taxon                                XmpText     0  type="Struct"
Xmp.dwc.Taxon/dwc:taxonID                    XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:scientificNameID           XmpText    35  urn:lsid:ipni.org:names:37829-1:1.3
Xmp.dwc.Taxon/dwc:acceptedNameUsageID        XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:parentNameUsageID          XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:nameAccordingToID          XmpText    33  doi:10.1016/S0269-915X(97)80026-2
Xmp.dwc.Taxon/dwc:namePublishedInID          XmpText    29  http://hdl.handle.net/10199/7
Xmp.dwc.Taxon/dwc:taxonConceptID             XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:scientificName             XmpText    19  Ctenomys sociabilis
Xmp.dwc.Taxon/dwc:acceptedNameUsage          XmpText    46  Tamias minimus valid name for Eutamias minimus
Xmp.dwc.Taxon/dwc:parentNameUsage            XmpText     9  Rubiaceae
Xmp.dwc.Taxon/dwc:originalNameUsage          XmpText    36  Gasterosteus saltatrix Linnaeus 1768
Xmp.dwc.Taxon/dwc:nameAccordingTo            XmpText   235  McCranie, J. R., D. B. Wake, and L. D. Wilson. 1996. The taxonomic status of Bolitoglossa schmidti, with comments on the biology of the Mesoamerican salamander Bolitoglossa dofleini (Caudata: Plethodontidae). Carib. J. Sci. 32:395-398.
Xmp.dwc.Taxon/dwc:namePublishedIn            XmpText    68  Pearson O. P., and M. I. Christie. 1985. Historia Natural, 5(37):388
Xmp.dwc.Taxon/dwc:higherClassification       XmpText   124  Animalia;Chordata;Vertebrata;Mammalia;Theria;Eutheria;Rodentia;Hystricognatha;Hystricognathi;Ctenomyidae;Ctenomyini;Ctenomys
Xmp.dwc.Taxon/dwc:kingdom                    XmpText     8  Animalia
Xmp.dwc.Taxon/dwc:phylum                     XmpText     8  Chordata
Xmp.dwc.Taxon/dwc:class                      XmpText    10  Vertebrata
Xmp.dwc.Taxon/dwc:order                      XmpText     8  Mammalia
Xmp.dwc.Taxon/dwc:family                     XmpText     7  Felidae
Xmp.dwc.Taxon/dwc:genus                      XmpText     4  Puma
Xmp.dwc.Taxon/dwc:subgenus                   XmpText     4  Puma
Xmp.dwc.Taxon/dwc:specificEpithet            XmpText     8  concolor
Xmp.dwc.Taxon/dwc:taxonRank                  XmpText    10  subspecies
Xmp.dwc.Taxon/dwc:verbatimTaxonRank          XmpText    12  Agamospecies
Xmp.dwc.Taxon/dwc:infraspecificEpithet       XmpText     8  concolor
Xmp.dwc.Taxon/dwc:scientificNameAuthorship   XmpText    19  (Torr.) J.T. Howell
Xmp.dwc.Taxon/dwc:nomenclaturalCode          XmpText     4  ICBN
Xmp.dwc.Taxon/dwc:vernacularName             LangAlt     1  lang="x-default" Andean Condor
Xmp.dwc.Taxon/dwc:taxonomicStatus            XmpText     7  invalid
Xmp.dwc.Taxon/dwc:nomenclaturalStatus        XmpText    11  nom. ambig.
Xmp.dwc.Taxon/dwc:taxonRemarks               XmpText    39  this name is a mispelling in common use
Xmp.dwc.ResourceRelationship                 XmpText     0  type="Struct"
Xmp.dwc.ResourceRelationship/dwc:resourceRelationshipID XmpText     4  1234
Xmp.dwc.ResourceRelationship/dwc:resourceID  XmpText     4  1234
Xmp.dwc.ResourceRelationship/dwc:relatedResourceID XmpText     4  1234
Xmp.dwc.ResourceRelationship/dwc:relationshipOfResource XmpText    12  duplicate of
Xmp.dwc.ResourceRelationship/dwc:relationshipAccordingTo XmpText    14  Julie Woodruff
Xmp.dwc.ResourceRelationship/dwc:relationshipEstablishedDate XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.ResourceRelationship/dwc:relationshipRemarks XmpText    30  pollinator captured in the act
Xmp.dwc.MeasurementOrFact                    XmpText     0  type="Struct"
Xmp.dwc.MeasurementOrFact/dwc:measurementID  XmpText     4  1234
Xmp.dwc.MeasurementOrFact/dwc:measurementType XmpText    11  tail length
Xmp.dwc.MeasurementOrFact/dwc:measurementValue XmpText     2  45
Xmp.dwc.MeasurementOrFact/dwc:measurementAccuracy XmpText     4  0.01
Xmp.dwc.MeasurementOrFact/dwc:measurementUnit XmpText     2  mm
Xmp.dwc.MeasurementOrFact/dwc:measurementDeterminedDate XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.MeasurementOrFact/dwc:measurementDeterminedBy XmpText    18  Javier de la Torre
Xmp.dwc.MeasurementOrFact/dwc:measurementMethod XmpText    20  barometric altimeter
Xmp.dwc.MeasurementOrFact/dwc:measurementRemarks XmpText    19  tip of tail missing
""",
        """Xmp.exif.DateTimeDigitized                   Date and Time Digitized        XmpText    29  2008-03-14T11:31:48.098-07:00
Xmp.exif.DateTimeOriginal                    Date and Time Original         XmpText    25  2008-03-14T13:59:26-06:00
Xmp.photoshop.DateCreated                    Date Created                   XmpText    29  2008-03-14T13:59:26.054-06:00
Xmp.xmp.MetadataDate                         Metadata Date                  XmpText    29  2013-02-07T21:56:33.820-06:00
Xmp.xmp.CreateDate                           Create Date                    XmpText    24  2008-03-14T20:59:26.535Z
Xmp.xmp.ModifyDate                           Modify Date                    XmpText    25  2013-01-27T14:02:29-06:00
Xmp.dwc.Event/dwc:earliestDate               Event/dwc:earliestDate         XmpText    25  2012-09-03T00:00:00-06:00
Xmp.dwc.Event/dwc:latestDate                 Event/dwc:latestDate           XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.Event/dwc:verbatimEventDate          Event/dwc:verbatimEventDate    XmpText    11  spring 1910
Xmp.dwc.ResourceRelationship/dwc:relationshipEstablishedDate ResourceRelationship/dwc:relationshipEstablishedDate XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.MeasurementOrFact/dwc:measurementDeterminedDate MeasurementOrFact/dwc:measurementDeterminedDate XmpText    25  2013-01-27T00:00:00-06:00
"""
    ]

    stderr = [""] * 2

    retval = [0] * 2


@CopyFiles("$data_path/exiv2.dc.dwc.i18n.jpg")
class DarwinCoreXmpMetadataModify(metaclass=CaseMeta):

    filename = path("$data_path/exiv2.dc.dwc.i18n_copy.jpg")
    dataname = path("$data_path/exiv2.dc.dwc.i18n.txt")

    commands = [
        "$exiv2 -pa $filename",
        "$exiv2 -m $dataname $filename",
        "$exiv2 -pa $filename"
    ]

    stdout = [
        "",
        "",
        """Xmp.dwc.Record                               XmpText     0  type="Struct"
Xmp.dwc.Record/dwc:basisOfRecord             XmpText    14  FossilSpecimen
Xmp.dwc.Record/dwc:collectionCode            XmpText     7  Mammals
Xmp.dwc.Record/dwc:collectionID              XmpText    29  urn:lsid:biocol.org:col:34818
Xmp.dwc.Record/dwc:dataGeneralizations       XmpText    23  Coordinates generalized
Xmp.dwc.Record/dwc:datasetID                 XmpText     3  MVZ
Xmp.dwc.Record/dwc:datasetName               XmpText    17  Grinnell Resurvey
Xmp.dwc.Record/dwc:informationWithheld       XmpText    20  location information
Xmp.dwc.Record/dwc:institutionCode           XmpText     3  CDS
Xmp.dwc.Record/dwc:institutionID             XmpText    14  Charles Darwin
Xmp.dwc.Record/dwc:ownerInstitutionCode      XmpText     3  NPS
Xmp.dwc.Record/dwc:dynamicProperties         XmpBag      1  targusLengthInMeters=0.014; weightInGrams=120
Xmp.dwc.Occurrence                           XmpText     0  type="Struct"
Xmp.dwc.Occurrence/dwc:behavior              XmpText     8  roosting
Xmp.dwc.Occurrence/dwc:catalogNumber         XmpText     9  2008.1334
Xmp.dwc.Occurrence/dwc:disposition           XmpText    13  in collection
Xmp.dwc.Occurrence/dwc:establishmentMeans    XmpText    10  cultivated
Xmp.dwc.Occurrence/dwc:individualCount       XmpText     2  23
Xmp.dwc.Occurrence/dwc:individualID          XmpText    10  U.amer. 44
Xmp.dwc.Occurrence/dwc:lifeStage             XmpText     3  egg
Xmp.dwc.Occurrence/dwc:occurrenceDetails     XmpText    50  http://mvzarctos.berkeley.edu/guid/MVZ:Mamm:165861
Xmp.dwc.Occurrence/dwc:occurrenceID          XmpText    28  urn:lsid:nhm.ku.edu:Herps:32
Xmp.dwc.Occurrence/dwc:occurrenceRemarks     XmpText    10  found dead
Xmp.dwc.Occurrence/dwc:occurrenceStatus      XmpText     7  present
Xmp.dwc.Occurrence/dwc:recordNumber          XmpText     8  OPP 7101
Xmp.dwc.Occurrence/dwc:reproductiveCondition XmpText    16  non-reproductive
Xmp.dwc.Occurrence/dwc:sex                   XmpText     6  female
Xmp.dwc.Occurrence/dwc:associatedMedia       XmpBag      1  http://arctos.database.museum/SpecimenImages/UAMObs/Mamm/2/P7291179.JPG
Xmp.dwc.Occurrence/dwc:associatedOccurrences XmpBag      1  sibling of
Xmp.dwc.Occurrence/dwc:associatedReferences  XmpBag      1  Christopher J.	89(3):755-76
Xmp.dwc.Occurrence/dwc:associatedSequences   XmpBag      1  GenBank:U34853.1
Xmp.dwc.Occurrence/dwc:associatedTaxa        XmpBag      1  host:Quercus alba
Xmp.dwc.Occurrence/dwc:otherCatalogNumbers   XmpBag      1  FNMH:Mammal:1234
Xmp.dwc.Occurrence/dwc:preparations          XmpBag      1  skin
Xmp.dwc.Occurrence/dwc:previousIdentifications XmpBag      1  Anthus sp.,
Xmp.dwc.Occurrence/dwc:recordedBy            XmpBag      1  Oliver P.
Xmp.dwc.Event                                XmpText     0  type="Struct"
Xmp.dwc.Event/dwc:day                        XmpText     2  31
Xmp.dwc.Event/dwc:earliestDate               XmpText    25  2012-09-03T00:00:00-06:00
Xmp.dwc.Event/dwc:latestDate                 XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.Event/dwc:endDayOfYear               XmpText     3  366
Xmp.dwc.Event/dwc:eventID                    XmpText     4  1234
Xmp.dwc.Event/dwc:eventTime                  XmpText    14  01:41:33+03:00
Xmp.dwc.Event/dwc:eventDate                  XmpText    25  1899-12-30T01:41:33+03:00
Xmp.dwc.Event/dwc:fieldNotes                 XmpText    15  notes available
Xmp.dwc.Event/dwc:fieldNumber                XmpText     6  RV Sol
Xmp.dwc.Event/dwc:habitat                    XmpText    11  oak savanna
Xmp.dwc.Event/dwc:month                      XmpText     2  10
Xmp.dwc.Event/dwc:samplingEffort             XmpText    14  40 trap-nights
Xmp.dwc.Event/dwc:samplingProtocol           XmpText    13  Penguins from
Xmp.dwc.Event/dwc:startDayOfYear             XmpText     1  1
Xmp.dwc.Event/dwc:verbatimEventDate          XmpText    11  spring 1910
Xmp.dwc.Event/dwc:year                       XmpText     4  1999
Xmp.dwc.Event/dwc:eventRemarks               XmpText    16  lang=en-us after
Xmp.dwc.dctermsLocation                      XmpText     0  type="Struct"
Xmp.dwc.dctermsLocation/dwc:continent        XmpText    10  Antarctica
Xmp.dwc.dctermsLocation/dwc:coordinatePrecision XmpText     7  0.00001
Xmp.dwc.dctermsLocation/dwc:coordinateUncertaintyInMeters XmpText     2  30
Xmp.dwc.dctermsLocation/dwc:country          XmpText     7  Denmark
Xmp.dwc.dctermsLocation/dwc:countryCode      XmpText     2  AR
Xmp.dwc.dctermsLocation/dwc:county           XmpText     8  Missoula
Xmp.dwc.dctermsLocation/dwc:decimalLatitude  XmpText    11  -41.0983423
Xmp.dwc.dctermsLocation/dwc:decimalLongitude XmpText    12  -121.1761111
Xmp.dwc.dctermsLocation/dwc:footprintSRS     XmpText    12  GCS_WGS_1984
Xmp.dwc.dctermsLocation/dwc:footprintSpatialFit XmpText     1  1
Xmp.dwc.dctermsLocation/dwc:footprintWKT     XmpText    12  POLYGON ((10
Xmp.dwc.dctermsLocation/dwc:geodeticDatum    XmpText     5  WGS84
Xmp.dwc.dctermsLocation/dwc:georeferenceProtocol XmpText    34  MaNIS/HerpNet/ORNIS Georeferencing
Xmp.dwc.dctermsLocation/dwc:georeferenceRemarks XmpText    16  assumed distance
Xmp.dwc.dctermsLocation/dwc:georeferenceVerificationStatus XmpText    21  requires verification
Xmp.dwc.dctermsLocation/dwc:higherGeographyID XmpText    11  TGN:1002002
Xmp.dwc.dctermsLocation/dwc:island           XmpText    13  Isla Victoria
Xmp.dwc.dctermsLocation/dwc:islandGroup      XmpText    21  Alexander Archipelago
Xmp.dwc.dctermsLocation/dwc:locality         XmpText     5  25 km
Xmp.dwc.dctermsLocation/dwc:locationAccordingTo XmpText    15  Getty Thesaurus
Xmp.dwc.dctermsLocation/dwc:locationID       XmpText     7  EBT3000
Xmp.dwc.dctermsLocation/dwc:locationRemarks  XmpText    11  under water
Xmp.dwc.dctermsLocation/dwc:maximumDepthInMeters XmpText     3  100
Xmp.dwc.dctermsLocation/dwc:maximumDistanceAboveSurfaceInMeters XmpText     9  1.5 meter
Xmp.dwc.dctermsLocation/dwc:maximumElevationInMeters XmpText     3  200
Xmp.dwc.dctermsLocation/dwc:minimumDepthInMeters XmpText     4  25.5
Xmp.dwc.dctermsLocation/dwc:minimumDistanceAboveSurfaceInMeters XmpText     9  1.5 meter
Xmp.dwc.dctermsLocation/dwc:minimumElevationInMeters XmpText     3  100
Xmp.dwc.dctermsLocation/dwc:municipality     XmpText    10  Holzminden
Xmp.dwc.dctermsLocation/dwc:pointRadiusSpatialFit XmpText     1  1
Xmp.dwc.dctermsLocation/dwc:stateProvince    XmpText     7  Montana
Xmp.dwc.dctermsLocation/dwc:verbatimCoordinateSystem XmpText     3  UTM
Xmp.dwc.dctermsLocation/dwc:verbatimCoordinates XmpText     4  41 5
Xmp.dwc.dctermsLocation/dwc:verbatimDepth    XmpText     9  100-200 m
Xmp.dwc.dctermsLocation/dwc:verbatimElevation XmpText     9  100-200 m
Xmp.dwc.dctermsLocation/dwc:verbatimLatitude XmpText     4  41 5
Xmp.dwc.dctermsLocation/dwc:verbatimLocality XmpText     5  25 km
Xmp.dwc.dctermsLocation/dwc:verbatimLongitude XmpText     5  121 5
Xmp.dwc.dctermsLocation/dwc:verbatimSRS      XmpText     5  WGS84
Xmp.dwc.dctermsLocation/dwc:waterBody        XmpText    12  Indian Ocean
Xmp.dwc.dctermsLocation/dwc:georeferenceSources XmpBag      1  USGS 1:24,000
Xmp.dwc.dctermsLocation/dwc:georeferencedBy  XmpBag      1  Kristina Yamamoto
Xmp.dwc.dctermsLocation/dwc:higherGeography  XmpBag      1  South America;
Xmp.dwc.GeologicalContext                    XmpText     0  type="Struct"
Xmp.dwc.GeologicalContext/dwc:bed            XmpText     8  Lava Dam
Xmp.dwc.GeologicalContext/dwc:earliestAgeOrLowestStage XmpText     8  Atlantic
Xmp.dwc.GeologicalContext/dwc:earliestEonOrLowestEonothem XmpText    11  Phanerozoic
Xmp.dwc.GeologicalContext/dwc:earliestEpochOrLowestSeries XmpText    11  Pleistocene
Xmp.dwc.GeologicalContext/dwc:earliestEraOrLowestErathem XmpText     8  Cenozoic
Xmp.dwc.GeologicalContext/dwc:earliestPeriodOrLowestSystem XmpText     7  Neogene
Xmp.dwc.GeologicalContext/dwc:formation      XmpText    10  Notch Peak
Xmp.dwc.GeologicalContext/dwc:geologicalContextID XmpText     4  1234
Xmp.dwc.GeologicalContext/dwc:group          XmpText    10  Notch Peak
Xmp.dwc.GeologicalContext/dwc:highestBiostratigraphicZone XmpText     8  Atlantic
Xmp.dwc.GeologicalContext/dwc:latestAgeOrHighestStage XmpText     6  Boreal
Xmp.dwc.GeologicalContext/dwc:latestEonOrHighestEonothem XmpText    11  Proterozoic
Xmp.dwc.GeologicalContext/dwc:latestEpochOrHighestSeries XmpText    14  Ibexian Series
Xmp.dwc.GeologicalContext/dwc:latestPeriodOrHighestSystem XmpText     8  Tertiary
Xmp.dwc.GeologicalContext/dwc:lithostratigraphicTerms XmpText    10  Supergroup
Xmp.dwc.GeologicalContext/dwc:lowestBiostratigraphicZone XmpText     6  Boreal
Xmp.dwc.GeologicalContext/dwc:member         XmpText     8  Lava Dam
Xmp.dwc.Identification                       XmpText     0  type="Struct"
Xmp.dwc.Identification/dwc:dateIdentified    XmpText    25  2011-01-27T00:17:00-18:00
Xmp.dwc.Identification/dwc:identificationID  XmpText     4  1234
Xmp.dwc.Identification/dwc:identificationQualifier XmpText    14  aff. agrifolia
Xmp.dwc.Identification/dwc:identificationRemarks XmpText    21  Distinguished between
Xmp.dwc.Identification/dwc:identificationReferences XmpBag      1  Aves del
Xmp.dwc.Identification/dwc:identifiedBy      XmpBag      1  James L.
Xmp.dwc.Identification/dwc:typeStatus        XmpBag      1  holotype of
Xmp.dwc.Taxon                                XmpText     0  type="Struct"
Xmp.dwc.Taxon/dwc:acceptedNameUsage          XmpText    14  Tamias minimus
Xmp.dwc.Taxon/dwc:acceptedNameUsageID        XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:class                      XmpText    10  Vertebrata
Xmp.dwc.Taxon/dwc:family                     XmpText     7  Felidae
Xmp.dwc.Taxon/dwc:genus                      XmpText     4  Puma
Xmp.dwc.Taxon/dwc:infraspecificEpithet       XmpText     8  concolor
Xmp.dwc.Taxon/dwc:kingdom                    XmpText     8  Animalia
Xmp.dwc.Taxon/dwc:nameAccordingTo            XmpText    21  McCranie, J. comments
Xmp.dwc.Taxon/dwc:nameAccordingToID          XmpText    33  doi:10.1016/S0269-915X(97)80026-2
Xmp.dwc.Taxon/dwc:namePublishedIn            XmpText    10  Pearson O.
Xmp.dwc.Taxon/dwc:namePublishedInID          XmpText    29  http://hdl.handle.net/10199/7
Xmp.dwc.Taxon/dwc:namePublishedInYear        XmpText     4  2059
Xmp.dwc.Taxon/dwc:nomenclaturalCode          XmpText     4  ICBN
Xmp.dwc.Taxon/dwc:nomenclaturalStatus        XmpText    11  nom. ambig.
Xmp.dwc.Taxon/dwc:order                      XmpText     8  Mammalia
Xmp.dwc.Taxon/dwc:originalNameUsage          XmpText    22  Gasterosteus saltatrix
Xmp.dwc.Taxon/dwc:parentNameUsage            XmpText     9  Rubiaceae
Xmp.dwc.Taxon/dwc:parentNameUsageID          XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:phylum                     XmpText     8  Chordata
Xmp.dwc.Taxon/dwc:scientificName             XmpText    19  Ctenomys sociabilis
Xmp.dwc.Taxon/dwc:scientificNameAuthorship   XmpText    12  (Torr.) J.T.
Xmp.dwc.Taxon/dwc:scientificNameID           XmpText    35  urn:lsid:ipni.org:names:37829-1:1.3
Xmp.dwc.Taxon/dwc:specificEpithet            XmpText     8  concolor
Xmp.dwc.Taxon/dwc:subgenus                   XmpText     4  Puma
Xmp.dwc.Taxon/dwc:taxonConceptID             XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:taxonID                    XmpText    36  8fa58e08-08de-4ac1-b69c-1235340b7001
Xmp.dwc.Taxon/dwc:taxonRank                  XmpText    10  subspecies
Xmp.dwc.Taxon/dwc:taxonRemarks               XmpText    24  lang=en-us this name ...
Xmp.dwc.Taxon/dwc:taxonomicStatus            XmpText     7  invalid
Xmp.dwc.Taxon/dwc:verbatimTaxonRank          XmpText    12  Agamospecies
Xmp.dwc.Taxon/dwc:higherClassification       XmpBag      1  Animalia;Chordata;Vertebrata;Mammalia;Theria;Eutheria
Xmp.dwc.Taxon/dwc:vernacularName             LangAlt     2  lang="es-ES" Puma, lang="en-US" Cougar
Xmp.dwc.MeasurementOrFact                    XmpText     0  type="Struct"
Xmp.dwc.MeasurementOrFact/dwc:measurementAccuracy XmpText     4  0.01
Xmp.dwc.MeasurementOrFact/dwc:measurementDeterminedDate XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.MeasurementOrFact/dwc:measurementID  XmpText     4  1234
Xmp.dwc.MeasurementOrFact/dwc:measurementMethod XmpText    20  barometric altimeter
Xmp.dwc.MeasurementOrFact/dwc:measurementRemarks XmpText     6  tip of
Xmp.dwc.MeasurementOrFact/dwc:measurementType XmpText    11  tail length
Xmp.dwc.MeasurementOrFact/dwc:measurementUnit XmpText     2  mm
Xmp.dwc.MeasurementOrFact/dwc:measurementValue XmpText     2  45
Xmp.dwc.MeasurementOrFact/dwc:measurementDeterminedBy XmpBag      1  Javier de
Xmp.dwc.ResourceRelationship                 XmpText     0  type="Struct"
Xmp.dwc.ResourceRelationship/dwc:relatedResourceID XmpText     4  1234
Xmp.dwc.ResourceRelationship/dwc:relationshipAccordingTo XmpText    14  Julie Woodruff
Xmp.dwc.ResourceRelationship/dwc:relationshipEstablishedDate XmpText    25  2013-01-27T00:00:00-06:00
Xmp.dwc.ResourceRelationship/dwc:relationshipOfResource XmpText    12  duplicate of
Xmp.dwc.ResourceRelationship/dwc:relationshipRemarks XmpText    19  pollinator captured
Xmp.dwc.ResourceRelationship/dwc:resourceID  XmpText     4  1234
Xmp.dwc.ResourceRelationship/dwc:resourceRelationshipID XmpText     4  1234
Xmp.dc.language                              XmpBag      1  latin
Xmp.dc.rights                                LangAlt     2  lang="es-ES" CC Alan Pater, lang="en-US" Alan Pater CC
"""
    ]
    stderr = [""] * 3
    retval = [0] * 3
