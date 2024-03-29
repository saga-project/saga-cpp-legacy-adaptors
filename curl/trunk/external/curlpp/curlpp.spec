Summary: cURLpp is a libcurl C++ wrapper
Name: cURLpp
Version: 0.7.2
Release: 1
License: MIT
Group: Development/Libraries
URL: http://rrette.com/curlpp.html
Source0: curlpp-0.7.2.tar.gz
BuildRoot: %{_tmppath}/curlpp-0.7.2-%{release}-buildroot
Requires: curl >= 7.10.0


%package	devel
Summary:	The includes and libs to develop with cURLpp
Group:		Development/Libraries
Requires:	curl >= 7.10.0
Provides:	curlpp-devel

%description
cURLpp is a libcurl C++ wrapper. There is the libcurl description: "libcurl is a free and easy-to-use client-side URL transfer library, supporting FTP, FTPS, HTTP, HTTPS, GOPHER, TELNET, DICT, FILE and LDAP. libcurl supports HTTPS certificates, HTTP POST, HTTP PUT, FTP uploading, kerberos, HTTP form based upload, proxies, cookies, user+password authentication, file transfer resume, http proxy tunneling and more!

libcurl is highly portable, it builds and works identically on numerous platforms, including Solaris, Net/Free/Open BSD, Darwin, HPUX, IRIX, AIX, Tru64, Linux, Windows, Amiga, OS/2, BeOs, Mac OS X, Ultrix, QNX, OpenVMS, RISC OS and more... "

%description devel
This packages contains all the libs and headers to develop applications using cURLpp.


%prep
%setup -qn curlpp-0.7.2


%build
%configure
make 

%install
[ "%{buildroot}" != "/" ] && rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files 
%defattr(-,root,root)
%{_libdir}/libcurlpp.so*


%files devel
%defattr(-,root,root)
%attr(0755,root,root) %{_bindir}/curlpp-config
%dir %{_includedir}/curlpp
%{_includedir}/curlpp/*.hpp
%{_includedir}/curlpp/*.inl
%{_includedir}/curlpp/*.h
%dir %{_includedir}/curlpp/utilspp
%dir %{_includedir}/curlpp/utilspp/singleton
%{_includedir}/curlpp/utilspp/singleton/*.hpp
%{_includedir}/curlpp/utilspp/singleton/*.inl
%dir %{_includedir}/utilspp
%{_includedir}/utilspp/*.hpp
%{_includedir}/utilspp/*.inl
%dir %{_includedir}/utilspp/functor
%{_includedir}/utilspp/functor/*.hpp
%{_libdir}/libcurlpp.la
%{_libdir}/libcurlpp.a
%{_libdir}/pkgconfig/curlpp.pc
%dir 


%changelog

* Sun Jul 17 2005 Jean-Philippe Barrette-LaPierre <jpb@rrette.com> - 0.5.1-1
- removed {%name} use

* Wed Jan  5 2005 Jean-Philippe Barrette-LaPierre <jpb@rrette.com> - 0.3.2-rc1-1
- Version depends now on configure script

* Thu Sep 30 2004 Jean-Philippe Barrette-LaPierre <jpb@rrette.com> 0.3.1-1
- Removed any utilspp reference. (Not used anymore)

* Thu Jun 17 2004 Jean-Philippe Barrette-LaPierre <jpbarrette@savoirfairelinux.net> 0.3.1-1
- Removed the unusefull BuildRequires

* Mon Oct 20 2003 Jean-Philippe Barrette-LaPierre <jpbarrette@savoirfairelinux.net> - 0.3.0-2
- Added the devel package

* Wed Oct 15 2003 Jean-Philippe Barrette-LaPierre <jpb@rrette.com> - 0.3.0-1
- Initial build.


