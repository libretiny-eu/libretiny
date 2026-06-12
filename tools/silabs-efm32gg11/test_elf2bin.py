import shutil
import subprocess

import pytest
from elf2bin import elf_to_bin


@pytest.fixture
def fixture_elf(tmp_path):
    """Build a tiny ARM ELF using the system arm-none-eabi-gcc; skip if unavailable."""
    if not shutil.which("arm-none-eabi-gcc"):
        pytest.skip("arm-none-eabi-gcc not available")
    src = tmp_path / "tiny.c"
    src.write_text("int main(void){ return 0; }\n")
    elf = tmp_path / "tiny.elf"
    subprocess.run(
        [
            "arm-none-eabi-gcc",
            "-mcpu=cortex-m4",
            "-mthumb",
            "-nostartfiles",
            "-Wl,--entry=main",
            str(src),
            "-o",
            str(elf),
        ],
        check=True,
    )
    return elf


def test_elf_to_bin_produces_nonempty_bin(fixture_elf, tmp_path):
    out = tmp_path / "tiny.bin"
    elf_to_bin(fixture_elf, out)
    assert out.exists()
    assert out.stat().st_size > 0


def test_elf_to_bin_missing_objcopy_raises(monkeypatch, tmp_path):
    monkeypatch.setattr("shutil.which", lambda _: None)
    with pytest.raises(FileNotFoundError):
        elf_to_bin(tmp_path / "any.elf", tmp_path / "any.bin")
