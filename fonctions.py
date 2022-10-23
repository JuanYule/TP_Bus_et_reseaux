
# -*- coding: utf-8 -*-
# Code developé par RAMOS et YULE
import time
import serial
import serial.tools.list_ports
import threading
import sys

#montre tous les ports connectes 
myports = [tuple(p) for p in list(serial.tools.list_ports.comports())]

SERIAL_PORT = '/dev/ttyUSB0'

port_serial = [port for port in myports if SERIAL_PORT in port ][0]

#Configuration du port serial
ser = serial.Serial(
    port=SERIAL_PORT,
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

def check_presence(correct_port, interval=0.1):
    while True:
        myports = [tuple(p) for p in list(serial.tools.list_ports.comports())]
        if port_serial not in myports:
            sys.exit("\nSerial port n'est pas branche!")
            break
        time.sleep(interval)

def verification(user_Command):
    # Verification de la commande du user
    if(user_Command == 'GET_T'):
        GET_T(user_Command)
    elif(user_Command == 'GET_P'):
        GET_P(user_Command)
    elif(user_Command == 'GET_K'):
        GET_K(user_Command)
    elif(user_Command == 'GET_A'):
        GET_A(user_Command)
    elif(user_Command == 'SET_K=1234'):
        SET_K(user_Command)
    else:
        return 0

def GET_T(data):
    ''' GET temperature'''
    ser.write(data.encode())  #envoie le string GET_T
    print('Temperature')

def GET_P(data):
    ''' GET pr2essure'''
    ser.write(data.encode())  #envoie le string GET_T
    print('Pressure')
    
def GET_K(data):
    '''Coefficient K sur 10 caractères'''
    ser.write(data.encode())  #envoie le string GET_T
    print('Coefficient K')

def GET_A(data):
    ''' Angle sur 10 caractères'''
    ser.write(data.encode())  #envoie le string GET_T
    print('Angle')

def SET_K(data):
    ''' Fixe le coefficient K (en 1/100e)'''
    ser.write(data.encode())  #envoie le string GET_T
    print('Coefficient K 1/100e')

#Verification si le port seriel est brache
port_controller = threading.Thread(target=check_presence, args=(port_serial, 0.1,))
port_controller.setDaemon(True)
port_controller.start()
