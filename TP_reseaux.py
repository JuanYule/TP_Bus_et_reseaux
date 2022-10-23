# -*- coding: utf-8 -*-
# Code developé par RAMOS et YULE

from fonctions import *
import time

userCommand = '' #stokage de la valeur 
command_list = ['GET_T', 'GET_P', 'GET_K', 'GET_A', 'SET_K=1234', 'exit', 'EXIT']

while True:
    try:
        print('Bonjour, entrez la commande')
        userCommand = input() #La valeur de la commande est obtenu
        
        if(userCommand in command_list):
            if(userCommand == 'exit'or userCommand =='EXIT'):
                print("\n...Le programme est arrête par des commandes!")
                break
            else:
                verification(userCommand)
        else:
            print('Commande incorrecte')
            
    except (KeyboardInterrupt, SystemExit):
        print('\n keyboardinterrupt caught (again)')
        print('\n ...Program Stopped Manually!')
        raise
