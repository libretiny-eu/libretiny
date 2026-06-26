import shutil
import subprocess
from pathlib import Path
from typing import Optional


def _commander() -> str:
    p = shutil.which("commander")
    if not p:
        raise FileNotFoundError(
            "Simplicity Commander ('commander') not on PATH. Install from "
            "https://www.silabs.com/developers/simplicity-studio/simplicity-commander"
        )
    return p


def flash_bin(bin_path: Path, *, device: str, serialno: Optional[str] = None) -> None:
    """Flash a flat binary to the EFM32GG11 via Simplicity Commander."""
    cmd = [_commander(), "flash", "--device", device, str(bin_path)]
    if serialno:
        cmd.extend(["--serialno", serialno])
    subprocess.run(cmd, check=True)


def reset_device(serialno: Optional[str] = None) -> None:
    """Pulse RESET via Commander."""
    cmd = [_commander(), "device", "reset"]
    if serialno:
        cmd.extend(["--serialno", serialno])
    subprocess.run(cmd, check=True)
