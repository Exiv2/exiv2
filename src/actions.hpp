// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
#ifndef ACTIONS_HPP_
#define ACTIONS_HPP_

// *****************************************************************************
// included header files
#include "exiv2app.hpp"

// *****************************************************************************
// class declarations

namespace Exiv2 {
    class ExifData;
    class Image;
    class Metadatum;
    class PreviewImage;
}

// *****************************************************************************
// namespace extensions
/*!
  @brief Contains all action classes (task subclasses).
 */
namespace Action {

    //! Enumerates all tasks
    enum TaskType { none, adjust, print, rename, erase, extract, insert,
                    modify, fixiso, fixcom };

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class for all concrete actions.

      Task provides a simple interface that actions must implement and a few
      commonly used helpers.
     */
    class Task {
    public:
        //! Shortcut for an auto pointer.
        using UniquePtr = std::unique_ptr<Task>;
        //! Virtual destructor.
        virtual ~Task() = default;
        //! Virtual copy construction.
        UniquePtr clone() const;
        /*!
          @brief Application interface to perform a task.

          @param path Path of the file to process.
          @return 0 if successful.
         */
        virtual int run(const std::string& path) =0;
        
        /*!
          @brief Application interface to perform a task.

          @param path Path of the file to process.
          @return 0 if successful.
         */
        bool setBinary(bool b) { bool bResult = binary_ ; binary_ = b ; return bResult ;}
        bool binary() { return binary_ ; }

    private:
        //! Internal virtual copy constructor.
        virtual Task* clone_() const =0;
        
        //! copy binary_ from command-line params to task
        bool binary_ {false} ;

    }; // class Task

    /*!
      @brief Task factory.

      Creates an instance of the task of the requested type.  The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class TaskFactory {
    public:
        /*!
          @brief Get access to the task factory.

          Clients access the task factory exclusively through
          this method.
        */
        static TaskFactory& instance();

        //! Prevent copy construction: not implemented.
        TaskFactory(const TaskFactory& rhs) = delete;

        //! Destructor
        void cleanup();

        /*!
          @brief  Create a task.

          @param  type Identifies the type of task to create.
          @return An auto pointer that owns a task of the requested type.  If
                  the task type is not supported, the pointer is 0.
          @remark The caller of the function should check the content of the
                  returned auto pointer and take appropriate action (e.g., throw
                  an exception) if it is 0.
         */
        Task::UniquePtr create(TaskType type);

        /*!
          @brief Register a task prototype together with its type.

          The task factory creates new tasks of a given type by cloning its
          associated prototype. Additional tasks can be registered.  If called
          for a type which already exists in the list, the corresponding
          prototype is replaced.

          @param type Task type.
          @param task Pointer to the prototype. Ownership is transferred to the
                 task factory. That's what the auto pointer indicates.
        */
        void registerTask(TaskType type, Task::UniquePtr task);

    private:
        //! Prevent construction other than through instance().
        TaskFactory();

        //! Pointer to the one and only instance of this class.
        static TaskFactory* instance_;
        //! Type used to store Task prototype classes
        using Registry = std::map<TaskType, Task*>;
        //! List of task types and corresponding prototypes.
        Registry registry_;

    }; // class TaskFactory

    //! %Print the Exif (or other metadata) of a file to stdout
    class Print : public Task {
    public:
        ~Print() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Print>;
        UniquePtr clone() const;

        //! Print the Jpeg comment
        int printComment();
        //! Print list of available preview images
        int printPreviewList();
        //! Print Exif summary information
        int printSummary();
        //! Print Exif, IPTC and XMP metadata in user defined format
        int printList();
        //! Return true if key should be printed, else false
        static bool grepTag(const std::string& key);
        //! Return true if key should be printed, else false
        static bool keyTag(const std::string& key);
        //! Print all metadata in a user defined format
        int printMetadata(const Exiv2::Image* image);
        //! Print a metadatum in a user defined format, return true if something was printed
        bool printMetadatum(const Exiv2::Metadatum& md, const Exiv2::Image* image);
        //! Print the label for a summary line
        void printLabel(const std::string& label) const;
        /*!
          @brief Print one summary line with a label (if provided) and requested
                 data. A line break is printed only if a label is provided.
          @return 1 if a line was written, 0 if the key was not found.
         */
        int printTag(const Exiv2::ExifData& exifData,
                     const std::string& key,
                     const std::string& label ="") const;
        //! Type for an Exiv2 Easy access function
        using EasyAccessFct = Exiv2::ExifData::const_iterator (*)(const Exiv2::ExifData& ed);
        /*!
          @brief Print one summary line with a label (if provided) and requested
                 data. A line break is printed only if a label is provided.
          @return 1 if a line was written, 0 if the information was not found.
         */
        int printTag(const Exiv2::ExifData& exifData,
                     EasyAccessFct easyAccessFct,
                     const std::string& label ="",
                     EasyAccessFct easyAccessFctFallback =NULL) const;

    private:
        Print* clone_() const override;

        std::string path_;
        int align_{0};                // for the alignment of the summary output
    }; // class Print

    /*!
      @brief %Rename a file to its metadate creation timestamp,
             in the specified format.
     */
    class Rename : public Task {
    public:
        ~Rename() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Rename>;
        UniquePtr clone() const;

    private:
        Rename* clone_() const override;
    }; // class Rename

    //! %Adjust the Exif (or other metadata) timestamps
    class Adjust : public Task {
    public:
        ~Adjust() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Adjust>;
        UniquePtr clone() const;

    private:
        Adjust* clone_() const override;
        int adjustDateTime(Exiv2::ExifData& exifData,
                           const std::string& key,
                           const std::string& path) const;

        long adjustment_ {0};
        long yearAdjustment_ {0};
        long monthAdjustment_ {0};
        long dayAdjustment_ {0};

    }; // class Adjust

    /*!
      @brief %Erase the entire exif data or only the thumbnail section.
     */
    class Erase : public Task {
    public:
        ~Erase() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Erase>;
        UniquePtr clone() const;

        /*!
          @brief Delete the thumbnail image, incl IFD1 metadata from the file.
         */
        static int eraseThumbnail(Exiv2::Image* image);
        /*!
          @brief Erase the complete Exif data block from the file.
         */
        static int eraseExifData(Exiv2::Image* image);
        /*!
          @brief Erase all Iptc data from the file.
         */
        static int eraseIptcData(Exiv2::Image* image);
        /*!
          @brief Erase Jpeg comment from the file.
         */
        static int eraseComment(Exiv2::Image* image);
        /*!
          @brief Erase XMP packet from the file.
         */
        static int eraseXmpData(Exiv2::Image* image);
        /*!
          @brief Erase ICCProfile from the file.
         */
        static int eraseIccProfile(Exiv2::Image* image);

    private:
        Erase* clone_() const override;
        std::string path_;

    }; // class Erase

    /*!
      @brief %Extract the entire exif data or only the thumbnail section.
     */
    class Extract : public Task {
    public:
        ~Extract() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Extract>;
        UniquePtr clone() const;

        /*!
          @brief Write the thumbnail image to a file. The filename is composed by
                 removing the suffix from the image filename and appending
                 "-thumb" and the appropriate suffix (".jpg" or ".tif"), depending
                 on the format of the Exif thumbnail image.
         */
        int writeThumbnail() const;
        /*!
          @brief Write preview images to files.
         */
        int writePreviews() const;
        /*!
          @brief Write one preview image to a file. The filename is composed by
                 removing the suffix from the image filename and appending
                 "-preview<num>" and the appropriate suffix (".jpg" or ".tif"),
                 depending on the format of the Exif thumbnail image.
         */
        void writePreviewFile(const Exiv2::PreviewImage& pvImg, int num) const;
        /*!
          @brief Write embedded iccProfile files.
         */
        int writeIccProfile(const std::string& target) const;

    private:
        Extract* clone_() const override;
        std::string path_;

    }; // class Extract

    /*!
      @brief %Insert the Exif data from corresponding *.exv files.
     */
    class Insert : public Task {
    public:
        ~Insert() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Insert>;
        UniquePtr clone() const;

        /*!
          @brief Insert a Jpeg thumbnail image from a file into file \em path.
                 The filename of the thumbnail is expected to be the image
                 filename (\em path) minus its suffix plus "-thumb.jpg".
         */
        static int insertThumbnail(const std::string& path);

        /*!
          @brief Insert an XMP packet from a xmpPath into file \em path.
         */
        static int insertXmpPacket(const std::string& path,const std::string& xmpPath) ;
        /*!
          @brief Insert xmp from a DataBuf into file \em path.
         */
        static int insertXmpPacket(const std::string& path, const Exiv2::DataBuf& xmpBlob, bool usePacket = false);

        /*!
          @brief Insert an ICC profile from iccPath into file \em path.
         */
        static int insertIccProfile(const std::string& path,const std::string& iccPath) ;
        /*!
          @brief Insert an ICC profile from binary DataBuf into file \em path.
         */
        static int insertIccProfile(const std::string& path, Exiv2::DataBuf& iccProfileBlob);

    private:
        Insert* clone_() const override;

    }; // class Insert

    /*!
      @brief %Modify the Exif data according to the commands in the
             modification table.
     */
    class Modify : public Task {
    public:
        ~Modify() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<Modify>;
        UniquePtr clone() const;
        Modify() {}
        //! Apply modification commands to the \em pImage, return 0 if successful.
        static int applyCommands(Exiv2::Image* pImage);

    private:
        Modify* clone_() const override;
        //! Copy constructor needed because of UniquePtr member
        Modify(const Modify& /*src*/) : Task() {}

        //! Add a metadatum to \em pImage according to \em modifyCmd
        static int addMetadatum(Exiv2::Image* pImage,
                                const ModifyCmd& modifyCmd);
        //! Set a metadatum in \em pImage according to \em modifyCmd
        static int setMetadatum(Exiv2::Image* pImage,
                                const ModifyCmd& modifyCmd);
        //! Delete a metadatum from \em pImage according to \em modifyCmd
        static void delMetadatum(Exiv2::Image* pImage,
                                 const ModifyCmd& modifyCmd);
        //! Register an XMP namespace according to \em modifyCmd
        static void regNamespace(const ModifyCmd& modifyCmd);

    }; // class Modify

    /*!
      @brief %Copy ISO settings from any of the Nikon makernotes to the
             regular Exif tag, Exif.Photo.ISOSpeedRatings.
     */
    class FixIso : public Task {
    public:
        ~FixIso() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<FixIso>;
        UniquePtr clone() const;

    private:
        FixIso* clone_() const override;
        std::string path_;

    }; // class FixIso

    /*!
      @brief Fix the character encoding of Exif UNICODE user comments.
             Decodes the comment using the auto-detected or specified
             character encoding and writes it back in UCS-2.
     */
    class FixCom : public Task {
    public:
        ~FixCom() override = default;
        int run(const std::string& path) override;
        using UniquePtr = std::unique_ptr<FixCom>;
        UniquePtr clone() const;

    private:
        FixCom* clone_() const override;
        std::string path_;

    }; // class FixCom

}                                       // namespace Action

#endif                                  // #ifndef ACTIONS_HPP_
