from RPLCD import CharLCD, cleared, cursor
lcd = CharLCD(cols=16, rows=2, pin_rs=37, pin_e=35, pins_data=[33, 31, 29, 23])

smiley = (
    0b00000,
    0b01010,
    0b01010,
    0b00000,
    0b10001,
    0b10001,
    0b01110,
    0b00000,
)
lcd.create_char(0, smiley)
lcd.write_string(unichr(0))
