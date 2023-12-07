# Tuya Pinout Config

Device configuration (`user_param_key`) can be extracted to JSON, using bk7231tools from a full firmware dump.

Originally posted [by @blakadder](https://discord.com/channels/967863521511608370/983843871320580096/1059286760074530947) on Discord channel #resources, modified by me to include more keys and values.

Key(s)                                      | Meaning                               | Possible values
--------------------------------------------|---------------------------------------|---------------------------------------------------------
`crc`                                       |                                       |
`module`                                    |Actual hw board used                   | `CB3S` / `WB3S`, maybe others
`category`                                  |Device type encoded on a set of digits | 0502 - cw light<br>0505 - rgbcw light
`Jsonver`<br>`jv`                           |                                       |
**Common**                                  |                                       |
`netled_pin`<br>`netled1_pin`<br>`wfst_pin` | Status LED for WiFi                   |
`netled_lv`<br>`netled1_lv`<br>`wfst_lv`    | Status LED Active Level               | 0 - Active low<br>1 - Active high
`netled_reuse`                              |                                       |
`reset_pin` + `reset_lv`                    | Reset Button Pin + Active Level       |
`reset_t`                                   | Button press time to reset the device | 3/5/6/9/10 seconds
`iicscl`                                    | I²C SCL Pin                           |
`iicsda`                                    | I²C SDA Pin                           |
`net_trig`                                  |                                       |
`net_type`                                  |                                       |
`wfcfg`                                     | Pairing related - not relevant        | `spcl` / `spcl_auto` / `prod`
`wfct`                                      |                                       |
**Lights/bulbs**                            |                                       |
`cmod`                                      | Color Mode                            | `rgbcw` / `rgb` / `cw` / `c` / `rgbc`
`cwtype`                                    | Color temperature driver              | 0 - cool and warm white (CW)<br>1 - correlated color temperature (CCT)
`brightmin`, `brightmax`                    | Min/Max Brightness                    | 0%-100%
`cwmin`, `cwmax`                            | Cold-Warm Min/Max Brightness          | 0%-100%
`cwmaxp`                                    | Cold-Warm Max Power                   | 0%-100%
`colormin`, `colormax`                      | RGB Min/Max Brightness                | 0%-100%
`colormaxp`                                 | RGB Max Power                         | 0%-100%
`brightstep`<br>`bristep`                   | Brightness Step                       |
`defbright`                                 | Default Brightness                    | 0%-100%
`defcolor`                                  | Default Color                         | `c` / `r`
`deftemp`                                   | Default Color Temperature             | 0-100 when defcolor is cool white
`gmkr`, `gmkg`, `gmkb`                      |                                       |
`gmwr`, `gmwg`, `gmwb`                      |                                       |
`hsvstep`                                   |                                       |
`rgbt`                                      |Used in prod.tests, not relevant       |
`rstbr`                                     |Pairing related - not relevant         | 10-100
`rstcor`                                    |Pairing related - not relevant         | `c`/`r`
`rsttemp`                                   |Pairing related - not relevant         | 0-100
**PWM Lights**                              |                                       |
`r_pin` + `r_lv`                            | Red Channel Pin + Active Level        |
`g_pin` + `g_lv`                            | Green Channel Pin + Active Level      |
`b_pin` + `b_lv`                            | Blue Channel Pin + Active Level       |
`c_pin` + `c_lv`                            | Cool White Pin + Active Level         |
`w_pin` + `w_lv`                            | Warm White Pin + Active Level         |
`pwmhz`                                     | PWM Operating Frequency (Hz)          |
**I²C Lights**                              |                                       |
`dccur`<br>`ehccur`<br>`cjccur`             | Cold White Current                    |
`dwcur`<br>`ehwcur`<br>`cjwcur`             | Warm White Current                    |
`drgbcur`                                   | RGB Current                           |
`campere`                                   | Max current of SM2135 colored output  | 10-45 with a pitch of 5 and defaults to 20
`wampere`                                   | Max current of SM2135 white output    | 10-80 with a pitch of 5 and defaults to 30
`iicr`                                      | Red Channel Number                    | 0-5
`iicg`                                      | Green Channel Number                  | 0-5
`iicb`                                      | Blue Channel Number                   | 0-5
`iicc`                                      | Cold White Channel Number             | 0-5
`iicw`                                      | Warm White Channel Number             | 0-5
`iicccur`                                   | Cold White Current                    | 0
`iicwcur`                                   | Warm White Current                    | 5
**Sockets/switches**                        |                                       |
`btX_pin` + `btX_lv`                        | Button X Pin + Active Level           |
`btX_type`<br>`bt_type`                     | Button X Trigger Type                 | 0 - level_trig<br>1 - edge_trig
`rlX_pin` + `rlX_lv`                        | Relay X Pin + Active Level            |
`rlX_type`<br>`rl_type`                     | Relay X Type                          | 0 - Electric holding relay<br>1 - Magnetic holding relay
`rl_onX_pin` + `rl_onX_lv`                  | Relay ON Pin + Active Level           |
`rl_offX_pin` + `rl_offX_lv`                | Relay OFF Pin + Active Level          |
`rl1_dr_type`                               |                                       |
`rl_drvtime`                                |                                       |
`total_bt_pin` + `total_bt_lv`              |                                       |
**Power monitoring**                        |                                       |
`ele_fun_en`                                | Power Monitoring Enabled              | 1
`chip_type`                                 | Power Monitoring Chip Type            | 0 - BL0937<br>1 - HLW8012<br>2 - HLW8032<br>4 - BL0942
`ele_pin`                                   | CF Pin                                |
`vi_pin`                                    | CF1 Pin                               |
`sel_pin_pin` + `sel_pin_lv`                | SEL Pin + Active Level                | Active level is usually 1
`lose_vol`                                  | Under voltage threshold in V          |
`over_cur`                                  | Overcurrent threshold in mA           |
`over_vol`                                  | Overvoltage threshold in V            |
`sample_resistor`                           | Current shunt resistor value          | 1 - 1mΩ<br>2 - 2mΩ
`vol_def`                                   | Socket operating voltage              | 0 - 220V<br>1 - 110V
`work_voltage`                              | Socket operating voltage              |
**Infrared**                                |                                       |
`irfunc`                                    | IR Function                           | 0, 1
`infre`                                     | IR Transmitter Pin                    |
`infrr`<br>`ir`                             | IR Receiver Pin                       |
`irkXfun` + `irkXval`                       | IR Key X Function + Value             | X in 1..30
`irnightt`                                  |                                       |
`irstep`                                    |                                       |
`wgmod`, `swgmod`, `scgmod`                 |                                       |
**PIR**                                     |                                       |
`pirmod`                                    |                                       |
`pirfreq`                                   | PWM Operating Frequency (Hz) for PIR  | 1000
`pirlduty`                                  |                                       | 100
`pirmduty`                                  |                                       | 50
`pirhduty`                                  |                                       | 0
`pirin_pin` + `pirin_lv`                    | GPIO pin used to report motion + Active Level|
`pirsense_pin` + `pirsense_lv`              | PIR sensitivity pwm pin + Active Level|
`pirrange`                                  |                                       |
`pirwarn`                                   |                                       |
**Key-controlled**                          |                                       |
`key_pin` + `key_lv`                        | Key Pin + Active Level                |
`kXpin_pin` + `kXpin_lv`                    |                                       |
`kXdfunc`, `kXlfunc`, `kXsfunc`             |                                       |
`kXldir`, `kXsdir`                          |                                       |
`keyccfg1`, `keyccfg2`                      |                                       |
`keyfunc`, `keyglobefunc`                   |                                       |
`keylt`, `keynumber`                        |                                       |
**Other**                                   |                                       |
`buzzer_pwm`                                | Buzzer working PWM frequency          |
`ismusic`                                   |                                       | 0, 1
`ledX_pin` + `ledX_lv`                      | LED X Pin + Active Level              |
`led_pin` + `led_lv`                        | LED Pin + Active Level                |
**Unknown**                                 |                                       |
`0err`                                      |                                       |
`1err`                                      |                                       |
`adclimit`                                  |                                       |
`aging`                                     |                                       |
`alarm1_time`                               |                                       |
`alarm_t1`                                  |                                       |
`backlit_dp`                                |                                       |
`backlit_select`                            |                                       |
`bitseq`                                    |                                       |
`bleonoff`                                  |                                       |
`blindt`                                    |                                       |
`buzzer`                                    |                                       |
`cagt`                                      |Used in prod.tests, not relevant       | N/A
`cctseg`                                    |                                       |
`cd_flag2`                                  |                                       |
`cdsval`                                    |                                       |
`ch1_stat`                                  |                                       |
`ch_cddpidX`                                |                                       | X in 1..4
`ch_dpidX`                                  |                                       | X in 1..4
`ch_flagX`                                  |                                       | X in 1..4
`ch_num`                                    |                                       |
`clean_t`                                   |                                       |
`cntdown1`                                  |                                       |
`colorpfun`                                 |The power limit of the mix of colors   |0 - no limits<br>1 - limits specified
`ctrl_lv`                                   |                                       |
`ctrl_pin`                                  |                                       |
`customcode`                                |                                       |
`cyc_dpid`                                  |                                       |
`day`                                       |                                       |
`dctrl_select`                              |                                       |
`dimmod`                                    |                                       |
`dimt`                                      |                                       |
`dimval`                                    |                                       |
`dmod`                                      | Light driver                          | 0 - PWM<br>1 - SM16726B<br>2 - SM2135E
`defcolor`                                  | Default light color                   | 
`door1_magt_lv`                             |                                       |
`door1_magt_pin`                            |                                       |
`door_alarm_st1`                            |                                       |
`door_mag1`                                 |                                       |
`dusk`                                      |                                       |
`evenfall`                                  |                                       |
`evening`                                   |                                       |
`ffc_select`                                |                                       |
`inch_dp`                                   |                                       |
`indep_cfgbt`                               |                                       |
`init_conf`                                 |                                       |
`knum`                                      |                                       |
`ktime`                                     |                                       |
`leaderr`                                   |                                       |
`led_dp`                                    |                                       |
`lfunc`                                     |                                       |
`light_status_select`                       |                                       |
`lock_dp`                                   |                                       |
`lockt`                                     |                                       |
`micpin`                                    |                                       |
`mixway`                                    |                                       |
`mutex`                                     |                                       |
`mxcl_led_m`                                |                                       |
`netn_led`                                  |                                       |
`netnc`                                     |                                       |
`nety_led`                                  |                                       |
`netyc`                                     |                                       |
`night`                                     |                                       |
`nightbrig`                                 |                                       |
`nightcct`                                  |                                       |
`nightled`                                  |                                       |
`notdisturb`                                |Do not disturb (DND) mode              | 0 - DND disabled<br>1 - DND enabled
`on_off_cnt`                                |                                       |
`onoff1`                                    |                                       |
`onoff_clear_t`                             |                                       |
`onoff_n`                                   |                                       |
`onoff_rst_m`                               |                                       |
`onoff_rst_type`                            |                                       |
`onoff_type`                                |                                       |
`onoffmode`                                 |On/off gradient                        | 0 - Gradient is provided when the light is turned on/off.<br>1 - Gradient is not provided when the light is turned on/off.
`onofftime`                                 |                                       |
`owm`                                       |                                       |
`pairt`                                     |Pairing related - not relevant         | 6-600
`pmemory`                                   |Power-off memory                       | 0 - power-off memory disabled<br>1: power-off memory enabled
`preheatt`                                  |                                       |
`prodagain`                                 |Used in prod.tests, not relevant       | 0-1
`rand_dpid`                                 |                                       |
`remdmode`                                  |Pairing related - not relevant         | 0-1
`remote_add_dp`                             |                                       |
`remote_list_dp`                            |                                       |
`remote_select`                             |                                       |
`resistor`                                  |                                       |
`reuse_led_m`                               |                                       |
`rsthold`                                   |                                       |
`rstmode`                                   |Pairing related - not relevant         |
`rstnum`                                    |Pairing related - not relevant         |
`scenespct`                                 |                                       |
`series_ctrl`                               |                                       |
`sfunc`                                     |                                       |
`standtime`                                 |                                       |
`starterr`                                  |                                       |
`step_rate`                                 |                                       |
`switch1`                                   |                                       |
`tempmix`                                   |                                       |
`tempstep`                                  |                                       |
`title20`                                   |                                       |
`total_stat`                                |                                       |
`tracetime1`                                |                                       |
`trigdelay`                                 |                                       |
`trigmod`                                   |                                       |
`trl1_time`                                 |                                       |
`voice_ctrl1`                               |                                       |
`voice_ctrl_set1`                           |                                       |
`whiteseg`                                  |                                       |
`wt`                                        |Used in prod.tests, not relevant       | N/A
`zero_select`                               |                                       |
