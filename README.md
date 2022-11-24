# TP:  Bus et réseaux industriels
### Description
Le but du TP est d'acquérir les données du capteur BMP280 par le STM32 et ensuite de contrôler ces données par une interface API.
La figure suivante illustre l'architecture générale du système :
![Structure du projet](/img/TP_complet.png "Structure du projet")



**Table of Contents**
1. [TP1-Bus I2C](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp1-bus-i2c)
2. [TP2-Interfaçage STM32 - Raspberry](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp2-interfaçage-stm32---raspberry)
3. [TP3-Interface REST](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp3-interface-rest)
4. [TP4-Bus CAN](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#tp4-bus-can)
5. [Conclusion](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/README.md#conclusion)


## TP1 Bus I2C
Pour cette première partie, nous avons réalisé l'interrogation des capteurs par le bus I²C du microcontrôleur. L'objectif de cette partie est d'interfacer un STM32 avec des capteurs I²C. Ici, nous avons utilisé le BMP280 qui est un capteur de pression et de température développé par Bosch.

![architecture_TP1](/img/architecture_TP1.png "Architecture TP1")

 A partir de la datasheet du capteur BMP280, nous avons determiné les éléments suivants:
 1. Les adresses I²C possibles pour ce composant sont sur 7 bits. Si on connecte la broche SDO au GND, l’adresse du composant est 111 0110 (0x76). Si la broche est connectée à Vddio, c’est 111 0111 (0x77).
 2. Le registre permettant d’identifier ce composant est nommé « id » dont l’adresse est 0xD0. Sa valeur est 0x58 et peut être lue dès que le composant est sous tension.
 3. Le registre permettant de placer le composant en mode NORMAL est le « ctrl_meas» et son adresse est 0xF4. Il faut mettre les bits de valeur 11 dans mode [1 :0].
 4. Les registres contenant l’étalonnage du composant sont à l’adresse 0x88 jusqu'à l’adresse 0xA1.
 5. Les registres contenant la température du composant sont nommés « temp » et sont aux adresses allant de 0xFA à 0xFC. Le format de la donnée est unsigned sur 20 bits, ut[19 :0].
 6. Les registres contenant la pression du composant sont nommés « press » et sont aux adresses allant de 0xF7 à 0xF9. Le format de la donnée est unsigned sur 20 bits, up[19 :0].
 7. Les fonctions permettant le calcul de la température et de la pression compensées en format entier 32 bits sont à la page 45 et 46 de la datasheet.


 ### Setup du STM32

Dans cette sous-partie, nous avons determiné les broches pour configurer le Bus CAN, l'USART2, l'USART3 et la communication I2C. Le bus CAN sert à faire la connexion entre le moteur pas à pas et la carte STM32 :

|Pour la com bus CAN||
| :------------: | :---------------:|
| Rx | PB8 |
| Tx | PB9 |

La communication UART2 sert à faire la connexion entre le port USB de notre ordinateur et la carte STM32 :

|Pour l'USART 2||
| :------------: |:---------------:|
| Rx | PA3 |
| Tx | PA2 |

La communication UART3 sert à faire la connexion entre la raspberry Pi et la carte STM32 :

|Pour l'USART 3||
| :------------: |:---------------:|
| Rx | PC5 |
| Tx | PB10 |

Nous avons choisi pour la communication I2C entre le capteur de température et la STM32 les broches suivantes:

|Pour la cmmunication I2C||
| :------------: |:---------------:|
| SDA | PB7 |
| SCL | PB6 |

Ensuite, nous avons effectué la redirection du printf afin de pouvoir facilement déboguer notre programme sur la STM32. Ainsi, la fonction printf renvoie ses chaînes de caractères sur la liaison UART en USB. Il faut ajouter le code suivant au fichier stm32f4xx_hal_msp.c :

```
/* USER CODE BEGIN PV */  
extern UART_HandleTypeDef huart2;  
/* USER CODE END PV */  

/* USER CODE BEGIN Macro */  
#ifdef __GNUC__ /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf    set to 'Yes') calls __io_putchar() */  
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)  
#else  
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)  
#endif /* __GNUC__ */  
/* USER CODE END Macro */  


/* USER CODE BEGIN 1 */  
/**  
  * @brief  Retargets the C library printf function to the USART.  
  * @param  None  
  * @retval None  
  */  
PUTCHAR_PROTOTYPE  
{  
  /* Place your implementation of fputc here */  
  /* e.g. write a character to the USART2 and Loop until the end of transmission */  
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);  

  return ch;  
}
/* USER CODE END 1 */  
```

Après avoir ajouté ces lignes dans le code du fichier stm32f4xx_hal_msp.c, nous avons réalisé un printf suivi d'un délai dans la boucle while :

![codeTestUart_TP1](img/helloTP1.png "code test UART TP1")

Puis, nous avons ouvert un terminal, lancé et testé le code. Nous avons obtenu le résultat suivant :

![resultatTestUart_TP1](img/resutatTP1Setup.png "resultat test UART TP1")

Ainsi, la chaine de caractère "hello" s'affichait toutes les secondes dans le terminal.

### Communication I²C avec le BMP280

On utilise les fonctions Transmit et Receive de la bibliothèque HAL pour réaliser la communication I²C. Il faut tout de même faire attention à l'adresse demandée de 8 bits alors que l'adresse I²C est sur 7 bits. Donc il faut décaler l'adresse I²C de 1 vers la gauche lorsqu'on utilise les fonctions HAL.

#### Identification du BMP280
Dans un premier temps, il faut identifier notre capteur en lisant la valeur du registre ID. Pour cela, il faut envoyer l'adresse du registre ID, 0xD0, et recevoir 1 octet correspondant au contenu du registre, 0x58. Nous avons écrit la fonction *id_BMP280()* qui réalise ces instructions.
On affiche la valeur de l'ID :

![valeurID_TP1](/img/valeurIdTP1Setup.png "valeur ID TP1")

Le numéro d'identification du capteur BMP280 est bien 0x58, comme la documentation l'indique. Nous avons par la suite observé la forme des trames I²C à l'oscilloscope. Les sorties sont en collecteur ouvert, on peut donc voir sur l'oscilloscope que les signaux ne sont pas carrés mais de type RC. Pour changer et avoir des beaux signaux carrés, il faut ajouter une capacité et une résistance.

#### Configuration du BMP280

#### Récupération de l'étalonnage, de la température et de la pression

#### Calcul des températures et des pression compensées


## TP2 Interfaçage STM32 - Raspberry
L'objetif principal de ce TP est de configurer les interfaces entre le STM32 et le RaspberyPi 0. La image suivante illustre l'architecture du système.

![architecture_TP2](/img/architecture_TP2.png "Architecture TP2")

### Mise en routage du Raspberry PI Zero
En premier temps, nous avons télécharger l'image "Raspberry Pi os" et on l'installé sur la carte SD grace à l'outil BalenaEtcher.

Nous avons changé le nom de notre raspberry pour «raspberrypi6».

### Configuration de l'image
Apres de la installation nous avons crée deux fichier: ssh (fichier vide) et wpa_supplicant.conf nous avons le rempli avec le code suivante.

```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=FR

network={
 ssid="ese"
 psk="bus_ese"
}
```
Notre adresse IP est 192.168.88.247 et à partir d'elle nous pouvons se connecter avec des protocols SSH. La commande suivante montre la manière avec laquelle on se connecte à la raspberry.
``` ramos@192.168.88.247 ```
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
Pour se déloguer, il faut utiliser les commandes Ctrl + D et pour se déconnecter de la raspberry, il faut utiliser les commandes Ctrl + C. Il faut installer flask directement sur le serveur dans ramos et pas sur ese. Ensuite, nous avons créé un répertoire «interface REST» où tous les fichiers seront stockés. Les bibliothèques suivantes sont été installées à partir des commandes suivantes:
```
pip3 install pyserial
pip3 install flask
```
L'image suivante illustre le nouvel utilisateur sur la Raspberry Pi:

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
Ensuite, à partir de la ligne de code suivante nous initialisons notre serveur. Cette ligne de code permet d'éviter que notre serveur ne fonctionne qu'en loopback.
```
FLASK_APP=hello.py FLASK_ENV=development flask run --host 0.0.0.0
```
La constante ``` FLASK_ENV=development ``` permet de lancer un mode debug.
L'image suivante illustre les adresses pour se connecter au serveur web.

![IP](/img/adresses.png "IP")

Quand on ouvre un navigateur web et qu’on entre l’adresse publique, on obtient l’affichage suivant :

![serveur_web](/img/serveur_web.png "serveur web")

### Premier routage
Pour faire le premier routage nous avons ajouté les lignes suivantes au fichier hello.py:
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
Nous avons pris des réponses de serveur en format JSON car ils sont plus lisibles 

#### Reponse JSON
Nous avons ajouté une réponse JSON à partir de la bibliothèque «jsonify». Elle est accessible après cette ligne de code ```from flask import jsonify```. Cette fonction gère à la fois la conversion en json et l’ajout de l’entête.

#### Erreur 404
Nous avons ajouté un nouveau dossier (templates) dans le dossier de interface REST et à l'intérieur on place le fichier «page_not_found.html». Les lignes de code suivantes sont été ajoutés pour faire l'affichage de l'erreur.
```
@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404
```
#### Méthodes POST, PUT, DELETE
Dans cette partie, Nous avons élaboré les différentes méthodes pour les deux path. Le tabmeau suivant montre ces informations.

|   CRUB     |   Réponse du STM    | Path |Commentaire|
| :------------: | :---------------: |:-----:| :-----: |
| Create   | POST  | welcome/  | Change sentence |
| Retreive | GET   | welcome/  | Return sentence |
| Retreive | GET   | welcome/x | Return letter x |
| Update   | PUT   | welcome/x | Insert new word at position x |
| Update   | PATCH | welcome/x | Change letter at position x |
| Delete   | DELETE| welcome/x | Delete letter at position x |
| Delete   | DELETE| welcome/  | Delete sentece |

L'implémentation de ces fonctions se trouve dans le fichier «hello.py» à partir des lignes [15](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/interface%20REST/hello.py#L15) et [26](https://github.com/JuanYule/TP_Bus_et_reseaux/blob/main/interface%20REST/hello.py#L26) pour les path welcome/ et welcome/x, respectivement.

Pour tester le fonctionnement de l'API Rest, nous utilisons l'extention de Mozilla «Open RESTED» pour vérifier chaque requête pour différent Path.
La figure suivante illustre à titre d'exemple la réponse que nous avons obtenu au moment où nous avons fait la requete 'GET'.

![requete](/img/requete.png "Requete")

Pour conclure ce TP3, nous avons pu vérifier toutes les rêquetes pour chaque Path et nous avons validé ce fonctionnement sur la raspberry Pi.

## TP4 Bus CAN
Interface API Rest & pilotage d'actionneur par bus CAN

![architecture_TP4](/img/architecture_TP4.png "Architecture TP4")

Le moteur est piloté à partir de deux modes: automatique et manuel. La figure suivante montre le tableau de configuration.

![stepper_motor](/img/stepper_motor.png "stepper_motor")

![config_TP4](/img/configTP4.png "configuration TP4")

## Conclusion
