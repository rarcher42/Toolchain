import sys
import time
import math

cur_time = [0.0, 0.0, 0.0, 0.0]
last_time = [0.0, 0.0, 0.0, 0.0]    # Remember timestamp of last controller change
last_val = [-1, -1, -1, -1]         # Remember last MIDI controller value to determine direction
position = [0.0, 0.0, 0.0, 0.0]     # Calculated control position from rotation speed & direction.  Keep fractions, discard later if desired

def clamp(val):
    if val > 16383.0:
        val = 16383.0
    elif val < 0.0:
        val = 0.0
    return val

# Compute direction of motion
def get_dir(last, current):
    if (last > -1):
        if current == 127:
            dir = 1     # Must be increasing; at maximum already
        elif current == 0:
            dir = -1    # Must be decreasing; at zero already
        elif current > last:
            dir = 1
        else:
            dir = -1
    else:
        dir = 0         # No known last position - no motion yet
    return dir

def get_accel_power_law(elapsed_ms, fast_ms=0.1, exponent=1.2):
    t = max(float(elapsed_ms), fast_ms)
    u = (100.0 - t) / (100.0 - fast_ms)
    u = max(0.0, min(1.0, u))
    return 1.0 + 170.0 * (u ** exponent)

def get_accel_log_law(elapsed_ms, fast_ms=0.1, exponent=1.0):
    u = math.log(100.0/elapsed_ms) / math.log(100.0/fast_ms)
    # Perhaps unexpectedly, python's math library uses base e for log() by derault
    if (u < 0.0):
        u = 0.0
    elif (u > 1.0):
        u = 1.0     # Working for the Clampdown
    accel = 1 + math.pow(u, exponent) * 170.7
    return accel


def get_increment(rot_dir, key):
    assert (key < 4)
    lt = last_time[key]
    ts = cur_time[key]
    diff = 1000 * (ts - lt)
    increment = 1
    accel = get_accel_log_law(diff)
    increment = rot_dir * accel
    return increment

def cmd_note_off(cmd):
    c = midi.read(1)
    key = int.from_bytes(c, byteorder='little')
    c = midi.read(1)
    velocity = int.from_bytes(c, byteorder='little')
    print("NoteOFF(key=%d, velocity=%d)" % (key, velocity))

def cmd_note_on(cmd):
    c = midi.read(1)
    key = int.from_bytes(c, byteorder='little')
    c = midi.read(1)
    velocity = int.from_bytes(c, byteorder='little')
    print("NoteON(key=%d, velocity=%d)" % (key, velocity))

def cmd_controller(cmd):
    c = midi.read(1)
    key = int.from_bytes(c, byteorder='little') & 0x3
    cur_time[key] = time.monotonic()
    c = midi.read(1)
    value = int.from_bytes(c, byteorder='little')
    last = last_val[key]
    # Determine relative direction of rotation
    rot_dir = get_dir(last, value)
    last_val[key] = value   # Remember confirmed key position
    increment = get_increment(rot_dir, key)     # Speed sensitive control
    position[key] = clamp(position[key] + increment)  # Range-limit
    last_time[key] = cur_time[key]     # Remember when the command came in for speed determination
    print("Controller(key=%d, increment=%f, value=%f)" % (key, increment, position[key]))

dev = '/dev/dmmidi1'
midi = open(dev, 'rb')

print("midi=", midi)
while True:
    c = midi.read(1)
    d = int.from_bytes(c, byteorder='little')
    if d & 0x80:
        k = d & 0xF0
        cmd = d
        if (k == 0x80):
            cmd_note_off(cmd)
        elif (k == 0x90):
            cmd_note_on(cmd)
        elif (k == 0xB0):
            cmd_controller(cmd)

