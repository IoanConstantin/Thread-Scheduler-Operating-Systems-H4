# Thread-Scheduler
Abordare generala:
------------------
Pentru fiecare thread am retinut intr-o structura campurile prioritate,
thread_id, semaforul thread-ului, pozitia sa in array-ul de thread-uri,
handler-ul, cuanta si daca se afla in starea de waiting sau nu.
In functia so_fork deschid doar semaforul thread-ului doar daca acesta
este primul thread, altfel semaforul thread-ului va fi deschid in functia
scheduler. Pentru a imparti pe prioritati am folosit un array alocat
dinamic, fiecare element al acestui array avand un **array si o dimensiune
proprie pentru a lucra mai usor cu ele. 
In functia de scheduler aplic intai un Round Robin parcurgand prioritatile
dintre aceea a thread-ului respectiv si prioritatea maxima, apoi verific
starea thread-ului si aplic Round Robin parcurgand prioritatile dintre aceea
a thread-ului respectiv si cea minima.
La fel ca in cazul prioritatilor pentru I/O folosesc tot un array alocat
dinamic care are fiecare element un **array si o dimensiune. Ma folosesc de
aceeasi structura in ambele cazuri.

Dupa parerea mea tema este utila intrucat a clarificat pentru mine felul in
care functioneaza un planificator cu algoritmul Round Robin.

Cred ca implementarea este medie, se putea mai bine.

Am intampinat dificultati deoarece la un moment dat foloseam niste variabile
pe care le modificau mai multe thread-uri fara sa imi dau seama si imi picau
mai multe teste din aceasta cauza.

Pentru build folosim gcc si compilam biblioteca dinamica libscheduler.so.

Am folosit laboratorul 8 de SO.
