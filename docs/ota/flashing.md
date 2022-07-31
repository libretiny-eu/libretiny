It is possible to upload firmware binaries manually, using the provided command-line tool `uf2upload`. For this, you need the `.uf2` file generated after compilation (usually found in `.pio/build/my_board/`).

1. Open a command prompt/terminal and `cd` into your home directory (`c:\Users\username` on Windows, `/home/username` on Linux).
2. `cd .platformio/platforms/libretuya/tools/upload`
3. `python uf2upload.py my_firmware.uf2 uart COM96` (replace `my_firmware.uf2` with your file name and `COM96` with your upload port).
