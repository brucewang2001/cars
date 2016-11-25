from RPLCD import CharLCD
import socket
import fcntl
import struct

lcd = CharLCD(cols=16, rows=2, pin_rs=37, pin_e=35, pins_data=[33, 31, 29, 23])

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915, 
        struct.pack('256s', ifname[:15])
    )[20:24])

lcd.write_string("IP Address:") 

lcd.cursor_pos = (1, 0)
lcd.write_string(get_ip_address('eth0'))