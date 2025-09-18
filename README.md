# PacManLandTigerBoard

Progetto Pac-Man per la scheda LandTiger (LPC1768).

## Descrizione
Codice didattico che implementa una versione semplificata di Pac-Man utilizzando la GLCD e joystick/pulsanti della board.

## Struttura del repository
- `sample.uvprojx`, `sample.uvoptx` - file di progetto Keil uVision (obbligatori per aprire il progetto).
- `Source/` - tutto il codice sorgente organizzato per cartelle (GLCD, TouchPanel, timer, CMSIS, ecc.).
- File in root referenziati dal progetto: `button.h`, `IRQ_button.c`, `lib_button.c`, `RIT.h`, `IRQ_RIT.c`, `lib_RIT.c`, `joystick.h`, `lib_joystick.c`, `adc.h`, `IRQ_adc.c`, `lib_adc.c`.
- `Objects/`, `Listings/` - artefatti di build generati da Keil.
- `sample.uvguix.*` - impostazioni GUI per utenti diversi; conserva solo `sample.uvguix.manum` se vuoi mantenere le tue preferenze.

## Requisiti
- Keil uVision con Pack `Keil.LPC1700_DFP` installato.
- Toolchain ARM (ARMCLANG) compatibile.

## Come aprire e buildare
1. Apri `sample.uvprojx` in Keil uVision.
2. Se necessario, installa i Device Pack richiesti (Keil mostrerà istruzioni).
3. Build: `Project -> Build target`.
4. Per debuggare/flashare: collega la board e avvia il debug da Keil.