![OpenIPC logo][logo]

## OSD

An all-in-one daemon that exposes an HTTP frontend to
- adjust settings ;
- manage OSD regions ;
- stream AV content on supported IP cameras

### Examples

```
curl 192.168.1.17:9000/api/time?ts=1712320920

curl 192.168.1.17:9000/api/osd/0?font=comic&size=32.0&text=Frontdoor
curl 192.168.1.17:9000/api/osd/0?posy=72

curl "192.168.1.17:9000/api/osd/1?text=$B C:$C M:$M"

curl 192.168.1.17:9000/api/osd/2?text=
```

### Technical support and donations

Please **_[support our project](https://openipc.org/support-open-source)_** with donations or orders for development or maintenance. Thank you!

[logo]: https://openipc.org/assets/openipc-logo-black.svg

