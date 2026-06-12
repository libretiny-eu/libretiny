from unittest.mock import MagicMock, patch

import pytest
from flash import flash_bin, reset_device


def test_flash_bin_invokes_commander(tmp_path):
    bin_path = tmp_path / "fw.bin"
    bin_path.write_bytes(b"\x00\x01\x02")
    with patch("shutil.which", return_value="/usr/local/bin/commander"), patch(
        "subprocess.run"
    ) as run:
        run.return_value = MagicMock(returncode=0)
        flash_bin(bin_path, device="EFM32GG11B820F2048GM64")
        run.assert_called_once()
        cmd = run.call_args[0][0]
        assert "commander" in cmd[0]
        assert "flash" in cmd
        assert "--device" in cmd
        assert "EFM32GG11B820F2048GM64" in cmd
        assert str(bin_path) in cmd


def test_flash_bin_missing_commander_raises(tmp_path):
    with patch("shutil.which", return_value=None):
        with pytest.raises(FileNotFoundError):
            flash_bin(tmp_path / "fw.bin", device="EFM32GG11B820F2048GM64")


def test_reset_device_invokes_commander():
    with patch("shutil.which", return_value="/usr/local/bin/commander"), patch(
        "subprocess.run"
    ) as run:
        run.return_value = MagicMock(returncode=0)
        reset_device()
        cmd = run.call_args[0][0]
        assert "commander" in cmd[0]
        assert "device" in cmd
        assert "reset" in cmd
