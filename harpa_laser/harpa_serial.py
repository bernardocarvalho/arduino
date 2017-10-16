# https://github.com/bspaans/python-mingus/tree/master/mingus_examples
from mingus.midi import fluidsynth
from mingus.containers import Note
import serial
import numpy as np
from time import time 
#import time

fluidsynth.init('/Users/bernardo/.fluidsynth/default_sound_font.sf2')
fluidsynth.play_Note(Note("C-5"))
#time.sleep(1)

# If you're not using Linux, you'll need to change this
# check the Arduino IDE to see what serial port it's attached to
#ser = serial.Serial('/dev/ttyACM0', 115200)
#MAC  check ls //dev/cu.*
ser = serial.Serial('/dev/cu.usbmodem1421', 115200)

# set plot to animated

notes = ['C', 'D', 'E']

start_time = time()
duration = 20 # total seconds to collect data
end_time = start_time + duration

run = True

# collect the data and plot a moving frame
while run:
    ser.reset_input_buffer()
    data = ser.readline().split(' ')

    # sometimes the incoming data is garbage, so just 'try' to do this
    try:
        #print data
        #print data[3]
        note = data[3] + "-5"
        note_time = int(data[5])
        
        if (note_time > 0):
            print note_time
            fluidsynth.play_Note(Note(note))
        
    # if the try statement throws an error, just do nothing
    except: pass
    current_time = time()
    # when time's up, kill the collect+plot loop
    if current_time > end_time: run=False
    
    # update the plot

ser.close()
