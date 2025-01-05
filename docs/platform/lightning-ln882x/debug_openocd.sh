#!/bin/bash

#sudo openocd -f /usr/share/openocd/scripts/interface/stlink.cfg -f ../../../cores/lightning-ln882x/misc/ln882x.cfg -c 'reset_config srst_only' $*
sudo openocd -f /usr/share/openocd/scripts/interface/jlink.cfg -f ../../../cores/lightning-ln882x/misc/ln882x.cfg -c 'reset_config srst_only' $*
