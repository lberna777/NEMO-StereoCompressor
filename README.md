# Stereo Compressor

Plugin audio **AU + VST3** per macOS. Versione **v1.1 — Utility Pack 01**.

Catena di processing in serie:

```
INPUT ──► [HI-PASS] ──► [LO-PASS] ──► [COMPRESSOR] ──► [HABISSO sat] ──► [WIDENER M/S] ──► OUTPUT
```

- **Universal binary** (Apple Silicon + Intel)
- **macOS 11.0+** (testato sull'ultima release)
- **Logic Pro, GarageBand, Ableton, Cubase, Reaper, Studio One, FL Studio Mac**

Per installare: vedi [INSTALL.md](INSTALL.md).

---

## Indice

1. [Cosa fa](#cosa-fa)
2. [Parametri](#parametri)
3. [UI Tour](#ui-tour)
4. [Ricette di utilizzo](#ricette-di-utilizzo)
5. [Architettura del segnale](#architettura-del-segnale)
6. [Limiti noti](#limiti-noti)
7. [FAQ](#faq)
8. [Changelog](#changelog)

---

## Cosa fa

### 1 · Hi-Pass + Lo-Pass (filtri input)

Filtri Butterworth del 2° ordine in cascata. Servono a **ripulire il segnale** prima della compressione:

- **Hi-Pass**: rimuove rumble, low-end inutile, DC offset. Range 20-500 Hz.
- **Lo-Pass**: addolcisce alte frequenze aspre, simula la limitazione di banda di hardware analogico. Range 2-20 kHz.

Risposta in frequenza visualizzata nel display centrale, aggiornata in tempo reale.

### 2 · Compressore peak-detector stereo-linked

- **Detector peak** stereo: guarda il massimo tra L e R → entrambi i canali ricevono lo stesso gain → l'immagine stereo non "balla".
- **Envelope follower esponenziale** con attack/release separati.
- **Hard knee** classico.
- **Ratio a pulsanti** stile 1176: 4:1, 8:1, 12:1, 20:1, mutualmente esclusivi.
- **Makeup gain** post-compressione, pre-saturazione.

### 3 · HABISSO — saturazione tape

Waveshaper non lineare basato su `tanh`. A 0% = bypass perfetto. Al massimo aggiunge **armoniche pari + dispari** che addolciscono i picchi e danno "calore" e "presenza" tipo tape machine.

Compensazione automatica del livello (la saturazione non fa salire il volume in modo fastidioso).

Iconografia: il tentacolo accanto al knob diventa più vibrante man mano che alzi il valore.

### 4 · Stereo Widener M/S

Mid/Side encoder dopo la saturazione:

```
M = (L + R) / 2       ← contenuto centrale (kick, basso, voce)
S = (L - R) / 2       ← contenuto laterale (room, riverberi, ampi synth)
```

Solo il Side viene scalato per `Width`, poi riencode:

```
L = M + S·Width
R = M − S·Width
```

- `Width = 0.0` → mono perfetto
- `Width = 1.0` → segnale invariato
- `Width = 2.0` → laterali raddoppiati

---

## Parametri

| Sezione | Knob / Btn | Range | Default | Note |
|---|---|---|---|---|
| **EQ** | Hi-Pass | 20 → 500 Hz | 20 | Skew log. A 20 = bypass effettivo. |
| **EQ** | Lo-Pass | 2k → 20k Hz | 20000 | Skew log. A 20k = bypass effettivo. |
| **Comp** | Threshold | -60 → 0 dB | -12 | Sopra → compressione. |
| **Comp** | Ratio (4 btn) | 4 / 8 / 12 / 20 | 4 | Mutualmente esclusivi. |
| **Comp** | Attack | 0.1 → 200 ms | 10 | Più basso = più "pump", meno transienti. |
| **Comp** | Release | 10 → 2000 ms | 100 | Troppo veloce = pumping. |
| **Comp** | Makeup | 0 → 24 dB | 0 | Post-compressione. |
| **Sat** | **HABISSO** 🐙 | 0 → 100 % | 0 | Saturazione tape. |
| **Stereo** | Width | 0.0 → 2.0 | 1.3 | Side scaling M/S. |

---

## UI Tour

```
┌────────────────────────────────────────────────────────────┐
│                STEREO COMPRESSOR                           │
│             v1.1 · UTILITY PACK 01                         │
├─────┬────────────────────────────────────────────────┬─────┤
│     │  ┌──────────────────────────────────────────┐  │     │
│  I  │  │   FREQ RESPONSE (HP × LP)                │  │  O  │
│  N  │  │      ── curve ciano + fill ──            │  │  U  │
│     │  │   GR ████░░░░░░░░░░░░ -3.4 dB            │  │  T  │
│ ▓▓  │  └──────────────────────────────────────────┘  │ ▓▓  │
│ ▓▓  │   HP    LP    THR   ATT   REL   MAK            │ ▓▓  │
│ ▓▓  │   ○     ○      ○     ○     ○     ○             │ ▓▓  │
│     │                                                 │     │
│     │   RATIO  [4][8][12][20]   HABISSO 🐙   WIDTH    │     │
│     │                              ○                 ○│     │
└─────┴────────────────────────────────────────────────┴─────┘
```

- **Display centrale**: curva HP × LP in tempo reale + barra GR rossa in basso.
- **Meter verticali I/O** ai lati con segmenti dB e tacche -20 / -10 / -6 / -3 / 0.
- **Ratio buttons** stile 1176: il pulsante attivo si illumina di ciano.
- **Tentacolo**: si "anima" (alpha + spessore) man mano che alzi HABISSO.

---

## Ricette di utilizzo

### Master bus typebeat / trap

```
HP:        30 Hz       (taglia rumble sub-percepibile)
LP:        18 kHz      (addolcisce hi-hat aspri)
Threshold: -8 dB
Ratio:     4
Attack:    30 ms       (lascia passare i transienti del kick)
Release:   200 ms
Makeup:    2 dB
Habisso:   15%         (un pizzico di calore)
Width:     1.15
```

### Drum bus

```
HP:        40 Hz
LP:        20 kHz      (bypass)
Threshold: -10 dB
Ratio:     8
Attack:    5-10 ms
Release:   80 ms
Makeup:    3 dB
Habisso:   25%         (drum più "presenti")
Width:     1.0
```

### Synth / pad bus

```
HP:        60 Hz       (libera spazio per il basso)
LP:        16 kHz
Threshold: -18 dB
Ratio:     4
Attack:    20 ms
Release:   300 ms
Makeup:    2 dB
Habisso:   10%
Width:     1.6
```

### Bus vocale (rap)

```
HP:        100 Hz      (toglie il "boomy" della voce)
LP:        14 kHz      (toglie aria fredda)
Threshold: -16 dB
Ratio:     8
Attack:    8 ms
Release:   120 ms
Makeup:    4 dB
Habisso:   30%         (calore, "presenza vintage")
Width:     1.0
```

### Sample / loop

```
HP:        20 Hz
LP:        20 kHz
Threshold: -6 dB
Ratio:     4
Attack:    50 ms
Release:   250 ms
Makeup:    1 dB
Habisso:   20%
Width:     1.3
```

---

## Architettura del segnale

```
       L ─┐
          ├──► HP (Butterworth 2°) ──┐
       R ─┘                          │
                                     ▼
       L ─┐                       LP (Butterworth 2°)
          ├──► peak detect ──┐       │
       R ─┘                  │       ▼
                             ▼   COMPRESSOR (gain comp + env follower)
                       gain reduction
                             │       │
                             ▼       ▼
                          HABISSO (tanh waveshaper + auto-comp gain)
                             │
                             ▼
                          M/S encode  →  Side *= Width  →  M/S decode
                             │
                             ▼
                          OUTPUT
```

Codice DSP: tutto in [PluginProcessor.cpp:processBlock](source/PluginProcessor.cpp). Zero latenza, processing in-place.

---

## Limiti noti

- **No sidechain esterno** (detector usa il segnale in input).
- **No knee variabile** (hard knee fisso). Per soft → usa ratio 4:1.
- **No lookahead** (attack puramente reattivo).
- **No oversampling** in HABISSO → a 100% con segnali pieni di alte frequenze può comparire aliasing leggero. Tienilo sotto il 70% sul master.
- **Width > 1.5 + mix mono** = rischio cancellazioni di fase. Sempre testare in mono.

---

## FAQ

**Si può usare solo come compressore?**
Sì → HP = 20, LP = 20k, Habisso = 0, Width = 1.0 → la catena è praticamente bypass tranne il compressore.

**Si può usare solo come EQ HP/LP?**
Sì → Ratio 4 + Threshold 0 dB → il compressore non comprime mai.

**Solo widener?**
Sì → vedi sopra + bypass dei filtri.

**Funziona su tracce mono?**
Sì, ma il widener è inerte (Side = 0). Filtri, compressore, saturazione funzionano regolarmente.

**Perché HABISSO si chiama così?**
Tape saturation = "scendere nell'abisso del calore analogico". Il tentacolo è il logo.

**Logic dice "failed validation"**
Quarantena Gatekeeper. Vedi [INSTALL.md → Gatekeeper](INSTALL.md#step-7--togli-la-quarantena-gatekeeper).

**Posso farlo girare su Windows?**
JUCE supporta Windows VST3 — serve riadattare INSTALL.md (Visual Studio 2022 + CMake).

---

## Changelog

### v1.1 — Utility Pack 01 (current)
- **NEW**: filtri Hi-Pass + Lo-Pass con visualizzazione risposta in frequenza
- **NEW**: HABISSO — saturazione tape via waveshaper tanh, con icona tentacolo animata
- **NEW**: Ratio a pulsanti stile 1176 (4 / 8 / 12 / 20)
- **UI**: restyle neomodern ispirato Acustica DOVE — pannello chiaro, knob lucidi, meter verticali I/O
- **UI**: display centrale con curva EQ + GR bar overlay

### v1.0
- Compressore peak-detector stereo-linked
- Stereo widener M/S
- 6 knob, GR meter testuale

---

## Crediti

Plugin scritto in C++ con [JUCE 8.0.8](https://juce.com). Reference visivo: Acustica DOVE.
