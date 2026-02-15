# -*- coding: utf-8 -*-

import system_tests

class FujiTags(metaclass=system_tests.CaseMeta):

    filesAndExpectedOutput = [
("FujiTagsDRangeAutoRating1.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  0
Exif.Fujifilm.HighlightTone                  SLong       1  0
Exif.Fujifilm.Rating                         Long        1  1
Exif.Fujifilm.DRangePriority                 Short       1  Auto
Exif.Fujifilm.DRangePriorityAuto             Short       1  Weak
""")
,
("FujiTagsDRangeWeakRating2.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  0
Exif.Fujifilm.HighlightTone                  SLong       1  0
Exif.Fujifilm.Rating                         Long        1  2
Exif.Fujifilm.DRangePriority                 Short       1  Fixed
Exif.Fujifilm.DRangePriorityFixed            Short       1  Weak
""")
,
("FujiTagsDRangeStrongRating3.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  0
Exif.Fujifilm.HighlightTone                  SLong       1  0
Exif.Fujifilm.Rating                         Long        1  3
Exif.Fujifilm.DRangePriority                 Short       1  Fixed
Exif.Fujifilm.DRangePriorityFixed            Short       1  Strong
"""),
("FujiTagsSTone0HTone0Rating4.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  0
Exif.Fujifilm.HighlightTone                  SLong       1  0
Exif.Fujifilm.Rating                         Long        1  4
"""),
("FujiTagsSTone1HTone-1Rating5.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  +1
Exif.Fujifilm.HighlightTone                  SLong       1  -1
Exif.Fujifilm.Rating                         Long        1  5
"""),
("FujiTagsSTone4HTone-2.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  +4
Exif.Fujifilm.HighlightTone                  SLong       1  -2
Exif.Fujifilm.Rating                         Long        1  0
"""),
("FujiTagsSTone-2HTone4.jpg",
"""Exif.Fujifilm.ShadowTone                     SLong       1  -2
Exif.Fujifilm.HighlightTone                  SLong       1  +4
Exif.Fujifilm.Rating                         Long        1  0
""")
]

    tags = ["Exif.Fujifilm.ShadowTone",
            "Exif.Fujifilm.HighlightTone",
            "Exif.Fujifilm.Rating",
            "Exif.Fujifilm.DRangePriority",
            "Exif.Fujifilm.DRangePriorityAuto",
            "Exif.Fujifilm.DRangePriorityFixed"]

    tagcmd = "$exiv2 -K " + " -K ".join(tags)
    commands = ["$tagcmd $data_path/" + f for f, _ in filesAndExpectedOutput]
    stdout = [e for _, e in filesAndExpectedOutput]
    stderr = [""] * len(filesAndExpectedOutput)
    retval = [0] * len(filesAndExpectedOutput)
