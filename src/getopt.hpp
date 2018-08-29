#ifndef GETOPT_H
#define GETOPT_H

#include <ctype.h>
#include <stdio.h>
#include <string>

namespace Util {

    extern int optind ;
    extern int opterr ;
    extern int optopt ;
    extern int optpos ;
    extern const char* optarg;

    int getopt(int argc, char * const argv[], const char *optstring);

    // *********************************************************************
    // class definitions

    /*!
      @brief Parse the command line options of a program.

      A wrapper around the POSIX %getopt(3) function.  Parses the command line
      options and passes each option to virtual option().  A derived class
      implements this method to handle options as needed.  Similarly,
      remaining non-option parameters are passed to the virtual nonoption()
      method.
     */
    class Getopt {
    public:
        //! Default constructor.
        Getopt();

        //! Destructor.
        virtual ~Getopt();

        /*!
          @brief Parse command line arguments.

          Parses the command line arguments. Calls option() with the
          character value of the option and its argument (if any) for each
          recognized option and with ':' or '?' for unrecognized options.
          See the manual pages for %getopt(3) for details.  In addition,
          nonoption() is invoked for each remaining non-option parameter on
          the command line.

          @param argc Argument count as passed to main() on  program invocation.
          @param argv Argument array as passed to main() on  program invocation.
          @param optstring String containing the legitimate option characters.

          @return Number of errors (the sum of the return values from option()
                  and nonoption()).
         */
        int getopt(int argc, char* const argv[], const std::string& optstring);

        /*!
          @brief Callback used by getopt() to pass on each option and its
                 argument (if any).

          Implement this method in a derived class to handle the options as
          needed. See the manual pages for %getopt(3) for further details, in
          particular, the semantics of optarg and optopt.

          @param opt Value of the option character as returned by %getopt(3).
          @param optarg The corresponding option argument.
          @param optopt The actual option character in case of an unrecognized
                 option or a missing option argument (opt is '?' or ':').

          @return 0 if successful, 1 in case of an error.
         */
        virtual int option(int opt, const std::string& optarg, int optopt) = 0;

        /*!
          @brief Callback used by getopt() to pass on each non-option parameter
                 found on the command line.

          Implement this method in a derived class to handle the non-option
          parameters as needed. The default implementation ignores all non-option
          parameters.

          @param argv The non-option parameter from the command line.

          @return 0 if successful, 1 in case of an error.
         */
        virtual int nonoption(const std::string& argv);

        //! Program name (argv[0])
        const std::string& progname() const { return progname_; }

        //! Total number of errors returned by calls to option()
        int errcnt() const { return errcnt_; }

    private:
        std::string progname_;
        int errcnt_;
    };

}; // namespace Util

#endif
