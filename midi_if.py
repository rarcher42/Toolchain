import sys
import time
import math

CSV_FILE = "SN1371.csv"
cur_time = [0.0, 0.0, 0.0, 0.0]
last_time = [0.0, 0.0, 0.0, 0.0]    # Remember timestamp of last controller change
last_val = [-1, -1, -1, -1]         # Remember last MIDI controller value to determine direction
position = [0.0, 0.0, 0.0, 0.0]     # Calculated control position from rotation speed & direction.  Keep fractions, discard later if desired
byte_table = bytearray(16384*16)    # Slot[i] @ byte_table[16*i] .. byte_table[16*i+15]


def get_channel(index, channel):
    base = int(16*index + 2*channel)
    val = ((byte_table[base+1] & 0xFF) << 8) | (byte_table[base] & 0xFF)
    return val

def get_vector(index):
    vec = []
    for ch in range(0, 8):
        vec.append(get_channel(index, ch))
    return vec


def process_dial_change(index):
        print("%05d :" % index)
        vectors = get_vector(index)
        s = "PS"
        for v in vectors:
            s += "%04X" % v
        return s


def read_csv(fn):
    table_index = 0
    ti = 0
    with open(fn) as fh:
        for line in fh:
            values = line.split(',')
            for i in range(1, len(values)):
                val = int(float(values[i].strip()))
                byte_table[ti] = val & 0xFF
                ti += 1
                byte_table[ti] = (val >> 8) & 0xFF
                ti += 1
            table_index += 1

    for i in range(0,1000):
        print("%05d: " % i, end="")
        for j in range(0,8):
            print("$%04X " % get_channel(i, j), end="")
        print("")
    print("table_index = ", table_index)


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
    if key == 0:
        process_dial_change(int(position[0]))
    else:
        print("Controller(key=%d, value=%f)" % (key, position[key]))

read_csv(CSV_FILE)
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

