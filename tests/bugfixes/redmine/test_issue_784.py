# -*- coding: utf-8 -*-

import system_tests


@system_tests.CopyFiles("$data_path/exiv2-bug784.jpg")
class WritingXmpLrHierarchicalSubjectWritesWrongDatatype(
        metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/784"

    filename = "$data_path/exiv2-bug784_copy.jpg"

    commands = [
        "$exiv2 -px $filename",
        "$exiv2 -M\"add Xmp.lr.hierarchicalSubject root|1st|2nd|3rd|4th|5th\" $filename",
        "$exiv2 -px $filename",
    ]

    stdout = [
        """Xmp.dc.format                                XmpText    10  image/jpeg
Xmp.dc.rights                                LangAlt     1  lang="x-default" Public Domain. Do whatever you like with this image
Xmp.dc.subject                               XmpBag      6  1st, 2nd, next1, next2, root, root0
Xmp.xmpMM.DocumentID                         XmpText    32  004D48F936062EF5085A81BF96D4C494
Xmp.xmpMM.OriginalDocumentID                 XmpText    32  004D48F936062EF5085A81BF96D4C494
Xmp.xmpMM.InstanceID                         XmpText    44  xmp.iid:f74f0d02-e921-134e-8107-1dda17aad853
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[1]/stEvt:instanceID        XmpText    44  xmp.iid:f74f0d02-e921-134e-8107-1dda17aad853
Xmp.xmpMM.History[1]/stEvt:when              XmpText    25  2015-03-24T20:35:55-05:00
Xmp.xmpMM.History[1]/stEvt:softwareAgent     XmpText    39  Adobe Photoshop Lightroom 4.4 (Windows)
Xmp.xmpMM.History[1]/stEvt:changed           XmpText     9  /metadata
Xmp.xmp.MetadataDate                         XmpText    25  2015-03-24T20:35:55-05:00
Xmp.crs.RawFileName                          XmpText    12  exiv2.lr.jpg
Xmp.lr.hierarchicalSubject                   XmpBag      2  root0|next1|next2, root|1st|2nd
""",
        "",
        """Xmp.dc.format                                XmpText    10  image/jpeg
Xmp.dc.rights                                LangAlt     1  lang="x-default" Public Domain. Do whatever you like with this image
Xmp.dc.subject                               XmpBag      6  1st, 2nd, next1, next2, root, root0
Xmp.xmpMM.DocumentID                         XmpText    32  004D48F936062EF5085A81BF96D4C494
Xmp.xmpMM.OriginalDocumentID                 XmpText    32  004D48F936062EF5085A81BF96D4C494
Xmp.xmpMM.InstanceID                         XmpText    44  xmp.iid:f74f0d02-e921-134e-8107-1dda17aad853
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[1]/stEvt:instanceID        XmpText    44  xmp.iid:f74f0d02-e921-134e-8107-1dda17aad853
Xmp.xmpMM.History[1]/stEvt:when              XmpText    25  2015-03-24T20:35:55-05:00
Xmp.xmpMM.History[1]/stEvt:softwareAgent     XmpText    39  Adobe Photoshop Lightroom 4.4 (Windows)
Xmp.xmpMM.History[1]/stEvt:changed           XmpText     9  /metadata
Xmp.xmp.MetadataDate                         XmpText    25  2015-03-24T20:35:55-05:00
Xmp.crs.RawFileName                          XmpText    12  exiv2.lr.jpg
Xmp.lr.hierarchicalSubject                   XmpBag      1  root|1st|2nd|3rd|4th|5th
"""
    ]

    stderr = [""] * 3
    retval = [0] * 3
