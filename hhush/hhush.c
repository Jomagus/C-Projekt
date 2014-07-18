#define _CRT_SECURE_NO_WARNINGS	
/* Da Visual Studio z.B. strncpy als unsicher ansieht (und strncpy_s haben m�chte, da bei strncpy ein Buffer Overflow auftreten kann),
was aber zu problemen mit gcc f�hren kann; Der Befehl ignoriert das Funktionen "unsicher" sind beim kompilieren in Visual Studio
(sonst w�rde das kompilieren gar nicht erst funktionieren, sondern nur zu einer Fehlermeldung f�hren) */

#include <stdio.h>		//f�r z.B. fgets und generell die Ein/Ausgabe
#include <stdlib.h>		//z.B. f�r NULL, malloc() und free()
#include <string.h>		//f�r strncpy, (memcpy)
#include <stdarg.h>		//f�r variable Parameteranzahl

void FehlerFunktion(char *Fehler)		//Wird zur Ausgabe von Fehlern verwendet
{
	fprintf(stderr, "Ein Fehler ist aufgetreten. Fehler: %s \n", Fehler);
	return;
}

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
		FehlerFunktion("Es konnte kein Speicher f�r die history reserviert werden.");
		return;
	}

	if (Anfang == NULL)		//Falls die Liste leer isi
	{
		Anfang = NeuerNode;
		NeuerNode->Next = NULL;
	}
	else
	{
		Anfang->Prev = NeuerNode;
		NeuerNode->Next = Anfang;
		Anfang = NeuerNode;

		/* Wir f�gen einen neuen Knoten immer am Anfang der Liste ein, dies tun wir, da wir so eine m�glichst einfache Struktur haben
		und desweiteren nie einen Knoten woanders einf�gen m�ssen (da wir diese doppelt verkettete Liste nur f�r history [n] brauchen) */
	}

	ListenAnzahl++;

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
	if (Anfang == NULL)	//Sollte nie vorkommen, da zuminedest der Befehl history [n] immer ausgegeben werden m�sste
	{
		FehlerFunktion("Die History ist leer");
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
	if ((HistoryFile == NULL) && (Sichern == 1))
	{
		FehlerFunktion("History-Datei konnte nicht zum schreiben ge�ffnet werden");
		return;
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

/*************************************************
*******Hier beginnt das Stufe-1-Input-Modul ******
*************************************************/

int InputStufe_1Fehler;		//Wird verwendet um bei Fehlern in Stufe 1 weitere Stufen abzubrechen

struct StackElement
{
	char InputText[256];
	int L�nge;
	struct StackElement *Next;
};

struct StackElement *AnfangStack = NULL;
int StackTiefe;

void Push(char Input[256], int L�nge)
{
	struct StackElement *NeuerNode = malloc(sizeof(struct StackElement));		//Speicher reservieren f�r das neue Stack Element

	if (NeuerNode == NULL)	//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher f�r das StackElement reserviert werden.");
		return;
	}

	if (AnfangStack == NULL)		//Falls die Liste leer isi
	{
		AnfangStack = NeuerNode;
		NeuerNode->Next = NULL;
	}
	else
	{
		NeuerNode->Next = AnfangStack;
		AnfangStack = NeuerNode;
	}

	StackTiefe++;

	memcpy(NeuerNode->InputText, Input, 256 * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden
	NeuerNode->L�nge = L�nge;

	return;
}

struct StackElement Pop(void)
{
	struct  StackElement R�ckgabe;		//Wir kopieren das oberste Stackelement in R�ckgabe

	if (AnfangStack == NULL)	//Falls der Stack leer ist
	{
		if (StackTiefe != 0)
		{
			FehlerFunktion("Fehler in der Stacktiefe");
		}
		R�ckgabe.L�nge = -1;
		R�ckgabe.Next = NULL;
	}
	else
	{
		memcpy(R�ckgabe.InputText, AnfangStack->InputText, 256 * sizeof(char));
		R�ckgabe.L�nge = AnfangStack->L�nge;
		R�ckgabe.Next = NULL;

		struct StackElement *HilfsZeiger = AnfangStack->Next;	//Wir geben den Speicherplatz f�r das oberste Stackelement frei
		free(AnfangStack);
		AnfangStack = HilfsZeiger;

		StackTiefe--;
	}
	
	return R�ckgabe;
}

void InputSchritt_1(char Input[256])
{
	InputStufe_1Fehler = 0;
	int InputL�nge;		//Speichert wie lang die Nutzereingabe ist, indem es angiebt Input[InputL�nge]='\n'

	for (InputL�nge = 0; Input[InputL�nge] != '\n'; InputL�nge++)
	{
		if (InputL�nge > 256)		//Schaut ob die Eingabe zu lang ist
		{
			FehlerFunktion("Eingabe zu lang");
			InputStufe_1Fehler = 1;
			return;
		}
	}

	int Z�hler;		//Z�hlt an welcher Stelle im String ein Pipe Symbol ist, von hinten an
	for (Z�hler= InputL�nge; Input[Z�hler] != '|'; Z�hler--)
	{
		if (Z�hler < 0)
		{
			break;
		}
	}

	if (Z�hler == 0)
	{
		//Fehlerbehandlung falls Eingabe mit '|' beginnt
	}

	if (Z�hler < 0)		//Wenn kein (weiteres) Pipe Symbol gefunden worden ist
	{
		Push(Input, InputL�nge);
		return;
	}
	else
	{
		char EinzelInput[256];			// Hier wird das Kommando hinter dem letzen Pipe Symbol ('|') gespeichert
		int RestInputZ�hler = 0;		//F�r die Numerierung des RestInputs
		int HilfsZ�hler;

		for (HilfsZ�hler = Z�hler + 1; HilfsZ�hler <= InputL�nge; HilfsZ�hler++)
		{
			EinzelInput[RestInputZ�hler] = Input[HilfsZ�hler];
			RestInputZ�hler++;
		}
		EinzelInput[RestInputZ�hler] = '\0';
		Input[Z�hler] = '\n';		//Das ehemals letze Pipes Symbol wird zu einer Newline, somit k�rzen wir den String um den letzen Befehl
		Input[Z�hler+  1] = '\0';	//Den String beende wir noch mit \0
		Push(EinzelInput, InputL�nge-Z�hler);
		InputSchritt_1(Input);
		return;
	}
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
		while (ImportPointer != NULL)			//Wir lesen die Datei Zeilenweise aus, bis wir am Ende der Datei sind (in der Datei sind maximal 1000 Befehle)
		{
			NeuesElement(ImportBuffer);
			ImportPointer = fgets(ImportBuffer, 256, HistoryFile);
		}
		fclose(HistoryFile);
	}

	/*******************************************
	*******Hier beginnt das Input-Modul ******
	********************************************/
		char Input[256];	//Wird zur Speicherung der Nutzereingabe verwendet




		while (1)
		{
			fgets(Input, sizeof(Input), stdin);
			InputSchritt_1(Input);
			struct StackElement Test = Pop();
			printf("%s", Test.InputText);
			Test = Pop();
			printf("%s", Test.InputText);























		}
	
	

	











	History(1, 1, 1000);			//Sichert die neusten 1000 Elemente der History in der Datei
	ListeL�schen();					//Um Speicherplatz freizugeben
	return 0;
}