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


### Documentation

For detailed information on the usage, supported functionalities, and configuration of the overlay manager, please refer to the [documentation section](doc/index.md). It includes example usage scenarios, an overview of the key features, and step-by-step instructions to help you get started with the overlay manager for your video streams.


### Disclaimer

This software is provided AS IS and for research purposes only. OpenIPC shall not be liable for any loss or damage caused by the use of these files or the use of, or reliance upon, any information contained within this project.


### Technical support and donations

Please **_[support our project](https://openipc.org/support-open-source)_** with donations or orders for development or maintenance. Thank you!

[logo]: https://openipc.org/assets/openipc-logo-black.svg