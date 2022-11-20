# TP:  Bus et réseaux industriels
### Description
Le but du tps est d'acquérir les données du capteur BMP280 par le STM32 et ensuite de contrôler ces données par une interface API.
La figure suivante illustre l'architecture générale du système :
![Structure du projet](/img/TP_complet.png "Structure du projet")



**Table of Contents**
1. [TP1-Bis I2C](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP1)
2. [TP2-Interfaçage STM32 - Raspberry](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP2)
3. [TP3-Interface REST](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP3)
4. [TP4-Bus CAN](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#TP4)

## TP1 Bus I2C
Interrogation des capteurs par le bus I²2C
 ![architecture_TP1](/img/architecture_TP1.png "Architecture TP1")

 Le BMP280 est un capteur de pression et température développé par Bosch (page produit).

 A partir de la datasheet du BMP280, nous avons determiné les éléments suivantes:
 1. Les adresses I²C possibles pour ce composant sont sur 7 bits. Si on connecte la broche SDO au GND, l’adresse du composant est 111 0110 (0x76). Si la broche est connectée à Vddio, c’est 111 0111 (0x77)
 2. Le registre permettant d’identifier ce composant est nommé « id » dont l’adresse est 0xD0. Sa valeur est 0x58 et peut être lue dès que le composant est sous tension 
 3. Le registre permettant de placer le composant en mode NORMAL est le « ctrl_meas» et son adresse est 0xF4. Il faut mettre les bits de valeur 11 dans mode [1 :0]
 4. Les registres contenant l’étalonnage du composant sont à l’adresse 0x88 à l’adresse 0xA1
 5. Les registres contenant la température du composant sont nommés « temp » et sont aux adresses allant de 0xFA à 0xFC. Le format de la donnée est unsigned sur 20 bits, ut[19 :0].
 6. Les registres contenant la pression du composant sont nommés « press » et sont aux adresses allant de 0xF7 à 0xF9. Le format de la donnée est unsigned sur 20 bits, up[19 :0].
 7. Les fonctions permettant le calcul de la température et de la pression compensées en format entier 32 bits sont à la page 45 et 46 de la datasheet.

 ### Setup du STM32

## TP2 Interfaçage STM32 - Raspberry
Interfaçage STM32 <-> Raspberry Pi
 ![architecture_TP2](/img/architecture_TP2.png "Architecture TP2")

|   Requête du RPi     |   Réponse du STM    | Commentaire |
| :------------: |:---------------:| :-----:|
| GET_T      | T=+12.50_C | Température compensée sur 10 caractères    |
| GET_P      | P=102300Pa |   Pression compensée sur 10 caractères     |
| SET_K=1234 | SET_K=OK   |    Fixe le coefficient K (en 1/100e)       |
| GET_K      | K=12.34000 |   Coefficient K sur 10 caractères          |
| GET_A      | A=125.7000 |    Angle sur 10 caractères                 |

## TP3 Interface REST
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

## TP4 Bus CAN
Interface API Rest & pilotage d'actionneur par bus CAN
 ![architecture_TP4](/img/architecture_TP4.png "Architecture TP4")
