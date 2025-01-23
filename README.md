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

4. Switch to the ports directory

```shell 
cd ./FreeBSD-ports/net/pfSense-pkg-zerotier
```
5. 

```shell
dir=$(pwd)/FreeBSD-ports/net/pfSense-pkg-zerotier &&\
ZEROTIER_VERSION=$(cat "${dir}/version.json" | jq -r '.zerotier_version') &&\
PKG_VERSION=$(cat "${dir}/version.json" | jq -r '.pkg_version') &&\
export ZEROTIER_VERSION &&\
export PKG_VERSION &&\
make -C "${dir}" clean &&\
make -C "${dir}" package

```