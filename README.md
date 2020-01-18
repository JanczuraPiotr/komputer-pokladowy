"Komputer pokˆadowy" dla samochodu Suzuki Vitara. Dodatkowe funkcje dla wˆasnych potrzeb.
Platforma sprz©towa : raspberry pi i arduino



### Notatki

Przydatne podczas konfigurowania moduˆu BT CAN  ELM327:
Moduˆ domy˜lnie widziany jest jako rfcomm9.

cd /dev
rm -f ttyS3
ln -s rfcomm0 ttyS3
chmod 766 rfcomm0

