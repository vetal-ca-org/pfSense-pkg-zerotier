# pfSense-pkg-zerotier
pfSense package to support zerotier.

# The package is provided as is, I don’t know for sure that my changes work properly

## I compiled this package for the current pfsense(bsd ver 15), you can find it in the releases

Precomiled Zerotier package will be auto installed. There are 2 kins of it: 

1. Fetched via fetch-zerotier.yml workflow. One, by default going with FreeBSD 15
2. Compiled in FreeBSD with poudriere. In theory, it is closer to what is required for pfSense. Both are having the ABI difference


## Install the packages


Adjust for the latest main build version

```shell
pkg add https://github.com/vetal-ca-org/pfSense-pkg-zerotier/releases/download/zerotier-1.14.2.0.00.6-feature-zerotier-main-build/pfSense-pkg-zerotier-1.14.2.0.00.6-feature-zerotier-main-build.pkg
```

To use fetched version:

```shell
ZEROTIER_PACKAGE_SOURCE=fetched \
pkg add https://github.com/vetal-ca-org/pfSense-pkg-zerotier/releases/download/zerotier-1.14.2.0.00.6-feature-zerotier-main-build/pfSense-pkg-zerotier-1.14.2.0.00.6-feature-zerotier-main-build.pkg
```

## Local build

1. Clone the ports repository
```shell
git clone https://github.com/pfsense/FreeBSD-ports.git
```
2. Clone this repository

```shell
git clone https://github.com/Vetal-ca/pfSense-pkg-zerotier.git
```

3. Copy this repository to the ports repository
```shell
cp -R ./pfSense-pkg-zerotier ./FreeBSD-ports/net/
```

4

```shell
dir=$(pwd)/FreeBSD-ports/net/pfSense-pkg-zerotier &&\
ZEROTIER_VERSION=$(cat "${dir}/version.json" | jq -r '.zerotier_version') &&\
PKG_VERSION=$(cat "${dir}/version.json" | jq -r '.pkg_version') &&\
GITHUB_OWNER="vetal-ca" &&\
GITHUB_REPO="pfSense-pkg-zerotier" &&\
export ZEROTIER_VERSION PKG_VERSION GITHUB_OWNER GITHUB_REPO &&\
make -C "${dir}" clean &&\
make -C "${dir}" package
```


## Build the Zerotier main package on FreeBSD with right ABI

```shell
pfsense_version="2_7_2" &&\
bsd_version="14.0-RELEASE" &&\
platform="amd64" &&\
sudo pkg install -y poudriere &&\
sudo cp /usr/local/etc/poudriere.conf.sample /usr/local/etc/poudriere.conf && \
echo "NO_ZFS=yes" | sudo tee -a /usr/local/etc/poudriere.conf && \
sudo poudriere jail -c -j "pfSense_${pfsense_version}" -v "${bsd_version}" -a "${platform}" &&\
sudo poudriere ports -c -p zerotier_custom

sudo poudriere bulk -j "pfSense_${pfsense_version}" -p zerotier_custom -f <(echo net/zerotier)

```

```shell
bsd_version="14.0"
platform="amd64"
jail_name="pfsense"

sudo pkg install -y poudriere git && \
sudo cp /usr/local/etc/poudriere.conf.sample /usr/local/etc/poudriere.conf && \
echo "NO_ZFS=yes" | sudo tee -a /usr/local/etc/poudriere.conf && \
#sudo poudriere jail -c -j "${jail_name}" -v "${bsd_version}-RELEASE" -a "${platform}" -m git+https -U "https://git.freebsd.org/src.git" -B "releng/${bsd_version}" &&\
srcdir=~/projects/freebsd-src
sudo ln -s ~/projects/obj /usr/obj
sudo poudriere jail -c -j "${jail_name}" -v "${bsd_version}-RELEASE" -a "${platform}" -m src=${srcdir}
sudo poudriere ports -c -p zerotier_custom -m git+https -B 2024Q1


sudo mkdir -p /usr/ports/distfiles
sudo chown $(whoami) /usr/ports/distfiles

echo net/zerotier > /tmp/pkglist
sudo poudriere bulk -j "${jail_name}" -p zerotier_custom -f /tmp/pkglist
```


FreeBSD source:

```shell
bsd_version="14.0"
srcdir=~/projects/freebsd-src
objdir=/home/vetal/projects/obj

# Clone the source if you haven't already
git clone --depth 1 -b releng/${bsd_version} https://git.freebsd.org/src.git $srcdir

# Make sure your object directory exists and is writable
mkdir -p "$objdir"

# Build world and kernel as a regular user
cd $srcdir
MAKEOBJDIRPREFIX="$objdir" make -j$(sysctl -n hw.ncpu) buildworld buildkernel
```

sudo poudriere jail -c -j "${jail_name}" -v "${bsd_version}" -a "${platform}" &&\



Trying clean build
```shell
jail_name="pfSense" ; branch="RELENG_2_7_2" ; fbsd_version="releng/14.0" ; platform="amd64"
sudo poudriere jail -c -j "$jail_name" -v "$fbsd_version" -a "${platform}" -m git+https -b "$branch"
```
Packages are at `/usr/local/poudriere/data/packages/<jail_name>-<ports_tree>/All/`


```shell

setenv ABI FreeBSD:14:amd64
setenv OSVERSION `sysctl -n kern.osreldate`
make WRKDIRPREFIX=/home/vetal/tmp PREFIX=/home/vetal/local package
```