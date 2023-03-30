set(src
"main.c"
../project/Bluetooth/bluetooth.c 
../project/Display/Static/display_static.c 
../project/Display/Static/display_toggle.c 
../project/Display/Scroll/display_scroll.c 
../project/Display/Clock/display_clock.c 
../project/Display/display.c 
../project/Receive/receive.c 
../project/Receive/setting.c 
../project/Receive/getting.c 
../project/Receive/response.c
../project/Scanning/scanning.c 
../project/Timer/timer.c 
../project/I2C/i2c_device.c
../project/I2C/at24cxx.c
../project/I2C/rx8025t.c
../project/Key/key.c
)
 
set(inc
"."
../project/Bluetooth
../project/Display
../project/Display/Static
../project/Display/Scroll
../project/Display/Clock
../project/Receive
../project/Scanning
../project/Timer
../project/I2C
../project/Key
)
 
 
idf_component_register(
SRCS ${src}
INCLUDE_DIRS ${inc})