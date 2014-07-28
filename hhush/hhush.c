#define _CRT_SECURE_NO_WARNINGS	
/* Da Visual Studio z.B. strncpy als unsicher ansieht (und strncpy_s haben m�chte, da bei strncpy ein Buffer Overflow auftreten kann),
was aber zu problemen mit gcc f�hren kann; Der Befehl ignoriert das Funktionen "unsicher" sind beim kompilieren in Visual Studio
(sonst w�rde das kompilieren gar nicht erst funktionieren, sondern nur zu einer Fehlermeldung f�hren) */

#include <stdio.h>		//f�r z.B. fgets und generell die Ein/Ausgabe
#include <stdlib.h>		//z.B. f�r NULL, malloc() und free()
#include <string.h>		//f�r memcpy
#include <stdarg.h>		//f�r variable Parameteranzahl

#define INPUT_SIZE_MAX 256		//falls sp�ter l�ngere Inputs bearbeitet werden sollen

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
	char Eingabe[INPUT_SIZE_MAX];
	struct Node *Prev;
	struct Node *Next;
};

struct Node *Anfang = NULL;

int ListenAnzahl = 0;		//Gibt die Anzahl der gespeicherten Elemente an

void NeuesElement(char Input[INPUT_SIZE_MAX])
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

	//strncpy(NeuerNode->Eingabe, Input, INPUT_SIZE_MAX);

	memcpy(NeuerNode->Eingabe, Input, INPUT_SIZE_MAX * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden

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
*******Hier beginnt das Stufe-0-Input-Modul ******
*************************************************/

int InputStufe_0(char Input[INPUT_SIZE_MAX])		//gibt 0 zur�ck, falls die Eingabe nicht "leer" ist
{
	int Z�hler;
	for (Z�hler = 0; Input[Z�hler] != '\n'; Z�hler++)
	{
		if ((Input[Z�hler] != '\t') && (Input[Z�hler] != ' '))
		{
			return 0;
		}

		if (Z�hler > INPUT_SIZE_MAX)
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
	char InputText[INPUT_SIZE_MAX];
	struct StackElement *Next;
};

struct StackElement *AnfangStack = NULL;
int StackTiefe;

void Push(char Input[INPUT_SIZE_MAX])
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

	memcpy(NeuerNode->InputText, Input, INPUT_SIZE_MAX * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden

	return;
}

struct StackElement Pop(void)
{
	struct  StackElement R�ckgabe;		//Wir kopieren das oberste Stackelement in R�ckgabe

	if (AnfangStack == NULL)	//Falls der Stack leer ist
	{
		FehlerFunktion("Input_1 Stack ist leer");
		R�ckgabe.Next = NULL;
	}
	else
	{
		memcpy(R�ckgabe.InputText, AnfangStack->InputText, INPUT_SIZE_MAX * sizeof(char));
		R�ckgabe.Next = NULL;

		struct StackElement *HilfsZeiger = AnfangStack->Next;	//Wir geben den Speicherplatz f�r das oberste Stackelement frei
		free(AnfangStack);
		AnfangStack = HilfsZeiger;

		StackTiefe--;
	}
	
	return R�ckgabe;
}

void InputStufe_1(char Input[INPUT_SIZE_MAX])
{
	InputStufe_1Fehler = 0;
	int InputL�nge;		//Speichert wie lang die Nutzereingabe ist, indem es angiebt Input[InputL�nge]='\n'

	for (InputL�nge = 0; Input[InputL�nge] != '\n'; InputL�nge++)
	{
		if (InputL�nge > INPUT_SIZE_MAX)		//Schaut ob die Eingabe zu lang ist
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
		Push(Input);
		return;
	}
	else
	{
		char EinzelInput[INPUT_SIZE_MAX];			//Hier wird das Kommando hinter dem letzen Pipe Symbol ('|') gespeichert
		int RestInputZ�hler = 0;		//F�r die Numerierung des RestInputs
		int HilfsZ�hler;

		for (HilfsZ�hler = Z�hler + 1; HilfsZ�hler <= InputL�nge; HilfsZ�hler++)		//Kopiert das Kommando hinter dem letzen Pipe Symbol in EinzelInput
		{
			EinzelInput[RestInputZ�hler] = Input[HilfsZ�hler];
			RestInputZ�hler++;
		}
		EinzelInput[RestInputZ�hler] = '\0'; //Beendet den String des neusten Kommandos
		Input[Z�hler] = '\n';		//Das ehemals letze Pipes Symbol wird zu einer Newline, somit k�rzen wir den String um den letzen Befehl
		Input[Z�hler+  1] = '\0';	//Den String beende wir noch mit \0
		Push(EinzelInput);
		InputStufe_1(Input);
		return;
	}
}

/*************************************************
*******Hier beginnt das Stufe-2-Input-Modul ******
*************************************************/

void InputStufe_2(void)				//iteriert �ber dem Stufe-1-Stack und ersetzt alle Tabs durch Leerzeichen
{
	struct StackElement *Hilfspointer = AnfangStack;
	int Z�hler;
	while (Hilfspointer != NULL)	//diese While Konstuktion iteriert �ber dem Stufe-1-Stack
	{
		for (Z�hler = 0; Hilfspointer->InputText[Z�hler] != '\n'; Z�hler++)		//diese for-Schleife l�uft �ber den Nutzerinput
		{
			if (Hilfspointer->InputText[Z�hler] == '\t')
			{
				Hilfspointer->InputText[Z�hler] = ' ';
			}
		}
		Hilfspointer = Hilfspointer->Next;
	}
	return;
}

/*************************************************
*******Hier beginnt das Stufe-3-Input-Modul ******
*************************************************/

void InputStufe_3(void)				//iteriert �ber dem Stufe-1-Stack und minimiert die Leerzeichen maximal
{
	struct StackElement *Hilfspointer = AnfangStack;
	int Z�hler;							//wird als Z�hler f�r das Char Array verwendet, dass die bereinigte Nutzereingabe enth�lt
	while (Hilfspointer != NULL)		//diese While Konstuktion iteriert �ber dem Stufe-1-Stack (wie bereits in Stufe-2)
	{
		if (Hilfspointer->InputText[0] == ' ')		//Wenn die Eingabe mit einem Leerzeichen beginnt
		{
			for (Z�hler = 0; Hilfspointer->InputText[Z�hler] != '\n'; Z�hler ++)
			{
				if (Z�hler == INPUT_SIZE_MAX-1)				//Ausnahmesituation bei sehr langen Eingaben (kann dies �berhaupt eintreten?)
				{
					Hilfspointer->InputText[Z�hler] = '\0';
					break;
				}
				Hilfspointer->InputText[Z�hler] = Hilfspointer->InputText[Z�hler + 1];	//verschiebe die komplette Eingabe "eins nach vorne" (auf Pos. 0 war ein Leerzeichen)
			}
			continue;		//Starte von Vorne, falls auch auf Position 1 ein Leerzeichen war
		}

		for (Z�hler = 0; Hilfspointer->InputText[Z�hler] != '\n'; Z�hler++)	//hier wird �ber den Nutzerinput gelaufen, desweiteren ist das erste Zeichen kein Leerzeichen
		{
			/* Wir wollen wenn mehrere Leerzeichen hintereinander stehen diese auf ein einzelnes Leerzeichen als Trenner von Eingaben reduzieren.
			Dazu schauen wir, ob hinter einem Leerzeichen ein weiteres steht und falls dies der Fall ist, verschieben wir alles (mitsammt des zweiten Leerzeichens)
			um eins nach vorne und �berschreiben so das erste mit dem Zweiten Leerzeichen. Wenn wir dies gemacht haben, wiederholen wir diesen Schritt auf dieser
			Position (deshalb Z�hler--; ), bis dort keine zwei Leerzeichen mehr hintereinander stehen. */
			if (Hilfspointer->InputText[Z�hler] == ' ' && Hilfspointer->InputText[Z�hler + 1] == ' ')
			{
				int HilfsZ�hler = Z�hler;
				for (; Hilfspointer->InputText[HilfsZ�hler] != '\n'; HilfsZ�hler++)
				{
					if (HilfsZ�hler == INPUT_SIZE_MAX - 1)				//Ausnahmesituation bei sehr langen Eingaben
					{
						Hilfspointer->InputText[HilfsZ�hler] = '\0';
						break;
					}
					Hilfspointer->InputText[HilfsZ�hler] = Hilfspointer->InputText[HilfsZ�hler + 1];		//verschiebe alles "eins nach vorne"
					Z�hler--;
				}
			}
		}

		if (Hilfspointer->InputText[Z�hler-1] == ' ')	//falls direkt vor dem \n noch ein Leerzeichen ist
		{
			Hilfspointer->InputText[Z�hler - 1] = '\n';
			Hilfspointer->InputText[Z�hler] = '\0';
		}

		Hilfspointer = Hilfspointer->Next;
	}
	return;
}

/*************************************************
*******Hier beginnt das Stufe-4-Input-Modul ******
*************************************************/

struct BefehlsListe
{
	char Befehl[INPUT_SIZE_MAX];
	struct BefehlsListe *Next;
};

struct BefehlsListe *BefehlsListenAnfang = NULL;
int BefehlAnzahl;		//Speichert wie viele Argumente �bergeben wurden (der Befehl mitgez�hlt; gut zum Vergleich, ob falsche menge an Argumenten eingegeben wurde)

/* Die Folgende Funktion InputStufe_4 ist eingentlich nur eine abgespeckte Kopie von InputStufe_1 (und NeuerBefehl() ist eigentlich Push(),
bzw. Pop() ist GetBefehl()), nur dass hier nicht bei Pipe Symbolen '|' sondern be Leerzeichen getrennt wird. Es spart mir nur sehr viel Arbeit, 
ein zweite Funktion zu schreiben, die quasi dasselbe tut, nur mit einem anderen Char. */

void NeuerBefehl(char Input[INPUT_SIZE_MAX])
{
	struct BefehlsListe *NeuerBefehl = malloc(sizeof(struct BefehlsListe));		//Speicher reservieren f�r das neuen Befehl

	if (NeuerBefehl == NULL)	//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher f�r den Befehl reserviert werden.");
		return;
	}

	if (BefehlsListenAnfang == NULL)		//Falls die Liste leer isi
	{
		BefehlsListenAnfang = NeuerBefehl;
		NeuerBefehl->Next = NULL;
	}
	else
	{
		NeuerBefehl->Next = BefehlsListenAnfang;
		BefehlsListenAnfang = NeuerBefehl;
	}

	BefehlAnzahl++;
	memcpy(NeuerBefehl->Befehl, Input, INPUT_SIZE_MAX * sizeof(char));

	return;
}

void InputStufe_4(char Input[INPUT_SIZE_MAX])
{
	int InputL�nge;		//Speichert wie lang die Nutzereingabe ist, indem es angiebt Input[InputL�nge]='\n'

	for (InputL�nge = 0; Input[InputL�nge] != '\n'; InputL�nge++)
	{
		//Z�hlt wie lange die Nutzereingabe ist
	}

	int Z�hler;		//Z�hlt an welcher Stelle im String ein Leerzeichen ist, von hinten an
	for (Z�hler = InputL�nge; Input[Z�hler] != ' '; Z�hler--)
	{
		if (Z�hler < 0)
		{
			break;
		}
	}

	if (Z�hler < 0)		//Wenn kein (weiteres) Leerzeichen gefunden worden ist
	{
		Input[InputL�nge] = '\0';
		NeuerBefehl(Input);
		return;
	}
	else
	{
		char EinzelInput[INPUT_SIZE_MAX];			//Hier wird das Kommando hinter dem letzen Pipe Symbol ('|') gespeichert
		int RestInputZ�hler = 0;		//F�r die Numerierung des RestInputs
		int HilfsZ�hler;

		for (HilfsZ�hler = Z�hler + 1; HilfsZ�hler <= InputL�nge; HilfsZ�hler++)		//Kopiert das Kommando hinter dem letzen Pipe Symbol in EinzelInput
		{
			EinzelInput[RestInputZ�hler] = Input[HilfsZ�hler];
			RestInputZ�hler++;
		}
		EinzelInput[RestInputZ�hler-1] = '\0'; //Beendet den String des neusten Kommandos
		Input[Z�hler] = '\0';		//Das ehemalige Leerzeichen wird zu '\0'
		NeuerBefehl(EinzelInput);
		InputStufe_4(Input);
		return;
	}
}

struct BefehlsListe GetBefehl(void)
{
	struct  BefehlsListe R�ckgabe;		//Wir kopieren das oberste Stackelement in R�ckgabe

	if (BefehlsListenAnfang == NULL)	//Falls die Befehlsliste leer ist
	{
		FehlerFunktion("Befehlsliste ist leer");
		R�ckgabe.Next = NULL;
		return R�ckgabe;
	}

	memcpy(R�ckgabe.Befehl, BefehlsListenAnfang->Befehl, INPUT_SIZE_MAX * sizeof(char));
	R�ckgabe.Next = NULL;			//aus Sicherheit


	struct BefehlsListe *HilfsZeiger = BefehlsListenAnfang->Next;	//Wir geben den Speicherplatz f�r das oberste Stackelement frei
	free(BefehlsListenAnfang);
	BefehlsListenAnfang = HilfsZeiger;
	BefehlAnzahl--;
	return R�ckgabe;
}

void BefehlsListeReset(void)		//wir l�schen die komplette Befehlsliste und geben ihren Speicherplatz wieder frei (bei ung�ltigen Funktionsaufrufen eingesetzt)
{
	while (BefehlsListenAnfang != NULL)
	{
		struct BefehlsListe *HilfsZeiger = BefehlsListenAnfang->Next;
		free(BefehlsListenAnfang);
		BefehlsListenAnfang = HilfsZeiger;
	}
	BefehlAnzahl = 0;
	return;
}

/*************************************************
*******Hier beginnt der Befehl-Interpreter ******
*************************************************/

int BefehlInterpreter(void)
{
	struct BefehlsListe Kommando = GetBefehl();
	if (strcmp(Kommando.Befehl, "exit") == 0)
	{
		return 0;
	}
	else if (strcmp(Kommando.Befehl, "date") == 0)
	{
		return 1;
	}
	else if (strcmp(Kommando.Befehl, "history") == 0)
	{
		return 2;
	}
	else if (strcmp(Kommando.Befehl, "echo") == 0)
	{
		return 3;
	}
	else if (strcmp(Kommando.Befehl, "ls") == 0)
	{
		return 4;
	}
	else if (strcmp(Kommando.Befehl, "cd") == 0)
	{
		return 5;
	}
	else if (strcmp(Kommando.Befehl, "grep") == 0)
	{
		return 6;
	}
	return -1;
}

void FunktionsAufrufer()
{
	switch (BefehlInterpreter())
	{
		case 0: /* EXIT */; break;
		case 1: /* DATE */; break;
		case 2: /* History */; break;
		case 3: /* echo */; break;
		case 4: /* ls */; break;
		case 5: /* cd */; break;
		case 6: /* grep */; break;
		default: /* no such program */
		break;
	}
}



int main(void)
{
	/*******************************************
	*******Hier wird die History importiert******
	********************************************/

	FILE *HistoryFile = fopen(".hhush.histfile", "rb");	//�ffnet die History-Datei
	if (HistoryFile != NULL)	//Falls die History-Datei nicht ge�ffnet werden konnte (im Regelfall also noch nicht erstellt wurde), wird nichts importiert
	{
		char ImportBuffer[INPUT_SIZE_MAX];
		char *ImportPointer = fgets(ImportBuffer, INPUT_SIZE_MAX, HistoryFile);
		while (ImportPointer != NULL)			//Wir lesen die Datei Zeilenweise aus, bis wir am Ende der Datei sind (in der Datei sind maximal 1000 Befehle)
		{
			NeuesElement(ImportBuffer);
			ImportPointer = fgets(ImportBuffer, INPUT_SIZE_MAX, HistoryFile);
		}
		fclose(HistoryFile);
	}

	/*******************************************
	*******Hier beginnt das Input-Modul ******
	********************************************/
		char Input[INPUT_SIZE_MAX];	//Wird zur Speicherung der Nutzereingabe verwendet




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
			NeuesElement(Input);		//speichert die Eingabe in der History; wird erst hier gemacht, da erst in Stufe 1 auf l�nge �berpr�ft wird
			InputStufe_2();				//ersetzt alle Tabs in der Eingabe durch Leerzeichen
			InputStufe_3();				//l�scht alle Leerzeichen, die keine Eingabe trennen

			while (StackTiefe > 0)		//diese Schleife f�hrt hintereinander die durch '|' getrennten Befehle aus
			{
				InputStufe_4(Pop().InputText);
				FunktionsAufrufer();

				if (BefehlAnzahl > 0)
				{
					FehlerFunktion("BefehlsListe nicht leer");
					BefehlsListeReset();
				}
			}


			//Bis hier steht "fertiger" Code


			//TODO ALLE Outputs werden zwischengespeichert, erst nach dem durchlaufen der while (StackTiefe > 0) Schleife ausgegeben





















		}
	
	

	











		History(1, 1, 1000);			//Sichert die neusten 1000 Elemente der History in der Datei
		ListeL�schen();					//Um Speicherplatz freizugeben
		return 0;
}