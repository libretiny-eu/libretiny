# Exception decoder

Configure built-in hard fault decoder in `platformio.ini`:

```ini
[env:my_board]
monitor_speed = 115200
monitor_filters = rtl_hard_fault_decoder
```
