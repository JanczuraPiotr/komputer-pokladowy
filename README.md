"Komputer pok�adowy" dla samochodu Suzuki Vitara. Dodatkowe funkcje dla w�asnych potrzeb.
Platforma sprz�towa : raspberry pi i arduino



### Notatki

Przydatne podczas konfigurowania modu�u BT CAN  ELM327:
Modu� domy�lnie widziany jest jako rfcomm9.

cd /dev
rm -f ttyS3
ln -s rfcomm0 ttyS3
chmod 766 rfcomm0

