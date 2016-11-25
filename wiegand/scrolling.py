framebuffer = [
    '',
    '',
]

def write_to_lcd(lcd, framebuffer, num_cols):
    lcd.home()
    for row in framebuffer:
        lcd.write_string(row.ljust(num_cols)[:num_cols])
        lcd.write_string('\r\n')

from RPLCD import CharLCD
lcd = CharLCD(cols=16, rows=2, pin_rs=37, pin_e=35, pins_data=[33, 31, 29, 23])
write_to_lcd(lcd, framebuffer, 16)

import time
long_string = 'This string is too long to fit'

def loop_string(string, lcd, framebuffer, row, num_cols, delay=0.5): #DELAY= CONTROLS THE SPEED OF SCROLL
    padding = ' ' * num_cols
    s = padding + string + padding
    for i in range(len(s) - num_cols + 1):
        framebuffer[row] = s[i:i+num_cols]
        write_to_lcd(lcd, framebuffer, num_cols)
        time.sleep(delay)

while True:
    loop_string(long_string, lcd, framebuffer, 1, 16)
    