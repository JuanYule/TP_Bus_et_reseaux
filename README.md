# TP:  Bus et réseaux industriels
### Description
Le but du tps est d'acquérir les données du capteur BMP280 par le STM32 et ensuite de contrôler ces données par une interface API.
La figure suivante illustre l'architecture générale du système :
![Structure du projet](/images/TP_complet.png "Structure du projet")



**Table of Contents**
1. [TP1](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP1)
2. [TP2](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP2)
3. [TP3](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP3)
4. [TP4](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP4)
5. 
## TP1
Interrogation des capteurs par le bus I²2C
 ![architecture_TP1](/images/architecture_TP1.png "Architecture TP1")
## TP2
Interfaçage STM32 <-> Raspberry Pi
 ![architecture_TP2](/images/architecture_TP2.png "Architecture TP2")

|   Requête du RPi     |   Réponse du STM    | Commentaire |
| :------------: |:---------------:| :-----:|
| GET_T      | T=+12.50_C | Température compensée sur 10 caractères    |
| GET_P      | P=102300Pa |   Pression compensée sur 10 caractères     |
| SET_K=1234 | SET_K=OK   |    Fixe le coefficient K (en 1/100e)       |
| GET_K      | K=12.34000 |   Coefficient K sur 10 caractères          |
| GET_A      | A=125.7000 |    Angle sur 10 caractères                 |

## TP3
Interface Web sur Raspberry Pi
|   CRUB     |   Réponse du STM    | Commentaire |
| :------------: |:---------------:| :-----:|
| Create   | POST  | welcome/  | Change sentence |
| Retreive | GET   | welcome/  | Return sentence |
| Retreive | GET   | welcome/x | Return letter x |
| Update   | PUT   | welcome/x | Insert new word at position x |
| Update   | PATCH | welcome/x | Change letter at position x |
| Delete   | DELETE| welcome/x | Delete letter at position x |
| Delete   | DELETE| welcome/  | Delete sentece |

## TP4
Interface API Rest & pilotage d'actionneur par bus CAN
 ![architecture_TP4](/images/architecture_TP4.png "Architecture TP4")
