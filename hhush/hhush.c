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


void ListeLöschen()		// Wir müssen nie einzelne Knoten löschen, also reicht eine Funktion, die alle Knoten löscht, und somit ihren Speicherplatz wieder freizugeben
{
	struct Node *Hilfszeiger = Anfang;
	while (Anfang != NULL)
	{
		Hilfszeiger = Anfang->Next;
		free(Anfang);
		Anfang = Hilfszeiger;
	}
	ListenAnzahl = 0;
	return;
}

void History(int Sichern, int parameterzahl, ...)		//History(0,0) = history; History(0,1,n) = history n; Histroy(1,1,1000) Sichert die neusten 1000 Elemente
{
	if (Anfang == NULL)	//Sollte nie vorkommen, da zuminedest der Befehl history [n] immer ausgegeben werden müsste; Beim Debugging aber nötig geworden
	{
		return;
	}

	va_list ArgumentPointer;
	int n = ListenAnzahl;		//Anzahl der Auszugeben Elemente wird hier gespeichert werden
	if (parameterzahl)			//Wurden Argumente angegeben, müssen sie abgefragt werden 
	{
		va_start(ArgumentPointer, parameterzahl);
		n = va_arg(ArgumentPointer, int);
		va_end(ArgumentPointer);
		if (n > ListenAnzahl)
		{
			n = ListenAnzahl;
		}
	}

	struct Node *Hilfszeiger;
	int Zähler = 0;										//Ist verantwortlich für die ID in der History
	FILE *HistoryFile = fopen(".hhush.histfile", "wb");	//Öffnet (und erstellt gegebenenfalls) die History-Datei
	if (HistoryFile == NULL)
	{
		//Fehlerbehandlung
	}

	for (; n > 0; n--)
	{
		int k;
		Hilfszeiger = Anfang;
		for (k = 1; k  < n; k++)
		{
			Hilfszeiger = Hilfszeiger->Next;
		}
		if (Sichern == 0)
		{
			printf("%i %s", Zähler, Hilfszeiger);
		}
		else if (Sichern == 1)
		{
			fputs(Hilfszeiger->Eingabe, HistoryFile);
		}

		Zähler++;
	}

	fclose(HistoryFile);
	return;
}

int main(void)
{
	FILE *HistoryFile = fopen(".hhush.histfile", "rb");	//Öffnet die History-Datei
	if (HistoryFile != NULL)	//Falls die History-Datei nicht geöffnet werden konnte (im Regelfall also noch nicht erstellt wurde), wird nichts importiert
	{
		char ImportBuffer[256];
		char *ImportPointer;
		do
		{
			ImportPointer = fgets(ImportBuffer, 256, HistoryFile);
			NeuesElement(ImportBuffer);
		} while (ImportPointer != NULL);
		fclose(HistoryFile);
	}



	{ 
		char input[256];
		History(0, 0);
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		NeuesElement(fgets(input, sizeof(input), stdin));
		
	}


	History(1, 1, 1000);			//Sichert die neusten 1000 Elemente der History in der Datei
	ListeLöschen();					//Um Speicherplatz freizugeben
	return 0;
}