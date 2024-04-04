![OpenIPC logo][logo]

## OSD

An all-in-one daemon that exposes an HTTP frontend to
- adjust settings ;
- manage OSD regions ;
- stream AV content on supported IP cameras

### Examples

```
curl 192.168.1.17:9000/osd/0?size=60
curl 192.168.1.17:9000/osd/0?text=example0

curl 192.168.1.17:9000/osd/1?size=80
curl 192.168.1.17:9000/osd/1?text=example1
curl 192.168.1.17:9000/osd/1?posx=1000
```

### Technical support and donations

Please **_[support our project](https://openipc.org/support-open-source)_** with donations or orders for development or maintenance. Thank you!

[logo]: https://openipc.org/assets/openipc-logo-black.svg

