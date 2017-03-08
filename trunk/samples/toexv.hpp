// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2017 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @version $Rev: 3777 $
 */
#ifndef _TOEXC_HPP_
#define _TOEXC_HPP_

class Params : public Util::Getopt {
private:
    std::string optstring_;
    bool first_;

public:
    bool help_;                    //!< Help option flag.
    bool iptc_;                    //!< Iptc option flag.
    bool exif_;                    //!< Exif option flag.
    bool ICC_ ;                    //!< ICC option flag.
    bool all_ ;                    //!< All option flag
    bool comment_;                 //!< JPEG comment option flag.
    bool xmp_;                     //!< XMP option flag.
    bool size_;                    //!< Size option flag.
    bool usage_;                   //!< Usage option flag.
    std::string read_;             //!< Source file
    std::string write_;            //!< Destination file

public:
    /*!
      @brief Default constructor. Note that optstring_ is initialized here.
     */
    Params( const char* opts);

    /*!
      @brief Call Getopt::getopt() with optstring, to initiate command line
             argument parsing, perform consistency checks after all command line
             arguments are parsed.

      @param argc Argument count as passed to main() on program invocation.
      @param argv Argument array as passed to main() on program invocation.

      @return 0 if successful, >0 in case of errors.
     */
    int getopt(int argc, char* const argv[]);

    //! Handle options and their arguments.
    virtual int option(int opt, const std::string& optarg, int optopt);

    //! Handle non-option parameters.
    virtual int nonoption(const std::string& argv);

    //! Print a minimal usage note to an output stream.
    int usage(std::ostream& os =std::cout) const;

    //! Print further usage explanations to an output stream.
    int help(std::ostream& os =std::cout) const;

    //! copy metadata from one image to another.
	void copyMetadata(Exiv2::Image::AutoPtr& readImage,Exiv2::Image::AutoPtr& writeImage);

}; // class Params

#endif // _TOEXV_HPP_
