contrib/buildserver/ReadMe.txt
------------------------------

T A B L E   of   C O N T E N T S
--------------------------------

1    Build Architecture
2    Install and configure Jenkins
2.1  Build Projects
2.2  Build Nodes
2.3  Jenkins User Interface Sidebar
3    Setting up the web server on the Mac Mini
3.1  Disable MacOS-X Server Profiles
3.2  Edit /etc/apache2/httpd.conf
3.3  Edit /etc/apache2/extra/httpd-vhosts.conf
3.4  Edit /etc/hosts
4    Setting up SSH
4.1  Setting up SSH on the Windows VM
4.2  Setting up SSH on Linux and MacOS-X

1 Build Architecture
--------------------
The build is executed by the script <exiv2dir>/contrib/buildserver/build.sh

1.1 Setting up VMs for Linux and Windows
----------------------------------------

You are expected to set up VMs as follows:

Option: --server server --user user
server              shell                    directory
------              -----                    ---------
$server             bash                     /Users/$user/gnu/github/exiv2/buildserver
$server-w7          msys32                    /home/$user/gnu/github/exiv2/buildserver
                    msys64                    /home/$user/gnu/github/exiv2/buildserver
                    c:\Cygwin64\bin\bash      /home/$user/gnu/github/exiv2/buildserver
$server-ubuntu      bash                      /home/$user/gnu/github/exiv2/buildserver

The script msys32.bat and msys64.bat are documented in README.md

1.2 Building packages
---------------------
The build is performed and the package is built in sub-directory: build.  For example:

./buildserver/build/exiv2-0.27.0.1-Darwin.tar.gz

The build of the "Source" is only performed on the Mac Mini in the directory:
/User/rmills/gnu/github/exiv2/exiv2/build
/User/rmills/gnu/github/exiv2/exiv2/build/exiv2-0.27.0.1-Source.tar.gz

If you build package_source in exiv2/buildserver/build, foo-Source.tar.gz is 29 bytes

2 Install and configure Jenkins on the MacMini
----------------------------------------------

I can't anything special about the setup as it has been running on the MacMini for several years.  I'll set up a new MacOS-X VM and install Jenkins to see if there's something unusual about this.

The builds are performed on the VMs and copied to directory /Users/rmills/Jenkins/builds/all by build.sh --publish

The --publish option uses the script categorize.py to manufacture links in ~/Jenkins/builds for Platforms, Dates and other useful indices.

At present there is no script to removed old/stale builds for builds/all

2.1) Build Projects
-------------------

There are two Jenkins projects to perform the build:
1) daily-github-exiv2-exiv2
This expects 6 builders (labels): macosx, linux, mingw, mingw32, cygwin, msvc
All builds are performed from MacOS-X using the build.sh script.

The build command is:
/User/rmills/gnu/github/exiv2/exiv2/contrib/buildserver/build.sh $label --clone

2) daily-github-exiv2-exiv2-publish
The project is executed by Jenkins when the build on every label finishes.
This project expects a single builder (macosx)

The build command is:
/User/rmills/gnu/github/exiv2/exiv2/contrib/buildserver/build.sh $label --publish

2.2) Build Nodes
----------------

You only configure the Mac because build.sh performs the builds over ssh.
Jenkins does not know about the Windows and Linux servers.

The 'master' (rmillsmm) should have the labels macosx linux cygwin mingw mingw32 msvc
Launch method: "Launch agent agents via SSH"
Host: rmillsmm
Credentials: rmills/********
Host Key Verification Strategy: "Manually provided key Verification Stragegy"
SSH Key is the public key for the machine (~/.ssh/id_rsa.pub)

2.3) Jenkins User Interface Sidebar
-----------------------------------

Once Jenkins is running, you can configure it with: http://exiv2.dyndns.org:8080/configure

Jenkins is actually in /Users/Shared/Jenkins

drwxrwxrwx+  61 _hidd _hidd 2.0K Oct 23 12:31 Home/
drwxrwxrwx+   5 _hidd _hidd  160 Oct 23 11:20 Remote/
drwxrwxrwx+ 417 _hidd _hidd  14K Oct 23 11:20 tmp/
drwxr-xr-x+   4 _hidd _hidd  128 Sep 29  2016 certs/
drwxr-xr-x+   3 _hidd _hidd   96 Nov  5  2015 Library/
594 rmills@rmillsmm:/Users/Shared/Jenkins $ ls -alt Home/userContent
total 256
drwxrwxrwx+ 61 _hidd  _hidd  1952 Oct 23 12:31 ..
drwxrwxrwx+ 19 _hidd  _hidd   608 Mar  8  2017 .
lrwxr-xr-x   1 rmills _hidd    28 Jul  6  2016 builds -> /Users/rmills/Jenkins/builds
-rw-r--r--+  1 _hidd  _hidd  1645 Apr 17  2016 icon-test.png
lrwxr-xr-x   1 rmills _hidd    32 Apr 17  2016 testfiles -> /Users/rmills/Jenkins/testfiles/
-rw-r--r--+  1 _hidd  _hidd 15594 Dec 17  2015 Exiv2Logo.png
-rw-r--r--+  1 _hidd  _hidd   316 Dec 17  2015 theme.css
-rw-r--r--+  1 _hidd  _hidd 10144 Dec 17  2015 Exiv2Logo50Background.png
-rw-r--r--+  1 _hidd  _hidd  2071 Dec 10  2015 builds.png
-rwxr-xr-x+  1 _hidd  _hidd  5205 Dec 10  2015 exiv2-icon.png
-rw-r--r--+  1 _hidd  _hidd  9837 Dec 10  2015 community.png
-rw-r--r--+  1 _hidd  _hidd  2756 Dec 10  2015 logo-24x24.png
-rw-r--r--+  1 _hidd  _hidd  2709 Dec 10  2015 Exiv2-24x24.png
-rw-r--r--+  1 _hidd  _hidd   765 Dec 10  2015 fingerprint.png
-rw-r--r--+  1 _hidd  _hidd  1893 Nov 30  2015 theme.js
-rw-r--r--+  1 _hidd  _hidd    38 Nov 30  2015 header
drwxr-xr-x+  2 _hidd  _hidd    64 Nov 30  2015 otherStuff
-rw-r--r--+  1 _hidd  _hidd    51 Nov 30  2015 index.html
-rwxrwxrwx+  1 _hidd  _hidd    84 Aug 12  2014 readme.txt
595 rmills@rmillsmm:/Users/Shared/Jenkins $


I've added a few sidebar links:

Text            URL                    Icon
Exiv2 Project   http://www.exiv2.org   /userContent/exiv2-icon.png
Daily Builds    /userContent/builds    /userContent/builds.png
Test Files      /userContent/testfiles /usrContent/icon-test.png

3 Setting up Apache on the MacMini
----------------------------------

3.1) Disable MacOS-X Server Profiles

3.2) Edit /etc/apache2/httpd.conf

LoadModule include_module libexec/apache2/mod_include.so
LoadModule vhost_alias_module libexec/apache2/mod_vhost_alias.so
Include /private/etc/apache2/extra/httpd-vhosts.conf

Change the document root:
DocumentRoot "/Users/rmills/clanmills"
<Directory "/Users/rmills/clanmills">

    DirectoryIndex default.shtml
    #
    # Possible values for the Options directive are "None", "All",
    # or any combination of:
    #   Indexes Includes FollowSymLinks SymLinksifOwnerMatch ExecCGI MultiViews
    #
    # Note that "MultiViews" must be named *explicitly* --- "Options All"
    # doesn't give it to you.
    #
    # The Options directive is both complicated and important.  Please see
    # http://httpd.apache.org/docs/2.4/mod/core.html#options
    # for more information.
    #
    Options FollowSymLinks Multiviews
    MultiviewsMatch Any

    #
    # AllowOverride controls what directives may be placed in .htaccess files.
    # It can be "All", "None", or any combination of the keywords:
    #   AllowOverride FileInfo AuthConfig Limit
    #
    AllowOverride All

    #
    # Controls who can get stuff from this server.
    #
    Require all granted
</Directory>

3.3) Edit /etc/apache2/extra/httpd-vhosts.conf

<VirtualHost *>
    ServerName     clanmills
    DocumentRoot   /Users/rmills/clanmills
    Options        +Includes
    DirectoryIndex default.shtml
    AccessFileName .htaccess
    <Files *>
        Options +Includes
    </Files>
    <Directory *>
        AllowOverride All
        Allow from all
    </Directory>
</VirtualHost>

<VirtualHost *>
    ServerName     exiv2
    DocumentRoot   /Users/rmills/exiv2
    Options        +Includes
    DirectoryIndex index.html
    AccessFileName .htaccess
    <Files *>
        Options +Includes
    </Files>
    <Directory *>
        AllowOverride All
        Allow from all
        Require all granted
    </Directory>
</VirtualHost>

3.4) Edit /etc/hosts

##
# Machines
# MM
192.168.0.216        rmillsmm
192.168.0.216        clanmills
192.168.0.216        exiv2

# MBP
192.168.0.217        rmillsmbp
192.168.0.217        klanmills

4 Setting up SSH

4.1) Setting up SSH on the Windows VM
--------------------------------------

I highly recommend using the Bitvise SSH server on Windows.  It's free for personal and open-source use.  You should configure it to run cmd.exe as the shell.  You should install msys32 and msys64 as described in README.md.  And put the scripts msys32.bat and msys64.bat on the PATH.

I installed two public keys into Bitvise (from rmillsmbp and rmillsmm) to avoid the headache of passwords.  The connection is always made from the build 'master' to the build node (from rmillsmm to rmillsmm-W7).  Copy ~/.ssh/id_rsa.pub into Bitvise using the dialog box "Manage Host Keys".  I believe I also use the "password cache" in Bitvise.

The command: echo "ls -alt" | ssh $user/$server-w7 msys32 will connect you to the msys32 shell and process whatever you pipe on standard-input.  The script build.sh typically uses HERE scripts.

554 rmills@rmillsmm:~ $ echo 'pwd;echo ++++ ;ls gnu' | ssh rmills@rmillsmm-w7 msys32
/c/Users/rmills
++++
exiv2
github
555 rmills@rmillsmm:~ $ echo 'cd && echo ++++  && cd gnu && dir' | ssh rmills@rmillsmm-w7 cmd
C:\Users\rmills>cd && echo ++++  && cd gnu && dir
C:\Users\rmills
++++
 Volume in drive C has no label.
 Volume Serial Number is 0899-EF40
 Directory of C:\Users\rmills\gnu
2018-07-02  05:59 AM    <DIR>          exiv2
2018-10-15  08:17 PM    <DIR>          github
               0 File(s)              0 bytes
              11 Dir(s)  98,327,109,632 bytes free
C:\Users\rmills\gnu>560 rmills@rmillsmm:~ $
556 rmills@rmillsmm:~

4.2) Setting up SSH on the Linux VM and MacOS-X
-----------------------------------------------

You should copy your public key (~/.ssh/id_rsa.pub) from rmillsmm to the server ~/.ssh/authorized_keys  There are many articles on the web about how to do this.

You should do this for both Linux and Mac.  To keep things simple, build.sh uses ssh for server communications (even when talking to himself!)

568 rmills@rmillsmm:~ $ echo 'uname -a;pwd;ls gnu' | ssh rmills@rmillsmm-ubuntu bash
Linux rmillsmm-ubuntu 4.15.0-36-generic #39-Ubuntu SMP Mon Sep 24 16:19:09 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
/home/rmills
exiv2
github
zlib
569 rmills@rmillsmm:~ $ echo 'uname -a;pwd;ls gnu/github' | ssh rmills@rmillsmm bash
Darwin rmillsmm.local 18.0.0 Darwin Kernel Version 18.0.0: Wed Aug 22 20:13:40 PDT 2018; root:xnu-4903.201.2~1/RELEASE_X86_64 x86_64
/Users/rmills
MacOSX-SDKs
adobe_xmp_sdk
conan
exiv2
mytimer
570 rmills@rmillsmm:~ $

Robin Mills
robin@clanmills.com
2018-10-23
