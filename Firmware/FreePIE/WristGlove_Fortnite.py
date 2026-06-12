import clr
import serial
import time

# ── CONFIGURATION ──────────────────────────────────────────────────
COM_PORT = "COM3"       # change to your Arduino Nano COM port
BAUD_RATE = 115200
DEADZONE = 2.0          # degrees per second minimum to move cursor
SENSITIVITY = 15.0      # cursor speed multiplier
AIM_HOLD_KEY = Key.MiddleButton  # hold to enable aim mode

# ── STATE ──────────────────────────────────────────────────────────
yaw = 0.0
pitch = 0.0
indexCurl = 0.0
middleCurl = 0.0
ringCurl = 0.0
thumbCurl = 0.0
pinkyCurl = 0.0
pinch0 = False
pinch1 = False
pinch2 = False
aimMode = False

# ── PARSE INCOMING SERIAL LINE ─────────────────────────────────────
def parseLine(line):
    global yaw, pitch, indexCurl, middleCurl
    global ringCurl, thumbCurl, pinkyCurl
    global pinch0, pinch1, pinch2

    try:
        parts = line.strip().split(' ')
        data = {}
        for part in parts:
            if ':' in part:
                key, val = part.split(':', 1)
                data[key] = val

        if 'I'   in data: indexCurl  = float(data['I'])
        if 'M'   in data: middleCurl = float(data['M'])
        if 'R'   in data: ringCurl   = float(data['R'])
        if 'T'   in data: thumbCurl  = float(data['T'])
        if 'P'   in data: pinkyCurl  = float(data['P'])
        if 'YAW' in data: yaw        = float(data['YAW'])
        if 'PIN' in data:
            pinStr = data['PIN']
            pinch0 = pinStr[0] == '1' if len(pinStr) > 0 else False
            pinch1 = pinStr[1] == '1' if len(pinStr) > 1 else False
            pinch2 = pinStr[2] == '1' if len(pinStr) > 2 else False
    except:
        pass

# ── APPLY DEADZONE ─────────────────────────────────────────────────
def applyDeadzone(value, deadzone):
    if abs(value) < deadzone:
        return 0.0
    if value > 0:
        return (value - deadzone) * SENSITIVITY
    else:
        return (value + deadzone) * SENSITIVITY

# ── MAIN UPDATE LOOP ───────────────────────────────────────────────
def update():
    global aimMode

    if starting:
        # Open serial connection to Arduino Nano
        serial.openPort(COM_PORT, BAUD_RATE)

    # Read incoming line from Nano
    line = serial.readLine()
    if line:
        parseLine(line)

    # ── AIM MODE TOGGLE ────────────────────────────────────────
    # Middle finger curl held = aim mode active
    # Release = cursor pauses, reposition wrist freely
    aimMode = middleCurl > 45.0

    if aimMode:
        # Apply deadzone and sensitivity to yaw
        cursorX = applyDeadzone(yaw, DEADZONE)

        # Move mouse
        mouse.deltaX = int(cursorX)
        mouse.deltaY = 0
    else:
        # Not aiming — no cursor movement
        mouse.deltaX = 0
        mouse.deltaY = 0

    # ── INPUTS ────────────────────────────────────────────────
    # Index curl = shoot (left click)
    keyboard.setPressed(Key.LeftButton, indexCurl > 45.0)

    # Thumb to index pinch = reload
    keyboard.setPressed(Key.R, pinch0)

    # Thumb to middle pinch = switch weapon
    keyboard.setPressed(Key.LeftShift, pinch1)

    # Thumb to ring pinch = crouch
    keyboard.setPressed(Key.LeftControl, pinch2)

    # Ring curl = jump
    keyboard.setPressed(Key.Space, ringCurl > 45.0)

    # Pinky curl = build mode
    keyboard.setPressed(Key.Q, pinkyCurl > 45.0)

freePIE.register(update)