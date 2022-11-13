
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

# port_serial = [port for port in myports if SERIAL_PORT in port ][0]

#Configuration du port serial
ser = serial.Serial(
    port=SERIAL_PORT,
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

def receptionRx():
    t_end = time.time() + 2 # secondes à attendre pour recevoir le message apres de la commande GET
    flag = 0
    while time.time() < t_end:
        bytesToRead = ser.in_waiting
        # print(bytesToRead)
        if(bytesToRead > 0):
            flag = 1
            res = ser.read(bytesToRead)
            print(res.decode('utf-8'))
            ser.reset_input_buffer()
    #         res = ser.readline()
    #         return res
    # if(flag == 0):
    #     ser.reset_input_buffer()
    #     print('\On ne peut pas recevoir des donnes de la STM32\n\r') 
    # else:
    #     pass

# def check_presence(correct_port, interval=0.1):
#     while True:
#         myports = [tuple(p) for p in list(serial.tools.list_ports.comports())]
#         if port_serial not in myports:
#             sys.exit("\nSerial port n'est pas branche!")
#             break
#         time.sleep(interval)

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
    elif(user_Command == 'SET_K'):
        SET_K(user_Command)
    else:
        return 0
    
def send_character(string):
    to_array = [char for char in string]
    print(to_array)
    for x in string:
        ser.write(x.encode())
        time.sleep(0.1)
    print('Transmition raspberry fini')

def GET_T(data):
    ''' GET temperature'''
    send_character(data)
    print('Valeur de la temperature est :')
    receptionRx()

def GET_P(data):
    ''' GET pressure'''
    send_character(data)  #envoie le string GET_P
    print('La pressure est:')
    receptionRx()
    
def GET_K(data):
    '''Coefficient K sur 10 caractères'''
    send_character(data)  #envoie le string GET_K
    print('Coefficient K est :')
    receptionRx()

def GET_A(data):
    ''' Angle sur 10 caractères'''
    send_character(data)  #envoie le string GET_A
    print("l'angle est:")
    receptionRx()

def SET_K(data):
    ''' Fixe le coefficient K (en 1/100e)'''
    send_character(data)  #envoie le string SET_K
    print('Coefficient K 1/100e est:')
    receptionRx()

