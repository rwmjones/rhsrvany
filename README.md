# rhsrvany

**Turn any Windows program or script into a service.**

When you need to run some program or batch script as a Windows
service, you either have to modify the program so it calls the Windows
service APIs (impossible for a batch script), or you use some helper
tool like Microsoft "SrvAny" that turns the program/script into a
service.

Unfortunately SrvAny is a proprietary program.

This project, **rhsrvany**, is a lightweight free software equivalent.

## More information about rhsrvany

For more information, see:

http://rwmj.wordpress.com/2010/04/29/tip-install-a-service-in-a-windows-vm/#content

Please send patches etc to the virt-tools mailing list:

http://www.redhat.com/mailman/listinfo/virt-tools-list

This program was originally written by Yuval Kashtan as part of the
Qumranet / RHEV-M / oVirt project, and released as free software by
Red Hat.

This program is currently maintained by Richard W.M. Jones.

## Note for virt-v2v users

You may have arrived here searching for a particular error message
raised by the [virt-v2v][^virt-v2v] tool:

```
virt-v2v: error: One of rhsrvany.exe or pvvxsvc.exe is missing in /usr/share/virt-tools.

One of them is required in order to install Windows firstboot scripts.

You can get one by building rhsrvany (https://github.com/rwmjones/rhsrvany)
```

[^virt-v2v]: https://libguestfs.org/virt-v2v.1.html

## Binary releases

This project does not release binary packages directly.  Binaries are
available on multiple Linux distributions which, normally, also ship
`virt-v2v`.

Please check if your distribution provides such binaries in the form
of a package.

It is possible to extract `rhsrvany.exe` from the distribution packages.

Here is ane example, using Fedora's RPM package. It should work on any
system that has the `rpm2cpio` tool.

- Get the latest RPM from Fedora of the mingw-srvany package[^mingw-srvany]
  (you want the "noarch" one)
- Extract the files using `rpm2cpio` (this tool is available on pretty much
  any distro)

[^mingw-srvany]: https://koji.fedoraproject.org/koji/packageinfo?packageID=13522

### Full example

The following code snippet would make `virt-v2v` happy on a Debian (or
derivative) system:

```sh
apt install -y rpm2cpio

wget -nd -O srvany.rpm https://kojipkgs.fedoraproject.org//packages/mingw-srvany/1.1/4.fc38/noarch/mingw32-srvany-1.1-4.fc38.noarch.rpm

rpm2cpio srvany.rpm | cpio -idmv \
  && mkdir /usr/share/virt-tools \
  && mv ./usr/i686-w64-mingw32/sys-root/mingw/bin/*exe /usr/share/virt-tools/
```

which installs `pnp_wait.exe` and `rhsrvany.exe` into `/usr/share/virt-tools/`
