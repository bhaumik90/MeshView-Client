# MeshView Client
Client software that runs on network devices and feeds data to [MeshView](https://github.com/bhaumik90/MeshView) server.

## Limitations
* Currently ported on [Contiki-OS](https://github.com/contiki-os/contiki) only

## How to use
1. Clone or download [Contiki-OS](https://github.com/contiki-os/contiki)
2. Clone **MeshView-Client** folder to **contiki/apps** folder using:
```
  cd contiki/apps
  git clone https://github.com/bhaumik90/MeshView-Client.git mv-client
```
3. To add MeshView in your project:
- Add `APPS+=mv-client` to your project Makefile
- In your project contiki process add `mv_client_init(nwType, nodeType)`

> Please check [MeshView-Client-Test](https://github.com/bhaumik90/MeshView-Client-Test) example to know more.