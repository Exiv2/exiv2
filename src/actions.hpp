// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  @file    actions.hpp
  @brief   Implements base class Task, TaskFactory and the various supported
           actions (derived from Task).
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Dec-03, ahu: created
 */
#pragma once

// *****************************************************************************
// included header files
#include "params.hpp"

#include <exiv2/image.hpp>
#include <exiv2/preview.hpp>

#include <map>

// *****************************************************************************
// namespace extensions
/*!
  @brief Contains all action classes (task subclasses).
 */
namespace Action
{
    //! Enumerates all tasks
    enum TaskType
    {
        none,
        adjust,
        print,
        rename,
        erase,
        extract,
        insert,
        modify,
        fixiso,
        fixcom
    };

    // *****************************************************************************
    // class definitions

    //! @brief Interface for all concrete actions.
    class Task
    {
    public:
        //! Shortcut for an auto pointer.
        virtual ~Task();
        //! Virtual copy construction.
        std::unique_ptr<Task> clone() const;
        //! @brief Application interface to perform a task.
        //! @param path Path of the file to process.
        //! @return 0 if successful.
        virtual int run(const std::string& path) = 0;

    private:
        //! Internal virtual copy constructor.
        virtual Task* clone_() const = 0;
    };

    //! @brief Task factory.
    //! Creates an instance of the task of the requested type.  The factory is implemented as a singleton, which can
    //! be accessed only through the static member function instance().
    class TaskFactory
    {
    public:
        //! @brief Get access to the task factory.
        //! Clients access the task factory exclusively through this method.
        static TaskFactory& instance();

        //! @brief Create a task.
        //! @param  type Identifies the type of task to create.
        //! @return An auto pointer that owns a task of the requested type. If the task type is not supported,
        //! the pointer is 0.
        //! @remark The caller of the function should check the content of the returned auto pointer and take
        //! appropriate action (e.g., throw an exception) if it is 0.
        std::unique_ptr<Task> create(TaskType type);

    private:
        //! Prevent construction other than through instance().
        TaskFactory();
        ~TaskFactory();

    public:
        TaskFactory& operator=(const TaskFactory& rhs) = delete;
        TaskFactory& operator=(const TaskFactory&& rhs) = delete;
        TaskFactory(const TaskFactory& rhs) = delete;
        TaskFactory(const TaskFactory&& rhs) = delete;

    private:
        //! List of task types and corresponding prototypes.
        std::map<TaskType, Task*> registry_;
    };

    //! %Print the Exif (or other metadata) of a file to stdout
    class Print : public Task
    {
    public:
        ~Print() override;
        int run(const std::string& path) override;
        std::unique_ptr<Print> clone() const;

    private:
        Print* clone_() const override;

        //! Print the Jpeg comment
        int printComment();
        //! Print list of available preview images
        int printPreviewList();
        //! Print Exif summary information
        int printSummary();
        //! Print Exif, IPTC and XMP metadata in user defined format
        int printList();
        //! Return true if key should be printed, else false
        bool grepTag(const std::string& key);
        //! Return true if key should be printed, else false
        bool keyTag(const std::string& key);
        //! Print all metadata in a user defined format
        int printMetadata(const Exiv2::Image* image);
        //! Print a metadatum in a user defined format, return true if something was printed
        bool printMetadatum(const Exiv2::Metadatum& md, const Exiv2::Image* image);
        //! Print the label for a summary line
        void printLabel(const std::string& label) const;
        //! @brief Print one summary line with a label (if provided) and requested data.
        //! A line break is printed only if a label is provided.
        //! @return 1 if a line was written, 0 if the key was not found.
        int printTag(const Exiv2::ExifData& exifData, const std::string& key, const std::string& label = "") const;
        //! Type for an Exiv2 Easy access function
        using EasyAccessFct = Exiv2::ExifData::const_iterator (*)(const Exiv2::ExifData&);
        //! @brief Print one summary line with a label (if provided) and requested data.
        //! A line break is printed only if a label is provided.
        //! @return 1 if a line was written, 0 if the information was not found.
        int printTag(const Exiv2::ExifData& exifData, EasyAccessFct easyAccessFct, const std::string& label) const;

        std::string path_;
        int align_;  // for the alignment of the summary output
    };

    //! @brief Rename a file to its metadate creation timestamp, in the specified format.
    class Rename : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<Rename> clone() const;

    private:
        Rename* clone_() const override;
    };

    //! %Adjust the Exif (or other metadata) timestamps
    class Adjust : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<Adjust> clone() const;

    private:
        Adjust* clone_() const override;
        int adjustDateTime(Exiv2::ExifData& exifData, const std::string& key, const std::string& path) const;

        long adjustment_;
        long yearAdjustment_;
        long monthAdjustment_;
        long dayAdjustment_;
    };

    //! @brief Erase the entire exif data or only the thumbnail section.
    class Erase : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<Erase> clone() const;

        //! @brief Delete the thumbnail image, incl IFD1 metadata from the file.
        int eraseThumbnail(Exiv2::Image* image) const;
        //! @brief Erase the complete Exif data block from the file.
        int eraseExifData(Exiv2::Image* image) const;
        //! @brief Erase all Iptc data from the file.
        int eraseIptcData(Exiv2::Image* image) const;
        //! @brief Erase Jpeg comment from the file.
        int eraseComment(Exiv2::Image* image) const;
        //! @brief Erase XMP packet from the file.
        int eraseXmpData(Exiv2::Image* image) const;
        //! @brief Erase ICCProfile from the file.
        int eraseIccProfile(Exiv2::Image* image) const;

    private:
        Erase* clone_() const override;
        std::string path_;
    };

    //! @brief %Extract the entire exif data or only the thumbnail section.
    class Extract : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<Extract> clone() const;

        //! @brief Write the thumbnail image to a file. The filename is composed by removing the suffix from the
        //! image filename and appending "-thumb" and the appropriate suffix (".jpg" or ".tif"), depending on the
        //! format of the Exif thumbnail image.
        int writeThumbnail() const;
        //! @brief Write preview images to files.
        int writePreviews() const;
        //! @brief Write one preview image to a file. The filename is composed by removing the suffix from the image
        //! filename and appending "-preview<num>" and the appropriate suffix (".jpg" or ".tif"), depending on the
        //! format of the Exif thumbnail image.
        void writePreviewFile(const Exiv2::PreviewImage& pvImg, int num) const;
        //! @brief Write embedded iccProfile files.
        int writeIccProfile(const std::string& path) const;

    private:
        Extract* clone_() const override;
        std::string path_;
    };

    //! @brief %Insert the Exif data from corresponding *.exv files.
    class Insert : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<Insert> clone() const;

        //! @brief Insert a Jpeg thumbnail image from a file into file \em path.
        //! The filename of the thumbnail is expected to be the image filename (\em path) minus its suffix plus
        //! "-thumb.jpg".
        int insertThumbnail(const std::string& path) const;

        //! @brief Insert an XMP packet from a xmpPath into file \em path.
        int insertXmpPacket(const std::string& path, const std::string& xmpPath) const;
        //! @brief Insert xmp from a DataBuf into file \em path.
        int insertXmpPacket(const std::string& path, const Exiv2::DataBuf& xmpBlob, bool usePacket = false) const;

        //! @brief Insert an ICC profile from iccPath into file \em path.
        int insertIccProfile(const std::string& path, const std::string& iccPath) const;
        //! @brief Insert an ICC profile from binary DataBuf into file \em path.
        int insertIccProfile(const std::string& path, Exiv2::DataBuf& iccProfileBlob) const;

    private:
        Insert* clone_() const override;
    };

    //! @brief Modify the Exif data according to the commands in the modification table.
    class Modify : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<Modify> clone() const;
        Modify() = default;
        //! Apply modification commands to the \em pImage, return 0 if successful.
        static int applyCommands(Exiv2::Image* pImage);

    private:
        Modify* clone_() const override;
        //! Copy constructor needed because of UniquePtr member
        Modify(const Modify & /*src*/) {}

        //! Add a metadatum to \em pImage according to \em modifyCmd
        static int addMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd);
        //! Set a metadatum in \em pImage according to \em modifyCmd
        static int setMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd);
        //! Delete a metadatum from \em pImage according to \em modifyCmd
        static void delMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd);
        //! Register an XMP namespace according to \em modifyCmd
        static void regNamespace(const ModifyCmd& modifyCmd);
    };

    //! @brief Copy ISO settings from any of the Nikon makernotes to the regular Exif tag, Exif.Photo.ISOSpeedRatings.
    class FixIso : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<FixIso> clone() const;

    private:
        FixIso* clone_() const override;
        std::string path_;
    };

    //! @brief Fix the character encoding of Exif UNICODE user comments.
    //! Decodes the comment using the auto-detected or specified character encoding and writes it back in UCS-2.
    class FixCom : public Task
    {
    public:
        int run(const std::string& path) override;
        std::unique_ptr<FixCom> clone() const;

    private:
        FixCom* clone_() const override;
        std::string path_;
    };

}  // namespace Action
