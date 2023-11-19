from tkinter import *

import matplotlib.pyplot as plt
import matplotlib
import matplotlib.dates as mdates
import datetime as dt

import time
import serial

# initialize serial communication
ser = serial.Serial(
    port='/dev/rfcomm0',
    baudrate=9600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

ser.isOpen()

HEIGHT = 550
WIDTH = 700

up_flag=False
low_flag=False

list_of_datetimes= []
values=[]

def up_thr_modify():
    up_thr_str = 'up' + str(up_str.get()) + '\r'
    ser.write(up_thr_str.encode()) # send up thr to arduino
    global up_flag
    up_flag=True
    
    
def low_thr_modify():
    low_thr_str = 'low' + str(low_str.get()) + '\r'
    ser.write(low_thr_str.encode()) # send low thr to arduino
    global low_flag
    low_flag=True

def update_humidity():
    recv = ''
    if ser.inWaiting() > 0: # check for serial data
        recv = ser.readline() # read serial data
    if recv != '': # check if received new moisture
        t.delete('1.0',END)
        t1.delete('1.0',END)
        t.insert(INSERT,str(float(recv))+'%') # diaplay moisture in %
        now=time.strftime("%d-%m-%y %H:%M:%S") # date and hour of sample 
        t1.insert(INSERT,now) # display date and hour of sample
        now1=dt.datetime.now() # date and hour of sample in graph
        list_of_datetimes.append(now1) # add to array
        values.append(float(recv))        
        if(up_flag and low_flag): # both up and low thr received
            if(float(recv)>float(low_str.get()) and float(recv)<float(up_str.get())):
                t2.delete('1.0',END)
                t2.insert(INSERT,"Good!")
            else:
                t2.delete('1.0',END)
                t2.insert(INSERT,"Not Good! Check planet")
    root.after(1000,update_humidity) # check serial data every 1 sec

def show_graph():

    axes= plt.gca()
    axes.xaxis.set_major_formatter(mdates.DateFormatter("%d-%m-%y %H:%M")) # date and format
    axes.xaxis.set_major_locator(mdates.AutoDateLocator()) # auto scale of date and time
    plt.suptitle('Moisture [%] vs. Date time')
    plt.xlabel('Date Time')
    plt.ylabel('Moisture [%]')
    plt.grid()
    plt.plot(list_of_datetimes, values) # x and y of the graph
    plt.gcf().autofmt_xdate() # rotate x axis lable by 45 degree
    plt.show()


root = Tk()
canvas = Canvas(root, height=HEIGHT, width=WIDTH,bg='blue').pack()
frame1=Frame(root,bg='blue')
frame1.place(relx=0.05,rely=0.1,relwidth=0.9,relheight=0.2)
a= Label(frame1,text="upper threshold:").pack(side="left")
up_str=StringVar()
b= Entry(frame1,textvariable=up_str,width="10").pack(side="left")
c=Button(frame1,text="press to send",font=14,command=up_thr_modify).pack(side="left")

frame2=Frame(root,bg='blue')
frame2.place(relx=0.05,rely=0.3,relwidth=0.9,relheight=0.2)
a= Label(frame2,text="lower threshold:").pack(side="left")
low_str=StringVar()
b= Entry(frame2,textvariable=low_str,width="10").pack(side="left")
c=Button(frame2,text="press to send",font=14,command=low_thr_modify).pack(side="left")


frame3=Frame(root,bg='blue')
frame3.place(relx=0.05,rely=0.5,relwidth=0.9,relheight=0.2)
a= Label(frame3,text="last moisture checked:").pack(side="left")
t= Text(frame3,height=1, width=10)
t.pack(side="left")
t1= Text(frame3,height=1, width=14)
t1.pack(side="left")

frame4=Frame(root,bg='blue')
frame4.place(relx=0.05,rely=0.7,relwidth=0.9,relheight=0.2)
a= Label(frame4,text="Status:").pack(side="left")
t2= Text(frame4,height=1, width=23)
t2.pack(side="left")



frame5=Frame(root,bg='blue')
frame5.place(relx=0.6,rely=0.7,relwidth=0.9,relheight=0.2)
c=Button(frame5,text="show graph",font=14,command=show_graph).pack(side="left")

frame6=Frame(root,bg='blue')
frame6.place(relx=0.55,rely=0.2,relwidth=0.7,relheight=0.2)
a= Label(frame6,text="1) Enter numbers between 0-100 \n 2) make sure up threshold > low threshold").pack(side="left")

root.after(1000,update_humidity) # check serial data every 1 sec

root.mainloop()


