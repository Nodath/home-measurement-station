import RPi.GPIO as GPIO
import random
import math
from random import randint

import paho.mqtt.client as mqtt

import tkinter as tk
import time
from tkinter import *
from PIL import ImageTk, Image
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)

from matplotlib.backend_bases import key_press_handler
from matplotlib.figure import Figure
from threading import Thread, Lock

#GLOBAL VARIABLE
#xAxis
iCounter  = 0
MQTT_SERVER = "192.168.43.196"
MQTT_PATH = "test/message"
client = mqtt.Client()
temperature = 0.0
humidity = 0.0
got_message = False
mutex = Lock()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(MQTT_PATH)
    
def on_message(client, userdata, msg):
    #print(msg.payload)
    msg_to_parse = msg.payload
    msg_list = msg_to_parse.split()
    global temperature
    temperature = float(msg_list[0])
    global humidity
    humidity = float(msg_list[1])
    got_message = True

class Page(tk.Frame):
    def __init__(self, *args, **kwargs):
        tk.Frame.__init__(self, *args, **kwargs)
    def show(self):
        self.lift()


class pgCharts(Page):
    def __init__(self, *args, **kwargs):
        Page.__init__(self, *args, **kwargs)
        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.fig.patch.set_facecolor('grey')
        self.canvas = FigureCanvasTkAgg(self.fig, master=self)  # A tk.DrawingArea.
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side="top", fill="both", expand=1)
        self.toolbar = NavigationToolbar2Tk(self.canvas, self)
        self.toolbar.update()
        self.canvas.get_tk_widget().pack(side="top", fill="both", expand=1)
        self.tTimer = [] #time Counter
        self.temperatureList = []  
        self.humidityList = []
        self.xSide = []
        self.ySide_1 = []
        self.ySide_2 = []

    
   def fsave_to_Charts(self):
        global iCounter
        iCounter+=1 
        if not math.isnan(temperature):
            self.temperatureList.append(temperature)
        if not math.isnan(humidity):
            self.humidityList.append(humidity)
              
        self.tTimer.append(iCounter)

        for i in range(len(self.tTimer)):
            self.xSide.append(self.tTimer[i])
            self.ySide_1.append(self.temperatureList[i])
            self.ySide_2.append(self.humidityList[i])


        self.fig.add_subplot(111).plot(self.xSide,self.ySide_1, self.ySide_2)
        self.fig.suptitle("Temperature&Humidity Real Time Chart")
        self.canvas.draw()
        self.fig.clf()
        self.tTimer.clear()
        self.temperatureList.clear()
        self.humidityList.clear()

class pgCloud(Page):
   def __init__(self, *args, **kwargs):
       Page.__init__(self, *args, **kwargs)
   

class MainView(tk.Frame):
    def __init__(self, *args, **kwargs):
        tk.Frame.__init__(self, *args, **kwargs)
        self.pCharts = pgCharts(self)
        pCloud = pgCloud(self)
        buttonframe = tk.Frame(self)
        buttonframe.pack(side="top", fill="x", expand=False)
        container = tk.Frame(self)
        container.pack(side="top", fill="both", expand=True)
        self.pCharts.place(in_=container, x=0, y=0, relwidth=1, relheight=1)
        pCloud.place(in_=container, x=0, y=0, relwidth=1, relheight=1)

        self.pCharts.show()
        self.fEmbeddedCall()
 
    def fEmbeddedCall(self):
        self.pCharts.fsave_to_Charts()
        self.after(1000,self.fEmbeddedCall)


def gui_thread():
    root = tk.Tk()
    main = MainView(root)
    main.pack(side="top", fill="both", expand=True)
    root.wm_geometry("800x400")
    for i in range(1, 100):    
        time.sleep(10)
        root.mainloop()
    

def mqtt_thread():
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_SERVER, 1883, 60)
    client.loop_forever()


if __name__ == "__main__":
    t1 = Thread(target = gui_thread)
    t2 = Thread(target = mqtt_thread)
    t1.start()
    t2.start()
