# Utilizare AI (Phase 1)

---

### 1. Unealta Folosită
Pentru această secțiune a proiectului, am folosit asistentul AI **Google Gemini**.

### 2. Prompt-urile Oferite
Interacțiunea a pornit după ce am implementat manual restul comenzilor *(--add, --list, --view, --remove_report, --update_threshold)*. I-am oferit AI-ului codul meu scris până în acel punct și structura de date Raport, alături de următorul prompt:
* *"am creat pana acum toate functiile de care aveam nevoie, insa la cea de filter am nevoie de asistenta AI. poti sa imi explici si sa generezi codul pentru parse_condition si match_condition"*


### 3. Ce a fost generat
Inițial, AI-ul a generat cele două funcții standard cerute de specificații:
1. parse_condition: O funcție care folosește sscanf cu formatul `%[^:]:%[^:]:%s` pentru a sparge string-ul de input în field, op și value.
2. match_condition: O funcție masivă care verifica pe rând fiecare câmp *(severity, timestamp, category, inspector)* și implementa manual toate cele 6 verificări de operatori logici (`==`, `!=`, `<`, `<=`, `>`, `>=`) pentru fiecare câmp numeric în parte, convertind string-urile cu atoi() și atol().

### 4. Ce am schimbat și de ce
Deși codul generat inițial era funcțional, **am decis să îl modific** pentru a fi mai ușor de înțeles și a nu se repeta.
* **Extragerea logicii:** Am observat că blocul imens de if-uri pentru operatorii matematici se repeta identic și pentru severity și pentru timestamp. Astfel, am creat eu o funcție separată, `cmp_numeric(long long val_raport, long long val_filtru, const char *op)`.
* **Alegerea tipului de date:** Am folosit **long long** ca argument pentru cmp_numeric astfel încât să o pot refolosi în siguranță atât pentru numere mici de 32 de biți *(severity tip int)*, cât și pentru numere mari de 64 de biți *(timestamp tip time_t)*, evitând un posibil *overflow*.

### 5. Ce am învățat
1. **Parsare în C:** Am învățat cum funcționează parsatorul `%[^:]` din sscanf, care permite citirea caracterelor dintr-un string până la întâlnirea unui delimitator ales.
2. **Gestionarea time_t:** Am înțeles diferența de reprezentare a memoriei și faptul că un timestamp trebuie tratat cu grijă (folosind atol și tipuri de date mai mari precum long long) pentru a evita trunchierea valorilor de 64 de biți.
3. **Analiza critică a codului generat:** Am realizat că primul output al AI-ului nu este mereu cel mai curat din punct de vedere arhitectural și că este datoria mea să intervin pentru a modulariza codul și a-l face mai curat și mai ușor de întreținut.