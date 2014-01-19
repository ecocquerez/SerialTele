#SerialTele

Simple programme permettant la lecture des informations en provenance de la sortie téléinformation
des compteurs électroniques EDF.
Une simple interface permet de décoder les informations de cette sortie et de la transformer
en informations compréhensibles et lisibles en utilisant un port série.

Une interface permettant de connecter la sortie téléinformation à une carte raspberry PI est disponible
sur le net. par exemple : [Interface de décodage](http://www.cocquerez.com/post/teleinformation-pulsadis-tempo-edf)

##Principe de fonctionnement
Le port série ne pouvant pas être simplement partagée et le débit des informations étant relativement
faible, j'ai préféré mettre en place une structure est partagée en mémoire. 
Cette structure est mise à jour en permanence. Les autres programmes peuvent ainsi accéder
aux informations en lisant directement cette mémoire partagée.
