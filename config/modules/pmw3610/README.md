# PMW3610 driver

This is the inorichi PMW3610 driver, vendored from commit `1d9c2c6` of
https://github.com/inorichi/zmk-pmw3610-driver. The local change guards keymap
layer access because ZMK peripheral builds do not include the keymap APIs.

The peripheral trackball therefore runs in movement mode; scroll and automouse
layer behavior remain available on the central half only.
