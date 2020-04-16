# Internet Of Shit

Der Internet of Shit Button bzw. Socket ist eine Platine, die das Internet der Dinge mit einem kleinen Akku erreicht.
Da vieles via WLAN verbreitet wird, ist es fragwürdig, warum alle Geräte einen festen Stromanschluss benötigen.
Andere komplett kabellose Lösungen mit WLAN müssen zu häufig wieder aufgeladen werden oder sind schlichtweg zu groß.
Der IoS Button/Socket ist ohne Betätigung komplett stromlos, lediglich ein paar Widerstände und MosFETs werden noch versorgt.

## Button
Eignet sich mit 3 frei programmierbaren Tasten als "Fernbedienung" von WLAN-Geräten.
Je nach dem, welches WLAN in Reichweite ist, können auch unterschiedliche Aktionen für die Tasten vergeben werden.
## Socket
Mit seinen 5 frei belegbaren Schalteingängen eignet sich der IoS Socket besonders für kompakte Festinstallation ohne Stromanschluss.
Etwa im Briefkasten mit zwei Tastern versehen zur Meldung, wann die Post da ist.
Jedoch nicht direkt geeignet um häufiger/länger betätigte Kontakte, wie Balkontüren, Fenster, ... zu überwachen.

## Programmierung
Auf dem Board ist ein CH340E verbaut, der den USB-Seriellwandler bereitstellt.
### Internet of Shit Button
Zum Programmieren muss ein Jumper über D0/GND gesetzt werden, dann kann man den Button mit einem Tastendruck anschalten.
Als Board sollte ein *ESP8285* ausgewählt werden.
### Internet of Shit Socket
Zum Programmieren muss PRG gedrückt werden und einer der Kontakte gebrückt werden, um den IoS Socket anzuschalten.
Als Board kann irgendein *ESP8266* Modul gewählt werden (etwa `WeMos D1 mini` oder `Generic ESP8266`).
#### Bereits eingeschaltete IoS Platine
Ohne oder mit falsch konfigurierter Software aktivieren die Platinen die Selbstabschaltung nicht.
Somit muss zum Programmieren zusätzlich zum Brücken von D0 + GND/Drücken von PRG noch zuätzlich RST mit GND kurz verbunden, bzw. RST kurz gedrückt werden.
D0/GND bzw. PRG sollte weiter verbunden/gedrückt bleiben währenddessen.
### Software
```
void setup() {
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  ...
  pinMode(16, INPUT);
}
```
#### Anschalten der Platine
Nach dem Booten (als erstes in `setup()`) sollte direkt GPIO16 als Ausgang und auf HIGH gesetzt werden.
Eine Wartezeit zuvor resultiert in einer längeren erforderlichen Betätigungszeit des Tasters/Kontaktes.
#### Ausschalten der Platine
Nach dem Beenden aller Aufgaben ist GPIO16 wieder als Eingang zu definieren.
Ein Deepsleep sollte nicht aktiviert werden.
Der Deepsleep verbraucht **zu wenig** Strom, sodass die Schaltung nicht wieder abschaltet.
Sollten Kontakte häufiger für längere Zeit geschaltet sein, ist dies ggf. nicht die richtige Lösung für das Problem.
