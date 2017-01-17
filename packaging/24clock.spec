Name:           24clock
Version:        PKG_VERSION
Release:        1%{?dist}
Summary:        24clock
License:        GPLv2
URL:            https://github.com/da-x/24clock
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:  mesa-libGL-devel
BuildRequires:  mesa-libGLU-devel
BuildRequires:  libXaw-devel
BuildRequires:  libXi-devel
BuildRequires:  freeglut-devel
BuildRequires:  libconfuse-devel

%description

%package        mate
Summary:        Mate screensaver of 24clock
Requires:       %{name} = %{version}-%{release}
Requires:       24clock
Requires:       mate-screensaver

%description    mate
Mate screensaver of 24clock

%prep
%setup -q

%build

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
cp 24clock $RPM_BUILD_ROOT/%{_bindir}

D=$RPM_BUILD_ROOT/%{_libexecdir}/mate-screensaver
mkdir -p ${D}
ln -s %{_bindir}/24clock ${D}/24clock

D=$RPM_BUILD_ROOT/%{_datarootdir}/applications/screensavers
mkdir -p ${D}
cat packaging/24clock.desktop.tmpl | sed "s#PREFIX#%{_libexecdir}#g" > ${D}/24clock.desktop

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/24clock

%files mate
%defattr(-,root,root,-)
%{_libexecdir}/mate-screensaver/24clock
%{_datarootdir}/applications/screensavers/24clock.desktop

%changelog
