# uf2ota.py

This is a tool for converting LibreTiny firmware images to UF2 format for OTA updates.

```console
$ python uf2ota.py
usage: uf2ota [-h] [--output OUTPUT] [--family FAMILY] [--board BOARD] [--version VERSION] [--fw FW] {info,dump,write} inputs [inputs ...]
uf2ota: error: the following arguments are required: action, inputs
```

# write

Generate a UF2 file from a firmware image or several images.

```console
$ python uf2ota.py write --family RTL8710B --board wr3 --version 0.4.0 --fw esphome:2022.6.0-dev "ota1;xip1.bin;ota2;xip2.bin"

$ ls -l out.uf2
-rw-r--r-- 1 Kuba None 605696 May 28 14:35 out.uf2
```

## inputs format

Format for `inputs` parameter is `part;file[;part;file]` (square brackets mean optional). First two (colon separated) values correspond to flashing OTA1 region, second two to OTA2.

Partition name can be suffixed by `+offset`, which causes writing the image file to the partition after some byte offset. Both files and/or partition names can be equal. Values can be empty (like `part;file;;` or `;;part;file`) if OTA1/2 images are not present in this file.

When using two different firmware binaries, they need to have the same `offset` and be of the same size.

`inputs` parameter can be repeated in order to embed multiple files in the UF2. For example:
```console
"bootloader;boot.bin" "ota1;xip1.bin;ota2;xip2.bin" "config;config1.bin;config;config2.bin"
```
will:

- flash the bootloader in both OTA schemes
- flash `xip1.bin` or `xip2.bin` to `ota1` or `ota2` partitions
- flash `config1.bin` or `config2.bin` to `config` partition

# info

This command shows some basic parameters of a UF2 image.

```console
$ python uf2ota.py info out.uf2
Family: RTL8710B
Tags:
 - BOARD: wr3
 - DEVICE_ID: 312d5ec5
 - LT_VERSION: 0.4.0
 - FIRMWARE: esphome
 - VERSION: 2022.6.0-dev
 - OTA_VERSION: 01
 - DEVICE: LibreTiny
 - LT_HAS_OTA1: 01
 - LT_HAS_OTA2: 01
 - LT_PART_1: ota1
 - LT_PART_2: ota2
 - LT_BINPATCH: fe0900500c009094989ca0
Data chunks: 1182
Total binary size: 302448
```

# dump

Dump UF2 file (only LibreTiny format) into separate firmware binaries.

```console
$ python uf2ota.py dump out.uf2

$ ls -1 out.uf2_dump/
esphome_2022.6.0-dev_lt0.4.0_wr3_1_ota1_0x0.bin
esphome_2022.6.0-dev_lt0.4.0_wr3_2_ota2_0x0.bin
```
