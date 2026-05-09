# Utilizare AI (Phase 1)

---

### 1. Unealta Folosită
Pentru această secțiune a proiectului, am folosit asistentul AI **Google Gemini**.

### 2. Prompt-urile Oferite
Interacțiunea a pornit după ce am implementat manual restul comenzilor (`--add`, `--list`, `--view`, `--remove_report`, `--update_threshold`). I-am oferit AI-ului codul meu scris până în acel punct și structura de date `Raport`, alături de următorul prompt:

> *"Am creat pana acum toate functiile de care aveam nevoie, insa la cea de filter am nevoie de asistenta AI. Poti sa imi explici si sa generezi codul pentru `parse_condition` si `match_condition`?"*

### 3. Ce a fost generat
Inițial, AI-ul a generat cele două funcții standard cerute de specificații:
1. **`parse_condition`**: O funcție care folosește `sscanf` cu formatul `%[^:]:%[^:]:%s` pentru a sparge string-ul de input în `field`, `op` și `value`.
2. **`match_condition`**: O funcție masivă care verifica pe rând fiecare câmp (*severity, timestamp, category, inspector*) și implementa manual toate cele 6 verificări de operatori logici (`==`, `!=`, `<`, `<=`, `>`, `>=`) pentru fiecare câmp numeric în parte, convertind string-urile cu `atoi()` și `atol()`.

### 4. Ce am schimbat și de ce
Deși codul generat inițial era funcțional, **am decis să îl modific** pentru a fi mai ușor de înțeles și a evita repetarea inutilă a codului.
* **Extragerea logicii:** Am observat că blocul imens de `if`-uri pentru operatorii matematici se repeta identic și pentru `severity` și pentru `timestamp`. Astfel, am creat eu o funcție separată, `cmp_numeric(long long val_raport, long long val_filtru, const char *op)`.
* **Alegerea tipului de date:** Am folosit `long long` ca argument pentru `cmp_numeric` astfel încât să o pot refolosi în siguranță atât pentru numere mici de 32 de biți (`severity` tip `int`), cât și pentru numere mari de 64 de biți (`timestamp` tip `time_t`), evitând un posibil *overflow*.

### 5. Ce am învățat
1. **Parsare în C:** Am învățat cum funcționează parsatorul `%[^:]` din `sscanf`, care permite citirea caracterelor dintr-un string până la întâlnirea unui delimitator ales.
2. **Gestionarea `time_t`:** Am înțeles diferența de reprezentare a memoriei și faptul că un timestamp trebuie tratat cu grijă (folosind `atol` și tipuri de date mai mari precum `long long`) pentru a evita trunchierea valorilor de 64 de biți.
3. **Analiza critică a codului generat:** Am realizat că primul output al AI-ului nu este mereu cel mai curat din punct de vedere arhitectural și că este datoria mea să intervin pentru a modulariza codul și a-l face mai curat și mai ușor de întreținut.


# Utilizare AI (Phase 2)

---

### 1. Prompt-urile Oferite
Pentru a doua fază, am adresat următoarele întrebări teoretice:
> * *"Poti sa imi explici cum functioneaza structura `struct sigaction`?"*
> * *"Poti sa imi explici si `sigint_action.sa_handler`, `sigemptyset(&sigint_action.sa_mask);`, `sigint_action.sa_flags`?"* (extrase din indicațiile problemei de laborator)
> * *"Cum pot face ca `monitor_reports.c` sa ruleze in fundal?"*

### 2. Ce a fost generat
Spre deosebire de Faza 1, unde am solicitat generarea de cod, în Faza 2 am folosit AI-ul predominant ca asistent educațional pentru a înțelege conceptele teoretice legate de semnale și execuția proceselor.
* **Structura `sigaction`:** AI-ul a generat explicații conceptuale detaliate, folosind analogia unui „formular de configurare” pe care programul îl trimite sistemului de operare. A detaliat rolul fiecărui câmp din `struct sigaction` (`sa_handler`, `sa_mask`, `sa_flags`).
* **Execuția în fundal:** Răspunzând la întrebarea despre cum mențin programul activ pentru a asculta semnale, AI-ul a detaliat logica unei bucle infinite controlate. A explicat conceptul de *„busy waiting”* (unde un simplu `while(running);` ar consuma 100% dintr-un nucleu CPU degeaba) și a oferit soluția eficientă a suspendării execuției folosind funcția `sleep(1)`.

### 3. Ce am schimbat și de ce
Intervenția AI-ului a constat în refactorizarea denumirilor variabilelor și clarificarea logicii, nu în generarea structurii de bază a programului. Am adaptat explicațiile primite pentru a denumi structurile clar (`sigint_action`, `sigusr1_action`) și am asigurat implementarea funcțiilor asincrone într-un mod sigur.

### 4. Ce am învățat
1. **Mecanismul `struct sigaction`:** Am înțeles de ce utilizarea `sigaction()` este preferată și mai robustă în programarea de sistem POSIX comparativ cu vechiul `signal()`, deoarece oferă un control granular asupra măștii de semnale și a flag-urilor de execuție.
2. **Comunicarea inter-proces (IPC):** Explicațiile m-au ajutat să vizualizez întregul flux de comunicare dintre `city_manager` și `monitor_reports`. Am înțeles că semnalele sunt tratate asincron de sistemul de operare, întrerupând funcții blocante (cum ar fi `sleep()`) pentru a declanșa imediat o funcție predefinită (handler-ul).