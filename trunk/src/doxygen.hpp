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
  @mainpage Exif and Iptc metadata manipulation library and tools v0.5

  @section overview Exiv2 Overview
 
  Exiv2 is a C++ library that provides
  - read and write access to the Exif and Iptc IIM metadata of an image 
    through %Exiv2 keys and standard C++ iterators 
  - Exif MakerNote support:
      - MakerNote tags can be accessed just like any other metadata
      - a sophisticated write algorithm avoids corrupting the MakerNote: 
         <BR>&nbsp;&nbsp;1) the MakerNote is not re-located if possible, and 
         <BR>&nbsp;&nbsp;2) MakerNote IFD offsets are re-calculated if the MakerNote
                            needs to be moved (for known IFD MakerNotes)
      - new camera make/model specific MakerNotes can be added to the library
        with minimum effort in OO-fashion (by subclassing MakerNote or IfdMakerNote)
  - set methods for Exif thumbnails (Jpeg only, Tiff thumbnails can be set from 
    individual tags)
  - extract and delete methods for Exif thumbnails (both, JPEG and TIFF thumbnails)
  - Doxygen API documentation
  - generic lower-level classes to access IFD (Image File Directory) data structures

  %Exiv2 is also a command line program to
  - print the Exif metadata of JPEG images (as summary info, interpreted values, or
    the plain data for each tag)
  - print the Iptc metadata of JPEG images
  - print the Jpeg comment of JPEG images
  - adjust the Exif timestamp (that's how it all started...)
  - rename Exif image files according to the Exif timestamp
  - extract Exif metadata, Iptc metadata and Jpeg comments from image files and 
    insert it again
  - extract the thumbnail image embedded in the Exif metadata
  - delete the thumbnail or the complete Exif metadata from an image

  @section getting-started Getting started

  <A HREF="../getting-started.html">A few pointers</A> to get you started with
  the %Exiv2 library without delay.

  @section metadata Supported Exif and MakerNote tags and Iptc datasets

  - <A HREF="../tags.xml">Standard Exif tags</A>
  - <A HREF="../tags-canon.xml">Canon MakerNote tags</A>
  - <A HREF="../tags-fujifilm.xml">Fujifilm MakerNote tags</A>
  - <A HREF="../tags-nikon1.xml">Nikon (format 1) MakerNote tags</A>
  - <A HREF="../tags-nikon2.xml">Nikon (format 2) MakerNote tags</A>
  - <A HREF="../tags-nikon3.xml">Nikon (format 3) MakerNote tags</A>
  - <A HREF="../tags-sigma.xml">Sigma/Foveon MakerNote tags</A>
  - <A HREF="../iptc.xml">Iptc datasets</A>

  @section makernote MakerNote Formats and Specifications

  A summary of <A HREF="../makernote.html">MakerNote structures</A> with links to 
  publicly available specifications.

  @section devel Development

  <P>%Exiv2 is maintained in a publicly available subversion repository. 
  You can 
  <A TITLE="Online source code repository" HREF="http://dev.robotbattle.com/~cvsuser/cgi-bin/ns_viewcvs.cgi/exiv2/trunk/">browse the code online</A>, 
  see a list of 
  <A TITLE="Last 50 commits" HREF="http://dev.robotbattle.com/cmtinfo_svn.php?r=10">recent code changes</A>, 
  browse the 
  <A title="Issue tracker" HREF="http://dev.robotbattle.com/bugs/main_page.php">bug tracking system</A> 
  and report bugs.</P>
  <P>To download the source code directly from the repository, you need a 
  <A title="Subversion distributions" HREF="http://subversion.tigris.org/project_packages.html">subversion client</A>. 
  If you use a command line client, change to the directory where you want to keep 
  the source code and type <CODE>svn co svn://dev.robotbattle.com/exiv2/trunk .</CODE>
  </P>
  <P>If you'd like to contribute code, please <A HREF="mailto:ahuggel@gmx.net">contact me</A>.

  @section license License

  <P>Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net></P>

  <P>%Exiv2 is free software; you can redistribute it and/or modify it under the
  terms of the <a href="../gpl-license.html">GNU General Public License</a> as
  published by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.</P>
 
  <P>%Exiv2 is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.</P>
 
  <P>You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA 02111-1307, USA.</P>

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
  Sample program to print the Iptc metadata of an image
 */
