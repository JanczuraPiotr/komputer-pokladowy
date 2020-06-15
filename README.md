"Komputer pok≈Çadowy" dla samochodu Suzuki Vitara. Dodatkowe funkcje dla w≈Çasnych potrzeb.
Platforma sprzƒôtowa : raspberry pi i arduino







### Notatki

Przydatne podczas konfigurowania moduàu BT CAN  ELM327:
Moduà domyòlnie widziany jest jako rfcomm9.

cd /dev
rm -f ttyS3
ln -s rfcomm0 ttyS3
chmod 766 rfcomm0

