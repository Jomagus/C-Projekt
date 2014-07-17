#define _CRT_SECURE_NO_WARNINGS	
/* Da Visual Studio z.B. strncpy als unsicher ansieht (und strncpy_s haben möchte, da bei strncpy ein Buffer Overflow auftreten kann),
was aber zu problemen mit gcc führen kann; Der Befehl ignoriert das Funktionen "unsicher" sind beim kompilieren in Visual Studio
(sonst würde das kompilieren gar nicht erst funktionieren, sondern nur zu einer Fehlermeldung führen) */

#include <stdio.h>		//Für fgets und generell die Ein/Ausgabe
#include <stdlib.h>		//z.B. für NULL, malloc() und free()
#include <string.h>		//für strncpy, (memcpy)
#include <stdarg.h>		//für variable Parameteranzahl

/*******************************************
*******Hier beginnt das History-Modul******
********************************************/

struct Node
{
	char Eingabe[256];
	struct Node *Next;
};

struct Node *Anfang = NULL;

int ListenFehler = 0;		//Wird auf 1 gesetzt, falls die Speicherplatzreservierung für die verkettete Liste fehlschlägt
int ListenAnzahl = 0;		//Gibt die Anzahl der gespeicherten Elemente an

void NeuesElement(char Input[256])
{
	struct Node *NeuerNode = malloc(sizeof(struct Node));		//Speicher reservieren für den neuen Knoten

	if (NeuerNode == NULL)	//Falls kein Speicher freigegeben werden konnte
	{
		ListenFehler = 1;
		return;
	}

	if (Anfang == NULL)		//Falls die Liste leer isi
	{
		Anfang = NeuerNode;
		NeuerNode->Next = NULL;
		ListenAnzahl++;
	}
	else
	{
		NeuerNode->Next = Anfang;
		Anfang = NeuerNode;
		ListenAnzahl++;

		/* Wir fügen einen neuen Knoten immer am Anfang der Liste ein, dies bedeutet bei einer einfach Verketteten Liste zwar, dass wir
		häufig über viele Knoten laufen müssen, vor allem bei dem History befehl, allerdings ergeben sich auch vorteile, durch die sehr einfache Struktur,
		die es leichter macht, den Überblick zu behalten um z.B. Speicherlecks bei der Programmierung zu vermeiden. Insgesammt ist es also ein Kompromiss:
		Wir benutzen weniger Speicherplatz (als eine Doppelt verkettete Liste) und haben eine sehr einfache Struktur (durch das Einfügen nur am Anfang und
		da wir keien dynamischen Arrays verwenden), dafür nehmen wir eine langsamere ausführung einiger Funktionen in kauf. */
	}

	//strncpy(NeuerNode->Eingabe, Input, 256);

	memcpy(NeuerNode->Eingabe, Input, 256 * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden

	return;
}


void ListeLöschen(int n)		// Wir müssen nie einzelne Knoten löschen, also reicht eine Funktion, die alle Knoten löscht, und somit ihren Speicherplatz wieder freizugeben
{
	FILE *HistoryFile = fopen(".hhush.histfile", "ab");	//Öffnet (und erstellt gegebenenfalls) die History-Datei
	struct Node *Hilfszeiger = Anfang;
	while (Anfang != NULL)
	{
		if (n == 1)
		{
			fputs(Anfang->Eingabe, HistoryFile);		//Routine zur Dateispeicherung ; falls n=0 => nur löschen für "history -c", ansonten history speichern mit n=1
		}
		Hilfszeiger = Anfang->Next;
		free(Anfang);
		Anfang = Hilfszeiger;
	}
	ListenAnzahl = 0;
	if (ferror(HistoryFile))
	{
		//Fehlerbehandlung einbauen für Fehler beim Schreiben
	}
	fclose(HistoryFile);
	return;
}

void History(int parameterzahl, ...)
{
	va_list ArgumentPointer;
	int n = -1;					//Anzahl der Auszugeben Elemente wird hier gespeichert werden
	if (parameterzahl)			//Wurden Argumente angegeben, müssen sie abgefragt werden 
	{
		va_start(ArgumentPointer, parameterzahl);
		n = va_arg(ArgumentPointer, int);
		va_end(ArgumentPointer);
		if (n <= 0)
		{
			//Fehlerbehandlung
		}
	}

	struct Node *Hilfszeiger;
	int Zähler = 0;				//Ist verantwortlich für die ID in der History

	if (n == -1)				//wenn -1 hierhin "durchgereicht" wird, ist die if-Schleife nicht durchlaufen worden, es werden also alle Kommandos ausgegeben
	{
		n = ListenAnzahl;
	}

	for (; n > 0; n--)
	{
		int k;
		Hilfszeiger = Anfang;
		for (k = 1; k < n; k++)
		{
			Hilfszeiger = Hilfszeiger->Next;
		}
		printf("%i %s", Zähler, Hilfszeiger);
		Zähler++;
	}

	return;
}

int main(void)
{

	{
		char input[256];
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		History(0);
		ListeLöschen(1);
	}


	return 0;
}