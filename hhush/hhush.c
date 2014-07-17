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
	struct Node *Prev;
	struct Node *Next;
};

struct Node *Anfang = NULL;

int ListenAnzahl = 0;		//Gibt die Anzahl der gespeicherten Elemente an

void NeuesElement(char Input[256])
{
	struct Node *NeuerNode = malloc(sizeof(struct Node));		//Speicher reservieren f�r den neuen Knoten

	if (NeuerNode == NULL)	//Falls kein Speicher freigegeben werden konnte
	{
		//Fehlerbehandlung noch machen
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
		Anfang->Prev = NeuerNode;
		NeuerNode->Next = Anfang;
		Anfang = NeuerNode;
		ListenAnzahl++;

		/* Wir f�gen einen neuen Knoten immer am Anfang der Liste ein, dies tun wir, da wir so eine m�glichst einfache Struktur haben
		und desweiteren nie einen Knoten woanders einf�gen m�ssen (da wir diese doppelt verkettete Liste nur f�r history [n] brauchen) */
	}

	//strncpy(NeuerNode->Eingabe, Input, 256);

	memcpy(NeuerNode->Eingabe, Input, 256 * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden

	return;
}


void ListeL�schen()		// Wir m�ssen nie einzelne Knoten l�schen, also reicht eine Funktion, die alle Knoten l�scht, und somit ihren Speicherplatz wieder freizugeben
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
	if (Anfang == NULL)	//Sollte nie vorkommen, da zuminedest der Befehl history [n] immer ausgegeben werden m�sste; Beim Debugging aber n�tig geworden
	{
		return;
	}

	va_list ArgumentPointer;
	int n = ListenAnzahl;		//Anzahl der Auszugeben Elemente wird hier gespeichert werden
	if (parameterzahl)			//Wurden Argumente angegeben, m�ssen sie abgefragt werden 
	{
		va_start(ArgumentPointer, parameterzahl);
		n = va_arg(ArgumentPointer, int);
		va_end(ArgumentPointer);
		if (n > ListenAnzahl)
		{
			n = ListenAnzahl;
		}
	}

	struct Node *Hilfszeiger = Anfang;
	int Z�hler = 0;										//Ist verantwortlich f�r die ID in der History
	FILE *HistoryFile = fopen(".hhush.histfile", "wb");	//�ffnet (und erstellt gegebenenfalls) die History-Datei
	if (HistoryFile == NULL)
	{
		//Fehlerbehandlung
	}

	int k = 1;
	for (k = 1; k < n; k++)								//Gehe n Tief in die Liste rein
	{
		Hilfszeiger = Hilfszeiger->Next;
	}

	for (; n > 0; n--)									//Gehe die Liste r�ckw�rts bis zum Start durch
	{
		if (Sichern == 0)
		{
			printf("%i %s", Z�hler, Hilfszeiger);
		}
		else if (Sichern == 1)
		{
			fputs(Hilfszeiger->Eingabe, HistoryFile);
		}
		Hilfszeiger = Hilfszeiger->Prev;
		Z�hler++;
	}

	// Es folgt eine Implementierung f�r eine einfach verkettete Liste (erst sp�ter habe ich die Liste doppelt verkettet, 
	// falls ein fallback n�tig ist, steht dies noch hier)
	//for (; n > 0; n--)
	//{
	//	int k;
	//	Hilfszeiger = Anfang;
	//	for (k = 1; k  < n; k++)
	//	{
	//		Hilfszeiger = Hilfszeiger->Next;
	//	}
	//	if (Sichern == 0)
	//	{
	//		printf("%i %s", Z�hler, Hilfszeiger);
	//	}
	//	else if (Sichern == 1)
	//	{
	//		fputs(Hilfszeiger->Eingabe, HistoryFile);
	//	}

	//	Z�hler++;
	//}


	fclose(HistoryFile);
	return;
}

int main(void)
{
	/*******************************************
	*******Hier wird die history importiert******
	********************************************/

	FILE *HistoryFile = fopen(".hhush.histfile", "rb");	//�ffnet die History-Datei
	if (HistoryFile != NULL)	//Falls die History-Datei nicht ge�ffnet werden konnte (im Regelfall also noch nicht erstellt wurde), wird nichts importiert
	{
		char ImportBuffer[256];
		char *ImportPointer = fgets(ImportBuffer, 256, HistoryFile);
		do
		{
			NeuesElement(ImportBuffer);
			ImportPointer = fgets(ImportBuffer, 256, HistoryFile);
		} while (ImportPointer != NULL);			//Wir lesen die Datei Zeilenweise aus, bis wir am Ende der Datei sind (in der Datei sind maximal 1000 Befehle)
		fclose(HistoryFile);
	}

	/*******************************************
	*******Hier beginnt ******
	********************************************/

	

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
	ListeL�schen();					//Um Speicherplatz freizugeben
	return 0;
}