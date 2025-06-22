# pfSense-pkg-zerotier
pfSense package to support zerotier.

# The package is provided as is, I don’t know for sure that my changes work properly
## how to fix zerotier for freebsd 14 & pfsense 2.7:
- [instructions taken from here](https://discuss.zerotier.com/t/freebsd-14-0-zerotier-401-error/16919)
- under root, open file `nano /var/db/zerotier-one/local.conf`
- add(if file new, if not remove first and last bracket):
  ```
  {
    "settings": {
        "allowManagementFrom": [ "127.0.0.1", "::1", "ffff:127.0.0.1" ]
    }
  }
  ```

## I compiled this package for the current pfsense(bsd ver 14), you can find it in the releases
Run `pkg add https://pkg.freebsd.org/FreeBSD:14:amd64/latest/All/zerotier-1.12.2.pkg` \
Run `pkg add https://github.com/asvdvl/pfSense-pkg-zerotier/releases/download/2.7/pfSense-pkg-zerotier-0.00.1.pkg`


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
bsd_version="main" &&\
platform="amd64" &&\
pfsense_branch="RELENG_2_8_0" &&\
sudo pkg install -y poudriere git &&\
sudo cp /usr/local/etc/poudriere.conf.sample /usr/local/etc/poudriere.conf && \
echo "NO_ZFS=yes" | sudo tee -a /usr/local/etc/poudriere.conf && \
sudo mkdir -p /usr/ports/distfiles
jail_name="pfSense"
sudo poudriere jail -c -j "${jail_name}" -v "${bsd_version}" -a "${platform}" -m git+https -b "${pfsense_branch}"
port_tree="zerotier" &&\
sudo poudriere ports -c -p "${port_tree}"
sudo poudriere bulk -j "${jail_name}" -p "${port_tree}" -f <(echo net/zerotier)
```


Package is built and can be found in `/usr/local/poudriere/data/packages/pfSense-zerotier/All/zerotier-1.14.2.pkg`


## Build zerotier in pfSense

Enable FreeBSD packages in pfSense

- `sudo nano /usr/local/etc/pkg/repos/pfSense.conf`
- `sudo nano /usr/local/etc/pkg/repos/FreeBSD.conf`

```
FreeBSD: { enabled: yes }
...
```

Then run the following commands in the shell:

```shell
sudo pkg update &&\
setenv ABI FreeBSD:14:amd64
setenv OSVERSION `sysctl -n kern.osreldate`

sudo pkg install -y git nano clang &&\
mkdir ~/projects && cd ~/projects &&\
git clone https://github.com/pfsense/FreeBSD-ports.git

cd FreeBSD-ports/net/zerotier && git fetch && git checkout main

make clean

```

