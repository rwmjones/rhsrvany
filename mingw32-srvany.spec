%define __strip %{_mingw32_strip}
%define __objdump %{_mingw32_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw32_findrequires}
%define __find_provides %{_mingw32_findprovides}

Name:		mingw32-srvany
Version:	1.0.0
Release:	1%{?dist}
Summary:	Utility for creating services for Windows

Group:		Development/Libraries
License:	BSD
URL:		http://github.com/rwmjones/rhsrvany
Source0:	rhsrvany-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:	noarch

BuildRequires:  redhat-rpm-config
BuildRequires:	mingw32-filesystem >= 56
BuildRequires:	mingw32-gcc
BuildRequires:	mingw32-gcc-c++
BuildRequires:	mingw32-binutils

%{?_mingw32_debug_package}

%description
Utility for creating a service from any MinGW Windows binary

%prep
%setup -q -n rhsrvany-%{version}

%build
%{_mingw32_configure} 
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_mingw32_bindir}/rhsrvany.exe

%changelog
* Mon Sep 13 2010 Andrew Beekhof <andrew@beekhof.net> - 1.0.0-1
- Initial build.
