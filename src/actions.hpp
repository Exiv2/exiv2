// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*!
  @file    actions.hpp
  @brief   Implements base class Task, TaskFactory and the various supported
           actions (derived from Task).
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Dec-03, ahu: created
 */
#ifndef ACTIONS_HPP_
#define ACTIONS_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <string>
#include <map>

// *****************************************************************************
// class declarations

namespace Exif {
    class ExifData;
}

// *****************************************************************************
// namespace extensions
/*!
  @brief Contains all action classes (task subclasses).
 */
namespace Action {

    //! Enumerates all tasks
    enum TaskType { none, adjust, print, rename };

// *****************************************************************************
// class definitions

    /*!
      @brief Base class for all concrete actions.

      Task provides a simple interface that actions must implement and a few
      commonly used helpers.
     */
    class Task {
    public:
        //! Shortcut for an auto pointer.
        typedef std::auto_ptr<Task> AutoPtr;
        //! Virtual copy construction.
        AutoPtr clone() const;
        /*!
          @brief Application interface to perform a task.

          @param path Path of the file to process.
          @return 0 if successful.
         */
        virtual int run(const std::string& path) =0;

    private:
        //! Internal virtual copy constructor.
        virtual Task* clone_() const =0;

    };

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

        /*!
          @brief  Create a task.

          @param  type Identifies the type of task to create.
          @return An auto pointer that owns a task of the requested type.  If
                  the task type is not supported, the pointer is 0.
          @remark The caller of the function should check the content of the
                  returned auto pointer and take appropriate action (e.g., throw
                  an exception) if it is 0.
         */
        Task::AutoPtr create(TaskType type);

        /*!
          @brief Register a task prototype together with its type.

          The task factory creates new tasks of a given type by cloning its
          associated prototype. Additional tasks can be registered.  If called
          for a type which already exists in the list, the corresponding
          prototype is replaced.

          @param type Task type.
          @param task Pointer to the prototype. Ownership is transfered to the
                 task factory. That's what the auto pointer indicates.
        */
        void registerTask(TaskType type, Task::AutoPtr task);

    private:
        //! Prevent construction other than through instance().
        TaskFactory();
        //! Prevent copy construction: not implemented.
        TaskFactory(const TaskFactory& rhs);

        //! Pointer to the one and only instance of this class.
        static TaskFactory* instance_;
        //! Type used to store Task prototype classes
        typedef std::map<TaskType, Task*> Registry;
        //! List of task types and corresponding prototypes.
        Registry registry_;

    }; // class TaskFactory

    //! %Print the %Exif (or other metadata) of a file to stdout
    class Print : public Task {
    public:
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Print> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual Task* clone_() const;
    };

    /*!
      @brief %Rename a file to its metadate creation timestamp, 
             in the specified format.
     */
    class Rename : public Task {
    public:
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Rename> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual Task* clone_() const;
    };

    //! %Adjust the %Exif (or other metadata) timestamps
    class Adjust : public Task {
    public:
        virtual int run(const std::string& path);
        typedef std::auto_ptr<Adjust> AutoPtr;
        AutoPtr clone() const;

    private:
        virtual Task* clone_() const;
        int adjustDateTime(Exif::ExifData& exifData,
                           const std::string& key, 
                           const std::string& path) const;

        long adjustment_;
    };

}                                       // namespace Action 

#endif                                  // #ifndef ACTIONS_HPP_
