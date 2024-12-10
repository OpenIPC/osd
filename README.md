![OpenIPC logo][logo]


## OSD

An all-in-one daemon that exposes an HTTP frontend to
- adjust settings ;
- manage OSD regions ;
- stream AV content on supported IP cameras


### Installation

Example of a quick test setup for SigmaStar processors:

```
curl -s -L https://github.com/OpenIPC/osd/releases/download/latest/osd-star -O
chmod 755 osd-star
```

The Buildroot package is ready; the resulting file will be called "osd_server":

```
make BOARD=ssc338q_lite br-osd-openipc
make BOARD=gk7205v200_lite br-osd-openipc
make BOARD=hi3516ev300_lite br-osd-openipc
```


### Examples

Parameters for each region can be passed in one or many calls:
```
curl "192.168.1.17:9000/api/osd/0?font=comic&size=32.0&text=Frontdoor"
curl 192.168.1.17:9000/api/osd/0?posy=72
```
N.B. Ampersands require the address to be enclosed with double quotes under Windows or to be escaped with a backslash under Unix OSes

Supported fonts (sourced from /usr/share/fonts/truetype/) can render Unicode characters:
```
curl 192.168.1.17:9000/api/osd/0?text=Entrée
```

Empty strings are used to clear the regions:
```
curl 192.168.1.17:9000/api/osd/1?text=
```

Specifiers starting with a dollar sign are used to represent real-time statistics:
```
curl 192.168.1.17:9000/api/osd/1?text=$B%20C:$C%20M:$M
```
N.B. Spaces have to be escaped with %20 in curl URL syntaxes

Showing the time and customizing the time format is done this way:
```
curl 192.168.1.17:9000/api/time?fmt=%25Y/%25m/%25d%20%25H:%25M:%25S
curl 192.168.1.17:9000/api/osd/2?text=$t&posy=120
```
N.B. Percent signs have to be escaped with %25 in curl URL syntaxes

UTC date and time can be set using Unix timestamps:
```
curl 192.168.1.17:9000/api/time?ts=1712320920
```

24-, 32-bit bitmap files (.bmp) and PNG files (.png) can be uploaded to a region using this command:
```
curl -F data=@.\Desktop\myimage.bmp 192.168.1.17:9000/api/osd/3
```
N.B. curl already implies "-X POST" when passing a file with "-F"


### Disclaimer

This software is provided AS IS and for research purposes only. OpenIPC shall not be liable for any loss or damage caused by the use of these files or the use of, or reliance upon, any information contained within this project.


### Technical support and donations

Please **_[support our project](https://openipc.org/support-open-source)_** with donations or orders for development or maintenance. Thank you!

[logo]: https://openipc.org/assets/openipc-logo-black.svg