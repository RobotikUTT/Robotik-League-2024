# programmation des voitures et télécommandes
pour programmer les XIAO-ESP32C3 présents sur les télécommandes et les voitures utilisez l'IDE Arduino (du moins pour les fichiers situés dans cette branche) 

Connecter la carte comme toute carte Arduino en choisissant le port mais surtout en sélectionnant manuelement la carte XIAO-ESP32C3, présente parmi les autres esp32 dans la catégorie esp32

## programmation des voitures
car oui il faut **commencer par les voitures** transférer le code correspondant puis utilisez votre terminal série pour récupérer son adresse mac. 
Elle devrait ressembler à ça: 
```
mon adresse mac : 64:E8:33:89:XX:XX
```
Si elle ne s'affiche pas essayer de reset la board ou débranchez et rebrancher la carte

## programmation des télécommandes
Utiliser ensuite le code controller pour programmer la télécommande en changeant l'adresse MAC "connectedAddress" à la ligne 6
```Arduino
// Car MAC address
uint8_t connectedAddress[6] = { 0x64, 0xE8, 0x33, 0x89, 0xXX, 0xXX };
```
Vous pouvez ensuite vérifier si la connexion est bien établi en bougeant le joystick

# debug
Si la voiture ne bouge pas vérifiez que la batterie de la voiture est branchée...

Sinon regardez le moniteur série de votre télécommande, il doit afficher: "Controller ready 🎮" si ce n'est pas le cas l'ESP est peut-être morte ou le code ne marche pas

Vérifiez aussi que l'adresse qui s'affiche est bien celle de la voiture que vous voulez commander
```
Controller ready 🎮
send data to : , 0x64, 0xE8, 0x33, 0x89, 0xC2, 0xA8
```

Si tout à l'air ok du côté de la télécommande rebranchez la voiture pour vérifier ce qu'elle envoie
Elle doit envoyer **"marche normalement askip"** en boucle, si c'est bien le cas, le problème vient de l'élec de la voiture (moteur mals branchés par exemple).

Si elle n'envoie rien à part son adresse MAC, c'est qu'elle ne reçoit rien: vérifiez plutôt la télécommande ou la communication entre les 2 cartes
(vérifiez que la télécommande est branchée aussi ...)

La voiture peut aussi envoyer **"marche presque"** qui veut dire que la taille du packet reçu n'est pas la bonne donc dans ce cas soit la communication ne fonctionne pas bien soit les programmes voiture
et télécommandes sont différents ou mals programmés

Si elle envoie **"Erreur initialisation ESP-NOW"** l'ESP est surement morte ou vous avez touché à l'initialisation dans le code


