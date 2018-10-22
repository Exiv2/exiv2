contrib/buildserver/ReadMe.txt
------------------------------

T A B L E   of   C O N T E N T S
--------------------------------

1    Build Architecture
2    Install and configure Jenkins
3    Setting up the web server on the Mac Mini
3.1  Disable MacOS-X Server Profiles
3.2  Edit /etc/apache2/httpd.conf
3.3  Edit /etc/apache2/extra/httpd-vhosts.conf
3.4  Edit /etc/hosts


1 Build Architecture
--------------------
To be written

2 Install and configure Jenkins on the MacMini
----------------------------------------------
To be written

3 Setting up the web server on the Mac Mini
-------------------------------------------

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


Robin Mills
robin@clanmills.com
2018-10-23
