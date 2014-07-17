#define _CRT_SECURE_NO_WARNINGS	
/* Da Visual Studio z.B. strncpy als unsicher ansieht (und strncpy_s haben m�chte, da bei strncpy ein Buffer Overflow auftreten kann),
was aber zu problemen mit gcc f�hren kann; Der Befehl ignoriert das Funktionen "unsicher" sind beim kompilieren in Visual Studio
(sonst w�rde das kompilieren gar nicht erst funktionieren, sondern nur zu einer Fehlermeldung f�hren) */

#include <stdio.h>		//F�r fgets und generell die Ein/Ausgabe
#include <stdlib.h>		//z.B. f�r NULL, malloc() und free()
#include <string.h>		//f�r strncpy, (memcpy)
#include <stdarg.h>		//f�r variable Parameteranzahl

/*******************************************
*******Hier beginnt das History-Modul******
********************************************/

struct Node
{
	char Eingabe[256];
	struct Node *Next;
};

struct Node *Anfang = NULL;

int ListenFehler = 0;		//Wird auf 1 gesetzt, falls die Speicherplatzreservierung f�r die verkettete Liste fehlschl�gt
int ListenAnzahl = 0;		//Gibt die Anzahl der gespeicherten Elemente an

void NeuesElement(char Input[256])
{
	struct Node *NeuerNode = malloc(sizeof(struct Node));		//Speicher reservieren f�r den neuen Knoten

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

		/* Wir f�gen einen neuen Knoten immer am Anfang der Liste ein, dies bedeutet bei einer einfach Verketteten Liste zwar, dass wir
		h�ufig �ber viele Knoten laufen m�ssen, vor allem bei dem History befehl, allerdings ergeben sich auch vorteile, durch die sehr einfache Struktur,
		die es leichter macht, den �berblick zu behalten um z.B. Speicherlecks bei der Programmierung zu vermeiden. Insgesammt ist es also ein Kompromiss:
		Wir benutzen weniger Speicherplatz (als eine Doppelt verkettete Liste) und haben eine sehr einfache Struktur (durch das Einf�gen nur am Anfang und
		da wir keien dynamischen Arrays verwenden), daf�r nehmen wir eine langsamere ausf�hrung einiger Funktionen in kauf. */
	}

	//strncpy(NeuerNode->Eingabe, Input, 256);

	memcpy(NeuerNode->Eingabe, Input, 256 * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden

	return;
}


void ListeL�schen(int n)		// Wir m�ssen nie einzelne Knoten l�schen, also reicht eine Funktion, die alle Knoten l�scht, und somit ihren Speicherplatz wieder freizugeben
{
	FILE *HistoryFile = fopen(".hhush.histfile", "ab");	//�ffnet (und erstellt gegebenenfalls) die History-Datei
	struct Node *Hilfszeiger = Anfang;
	while (Anfang != NULL)
	{
		if (n == 1)
		{
			fputs(Anfang->Eingabe, HistoryFile);		//Routine zur Dateispeicherung ; falls n=0 => nur l�schen f�r "history -c", ansonten history speichern mit n=1
		}
		Hilfszeiger = Anfang->Next;
		free(Anfang);
		Anfang = Hilfszeiger;
	}
	ListenAnzahl = 0;
	if (ferror(HistoryFile))
	{
		//Fehlerbehandlung einbauen f�r Fehler beim Schreiben
	}
	fclose(HistoryFile);
	return;
}

void History(int parameterzahl, ...)
{
	va_list ArgumentPointer;
	int n = -1;					//Anzahl der Auszugeben Elemente wird hier gespeichert werden
	if (parameterzahl)			//Wurden Argumente angegeben, m�ssen sie abgefragt werden 
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
	int Z�hler = 0;				//Ist verantwortlich f�r die ID in der History

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
		printf("%i %s", Z�hler, Hilfszeiger);
		Z�hler++;
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
		ListeL�schen(1);
	}


	return 0;
}