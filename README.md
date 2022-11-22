# TP:  Bus et réseaux industriels
### Description
Le but du tps est d'acquérir les données du capteur BMP280 par le STM32 et ensuite de contrôler ces données par une interface API.
La figure suivante illustre l'architecture générale du système :
![Structure du projet](/img/TP_complet.png "Structure du projet")



**Table of Contents**
1. [TP1-Bus I2C](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp1-bus-i2c)
2. [TP2-Interfaçage STM32 - Raspberry](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp2-interfaçage-stm32---raspberry)
3. [TP3-Interface REST](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp3-interface-rest)
4. [TP4-Bus CAN](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp4-bus-can)
5. [Conclusion](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#conclusion)


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

Dans cette partie, nous avons configure les broches pour configurer le Bus CAN, l'USART2 et l'USART3 et la communication I2C :

|Pour la com bus CAN||
| :------------: | :---------------:|
| Rx | PB8 |
| Tx | PB9 |

|Pour l'USART 2||
| :------------: |:---------------:|
| Rx | PA3 |
| Tx | PA2 |

|Pour l'USART 3||
| :------------: |:---------------:|
| Rx | PC5 |
| Tx | PB10 |

Nous avons choisi pour la communciation I2C entre le capteur de temperature les pins suivantes:

|Pour la cmmunication I2C||
| :------------: |:---------------:|
| SDA | PB7 |
| SCL | PB6 |


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
Dans cette partie, nous avons implémenté une interface Web sur Raspberry Pi. Tout d'abord, nous procédons à l'intallation et la vérification du serveur Python.
Nous avons vérifié le hostname de la raspberry dans le fichier /etc hostname et c’est : raspberry6
Puis, nous avons modifié le hostname du fichier /etc/hosts raspberry6 pour avoir le même nom, car nous avions un problème de sudo.

Nous avons créé un profil nommé ramos à partir des commandes ci-dessous et nous nous sommes connectés à celui-ci :
```
sudo adduser ramos
sudo usermod -aG sudo ramos
sudo usermod -aG dialout ramos
```
Pour se déloguer, il faut utiliser les commandes Ctrl + D et pour se déconnecter de la raspberry, il faut utiliser les commandes Ctrl + C. Il faut installer flask directement sur le serveur dans ramos et pas sur ese. Ensuite, nous avons créé un répertoire «interface REST» où tous les fichiers seront stockés. Les bibliothèques suivantes sont été installés à partir des commandes suivantes:
```
pip3 install pyserial
pip3 install flask
```
La image suivante illustre le nouveau utilisateur sur la Raspberry Pi:
![user_ramos](/img/user_ramos.png "User Ramos")

### Premier fichier Web
Nous avons d'abord créé un fichier (hello.py) pour notre premier serveur web de la manière suivante
```
from flask import Flask
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, World!\n'
```
Ensuite, à partir de la ligne de code suivante nous initialisons notre serveur. Cette ligne de code permet  d'eviter que notre serveur ne fonctionne qu'en loopback.
```
FLASK_APP=hello.py FLASK_ENV=development flask run --host 0.0.0.0
```
La constante ``` FLASK_ENV=development ``` permet de lancer un mode debug.
Quand on ouvre un navigateur web et qu’on entre l’adresse publique, on obtient l’affichage suivant :

![serveur_web](/img/serveur_web.png "serveur web")

### Première routage
Pour faire le prèmiere routage nous avons ajouté les lignes suivantes au fichier hello.py:
```
@app.route('/api/welcome/<int:index>')
def api_welcome_index(index):
    return welcome[index]
```
La figure suivante montre la reponse du serveur quand nous avons ajouté un index à la fin de l'adresse.

![reponse_routage_1](/img/reponse_routage_1.png "routage_1")

#### Questions
1. Quel est le rôle du décorateur @app.route?
Il ajoute une nouvelle branche au chemin initial.
2. Quel est le rôle du fragment <int:index>?
Il permet d’identifier le bit de la chaîne de caractère de la fonction welcome. Donc l’index 0 renvoie “W”. Si on met un index plus grand que la chaîne de caractère, le serveur renvoie une erreur 500 qui correspond à une erreur interne du serveur.

#### REMARQUE
Nous avons pris des reponse de serveur en formar JSON car ils sont plus lisables 

#### Reponse JSON
Nous avons ajoute une réponse JSON à partir de la bibliothèque «jsonify». Elle est accessible après ce ligne de code ```from flask import jsonify```. Cette fonction gère à la fois la conversion en json et l’ajout de l’entête.

#### Erreur 404
Nous avons ajouté un nouveau dossier (templates) dans le dossier de interface REST et à l'interiur on place le fichier «page_not_found.html». Les lignes de code suivantes sont été ajutés pour faire l'affichage de l'erreur.
```
@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404
```
#### Méthodes POST, PUT, DELETE
Dans cette partie, Nous avons élaboré les différentes méthodes pour es deux path. Le tabeau suivant montre cet information.

|   CRUB     |   Réponse du STM    | Path |Commentaire|
| :------------: | :---------------: |:-----:| :-----: |
| Create   | POST  | welcome/  | Change sentence |
| Retreive | GET   | welcome/  | Return sentence |
| Retreive | GET   | welcome/x | Return letter x |
| Update   | PUT   | welcome/x | Insert new word at position x |
| Update   | PATCH | welcome/x | Change letter at position x |
| Delete   | DELETE| welcome/x | Delete letter at position x |
| Delete   | DELETE| welcome/  | Delete sentece |

L'implementation de ces fonctions se trouve à partir de la ligne [15](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/interface%20REST/hello.py#L15) et [26](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/interface%20REST/hello.py#L26) pour les path welcome/ et welcome/x, respectivement.

## TP4 Bus CAN
Interface API Rest & pilotage d'actionneur par bus CAN

![architecture_TP4](/img/architecture_TP4.png "Architecture TP4")

## Conclusion