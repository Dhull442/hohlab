source ~/peda/peda.py
file _tmp/hoh.exe
target remote localhost:1234
b c_lapic_internal
