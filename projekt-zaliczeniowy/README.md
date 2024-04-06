 
#  Wersja rozproszona narzędzia arp-scan

## Opis projektu

Projekt jest aplikacją umożliwiającą skanowanie sieci w poszukiwaniu hostów poprzez użycie protokołu `ARP (Address Resolution Protocol)`. Użytkownik wybiera:
* interfejs sieciowy, na którym ma zostać przeprowadzone skanowanie. 
* adres serwera docelowego, na który zostaną przekazane i wyświetlone uzyskane adresy MAC oraz IP. 

### Wykorzystane technologie
* aplikacja jest napisana w języku `C` 
* wykorzystuje biblioteki `libpcap`, `libnet`  
 
## Sposób uruchomienia 

* W pierwszej kolejności uruchom program serwera nasłuchującego `(server.c)`. Pozwala to na poprawne nawiązanie połączenia między gniazdami. 
* Następnie uruchom program skanujący `(arp-scan.c)`, podając jako argumenty interfejs sieciowy oraz adres serwera docelowego.

### arp-scan.c 
`Do poprawnej pracy, wymaga uprawnień administratora (root)!`
* kompilacja:
```bash
gcc -Wall arp-scan.c -o arp-scan -lpcap -lnet 
```
* użycie: 
```bash
./arp-scan <server_addr> 
```
### server.c 
* kompilacja:
```bash
gcc -Wall server.c -o server 
```
* użycie: 
```bash
/server
```
