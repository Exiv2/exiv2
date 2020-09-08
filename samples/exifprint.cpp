// ***************************************************************** -*- C++ -*-
// exifprint.cpp
// Sample program to print the Exif metadata of an image

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

/* ------------------------------------------------- */
/* Robin's darktable emulator                        */
/* ------------------------------------------------- */
typedef void GList;

static GList* exiv2_taglist = NULL;

static void _get_xmp_tags(const char *prefix, GList** /*taglist*/)
{
  const Exiv2::XmpPropertyInfo *pl = Exiv2::XmpProperties::propertyList(prefix);
  if(pl)
  {
    for (int i = 0; pl[i].name_ != 0; ++i)
    {
      // char *tag = dt_util_dstrcat(NULL, "Xmp.%s.%s,%s", prefix, pl[i].name_, _get_exiv2_type(pl[i].typeId_));
      // *taglist = g_list_prepend(*taglist, tag);
        std::cout << "Xmp." << prefix <<"."<< pl[i].name_ << " __ " << pl[i].typeId_ << std::endl;
    }
  }
}

static void set_exiv2_taglist()
{
    const Exiv2::GroupInfo *groupList = Exiv2::ExifTags::groupList();
    if(groupList)
    {
      while(groupList->tagList_)
      {
          const Exiv2::TagInfo *tagInfo = groupList->tagList_();
          while(tagInfo->tag_ != 0xFFFF)
          {
            //char *tag = dt_util_dstrcat(NULL, "Exif.%s.%s,%s", groupList->groupName_, tagInfo->name_, _get_exiv2_type(tagInfo->typeId_));
            // exiv2_taglist = g_list_prepend(exiv2_taglist, tag);
              std::cout << "Exif." << groupList->groupName_ << "."<<   tagInfo->name_ << " __ " << tagInfo->typeId_ << std::endl;
              tagInfo++;
          }
          groupList++;
      }
    }
    const Exiv2::DataSet *iptcEnvelopeList = Exiv2::IptcDataSets::envelopeRecordList();
    while(iptcEnvelopeList->number_ != 0xFFFF)
    {
      //char *tag = dt_util_dstrcat(NULL, "Iptc.Envelope.%s,%s", iptcEnvelopeList->name_, _get_exiv2_type(iptcEnvelopeList->type_));
      // exiv2_taglist = g_list_prepend(exiv2_taglist, tag);
        
        std::cout << "Iptc.Envelope." << iptcEnvelopeList->name_ << " __ " << iptcEnvelopeList->type_ << std::endl; iptcEnvelopeList++;
        iptcEnvelopeList++;
    }

    const Exiv2::DataSet *iptcApplication2List = Exiv2::IptcDataSets::application2RecordList();
    while(iptcApplication2List->number_ != 0xFFFF)
    {
      //char *tag = dt_util_dstrcat(NULL, "Iptc.Application2.%s,%s", iptcApplication2List->name_, _get_exiv2_type(iptcApplication2List->type_));
      // exiv2_taglist = g_list_prepend(exiv2_taglist, tag);
      std::cout << "Iptc.Application2." <<   iptcApplication2List->name_ << " __ " << iptcApplication2List->type_ << std::endl;

      iptcApplication2List++;
    }
    
    _get_xmp_tags("dc", &exiv2_taglist);
    _get_xmp_tags("xmp", &exiv2_taglist);
    _get_xmp_tags("xmpRights", &exiv2_taglist);
    _get_xmp_tags("xmpMM", &exiv2_taglist);
    _get_xmp_tags("xmpBJ", &exiv2_taglist);
    _get_xmp_tags("xmpTPg", &exiv2_taglist);
    _get_xmp_tags("xmpDM", &exiv2_taglist);
    _get_xmp_tags("pdf", &exiv2_taglist);
    _get_xmp_tags("photoshop", &exiv2_taglist);
    _get_xmp_tags("crs", &exiv2_taglist);
    _get_xmp_tags("tiff", &exiv2_taglist);
    _get_xmp_tags("exif", &exiv2_taglist);
    _get_xmp_tags("exifEX", &exiv2_taglist);
    _get_xmp_tags("aux", &exiv2_taglist);
    _get_xmp_tags("iptc", &exiv2_taglist);
    _get_xmp_tags("iptcExt", &exiv2_taglist);
    _get_xmp_tags("plus", &exiv2_taglist);
    _get_xmp_tags("mwg-rs", &exiv2_taglist);
    _get_xmp_tags("mwg-kw", &exiv2_taglist);
    _get_xmp_tags("dwc", &exiv2_taglist);
    _get_xmp_tags("dcterms", &exiv2_taglist);
    _get_xmp_tags("digiKam", &exiv2_taglist);
    _get_xmp_tags("kipi", &exiv2_taglist);
    _get_xmp_tags("GPano", &exiv2_taglist);
    _get_xmp_tags("lr", &exiv2_taglist);
    _get_xmp_tags("MP", &exiv2_taglist);
    _get_xmp_tags("MPRI", &exiv2_taglist);
    _get_xmp_tags("MPReg", &exiv2_taglist);
    _get_xmp_tags("acdsee", &exiv2_taglist);
    _get_xmp_tags("mediapro", &exiv2_taglist);
    _get_xmp_tags("expressionmedia", &exiv2_taglist);
    _get_xmp_tags("MicrosoftPhoto", &exiv2_taglist);
}

int main(int argc, char** const argv )
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
    
    int rc = 0 ;

    int count = 0 ;    
    if ( argc % 2 ) {
    	for ( int i = 1 ; i < argc ; i += 2 ) {
    	    count++;
    		const char* prefix = argv[i+0];
    		const char* uri    = argv[i+1];
    		std::cout << "registerNS " << prefix << ":" << uri << std::endl;
    		Exiv2::XmpProperties::registerNs(uri,prefix);
    	}
	    set_exiv2_taglist();
    }
    
    if ( !count  && argc > 1 ) {
    	printf("usage: %s [prefix uri]+\n",argv[0]);
    	rc = 1 ;
    }

    return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
