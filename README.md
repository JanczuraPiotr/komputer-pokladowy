"Komputer pokładowy" dla samochodu Land Rover Discovery 3. Dodatkowe funkcje dla własnych potrzeb.

Platforma sprzętowa : raspberry pi i arduino







### Notatki

Przydatne podczas konfigurowania modułu BT CAN  ELM327:
Moduł domyślnie widziany jest jako rfcomm9.

cd /dev

rm -f ttyS3

ln -s rfcomm0 ttyS3

chmod 766 rfcomm0

