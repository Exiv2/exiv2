// SPDX-License-Identifier: GPL-2.0-or-later

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
#include "exiv2app.hpp"

#include <unordered_map>

// *****************************************************************************
// class declarations

namespace Exiv2 {
class ExifData;
class Image;
class Metadatum;
class PreviewImage;
}  // namespace Exiv2

// *****************************************************************************
// namespace extensions
/// @brief Contains all action classes (task subclasses).
namespace Action {
//! Enumerates all tasks
enum TaskType {
  none,
  adjust,
  print,
  rename,
  erase,
  extract,
  insert,
  modify,
  fixiso,
  fixcom,
};

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

  Task() = default;
  Task(const Task&) = default;
  Task& operator=(const Task&) = default;

  //! Virtual copy construction.
  [[nodiscard]] virtual UniquePtr clone() const = 0;

  /// @brief Application interface to perform a task.
  /// @param path Path of the file to process.
  /// @return 0 if successful.
  virtual int run(const std::string& path) = 0;

  bool setBinary(bool b) {
    bool bResult = binary_;
    binary_ = b;
    return bResult;
  }

  [[nodiscard]] bool binary() const {
    return binary_;
  }

 private:
  //! copy binary_ from command-line params to task
  bool binary_{false};
};  // class Task

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
    Clients access the task factory exclusively through this method. (SINGLETON)
  */
  static TaskFactory& instance();

  ~TaskFactory() = default;
  //! Prevent copy construction: not implemented.
  TaskFactory(const TaskFactory&) = delete;
  TaskFactory& operator=(const TaskFactory&) = delete;

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

 private:
  //! Prevent construction other than through instance().
  TaskFactory();

  //! List of task types and corresponding prototypes.
  std::unordered_map<TaskType, Task::UniquePtr> registry_;
};

//! %Print the Exif (or other metadata) of a file to stdout
class Print : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

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
  int printTag(const Exiv2::ExifData& exifData, const std::string& key, const std::string& label = "") const;
  //! Type for an Exiv2 Easy access function
  using EasyAccessFct = Exiv2::ExifData::const_iterator (*)(const Exiv2::ExifData&);
  /*!
    @brief Print one summary line with a label (if provided) and requested
           data. A line break is printed only if a label is provided.
    @return 1 if a line was written, 0 if the information was not found.
   */
  int printTag(const Exiv2::ExifData& exifData, EasyAccessFct easyAccessFct, const std::string& label = "",
               EasyAccessFct easyAccessFctFallback = nullptr) const;

 private:
  std::string path_;
  int align_{0};  // for the alignment of the summary output
};

/// @brief %Rename a file to its metadata creation timestamp, in the specified format.
class Rename : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;
};  // class Rename

//! %Adjust the Exif (or other metadata) timestamps
class Adjust : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

 private:
  int adjustDateTime(Exiv2::ExifData& exifData, const std::string& key, const std::string& path) const;

  int64_t adjustment_{0};
  int64_t yearAdjustment_{0};
  int64_t monthAdjustment_{0};
  int64_t dayAdjustment_{0};

};  // class Adjust

/// @brief %Erase the entire exif data or only the thumbnail section.
class Erase : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

  /// @brief Delete the thumbnail image, incl IFD1 metadata from the file.
  static int eraseThumbnail(Exiv2::Image* image);

  /// @brief Erase the complete Exif data block from the file.
  static int eraseExifData(Exiv2::Image* image);

  /// @brief Erase all Iptc data from the file.
  static int eraseIptcData(Exiv2::Image* image);

  /// @brief Erase Jpeg comment from the file.
  static int eraseComment(Exiv2::Image* image);

  /// @brief Erase XMP packet from the file.
  static int eraseXmpData(Exiv2::Image* image);

  /// @brief Erase ICCProfile from the file.
  static int eraseIccProfile(Exiv2::Image* image);

 private:
  std::string path_;
};

/// @brief %Extract the entire exif data or only the thumbnail section.
class Extract : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

  /*!
    @brief Write the thumbnail image to a file. The filename is composed by
           removing the suffix from the image filename and appending
           "-thumb" and the appropriate suffix (".jpg" or ".tif"), depending
           on the format of the Exif thumbnail image.
   */
  [[nodiscard]] int writeThumbnail() const;

  /// @brief Write preview images to files.
  [[nodiscard]] int writePreviews() const;

  /// @brief Write one preview image to a file. The filename is composed by removing the suffix from the image
  /// filename and appending "-preview<num>" and the appropriate suffix (".jpg" or ".tif"), depending on the
  /// format of the Exif thumbnail image.
  void writePreviewFile(const Exiv2::PreviewImage& pvImg, size_t num) const;

  /// @brief Write embedded iccProfile files.
  [[nodiscard]] int writeIccProfile(const std::string& target) const;

 private:
  std::string path_;
};

/// @brief %Insert the Exif data from corresponding *.exv files.
class Insert : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

  /*!
    @brief Insert a Jpeg thumbnail image from a file into file \em path.
           The filename of the thumbnail is expected to be the image
           filename (\em path) minus its suffix plus "-thumb.jpg".
   */
  static int insertThumbnail(const std::string& path);

  /// @brief Insert an XMP packet from a xmpPath into file \em path.
  static int insertXmpPacket(const std::string& path, const std::string& xmpPath);

  /// @brief Insert xmp from a DataBuf into file \em path.
  static int insertXmpPacket(const std::string& path, const Exiv2::DataBuf& xmpBlob, bool usePacket = false);

  /// @brief Insert an ICC profile from iccPath into file \em path.
  static int insertIccProfile(const std::string& path, const std::string& iccPath);

  /// @brief Insert an ICC profile from binary DataBuf into file \em path.
  static int insertIccProfile(const std::string& path, Exiv2::DataBuf&& iccProfileBlob);
};

/// @brief %Modify the Exif data according to the commands in the modification table.
class Modify : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;
  //! Apply modification commands to the \em pImage, return 0 if successful.
  static int applyCommands(Exiv2::Image* pImage);

 private:
  //! Add a metadatum to \em pImage according to \em modifyCmd
  static int addMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd);
  //! Set a metadatum in \em pImage according to \em modifyCmd
  static int setMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd);
  //! Delete a metadatum from \em pImage according to \em modifyCmd
  static void delMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd);
  //! Register an XMP namespace according to \em modifyCmd
  static void regNamespace(const ModifyCmd& modifyCmd);
};

/// @brief %Copy ISO settings from any of the Nikon makernotes to the regular Exif tag, Exif.Photo.ISOSpeedRatings.
class FixIso : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

 private:
  std::string path_;
};

/// @brief Fix the character encoding of Exif UNICODE user comments.
///
/// Decodes the comment using the auto-detected or specified character encoding and writes it back in UCS-2.
class FixCom : public Task {
 public:
  int run(const std::string& path) override;
  [[nodiscard]] Task::UniquePtr clone() const override;

 private:
  std::string path_;
};

}  // namespace Action

#endif  // #ifndef ACTIONS_HPP_
