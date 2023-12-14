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
