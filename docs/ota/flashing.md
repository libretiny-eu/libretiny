It is possible to upload firmware binaries manually, using the command-line tool `ltchiptool`. For this, you need the `.uf2` file generated after compilation (usually found in `.pio/build/my_board/`).

1. Install Python. Afterwards, run `pip install ltchiptool`.
2. `ltchiptool uf2 upload my_firmware.uf2 uart COM96` (replace `my_firmware.uf2` with your file name and `COM96` with your upload port).
