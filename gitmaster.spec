Name:           gitmaster
Version:        0.8.0
Release:        1%{?dist}
Summary:        Lightweight and fast cross-platform Git GUI client

License:        GPL-3.0-only
URL:            https://github.com/bokic/gitmaster
Source0:        %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  ninja-build
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  libgit2-devel

Requires:       qt6-qtbase
Requires:       qt6-qtsvg
Requires:       libgit2

%description
GitMaster is a lightweight, fast, cross-platform GUI client for Git,
built using C++ (C++20), Qt6, and libgit2.

%prep
%autosetup -p1

%build
%cmake -G Ninja -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%files
%{_bindir}/gitmaster
%{_datadir}/applications/gitmaster.desktop
%{_datadir}/icons/hicolor/scalable/apps/gitmaster.svg

%changelog
* Mon Jul 13 2026 Developer - 0.8.0-1
- Initial RPM release for GitMaster 0.8.0
