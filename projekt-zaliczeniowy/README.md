Nazwa projektu: 
	Wersja rozproszona narzędzia arp-scan.
	
Opis projektu: 
	Projekt jest aplikacją umożliwiającą skanowanie sieci w poszukiwaniu hostów poprzez użycie protokołu ARP (Address Resolution Protocol). Program umożliwia użytkownikowi wybranie interfejsu sieciowego, na którym ma zostać przeprowadzone skanowanie oraz adresu serwera docelowego, na który zostaną przekazane i wyświetlone uzyskane adresy MAC oraz IP. Aplikacja jest napisana w języku C i wykorzystuje biblioteki libpcap, libnet oraz gniazda sieciowe. Do poprawnej pracy, wymaga uprawnień administratora (root). 
	
Zawartość projektu:
	1. arp-scan.c
		Compilation:  gcc -Wall arp-scan.c -o arp-scan -lpcap -lnet
		Usage:        ./arp-scan <interface> <server_addr>
		NOTE:         This program requires root privileges.
	2. server.c
		Compilation:  gcc -Wall server.c -o server
		Usage:        /server 
	
Sposób uruchomienia programów projektu:
	1. W pierwszej kolejności uruchom program serwera, który jest dostępny jako ./server. Pozwala to na poprawne nawiązanie połączenia między gniazdami.
    2. Następnie uruchom program skanujący ./arp-scan, podając jako argumenty interfejs sieciowy oraz adres serwera docelowego. 

 