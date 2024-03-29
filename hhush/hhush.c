#define _CRT_SECURE_NO_WARNINGS	
/* Da Visual Studio z.B. strncpy als unsicher ansieht (und strncpy_s haben moechte, da bei strncpy ein Buffer Overflow auftreten kann),
was aber zu problemen mit gcc fuehren kann. Gleiches gilt fuer fopen (statt fopen_s); Der Befehl ignoriert das Funktionen "unsicher" sind beim 
kompilieren in Visual Studio (sonst wuerde das kompilieren gar nicht erst funktionieren, sondern nur zu einer Fehlermeldung fuehren) */

#include <stdio.h>		//fuer z.B. fgets und generell die Ein/Ausgabe
#include <stdlib.h>		//z.B. fuer NULL, malloc() und free()
#include <string.h>		//fuer memcpy
#include <time.h>		//fuer das date-Programm
#include <sys/types.h>	//fuer Verzeichnisfunktionen
#include <dirent.h>		//ebenso fuer Verzeichnisfunktionen

#ifdef WIN32			//fuegt den Header fuer weitere Verzeichnisfunktionen ein
//#i nclude <direct.h>	//entferne das Leerzeichen sowie "//" damit das Programm auch unter Windows lauft (noetig gewurden durch Headertest)
#else
#include <unistd.h>
#endif

#define INPUT_SIZE_MAX 256						//falls spaeter laengere Inputs bearbeitet werden sollen

void FehlerFunktion(char *Fehler)				//Wird zur Ausgabe von Fehlern verwendet
{
	fprintf(stderr, "Ein Fehler ist aufgetreten. Fehler: %s \n", Fehler);
	return;
}

int PipeFehler = 0;								//wird auf 1 gesetzt, bei ungueltigem Kommando; auf 2 bei ungueltigen Argumenten

/*************************************************
*******Hier beginnt der Pipe-Buffer *************
*************************************************/

/* Der Pipe-Buffer ist ein Char-Array beliebiger groesse, fuer das dynamisch Speicher reserviert wird.
Die beliebig grosse Ausgabe eines Unterprogramms kann so zwischengespeichert werden, falls Piping
verwendet wird. */

char *GlobalPipeBufferPointer = NULL;

void WritePipeBuffer(char *Input)				//schreibt den Input in einen dynamisch zugewiesenen Speicherbereich und gibt einen Pointer darauf zurueck
{
	if (GlobalPipeBufferPointer != NULL)
	{
		FehlerFunktion("PipeBuffer ist noch nicht leer");
		return;
	}
	int InputLeange = strlen(Input);			//schaut wie lang der Input ist
	char *ReturnPointer = malloc((InputLeange + 1)* sizeof(char));		//reserviert Speicher fuer die Eingabe + '\0'
	if (ReturnPointer == NULL)
	{
		FehlerFunktion("Es konnte kein Speicher fuer den PipeBuffer zugewiesen werden");
		return;
	}
	memcpy(ReturnPointer, Input, (InputLeange + 1)* sizeof(char));
	GlobalPipeBufferPointer = ReturnPointer;
	return;
}

void WipePipeBuffer(void)						//loescht den PipeBufferPointer
{
	if (GlobalPipeBufferPointer == NULL)
	{
		FehlerFunktion("PipeBuffer ist schon leer");
		return;
	}
	free(GlobalPipeBufferPointer);
	GlobalPipeBufferPointer = NULL;
	return;
}

/* Im laufe der Entwicklung dieses Programms wurde mir klar, dass es einfacher ist, eine Funktion zu haben, in die man Zeilenweise schreiben kann, als in jedem Unterprogramm
dafuer zu sorgen, dass die komplette Ausgabe in einem einzelnen String steht, der dann uebergeben wird. (In der Echo Funktion klappte dies noch, aber fuer history und ls war es dann 
doch einfacher, diese Funktion zu entwickeln.*/

void AppendPipeBuffer(char *Input)
{
	int InputLeange = strlen(Input);			//schaut wie lang der Input ist
	if (GlobalPipeBufferPointer == NULL)		//wenn der PipeBuffer noch leer ist, verwenden wir einfach die WritePipeBuffer-Funktion
	{
		WritePipeBuffer(Input);
		return;
	}

	int AlteBufferLeange = strlen(GlobalPipeBufferPointer);		//schaut wie viel schon im PipeBuffer steht
	char *ZwischenPointer = realloc(GlobalPipeBufferPointer, (AlteBufferLeange + InputLeange + 1)*sizeof(char));	//wir vergroessern den reservierten Speicher fuer den neuen Input (+1*'\0')
	if (ZwischenPointer == NULL)				//falls die Speicherplatzvergroesserung nicht funktioniert; man beachte, dass GlobalPipeBufferPointer in dem Fall nicht veraendert wird
	{
		FehlerFunktion("Es konnte kein weiterer Speicher fuer den PipeBuffer zugewiesen werden");
		return;
	}
	GlobalPipeBufferPointer = ZwischenPointer;	//da realloc den Speicherblock verschieben kann
	strcat(GlobalPipeBufferPointer, Input);		//wir haengen den Input hinten an den bestehenden Speicherbereich
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

int ListenAnzahl = 0;							//Gibt die Anzahl der gespeicherten Elemente an

void NeuesElement(char Input[INPUT_SIZE_MAX])
{
	struct Node *NeuerNode = malloc(sizeof(struct Node));		//Speicher reservieren fuer den neuen Knoten

	if (NeuerNode == NULL)						//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher fuer die history reserviert werden.");
		return;
	}

	if (Anfang == NULL)							//Falls die Liste leer isi
	{
		Anfang = NeuerNode;
		NeuerNode->Next = NULL;
	}
	else
	{
		Anfang->Prev = NeuerNode;
		NeuerNode->Next = Anfang;
		Anfang = NeuerNode;

		/* Wir fuegen einen neuen Knoten immer am Anfang der Liste ein, dies tun wir, da wir so eine moeglichst einfache Struktur haben
		und desweiteren nie einen Knoten woanders einfuegen muessen (da wir diese doppelt verkettete Liste nur fuer history [n] brauchen) */
	}
	ListenAnzahl++;
	
	memcpy(NeuerNode->Eingabe, Input, INPUT_SIZE_MAX * sizeof(char)); //Sicherer als strncpy, da Bufferoverflows verhindert werden
	return;
}

void ListeLoeschen(void)						// Wir muessen nie einzelne Knoten loeschen, also reicht eine Funktion, die alle Knoten loescht, und somit ihren Speicherplatz wieder freizugeben
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

void History(int Sichern, int Parameterzahl, int AusgabeWunsch)		//History(0,0,0) = history; History(0,1,n) = history n; Histroy(1,1,1000) Sichert die neusten 1000 Elemente
{
	if (Anfang == NULL)							//Sollte nie vorkommen, da zuminedest der Befehl history [n] immer ausgegeben werden muesste
	{
		FehlerFunktion("Die History ist leer");
		return;
	}

	int n = ListenAnzahl;						//Anzahl der Auszugeben Elemente wird hier gespeichert werden
	if (Parameterzahl)							//Wurden Argumente angegeben, muessen sie abgefragt werden 
	{
		n = AusgabeWunsch;
		if (n < 0)
		{
			PipeFehler = 2;
			return;
		}
		if (n > ListenAnzahl)
		{
			n = ListenAnzahl;
		}
	}

	struct Node *Hilfszeiger = Anfang;
	unsigned int Zaehler = ListenAnzahl - n;				//Ist verantwortlich fuer die ID in der History
	FILE *HistoryFile = fopen(".hhush.histfile", "wb");		//oeffnet (und erstellt gegebenenfalls) die History-Datei
	if ((HistoryFile == NULL) && (Sichern == 1))
	{
		FehlerFunktion("History-Datei konnte nicht zum schreiben geoeffnet werden");
		return;
	}

	int k = 1;
	for (k = 1; k < n; k++)						//Gehe n Tief in die Liste rein
	{
		Hilfszeiger = Hilfszeiger->Next;
	}

	for (; n > 0; n--)							//Gehe die Liste rueckwaerts bis zum Start durch
	{
		if (Sichern == 0)
		{
			char ZwischenSpeicher[INPUT_SIZE_MAX + 11];	
			/* falls Befehle maximaler Leange ausgegeben werden muessen, die Auch noch 10-stellige Numerierungen aufweisen 
			(der Wertebereich von unsigned Integer unter 32-Bit Linux ist 4.294.967.295 , also 10-stellig.)*/
			sprintf(ZwischenSpeicher, "%d %s", Zaehler, Hilfszeiger->Eingabe);
			AppendPipeBuffer(ZwischenSpeicher);
		}
		else if (Sichern == 1)
		{
			fputs(Hilfszeiger->Eingabe, HistoryFile);
		}
		Hilfszeiger = Hilfszeiger->Prev;
		Zaehler++;
	}

	fclose(HistoryFile);
	return;
}

/*************************************************
*******Hier beginnt das Stufe-0-Input-Modul ******
*************************************************/

int InputStufe_0(char Input[INPUT_SIZE_MAX])	//gibt 0 zurueck, falls die Eingabe nicht "leer" ist
{
	int Zaehler;
	for (Zaehler = 0; Input[Zaehler] != '\n'; Zaehler++)
	{
		if ((Input[Zaehler] != '\t') && (Input[Zaehler] != ' '))
		{
			return 0;
		}

		if (Zaehler > INPUT_SIZE_MAX)
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

int InputStufe_1Fehler;							//Wird verwendet um bei Fehlern in Stufe 1 weitere Stufen abzubrechen

struct StackElement
{
	char InputText[INPUT_SIZE_MAX];
	struct StackElement *Next;
};

struct StackElement *AnfangStack = NULL;
int StackTiefe;

void Push(char Input[INPUT_SIZE_MAX])
{
	struct StackElement *NeuerNode = malloc(sizeof(struct StackElement));		//Speicher reservieren fuer das neue Stack Element

	if (NeuerNode == NULL)						//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher fuer das StackElement reserviert werden.");
		return;
	}

	if (AnfangStack == NULL)					//Falls die Liste leer isi
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
	struct  StackElement Rueckgabe;				//Wir kopieren das oberste Stackelement in Rueckgabe

	if (AnfangStack == NULL)					//Falls der Stack leer ist
	{
		FehlerFunktion("Input_1 Stack ist leer");
		Rueckgabe.Next = NULL;
	}
	else
	{
		memcpy(Rueckgabe.InputText, AnfangStack->InputText, INPUT_SIZE_MAX * sizeof(char));
		Rueckgabe.Next = NULL;

		struct StackElement *HilfsZeiger = AnfangStack->Next;	//Wir geben den Speicherplatz fuer das oberste Stackelement frei
		free(AnfangStack);
		AnfangStack = HilfsZeiger;

		StackTiefe--;
	}

	return Rueckgabe;
}

void InputStufe_1(char Input[INPUT_SIZE_MAX])
{
	InputStufe_1Fehler = 0;
	int InputLaenge;							//Speichert wie lang die Nutzereingabe ist, indem es angiebt Input[InputLaenge]='\n'

	for (InputLaenge = 0; Input[InputLaenge] != '\n'; InputLaenge++)
	{
		if (InputLaenge > INPUT_SIZE_MAX)		//Schaut ob die Eingabe zu lang ist
		{
			FehlerFunktion("Eingabe zu lang");
			InputStufe_1Fehler = 1;
			return;
		}
	}

	int Zaehler;								//Zaehlt an welcher Stelle im String ein Pipe Symbol ist, von hinten an
	for (Zaehler = InputLaenge; Input[Zaehler] != '|'; Zaehler--)
	{
		if (Zaehler < 0)
		{
			break;
		}
	}

	if (Zaehler < 0)							//Wenn kein (weiteres) Pipe Symbol gefunden worden ist
	{
		Push(Input);
		return;
	}
	else
	{
		char EinzelInput[INPUT_SIZE_MAX];		//Hier wird das Kommando hinter dem letzen Pipe Symbol ('|') gespeichert
		int RestInputZaehler = 0;				//Fuer die Numerierung des RestInputs
		int HilfsZaehler;

		for (HilfsZaehler = Zaehler + 1; HilfsZaehler <= InputLaenge; HilfsZaehler++)		//Kopiert das Kommando hinter dem letzen Pipe Symbol in EinzelInput
		{
			EinzelInput[RestInputZaehler] = Input[HilfsZaehler];
			RestInputZaehler++;
		}
		EinzelInput[RestInputZaehler] = '\0';	//Beendet den String des neusten Kommandos
		Input[Zaehler] = '\n';					//Das ehemals letze Pipes Symbol wird zu einer Newline, somit kuerzen wir den String um den letzen Befehl
		Input[Zaehler + 1] = '\0';				//Den String beenden wir noch mit \0
		Push(EinzelInput);
		InputStufe_1(Input);
		return;
	}
}

/*************************************************
*******Hier beginnt das Stufe-2-Input-Modul ******
*************************************************/

void InputStufe_2(void)							//iteriert ueber dem Stufe-1-Stack und ersetzt alle Tabs durch Leerzeichen
{
	struct StackElement *Hilfspointer = AnfangStack;
	int Zaehler;
	while (Hilfspointer != NULL)				//diese While Konstuktion iteriert ueber dem Stufe-1-Stack
	{
		for (Zaehler = 0; Hilfspointer->InputText[Zaehler] != '\n'; Zaehler++)		//diese for-Schleife laeuft ueber den Nutzerinput
		{
			if (Hilfspointer->InputText[Zaehler] == '\t')
			{
				Hilfspointer->InputText[Zaehler] = ' ';
			}
		}
		Hilfspointer = Hilfspointer->Next;
	}
	return;
}

/*************************************************
*******Hier beginnt das Stufe-3-Input-Modul ******
*************************************************/

void InputStufe_3(void)							//iteriert ueber dem Stufe-1-Stack und minimiert die Leerzeichen maximal
{
	struct StackElement *Hilfspointer = AnfangStack;
	int Zaehler;								//wird als Zaehler fuer das Char Array verwendet, dass die bereinigte Nutzereingabe enthaelt
	while (Hilfspointer != NULL)				//diese While Konstuktion iteriert ueber dem Stufe-1-Stack (wie bereits in Stufe-2)
	{
		if (Hilfspointer->InputText[0] == ' ')	//Wenn die Eingabe mit einem Leerzeichen beginnt
		{
			for (Zaehler = 0; Hilfspointer->InputText[Zaehler] != '\n'; Zaehler++)
			{
				if (Zaehler == INPUT_SIZE_MAX - 1)				//Ausnahmesituation bei sehr langen Eingaben (kann dies ueberhaupt eintreten?)
				{
					Hilfspointer->InputText[Zaehler] = '\0';
					break;
				}
				Hilfspointer->InputText[Zaehler] = Hilfspointer->InputText[Zaehler + 1];	//verschiebe die komplette Eingabe "eins nach vorne" (auf Pos. 0 war ein Leerzeichen)
			}
			continue;							//Starte von Vorne, falls auch auf Position 1 ein Leerzeichen war
		}

		for (Zaehler = 0; Hilfspointer->InputText[Zaehler] != '\n'; Zaehler++)	//hier wird ueber den Nutzerinput gelaufen, desweiteren ist das erste Zeichen kein Leerzeichen
		{
			/* Wir wollen wenn mehrere Leerzeichen hintereinander stehen diese auf ein einzelnes Leerzeichen als Trenner von Eingaben reduzieren.
			Dazu schauen wir, ob hinter einem Leerzeichen ein weiteres steht und falls dies der Fall ist, verschieben wir alles (mitsammt des zweiten Leerzeichens)
			um eins nach vorne und ueberschreiben so das erste mit dem Zweiten Leerzeichen. Wenn wir dies gemacht haben, wiederholen wir diesen Schritt auf dieser
			Position (deshalb Zaehler--; ), bis dort keine zwei Leerzeichen mehr hintereinander stehen. */
			if (Hilfspointer->InputText[Zaehler] == ' ' && Hilfspointer->InputText[Zaehler + 1] == ' ')
			{
				int HilfsZaehler = Zaehler;
				for (; Hilfspointer->InputText[HilfsZaehler] != '\n'; HilfsZaehler++)
				{
					if (HilfsZaehler == INPUT_SIZE_MAX - 1)				//Ausnahmesituation bei sehr langen Eingaben
					{
						Hilfspointer->InputText[HilfsZaehler] = '\0';
						break;
					}
					Hilfspointer->InputText[HilfsZaehler] = Hilfspointer->InputText[HilfsZaehler + 1];		//verschiebe alles "eins nach vorne"
				}
				Zaehler--;
			}
		}

		if (Hilfspointer->InputText[Zaehler - 1] == ' ')	//falls direkt vor dem \n noch ein Leerzeichen ist
		{
			Hilfspointer->InputText[Zaehler - 1] = '\n';
			Hilfspointer->InputText[Zaehler] = '\0';
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
int BefehlAnzahl;								//Speichert wie viele Argumente uebergeben wurden (der Befehl mitgezaehlt; gut zum Vergleich, ob falsche menge an Argumenten eingegeben wurde)

/* Die Folgende Funktion InputStufe_4 ist eingentlich nur eine abgespeckte Kopie von InputStufe_1 (und NeuerBefehl() ist eigentlich Push(),
bzw. Pop() ist GetBefehl()), nur dass hier nicht bei Pipe Symbolen '|' sondern be Leerzeichen getrennt wird. Es spart mir nur sehr viel Arbeit,
ein zweite Funktion zu schreiben, die quasi dasselbe tut, nur mit einem anderen Char. */

void NeuerBefehl(char Input[INPUT_SIZE_MAX])
{
	struct BefehlsListe *NeuerBefehl = malloc(sizeof(struct BefehlsListe));		//Speicher reservieren fuer das neuen Befehl

	if (NeuerBefehl == NULL)					//Falls kein Speicher freigegeben werden konnte
	{
		FehlerFunktion("Es konnte kein Speicher fuer den Befehl reserviert werden.");
		return;
	}

	if (BefehlsListenAnfang == NULL)			//Falls die Liste leer isi
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

void InputStufe_4(char *Input)
{
	int InputLaenge;							//Speichert wie lang die Nutzereingabe ist, indem es angiebt Input[InputLaenge]='\n'

	for (InputLaenge = 0; Input[InputLaenge] != '\n'; InputLaenge++)
	{
												//Zaehlt wie lange die Nutzereingabe ist
	}

	int Zaehler;								//Zaehlt an welcher Stelle im String ein Leerzeichen ist, von hinten an
	for (Zaehler = InputLaenge; Input[Zaehler] != ' '; Zaehler--)
	{
		if (Zaehler == 0)
		{
			break;
		}
	}

	if (Zaehler == 0)							//Wenn kein (weiteres) Leerzeichen gefunden worden ist
	{
		Input[InputLaenge] = '\0';
		NeuerBefehl(Input);
		return;
	}
	else
	{
		char EinzelInput[INPUT_SIZE_MAX];		//Hier wird das Kommando hinter dem letzen Pipe Symbol ('|') gespeichert
		int RestInputZaehler = 0;				//Fuer die Numerierung des RestInputs
		int HilfsZaehler;

		for (HilfsZaehler = Zaehler + 1; HilfsZaehler <= InputLaenge; HilfsZaehler++)		//Kopiert das Kommando hinter dem letzen Pipe Symbol in EinzelInput
		{
			EinzelInput[RestInputZaehler] = Input[HilfsZaehler];
			RestInputZaehler++;
		}
		EinzelInput[RestInputZaehler - 1] = '\0'; //Beendet den String des neusten Kommandos
		Input[Zaehler] = '\0';					//Das ehemalige Leerzeichen wird zu '\0'
		NeuerBefehl(EinzelInput);
		InputStufe_4(Input);
		return;
	}
}

struct BefehlsListe GetBefehl(void)
{
	struct  BefehlsListe Rueckgabe;				//Wir kopieren das oberste Stackelement in Rueckgabe

	if (BefehlsListenAnfang == NULL)			//Falls die Befehlsliste leer ist
	{
		FehlerFunktion("Befehlsliste ist leer");
		Rueckgabe.Next = NULL;
		return Rueckgabe;
	}

	memcpy(Rueckgabe.Befehl, BefehlsListenAnfang->Befehl, INPUT_SIZE_MAX * sizeof(char));
	Rueckgabe.Next = NULL;						//aus Sicherheit


	struct BefehlsListe *HilfsZeiger = BefehlsListenAnfang->Next;	//Wir geben den Speicherplatz fuer das oberste Stackelement frei
	free(BefehlsListenAnfang);
	BefehlsListenAnfang = HilfsZeiger;
	BefehlAnzahl--;
	return Rueckgabe;
}

void BefehlsListeReset(void)					//wir loeschen die komplette Befehlsliste und geben ihren Speicherplatz wieder frei (bei ungueltigen Funktionsaufrufen eingesetzt)
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
*******Hier beginnt das Exit-Programm ************
*************************************************/

int ExitVariable = 1;							//wird auf 0 gesetzt, wenn das Programm verlassen werden soll

void ExitProgramm(void)
{
	if (BefehlAnzahl == 0 && GlobalPipeBufferPointer == NULL)	//darf nur ohne Argumente verwendet werden
	{
		ExitVariable = 0;
		return;
	}
	else
	{
		PipeFehler = 2;
		return;
	}
}

/*************************************************
*******Hier beginnt das Date-Programm ************
*************************************************/

void DateProgramm(void)
{
	if (BefehlAnzahl == 0 && GlobalPipeBufferPointer == NULL)	//darf nur ohne Argumente verwendet werden
	{
		time_t Zeit;							//wir legen eine Zeit-Stuktur an
		time(&Zeit);							//wir speichern dort die aktuelle Zeit
		WritePipeBuffer(ctime(&Zeit));			//die Funktion ctime(time_t *time) gibt die Zeit als String aus, der (ganz Zufaellig) wie im Beispiel der Aufgabe formatiert ist
		return;
	}
	else
	{
		PipeFehler = 2;
		return;
	}
}

/*************************************************
*******Hier beginnt das Echo-Programm ************
*************************************************/

/* Die Aufgabenstellung fordert nicht, dass man die Ausgabe beliebiger Programme als eingabe fuer echo implementieren soll.
Deshalb habe ich es auch nicht getan, und wenn ein gepipetes Programm eine Ausgabe macht, wird ein darauffolgender echo-Befehl
immer fehlschlagen. */

void EchoProgramm(void)
{
	if (GlobalPipeBufferPointer == NULL)		//schaut, dass kein vorheriges Programm eine Ausgabe machte
	{
		if (BefehlAnzahl == 0)					//falls keine Argumente uebergeben worden sind
		{
			WritePipeBuffer("\n\0");
			return;
		}

		char ZwischenSpeicher[INPUT_SIZE_MAX];	//da eine Eingabe maximal 256 Zeichen lang ist, wird die Ausgabe des Echo Befehls nie laenger als dies sein (eig. noch -5 Zeichen fuer "echo ")
		unsigned int ZaehlerEins = 0;			//wird zum Zeahlen den Zwischenspeichers verwendet (unsigned, da strlen auch unsigned int verwendet)
		int ZaehlerZwei = 0;					//wird auch dazu verwendet
		while (BefehlAnzahl > 0)
		{
			ZaehlerEins = 0;
			struct BefehlsListe NeuesArgument = GetBefehl();
			while (ZaehlerEins < strlen(NeuesArgument.Befehl))
			{
				ZwischenSpeicher[ZaehlerEins+ZaehlerZwei] = NeuesArgument.Befehl[ZaehlerEins];
				ZaehlerEins++;
			}
			ZaehlerZwei = ZaehlerEins + ZaehlerZwei;
			ZwischenSpeicher[ZaehlerZwei] = ' ';
			ZaehlerZwei++;
		}
		ZwischenSpeicher[ZaehlerZwei - 1] = '\n';	//wir (er)setzen nach dem letzen Argument (das Leerzeichen durch) Newline
		ZwischenSpeicher[ZaehlerZwei] = '\0';		//und schliessen dann den String ab
		WritePipeBuffer(ZwischenSpeicher);
		return;
	}
	else
	{
		PipeFehler = 2;
		return;
	}
}

/*************************************************
*******Hier beginnt das History-Programm *********
*************************************************/

void HistoryProgramm(void)
{
	if (BefehlAnzahl > 1)						//es darf maximal 1 Argument uebergeben werden
	{
		PipeFehler = 2;
		return;
	}
	else if (BefehlAnzahl == 1)
	{
		struct BefehlsListe NeuesArgument = GetBefehl();
		if (strcmp(NeuesArgument.Befehl, "-c") == 0)		//schaut ob die history geloescht werden sollte
		{
			ListeLoeschen();
			return;
		}
		int n = atoi(NeuesArgument.Befehl);		//wandelt den String in eine Ganzzahl um
		if (n == 0)								//atoi gibt 0 zurueck, wenn die umwandlung fehlschlaegt (desweiteren macht die Ausgabe von 0 History Elementen keinen Sinn)
		{
			PipeFehler = 2;
			return;
		}
		History(0, 1, n);
		return;
	}
	else
	{
		History(0, 0, 0);
		return;
	}
}

/*************************************************
*******Hier beginnt das LS-Programm **************
*************************************************/

void LSProgramm(void)
{
	if (GlobalPipeBufferPointer != NULL || BefehlAnzahl != 0)	//schaut ob die Aufrugparameter in Ordnung sind
	{
		PipeFehler = 2;
		return;
	}
	char *PathName;								//wird zur Speicherung des Pathnamen verwendet
	#ifdef WIN32
	PathName = _getcwd(NULL, 0);				//holt sich den Pathname unter Windows
	#else
	PathName = getcwd(NULL, 0);					//holt sich den Pathname unter Linux
	#endif
	if (PathName == NULL)
	{
		FehlerFunktion("Konnte keinen Speicher fuer das Verzeichniss im LS-Programm reservieren");
		return;
	}

	DIR *Verzeichniss;							//Verzeichnisspointer fuer das auslesen des aktuellen Verzeichnisses
	Verzeichniss = opendir(PathName);			//oeffnen des aktuellen Verzeichnisses
	free(PathName);								//giebt den Speicher fuer den Verzeichnissnamen wieder frei
	if (Verzeichniss == NULL)
	{
		FehlerFunktion("Fehler beim oeffnen des Verzeichnisses");
		return;
	}

	struct dirent *VerzeichnissZeiger;	//Zeiger der Element fuer Element das Verzeichniss ausliest
	while ((VerzeichnissZeiger = readdir(Verzeichniss)) != NULL)	//wir gehen alle Elemente im Verzeichniss durch
	{
		int DateiNamenLaenge;					//holt die Laenge des auszugebenden Dateinamens
		#ifdef WIN32
		DateiNamenLaenge = VerzeichnissZeiger->d_namlen;	
		#else
		DateiNamenLaenge = VerzeichnissZeiger->d_reclen;
		#endif
		char *Zwischenspeicher = malloc((DateiNamenLaenge + 2)*sizeof(char));	//holt sich Speicher fuer Dateinamen+'\n'+'\0'
		if (Zwischenspeicher == NULL)
		{
			FehlerFunktion("Konnte keinen Zwischenspeicher fuer den Dateinamen reservieren");
			break;
		}

		sprintf(Zwischenspeicher, "%s\n", VerzeichnissZeiger->d_name);	//wir haengen eine Newline an den Dateinamen
		if (Zwischenspeicher[0] != '.')									//und schauen dann, dass die Datei nicht mit einem Punkt beginnt
		{
			AppendPipeBuffer(Zwischenspeicher);							//bevor wir ihn an die "Ausgabe" weiterreichen
		}
		free(Zwischenspeicher);											//und dann unseren Zwischenspeicher wieder freigeben
	}
	closedir(Verzeichniss);						//schliesst das Verzeichniss wieder
	return;
}

/*************************************************
*******Hier beginnt das CD-Programm **************
*************************************************/

void CDProgramm(void)
{
	if (GlobalPipeBufferPointer != NULL || BefehlAnzahl != 1)	//schaut ob die Aufrugparameter in Ordnung sind
	{
		PipeFehler = 2;
		return;
	}
	char Kommando[INPUT_SIZE_MAX];
	memcpy(Kommando, GetBefehl().Befehl, INPUT_SIZE_MAX*sizeof(char)); //holt sich das uebergebene Argument in "Kommando"

	int Erfolg;									//speichert ob das Verzeichniss existiert
	#ifdef WIN32								//damit das Programm auch unter Windows funktioniert
	Erfolg = _chdir(Kommando);
	#else
	Erfolg = chdir(Kommando);
	#endif
	
	if (Erfolg == -1)							//gibt bei einem Fehler in chdir diese Ausgabe aus
	{
		WritePipeBuffer("no such file or directory\n");
	}
	return;
}

/*************************************************
*******Hier beginnt das Grep-Programm ************
*************************************************/

/* Die folgende Funktion "klaut" den PipeBuffer vom GlobalPipeBufferPointer.
So koennen die bereits bekannten PipeBuffer-Funktionen verwendet werden, waehrend 
der fuers Piping eig. im PipeBuffer noch  zwischengespeicherte Inhalt fuer das Grep
Programm verwendet wird.
Wir wollen jede gefundene Zeile (mit uebereinstimmung) sofort in den PipeBuffer
schreiben, dass geht nur, wenn dort nicht noch die vorherige Ausgabe gespeichert ist.
Mit dieser Funktion wird dies moeglich. */

char *PipeBufferGrepCopy(void)
{
	char *ReturnPointer = GlobalPipeBufferPointer;
	GlobalPipeBufferPointer = NULL;
	return ReturnPointer;
}

/* Es folgt quasi derselbe Code wie des PipeBuffers,
da wir immer die aktuell zu betrachtende Zeile in einen Buffer kopieren,
um auf ihr dann nach den Grep Argumenten zu suchen. Der Code des PipeBuffers
ist darauf ausgelegt, einen beliebigen String dynamisch zu speichern, genau
das, was wir hier tun wollen. */

char *GrepBufferPointer = NULL;

void WriteGrepBuffer(char *Input)				//schreibt den Input in einen dynamisch zugewiesenen Speicherbereich und gibt einen Pointer darauf zurueck
{
	if (GrepBufferPointer != NULL)
	{
		FehlerFunktion("GrepBuffer ist noch nicht leer");
		return;
	}
	int InputLeange = strlen(Input);			//schaut wie lang der Input ist
	char *ReturnPointer = malloc((InputLeange + 1)* sizeof(char));		//reserviert Speicher fuer die Eingabe + '\0'
	if (ReturnPointer == NULL)
	{
		FehlerFunktion("Es konnte kein Speicher fuer den GrepBuffer zugewiesen werden");
		return;
	}
	memcpy(ReturnPointer, Input, (InputLeange + 1)* sizeof(char));
	GrepBufferPointer = ReturnPointer;
	return;
}

void WipeGrepBuffer(void)						//loescht den GrepBufferPointer
{
	if (GrepBufferPointer == NULL)
	{
		FehlerFunktion("GrepBuffer ist schon leer");
		return;
	}
	free(GrepBufferPointer);
	GrepBufferPointer = NULL;
	return;
}

int PipeCopyNewLineInGrepBuffer(char *PipeAusgabe)	//kopiert die neuste Ausgabezeile in den GrepBuffer; wenn am Ende angekommen =0
{
	if (PipeAusgabe[0] == '\0')					//schaut ob die Eingabe leer ist
	{
		return 0;
	}
	if (PipeAusgabe[0] == '\n')					//behandelt den Sonderfall einer Freizeile
	{
		int NeuZeilenZaehler;
		int EingabeLaenge = strlen(PipeAusgabe);
		for (NeuZeilenZaehler = 0; NeuZeilenZaehler < EingabeLaenge - 1; NeuZeilenZaehler++)
		{
			PipeAusgabe[NeuZeilenZaehler] = PipeAusgabe[NeuZeilenZaehler + 1];
		}
		PipeAusgabe[EingabeLaenge-1] = '\0';
		return 1;
	}

	unsigned int ErsteNewline;
	for (ErsteNewline= 0; PipeAusgabe[ErsteNewline] != '\n'; ErsteNewline++)
	{
		//zeahlt an welcher Stelle das erste Newline ist
		if (PipeAusgabe[ErsteNewline] == '\0')
		{
			break;
		}
	}

	char *NeusteZeile = malloc((ErsteNewline + 2) * sizeof(char));		//reserviert Speicher fuer die neuste Zeile + '\0' (+2 da ErsteNewline bei 0 beginnt)
	if (NeusteZeile == NULL)
	{
		FehlerFunktion("konnte keinen Speicher in PipeCopyGrep reservieren");
		return 0;
	}

	memcpy(NeusteZeile, PipeAusgabe, (ErsteNewline+1)*sizeof(char));	//kopiert erste Zeile in NeusteZeile
	NeusteZeile[ErsteNewline+1] = '\0';

	//nun ueberschreiben wir die neuste Zeile mit der Zeile dannach

	unsigned int Zeahler;
	unsigned int InputLaenge = strlen(PipeAusgabe);

	for (Zeahler = 0; Zeahler <= InputLaenge-ErsteNewline ; Zeahler++)
	{
		PipeAusgabe[Zeahler] = PipeAusgabe[Zeahler + ErsteNewline];
	}
	PipeAusgabe[Zeahler] = '\0';

	WriteGrepBuffer(NeusteZeile);	//schreibe die NeusteZeile in den GrepBuffer
	free(NeusteZeile);				//gebe den Speicherplatz fuer die NeusteZeile wieder frei
	return 1;
}

void SucheInZeile(char *Suchstring)	//sucht nach Suchstring auf dem GrepBuffer
{
	if (GrepBufferPointer == NULL)	//falls nichts zum durchsuchen da ist (tritt bei jeder Freizeile auf)
	{
		return;
	}
	if (strstr(GrepBufferPointer, Suchstring) != NULL)
	{
		AppendPipeBuffer(GrepBufferPointer);
	}
	WipeGrepBuffer();
	return;
}

void GrepProgramm(void)
{
	if (BefehlAnzahl == 0  || BefehlAnzahl > 2)	//schaut ob Syntax eingehalten wurde
	{
		PipeFehler = 2;
		return;
	}

	char *SuchString = GetBefehl().Befehl;		//holt sich den Suchstring

	if (GlobalPipeBufferPointer != NULL)		//falls gepipet wurde
	{
		if (BefehlAnzahl != 0)					//schaut ob Syntax verletzt wurde (es darf keine Datei angegeben werden)
		{
			PipeFehler = 2;
			return;
		}
		char *PipeAusgabenPointer = PipeBufferGrepCopy();	//hier liegt die Ausgabe des voherigen Programms

		while (PipeCopyNewLineInGrepBuffer(PipeAusgabenPointer))
		{
			SucheInZeile(SuchString);
		}
		free(PipeAusgabenPointer);
	}
	else										//falls nicht gepipet wurde
	{
		if (BefehlAnzahl == 0)					//falls keine Datei angegeben wurde
		{
			PipeFehler = 2;
			return;
		}
		char *DateiString = GetBefehl().Befehl;
		FILE *FilePointer = fopen(DateiString, "r");	//oeffnet die Datei im lese modus
		if (FilePointer == NULL)						//falls die Datei nich geoeffnet werden konnte
		{
			WritePipeBuffer("no such file or directory\n");
			return;
		}

		/* Hier stand ich vor der Wahl: Entweder verwende ich:
		ssize_t getline (char **lineptr, size_t *n, FILE *stream)
		eine nicht ANSI-C, allerdings unter gcc verf�gbare Funktion, welche ich aber nicht gut unter Windows Debuggen koennte,
		oder ich benutze ein aufwaendigeres System, dass dynamisch Speicherplatz fuer die Zeile reserviert. Ich habe mich fuer
		letzteres geschrieben, weil ich weiterhin unter Visual Studio programmieren wollte. Fuer meine Implementierung habe
		ich Ideen aus http://www.computerbase.de/forum/showthread.php?t=931844 verwendet. Meine Implementierung ist eine leicht angepasste
		Version von "c0mp4ct"s Codebeispiel vom 31.07.2011, 12:22. Die Anpassungen von mir lesen Zeile fuer Zeile aus, statt nur die erste Zeile.
		Desweiteren stammen alle Kommentare von mir.
		Theoretisch k�nnte ich auch die komplette Datei in den Hauptspeicher kopieren und dann so handhaben, wie oben wenn gepipet wurde,
		d.h. ich koennte auch einfach PipeCopyNewLineInGrepBuffer(char *Pointer) usw. verwenden. Dies wollte ich aber nicht tun, da beliebig
		grosse Dateien verarbeitet werden koennen sollen, aber der Addressierbare Hauptspeicher begrenzt ist. (Und wer moechte schon fuer eine
		Datei 3GB Speicher reservieren) */

		int Schleifenabbruch = 1;		//wird fuer den Abbruch bei EOF verwendet
		while (Schleifenabbruch)		//diese Schleife tut die Datei Zeilenweise einlesen
		{
			unsigned int BlockLaenge = 128;		//Blockgroesse in denen zusaetzlicher Speicherplatz reserviert wird
			unsigned int AktuelleGroesse = 0;	//Wie gross der aktuell reservierte Speicher ist
			unsigned int Zaehler = 0;			//Zaehlt das Char-Array durch
			int Zeichen = EOF;					//Hier wird das aktuell eingelesene Zeichen zwischengespeichert
			char *StringSpeicher = malloc(BlockLaenge * sizeof(char));		//Wir reservieren einen Speicherblock

			if (StringSpeicher == NULL)			//Wir schauen ob wir den Speicherplatz ueberhaupt reservieren konnten
			{
				FehlerFunktion("Konnte keinen Speicher fuer das lesen aus Datei reservieren (A)");
				break;
			}

			AktuelleGroesse = BlockLaenge;		//Am Anfang haben wir Speicher fuer eine Blocklaenge reserviert

			while ((Zeichen = fgetc(FilePointer)) != '\n')	//Wir lesen die Datei Zeichen fuer Zeichen, bis eine Newline kommt
			{
				if (Zeichen == EOF)							//Falls wir am Ende der Datei sind, brechen wir beide Schleifen ab
				{
					Schleifenabbruch = 0;
					break;
				}


				StringSpeicher[Zaehler] = (char)Zeichen;	//Wir kopieren das eingelesene Zeichen in unseren Speicherbereich
				Zaehler++;
				if (Zaehler == AktuelleGroesse)		//Falls wir am Ende unseres Speicherblocks sind, vergroessern wir ihn
				{
					AktuelleGroesse = Zaehler + BlockLaenge;
					if ((StringSpeicher = realloc(StringSpeicher, AktuelleGroesse * sizeof(char))) == NULL)
					{
						FehlerFunktion("Konnte keinen Speicher fuer das lesen aus Datei reservieren (B)");
						fclose(FilePointer);
						return;
					}
				}
			}
			StringSpeicher[Zaehler] = '\n';		//Wir fuegen eine Newline ein
			StringSpeicher[Zaehler + 1] = '\0';	//und Terminieren dann den String noch (Beachte Zaehler+1 <= AktuelleGroesse, daher ueberschreiben wir nicht die Grenzen)
			WriteGrepBuffer(StringSpeicher);	//dannach schreiben wir diesen String in den Grep-Buffer
			free(StringSpeicher);				//und geben den Speicherplatz wieder frei
			StringSpeicher = NULL;				//nur zur Sicherheit
			SucheInZeile(SuchString);			//dann fuehren wir unsere Suche aus
		}
		fclose(FilePointer);				//macht den Dateistream wieder zu
	}
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
	case 0: ExitProgramm(); break;
	case 1: DateProgramm(); break;
	case 2: HistoryProgramm(); break;
	case 3: EchoProgramm(); break;
	case 4: LSProgramm(); break;
	case 5: CDProgramm(); break;
	case 6: GrepProgramm(); break;
	default: PipeFehler = 1; break;
	}
}

/*************************************************
*******Hier beginnt das Haupt-Programm  **********
*************************************************/

int main(void)
{
	/*******************************************
	*******Hier wird die History importiert******
	********************************************/

	FILE *HistoryFile = fopen(".hhush.histfile", "rb");	//oeffnet die History-Datei
	if (HistoryFile != NULL)	//Falls die History-Datei nicht geoeffnet werden konnte (im Regelfall also noch nicht erstellt wurde), wird nichts importiert
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
	static char Input[INPUT_SIZE_MAX];		//wird zur Speicherung der Nutzereingabe verwendet
	//Anmerkung: DIESES VERDAMMTE KLEINE "STATIC" HAT MIR SO VERDAMMT VIEL AERGER BEREITET! ICH VERFLUCHE ES!
	char *PathName;				//wird zur Speicherung des Pathnamen verwendet
	
	while (ExitVariable)
	{
		#ifdef WIN32
		PathName = _getcwd(NULL, 0);	//holt sich den Pathname
		#else
		PathName = getcwd(NULL, 0);	//holt sich den Pathname
		#endif
		
		if (PathName == NULL)
		{
			FehlerFunktion("Pathname Speicherreservierung funktionierte nicht");
			printf("Unbekannt $ ");		//gibt den Prompt aus
		}
		else
		{
			printf("%s $ ", PathName);		//gibt den Prompt aus
		}
		
		free(PathName);		//gibt den Speicherplatz der fuer den String des Pathnames reserviert wurde wider frei

		fgets(Input, sizeof(Input), stdin);
		if (InputStufe_0(Input))	//falls nur Leerzeichen/Tabs eingegeben wurden, wird neu angefangen
		{
			continue;
		}
		NeuesElement(Input);		//speichert die Eingabe in der History
		InputStufe_1(Input);
		if (InputStufe_1Fehler == 1)
		{
			FehlerFunktion("Fehler in der Input Stufe 1");
			continue;
		}
		InputStufe_2();				//ersetzt alle Tabs in der Eingabe durch Leerzeichen
		InputStufe_3();				//loescht alle Leerzeichen, die keine Eingabe trennen

		while (StackTiefe > 0)		//diese Schleife fuehrt hintereinander die durch '|' getrennten Befehle aus
		{
			InputStufe_4(Pop().InputText);
			FunktionsAufrufer();

			//ab hier beginnt Fehlerbehandlung

			if (PipeFehler != 0)	//die Pipe wird nicht weiter ausgefuehrt, wenn ein Unterprogramm fehlschlaegt
			{
				while (StackTiefe > 0)	//loesche den Stack mit den weiteren gepipten Befehlen
				{
					Pop();
				}
				BefehlsListeReset();	//loesche die Befehlsliste
				if (GlobalPipeBufferPointer != NULL)		//falls nicht leer, wird der PipeBuffer geloescht
				{
					WipePipeBuffer();
				}
				if (GrepBufferPointer != NULL)				//falls nicht leer, wird der GrepBuffer geloescht
				{
					WipeGrepBuffer();
				}
				if (PipeFehler == 1)
				{
					ExitVariable = 1;						//Bsp.: "exit | daate" wuerde immernoch exit ausfuehren, wenn diese Line nicht waere
					WritePipeBuffer("command not found\n");
				}
				if (PipeFehler == 2)
				{
					WritePipeBuffer("invalid arguments\n");
				}
				PipeFehler = 0;
				break;
			}
		}
		
		if (GlobalPipeBufferPointer != NULL)	//gibt den PipeBuffer aus, nachdem alle Kommandos durchlaufen worden sind
		{
			printf("%s", GlobalPipeBufferPointer);
			WipePipeBuffer();
		}
	}

	History(1, 1, 1000);			//Sichert die neusten 1000 Elemente der History in der Datei
	ListeLoeschen();				//Um Speicherplatz freizugeben
	return 0;
}