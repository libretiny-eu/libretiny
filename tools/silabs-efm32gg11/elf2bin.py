import shutil
import subprocess
from pathlib import Path


def elf_to_bin(elf_path: Path, bin_path: Path) -> None:
    """Convert an ELF to a flat binary using arm-none-eabi-objcopy.

    Phase 1 path: no padding, no header, no signing. Phase 2/3 may
    wrap this with .gbl packaging if Gecko Bootloader lands.
    """
    objcopy = shutil.which("arm-none-eabi-objcopy")
    if not objcopy:
        raise FileNotFoundError("arm-none-eabi-objcopy not on PATH")
    subprocess.run(
        [objcopy, "-O", "binary", str(elf_path), str(bin_path)],
        check=True,
    )
