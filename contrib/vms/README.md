# Vagrant development boxes

This directory contains a `Vagrantfile` which can be used to automatically
create virtual machines for testing purposes. The virtual machines are
automatically provisioned with all required dependencies for building & testing
of exiv2 (the provisioning is shared with the GitLab CI).

The following Linux distributions are provided (the name in the brackets is the
name of the Vagrant VM):
- Fedora 28 ("Fedora")
- Debian 9 aka Stretch ("Debian")
- Archlinux ("Archlinux")
- Ubuntu 16.04 aka Bionic Beaver ("Ubuntu")
- CentOS 7 ("CentOS")
- OpenSUSE Tumbleweed ("OpenSUSE")

The Fedora, Archlinux and OpenSUSE boxes are the 'vanilla' distribution with
some additional packages installed.

For Debian and Ubuntu, we build gtest manually from source and install the
resulting library to /usr/lib/.

On CentOS, we have to install a `cmake3` and `python36` (the default cmake is
too old and a default python3 does not exist) which we symlink to
`/usr/bin/cmake` & `/usr/bin/python3` to retain a similar workflow to the other
distributions.

For further details, consult the shell scripts `setup.sh` and
`ci/install_dependencies.sh`.


All boxes come with `conan` installed via pip in the `vagrant` user's home	
directory and the `exiv2` git repository cloned.

Please note that these VMs are not continuously tested and the provisioning can
break. Please open an issue on GitHub if you happen to encounter a problem.


## Usage

Please install [Vagrant](https://www.vagrantup.com/) and a supported provider
(e.g. libvirt, VirtualBox).

Choose a box from the above list and run in the directory where the
`Vagrantfile` resides:
``` shell
vagrant up $name
```
where `$name` is the name in the brackets in the above list, e.g. `OpenSUSE` or
`Archlinux`. Depending on your default provider you may have to set the provider
manually via `vagrant up $name --provider $provider_name` (the Ubuntu image does
only support VirtualBox, which is not the default on Linux and will result in an
error unless you explicitly set the provider to `virtualbox`).

This will download a box from the vagrantcloud and set it up. Once the whole
process is finished, you can ssh into the machine via:

``` shell
vagrant ssh $name
```

Don't forget to turn it off via `vagrant halt $name` or the VM will keep
running! A VM can be discarded when it is no longer required via `vagrant
destroy $name` (Vagrant will keep the base box around in `~/.vagrant.d/boxes`
and libvirt sometimes leaves images around in `/var/lib/libvirt/` or
`/var/libvirt`, so check these folders too).


You can also setup & start all VMs at once via `vagrant up`, but keep in mind
that it will start 6 VMs and occupy between 10 and 20 GB of disk space.


# Notes for OpenSUSE Tumbleweed

Unfortunately the OpenSUSE Tumbleweed box cannot be provisioned easily with
Vagrant as it must perform a system upgrade first, which cannot be done
non-interactively. To get the OpenSUSE box up and running, follow these steps:

``` shell
$ vagrant up OpenSUSE
# you'll get a failure in the first provisioning script
$ vagrant ssh OpenSUSE
vagrant@opensuse-exiv2:~> su - # the root password is vagrant
Password:
opensuse-exiv2:~ # zypper refresh
opensuse-exiv2:~ # zypper dup
# zypper will now perform a system upgrade
# you'll probably get a few file conflicts, confirm the overwrite with 'yes'
# once the upgrade is done, exit the ssh session
$ vagrant halt OpenSUSE
$ vagrant up OpenSUSE
$ vagrant provision OpenSUSE
```

Provided the system upgrade went fine, you should now have an OpenSUSE
Tumbleweed virtual machine ready to go.
