#define _CRT_SECURE_NO_WARNINGS	
/* Da Visual Studio z.B. strncpy als unsicher ansieht (und strncpy_s haben möchte, da bei strncpy ein Buffer Overflow auftreten kann),
was aber zu problemen mit gcc führen kann; Der Befehl ignoriert das Funktionen "unsicher" sind beim kompilieren in Visual Studio
(sonst würde das kompilieren gar nicht erst funktionieren, sondern nur zu einer Fehlermeldung führen) */

#include <stdio.h>		//für z.B. fgets und generell die Ein/Ausgabe
#include <stdlib.h>		//z.B. für NULL, malloc() und free()
#include <string.h>		//für strncpy, (memcpy)
#include <stdarg.h>		//für variable Parameteranzahl

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
	struct Node *NeuerNode = malloc(sizeof(struct Node));		//Speicher reservieren für den neuen Knoten

	if (NeuerNode == NULL)	//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher für die history reserviert werden.");
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

		/* Wir fügen einen neuen Knoten immer am Anfang der Liste ein, dies tun wir, da wir so eine möglichst einfache Struktur haben
		und desweiteren nie einen Knoten woanders einfügen müssen (da wir diese doppelt verkettete Liste nur für history [n] brauchen) */
	}

	ListenAnzahl++;

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
	if (Anfang == NULL)	//Sollte nie vorkommen, da zuminedest der Befehl history [n] immer ausgegeben werden müsste
	{
		FehlerFunktion("Die History ist leer");
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

	struct Node *Hilfszeiger = Anfang;
	int Zähler = 0;										//Ist verantwortlich für die ID in der History
	FILE *HistoryFile = fopen(".hhush.histfile", "wb");	//Öffnet (und erstellt gegebenenfalls) die History-Datei
	if ((HistoryFile == NULL) && (Sichern == 1))
	{
		FehlerFunktion("History-Datei konnte nicht zum schreiben geöffnet werden");
		return;
	}

	int k = 1;
	for (k = 1; k < n; k++)								//Gehe n Tief in die Liste rein
	{
		Hilfszeiger = Hilfszeiger->Next;
	}

	for (; n > 0; n--)									//Gehe die Liste rückwärts bis zum Start durch
	{
		if (Sichern == 0)
		{
			printf("%i %s", Zähler, Hilfszeiger);
		}
		else if (Sichern == 1)
		{
			fputs(Hilfszeiger->Eingabe, HistoryFile);
		}
		Hilfszeiger = Hilfszeiger->Prev;
		Zähler++;
	}

	// Es folgt eine Implementierung für eine einfach verkettete Liste (erst später habe ich die Liste doppelt verkettet, 
	// falls ein fallback nötig ist, steht dies noch hier)
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
	//		printf("%i %s", Zähler, Hilfszeiger);
	//	}
	//	else if (Sichern == 1)
	//	{
	//		fputs(Hilfszeiger->Eingabe, HistoryFile);
	//	}
	//	Zähler++;
	//}


	fclose(HistoryFile);
	return;
}

/*************************************************
*******Hier beginnt das Stufe-0-Input-Modul ******
*************************************************/

int InputStufe_0(char Input[256])		//gibt 0 zurück, falls die Eingabe nicht "leer" ist
{
	int Zähler;
	for (Zähler = 0; Input[Zähler] != '\n'; Zähler++)
	{
		if ((Input[Zähler] != '\t') && (Input[Zähler] != ' '))
		{
			return 0;
		}

		if (Zähler > 256)
		{
			FehlerFunktion("Eingabe zu lang");
			return 1;
		}
	}
	return 1;
}

/*************************************************
*******Hier beginnt das Stufe-1-Input-Modul ******
*************************************************/

int InputStufe_1Fehler;		//Wird verwendet um bei Fehlern in Stufe 1 weitere Stufen abzubrechen

struct StackElement
{
	char InputText[256];
	struct StackElement *Next;
};

struct StackElement *AnfangStack = NULL;
int StackTiefe;

void Push(char Input[256])
{
	struct StackElement *NeuerNode = malloc(sizeof(struct StackElement));		//Speicher reservieren für das neue Stack Element

	if (NeuerNode == NULL)	//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher für das StackElement reserviert werden.");
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

	return;
}

struct StackElement Pop(void)
{
	struct  StackElement Rückgabe;		//Wir kopieren das oberste Stackelement in Rückgabe

	if (AnfangStack == NULL)	//Falls der Stack leer ist
	{
		FehlerFunktion("Input_1 Stack ist leer");
		Rückgabe.Next = NULL;
	}
	else
	{
		memcpy(Rückgabe.InputText, AnfangStack->InputText, 256 * sizeof(char));
		Rückgabe.Next = NULL;

		struct StackElement *HilfsZeiger = AnfangStack->Next;	//Wir geben den Speicherplatz für das oberste Stackelement frei
		free(AnfangStack);
		AnfangStack = HilfsZeiger;

		StackTiefe--;
	}
	
	return Rückgabe;
}

void InputStufe_1(char Input[256])
{
	InputStufe_1Fehler = 0;
	int InputLänge;		//Speichert wie lang die Nutzereingabe ist, indem es angiebt Input[InputLänge]='\n'

	for (InputLänge = 0; Input[InputLänge] != '\n'; InputLänge++)
	{
		if (InputLänge > 256)		//Schaut ob die Eingabe zu lang ist
		{
			FehlerFunktion("Eingabe zu lang");
			InputStufe_1Fehler = 1;
			return;
		}
	}

	int Zähler;		//Zählt an welcher Stelle im String ein Pipe Symbol ist, von hinten an
	for (Zähler= InputLänge; Input[Zähler] != '|'; Zähler--)
	{
		if (Zähler < 0)
		{
			break;
		}
	}

	if (Zähler == 0)
	{
		//Fehlerbehandlung falls Eingabe mit '|' beginnt
	}

	if (Zähler < 0)		//Wenn kein (weiteres) Pipe Symbol gefunden worden ist
	{
		Push(Input, InputLänge);
		return;
	}
	else
	{
		char EinzelInput[256];			//Hier wird das Kommando hinter dem letzen Pipe Symbol ('|') gespeichert
		int RestInputZähler = 0;		//Für die Numerierung des RestInputs
		int HilfsZähler;

		for (HilfsZähler = Zähler + 1; HilfsZähler <= InputLänge; HilfsZähler++)		//Kopiert das Kommando hinter dem letzen Pipe Symbol in EinzelInput
		{
			EinzelInput[RestInputZähler] = Input[HilfsZähler];
			RestInputZähler++;
		}
		EinzelInput[RestInputZähler] = '\0'; //Beendet den String des neusten Kommandos
		Input[Zähler] = '\n';		//Das ehemals letze Pipes Symbol wird zu einer Newline, somit kürzen wir den String um den letzen Befehl
		Input[Zähler+  1] = '\0';	//Den String beende wir noch mit \0
		Push(EinzelInput);
		InputStufe_1(Input);
		return;
	}
}

/*************************************************
*******Hier beginnt das Stufe-2-Input-Modul ******
*************************************************/

void InputStufe_2()				//iteriert über dem Stufe-1-Stack und ersetzt alle Tabs durch Leerzeichen
{
	struct StackElement *Hilfspointer = AnfangStack;
	int Zähler;
	while (Hilfspointer != NULL)	//diese While Konstuktion iteriert über dem Stufe-1-Stack
	{
		for (Zähler = 0; Hilfspointer->InputText[Zähler] != '\n'; Zähler++)		//diese for-Schleife läuft über den Nutzerinput
		{
			if (Hilfspointer->InputText[Zähler] == '\t')
			{
				Hilfspointer->InputText[Zähler] = ' ';
			}
		}
		Hilfspointer = Hilfspointer->Next;
	}
	return;
}

/*************************************************
*******Hier beginnt das Stufe-3-Input-Modul ******
*************************************************/

void InputStufe_3()				//iteriert über dem Stufe-1-Stack und minimiert die Leerzeichen maximal
{
	struct StackElement *Hilfspointer = AnfangStack;
	int Zähler;							//wird als Zähler für das Char Array verwendet, dass die bereinigte Nutzereingabe enthält
	while (Hilfspointer != NULL)		//diese While Konstuktion iteriert über dem Stufe-1-Stack (wie bereits in Stufe-2)
	{
		if (Hilfspointer->InputText[0] == ' ')		//Wenn die Eingabe mit einem Leerzeichen beginnt
		{
			for (Zähler = 0; Hilfspointer->InputText[Zähler] != '\n'; Zähler ++)
			{
				if (Zähler == 256-1)				//Ausnahmesituation bei sehr langen Eingaben (kann dies überhaupt eintreten?)
				{
					Hilfspointer->InputText[Zähler] = '\0';
					break;
				}
				Hilfspointer->InputText[Zähler] = Hilfspointer->InputText[Zähler + 1];	//verschiebe die komplette Eingabe "eins nach vorne" (auf Pos. 0 war ein Leerzeichen)
			}
			continue;		//Starte von Vorne, falls auch auf Position 1 ein Leerzeichen war
		}

		for (Zähler = 0; Hilfspointer->InputText[Zähler] != '\n'; Zähler++)	//hier wird über den Nutzerinput gelaufen, desweiteren ist das erste Zeichen kein Leerzeichen
		{
			/* Wir wollen wenn mehrere Leerzeichen hintereinander stehen diese auf ein einzelnes Leerzeichen als Trenner von Eingaben reduzieren.
			Dazu schauen wir, ob hinter einem Leerzeichen ein weiteres steht und falls dies der Fall ist, verschieben wir alles (mitsammt des zweiten Leerzeichens)
			um eins nach vorne und überschreiben so das erste mit dem Zweiten Leerzeichen. Wenn wir dies gemacht haben, wiederholen wir diesen Schritt auf dieser
			Position (deshalb Zähler--; ), bis dort keine zwei Leerzeichen mehr hintereinander stehen. */
			if (Hilfspointer->InputText[Zähler] == ' ' && Hilfspointer->InputText[Zähler + 1] == ' ')
			{
				int HilfsZähler = Zähler;
				for (; Hilfspointer->InputText[HilfsZähler] != '\n'; HilfsZähler++)
				{
					if (HilfsZähler == 256 - 1)				//Ausnahmesituation bei sehr langen Eingaben
					{
						Hilfspointer->InputText[HilfsZähler] = '\0';
						break;
					}
					Hilfspointer->InputText[HilfsZähler] = Hilfspointer->InputText[HilfsZähler + 1];		//verschiebe alles "eins nach vorne"
					Zähler--;
				}
			}
		}

		if (Hilfspointer->InputText[Zähler-1] == ' ')	//falls direkt vor dem \n noch ein Leerzeichen ist
		{
			Hilfspointer->InputText[Zähler - 1] = '\n';
			Hilfspointer->InputText[Zähler] = '\0';
		}

		Hilfspointer = Hilfspointer->Next;
	}
	return;
}




int main(void)
{
	/*******************************************
	*******Hier wird die History importiert******
	********************************************/

	FILE *HistoryFile = fopen(".hhush.histfile", "rb");	//Öffnet die History-Datei
	if (HistoryFile != NULL)	//Falls die History-Datei nicht geöffnet werden konnte (im Regelfall also noch nicht erstellt wurde), wird nichts importiert
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
			if (InputStufe_0(Input))	//falls nur Leerzeichen/Tabs eingegeben wurden, wird neu angefangen
			{
				continue;
			}
			InputStufe_1(Input);
			if (InputStufe_1Fehler == 1)
			{
				//Fehlerbehandlung
				continue;
			}
			NeuesElement(Input);		//speichert die Eingabe in der History; wird erst hier gemacht, da erst in Stufe 1 auf länge überprüft wird
			InputStufe_2();				//ersetzt alle Tabs in der Eingabe durch Leerzeichen
			InputStufe_3();				//löscht alle Leerzeichen, die keine Eingabe trennen







			int Testzähler=1;
			struct StackElement Test;
			while (StackTiefe>0)
			{
				Test = Pop();
				printf("%d: %s", Testzähler, Test.InputText);

				Testzähler++;
			}























		}
	
	

	











		History(1, 1, 1000);			//Sichert die neusten 1000 Elemente der History in der Datei
		ListeLöschen();					//Um Speicherplatz freizugeben
		return 0;
}