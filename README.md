# µBash
Progetto universitario realizzato da me ed un mio compagno di corso per l'esame di sistemi di elaborazione e trasmissione dell'informazione. \
Veniva richiesto di implementare una shell molto semplificata che implementasse i vari comandi tramite chiamate di sistema ed il comando `cd` built-in.\
## Scopo del progetto
Lo scopo del progetto era di esercitarsi nell'uso delle system call `fork`, `pipe` ed `exec` siccome un vincolo fondamentale da rispettare era di non utilizzare
la funzione system e nella redirezione I/O.
## Sintassi
Trattandosi di una versione semplificata, viene semplificato anche il parsing dei comandi adottando una sintassi più limitata:
* Deve essere presente uno spazio prima e dopo il simbolo |
* Non devono essere presenti spazi tra il simbolo `>` o `<` ed il file dove fare la redirezione I/O \
Per ulteriori dettagli consultare il pdf [micro-bash.pdf](micro-bash.pdf)

