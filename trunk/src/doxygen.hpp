// ***************************************************************** -*- C++ -*-
/*!
  @file    doxygen.hpp
  @brief   Additional documentation, this file contains no source code
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Feb-04, ahu: created
 */
/*!
  @mainpage Exif and IPTC metadata library and tools v0.12

  @section overview Exiv2 Overview

  %Exiv2 comprises of a C++ library and a command line utility to access image
  metadata. %Exiv2 is free software. The homepage of %Exiv2 is at
  <A HREF="http://www.exiv2.org/">http://www.exiv2.org/</A>.

  The %Exiv2 library provides
  - full read and write access to the Exif and IPTC metadata of an image through
    %Exiv2 keys and standard C++ iterators
  - a smart IPTC implementation that does not affect data that programs like
    Photoshop store in the same image segment
  - Exif <A HREF="makernote.html">MakerNote</A> support:
    - %MakerNote tags can be accessed just like any other Exif metadata
    - a sophisticated write algorithm avoids corrupting the %MakerNote:
      <br>&nbsp;&nbsp;1) the %MakerNote is not re-located if possible at all, and
      <br>&nbsp;&nbsp;2) %MakerNote %Ifd offsets are re-calculated if the
                         %MakerNote needs to be moved (for known %Ifd %MakerNotes)
  - extract and delete methods for Exif thumbnails (both, JPEG and TIFF thumbnails)
  - set methods for Exif thumbnails (JPEG only, TIFF thumbnails can be set from
    individual tags)
  - complete API documentation

  @section getting-started Getting started

  <A HREF="getting-started.html">A few pointers</A> to get you started with
  the %Exiv2 library without delay.

  @section metadata Exif and MakerNote tags and IPTC datasets

  - <A HREF="tags-exif.html">Standard Exif tags</A>
  - <A HREF="tags-canon.html">Canon MakerNote tags</A>
  - <A HREF="tags-fujifilm.html">Fujifilm MakerNote tags</A>
  - <A HREF="tags-minolta.html">Minolta MakerNote tags</A>
  - <A HREF="tags-nikon.html">Nikon MakerNote tags</A>
  - <A HREF="tags-olympus.html">Olympus MakerNote tags</A>
  - <A HREF="tags-panasonic.html">Panasonic MakerNote tags</A>
  - <A HREF="tags-sigma.html">Sigma/Foveon MakerNote tags</A>
  - <A HREF="tags-sony.html">Sony MakerNote tags</A>
  - <A HREF="tags-iptc.html">IPTC datasets</A>

  @section formats File Formats

  - JPEG
  - <A HREF="crw-mapping.html">Canon CRW</A>
  - TIFF and related RAW formats (Canon CR2, Nikon NEF, Pentax PEF, Sony SR2, Sony ARW, Adobe DNG, and Minolta MRW), read-only
  - PNG, read-only

  @section supp Support

  <p>Please write to the
  <a title="Exiv2 forum" href="http://uk.groups.yahoo.com/group/exiv2">Yahoo! group for Exiv2 help and discussions</a>.</p>
  <p><a title="Join the Exiv2 forum" href="http://uk.groups.yahoo.com/group/exiv2/join">Join the group</a> to post and receive messages or use <em><a title="Post to the Exiv2 forum" href="http://uk.groups.yahoo.com/group/exiv2/post">exiv2user/password</a></em> to post anonymously. Bug reports can be submitted directly to the
<a title="Issue tracker" href="http://dev.robotbattle.com/bugs/main_page.php"
  onclick="document.images['tracker'].src='http://www.whatcounter.com/dlcount.php?id=ahu&amp;url='+this.href">
bug tracking system</a>.</p>

  @section devel Development

  <p>%Exiv2 is maintained in a publicly available subversion repository.
  There is a <a title="Last 50 commits" href="http://dev.robotbattle.com/cmtinfo_svn.php?r=10">
  live list with the latest commits</a> to the repository, and you can
  <a title="Online source code repository" href="http://dev.robotbattle.com/~cvsuser/cgi-bin/ns_viewcvs.cgi/exiv2/trunk/">
  browse the source code</a> online.
  </p>
  <p>To check-out the current source code from the repository, you need a
  <a title="Get Subversion!"
  href="http://subversion.tigris.org/project_packages.html">subversion client</a>.
  If you use a command line client, change to the directory where you want to keep
  the source code and type:</p>

  @verbatim $ svn checkout svn://dev.robotbattle.com/exiv2/trunk . @endverbatim

  <p>To download the test data and test drivers for version 0.12 from
  the repository, change to your local exiv2-0.12 directory and use the
  following command:
  </p>

  @verbatim $ svn export svn://dev.robotbattle.com/exiv2/tags/0.12/test @endverbatim

  <P>If you'd like to contribute code, please <A HREF="mailto:ahuggel@gmx.net">contact me</A>.

  @section license License

  <P>Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net></P>

  <P>%Exiv2 is free software; you can redistribute it and/or modify it under the
  terms of the <a href="gpl-license.html">GNU General Public License</a> as
  published by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.</P>

  <P>Alternatively, %Exiv2 is also available with a commercial license, which
  allows it to be used in closed-source projects.
  <A HREF="mailto:ahuggel@gmx.net">Contact me</A> for more information.</P>

  <P>%Exiv2 is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.</P>

  <P>You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.</P>
*/
/*!
  @example addmoddel.cpp
  Sample usage of high-level metadata operations.
 */
/*!
  @example exifprint.cpp
  Sample program to print Exif data from an image.
 */
/*!
  @example exifcomment.cpp
  Sample program showing how to set the Exif comment of an image.
 */
/*!
  @example iptcprint.cpp
  Sample program to print the IPTC metadata of an image
 */
/*!
  @example iptceasy.cpp
  The quickest way to access, set or modify IPTC metadata
 */
