# Stereo Compressor

Plugin audio **AU + VST3** per macOS che applica in serie:

```
INPUT ──► [ COMPRESSORE ] ──► [ STEREO WIDENER ] ──► OUTPUT
```

Pensato per buss di mix e master di typebeat / produzioni hip-hop e trap: compatta la dinamica e subito dopo allarga l'immagine stereofonica via processing **Mid/Side**.

- **Universal binary** (Apple Silicon + Intel)
- **macOS 11.0+** (testato sull'ultima release)
- **Logic Pro, GarageBand, Ableton, Cubase, Reaper, Studio One, FL Studio Mac**

> Per installare: vedi [INSTALL.md](INSTALL.md).

---

## Indice
1. [Cosa fa, in dettaglio](#cosa-fa-in-dettaglio)
2. [Parametri](#parametri)
3. [La GR meter](#la-gr-meter)
4. [Ricette di utilizzo](#ricette-di-utilizzo)
5. [Architettura del segnale](#architettura-del-segnale)
6. [Limiti noti](#limiti-noti)
7. [FAQ](#faq)

---

## Cosa fa, in dettaglio

### Sezione 1 — Compressore

Compressore **peak-detector stereo linked**: il detector guarda il massimo tra L e R, quindi i due canali vengono ridotti dello **stesso gain**. Questo preserva l'immagine stereo durante la compressione (non "tira" verso il canale più forte).

- **Detector**: peak (non RMS) → reattivo ai transienti, ideale per drum e master.
- **Gain computer**: hard knee classico. Sotto soglia → segnale invariato. Sopra → riduzione proporzionale al *ratio*.
- **Envelope follower**: attack e release **separati**, coefficienti esponenziali ricalcolati ogni blocco dai parametri correnti.
- **Makeup gain**: applicato post-detector, prima del widener.

### Sezione 2 — Stereo widener (M/S)

Dopo il compressore il segnale viene riscritto in dominio Mid/Side:

```
Mid  = (L + R) / 2     ← contenuto centrale (kick, snare, basso, voce)
Side = (L - R) / 2     ← contenuto laterale (room, riverberi, synth larghi)
```

Solo il **Side** viene scalato per il parametro Width, poi si ridecoda:

```
L = Mid + Side * Width
R = Mid - Side * Width
```

- `Width = 0.0` → mono perfetto (Side annullato).
- `Width = 1.0` → segnale identico all'originale.
- `Width = 2.0` → laterale raddoppiato, immagine molto larga (ma rischio fase su mono).

---

## Parametri

| Knob | Range | Default | Note |
|---|---|---|---|
| **Threshold** | -60 → 0 dB | -12 | Sopra questa soglia il compressore inizia a ridurre. |
| **Ratio** | 1:1 → 20:1 | 4:1 | 1:1 = nessuna compressione. 20:1 = quasi limiter. Skew logaritmico sul knob. |
| **Attack** | 0.1 → 200 ms | 10 | Quanto velocemente reagisce ai picchi. Valori bassi = punch perso ma più controllo. |
| **Release** | 10 → 2000 ms | 100 | Quanto velocemente lascia andare. Troppo veloce → pumping. Troppo lento → la compressione "rimane attaccata". |
| **Makeup** | 0 → 24 dB | 0 | Compensazione del volume ridotto dalla compressione. |
| **Width** | 0.0 → 2.0 | 1.3 | Scaling del canale Side. Applicato **dopo** il compressore. |

---

## La GR meter

In basso al pannello, in tempo reale:

```
GR  -3.4 dB
```

Indica di quanti dB il compressore sta riducendo il segnale in quel momento, **mediato sul blocco audio corrente** e smoothed visivamente a 30 Hz.

- `GR 0.0 dB` → niente compressione (segnale sotto soglia o ratio = 1).
- `GR -2 / -4 dB` → compressione "musicale", l'orecchio non la sente esplicitamente.
- `GR -6 / -10 dB` → compressione evidente, usala su singole tracce non sul master.
- `GR < -10 dB` → stai schiacciando troppo, alza la soglia o abbassa il ratio.

---

## Ricette di utilizzo

### Master bus typebeat / trap

```
Threshold:  -8 dB        (poca riduzione, ~2-3 dB di GR)
Ratio:       2.5:1
Attack:      30 ms       (lascia passare i transienti del kick)
Release:     200 ms
Makeup:      2 dB
Width:       1.15        (leggera apertura, sicura in mono)
```
**Perché**: master glue + un filo di apertura senza sfasare il mix.

### Drum bus

```
Threshold:  -10 dB
Ratio:       4:1
Attack:      5-10 ms
Release:     80 ms
Makeup:      3 dB
Width:       1.0         (non allargare i drum: perdi il punch del kick centrale)
```

### Synth / melody bus

```
Threshold:  -18 dB
Ratio:       2:1
Attack:      20 ms
Release:     300 ms
Makeup:      0-2 dB
Width:       1.5-1.7     (apertura marcata per pad e arp)
```

### Buss vocale (rap)

```
Threshold:  -16 dB
Ratio:       3:1
Attack:      8 ms
Release:     120 ms
Makeup:      4 dB
Width:       1.0         (voce sempre al centro)
```

### Sample / loop già finiti

```
Threshold:  -6 dB
Ratio:       2:1
Attack:      50 ms
Release:     250 ms
Makeup:      1 dB
Width:       1.2-1.3     (apri leggermente il sample che spesso è già stretto)
```

---

## Architettura del segnale

```
                    ┌──────────────────────────────────┐
                    │           COMPRESSOR             │
       L ──┐        │                                  │     ┌── L
           ├───► peak detect ──► gain comp ──► env ────┼──┐  │
       R ──┘        │              ▲                   │  │  │
                    │       (threshold,                 │  │  │
                    │        ratio, A/R)                │  │  │
                    └──────────────────────────────────┘  │  │
                                                          ▼  ▼
                                                    apply gain
                                                       (L, R)
                                                          │
                                                          ▼
                    ┌──────────────────────────────────┐
                    │       STEREO WIDENER (M/S)       │
                    │                                  │
                    │   M = (L+R)/2                    │
                    │   S = (L-R)/2  ──► S *= Width    │
                    │   L = M + S                      │
                    │   R = M - S                      │
                    └──────────────┬───────────────────┘
                                   │
                                   ▼
                                OUTPUT
```

Codice: tutto in un unico loop sample-by-sample in [PluginProcessor.cpp:80](source/PluginProcessor.cpp:80), zero latenza, processing in-place.

---

## Limiti noti

- **No sidechain esterno**: il detector usa il segnale in input. Non puoi pilotarlo da un'altra traccia.
- **No knee variabile**: hard knee fisso. Per compressione "soft" usa ratio bassi (2-3:1).
- **No lookahead**: l'attack è puramente reattivo. Per transienti aggressivissimi (snare, click) è meglio un limiter dedicato dopo.
- **Width > 1.5 + mix mono = pericolo**: cancellazioni di fase. Sempre testare in mono.
- **Niente oversampling**: in casi estremi (ratio 20:1, attack 0.1 ms) può aliasare sui transienti. Per il master usa setting moderati.

---

## FAQ

**Si può usare solo come compressore (senza widener)?**
Sì → metti Width a `1.0`. Il widener diventa bypass matematico (mid+side = L, mid-side = R, ricostruzione esatta).

**Si può usare solo come widener?**
Sì → metti Ratio a `1.0`. Il compressore non riduce nulla, passa solo il widener.

**Funziona su tracce mono?**
Sì, ma il widener è inerte (Mid = segnale, Side = 0). Pratica: usalo solo su tracce stereo.

**Perché la mia compressione fa pumping?**
Release troppo veloce. Sali a 150-300 ms.

**Logic dice "failed validation"**
Quarantena Gatekeeper. Vedi [INSTALL.md → Gatekeeper](INSTALL.md#gatekeeper--quarantena).

**Posso farlo girare anche su Windows / FL Studio Windows?**
Il CMakeLists è cross-platform e JUCE supporta Windows, ma `INSTALL.md` copre solo macOS. Su Windows servono Visual Studio 2022 + CMake e si compila in VST3.

---

## Crediti

Plugin scritto in C++ con [JUCE 8.0.8](https://juce.com). Compressore: implementazione classica peak-detector linked. Widener: standard M/S matrix.
