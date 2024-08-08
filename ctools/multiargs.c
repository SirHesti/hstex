/**
 @file multiargs.c
 @brief CMD-Variablen aehnlich getopt einlesen
*//**
 @addtogroup c_multi_args
 @{
 @brief Mehrfache Argumente verwalten leicht gemacht
*/
#include "tools.h"
#include "ctools/multiargs.h"
/**
 @brief  CMD Zeilenparameter in Variablen schreiben
 @return Bei Fehler >0 ( unklare Situation, empfohlen Programmabbruch)
         0 wenn keine Fehler aufgetreten sind
 @param argc von int main (int argc, char **argv)
 @param argv von int main (int argc, char **argv)
 @param fmt durch '-','%' getrennte Befehlsräume, Beschreibungen @ref GetCMDLineParameter_fmt "mehr..."
 @param ... Variablen, in die die Argumente der Befehlzeile gespeichert werden.

 @anchor GetCMDLineParameter_fmt
  Detailierte fmt Beschreibung

 - -d Parameter d auswerten gefolgt von
 - \%b bool (TRUE oder FALSE)   dflt:FALSE
 - \%i Integer                  dflt:0
 - \%s String                   dflt:Leerer String (NICHT NULL)
 - \%c char                     dflt:0

 Wenn \%Parameter UpCase B,I,S,C angegeben werden, dann wird der dflt-Wert @b NICHT gesetzt.

 Leerzeichen in fmt dienen dem Programmierer zur übersichtlichkeit. Sind aber der Funktion egal.

 Hier soll es dem Programmierer einfach gemacht werden Parameter an main
 in Variablen zu speichern, bzw. auch sicher zu sein, das diese sonst einen
 klaren Wert aufweisen. Jeder Parameter wird durch ein durch 2 Zeichen
 beschrieben.

 - 1. Parameter mit vorgehenden Minus wie -d
 - 2. Typ der Variablen

 Beides zusammen ergeben die Beschreibung der Variablen, die als Parameter
 hinter dem langen String angegeben wird.

 Parameter die nicht von der Kommandozeile bedient werden enthalten
 trotzdem den Dflt-Wert, es sei denn Sie sind als nichtbesetzend definiert.

 @code
  Beispiel:
    int debug;
    char InputName[128];
    rslt=GetCMDLineParameter(argc, argv, "-d %b -i %s", &debug, &InputName);
    PROMPT> cmdname -d -i Heinzelmann
    wird dann debug auf 1=TRUE (weil angegeben) setzen InputName enthält die
    Heinzelmann.
 @endcode

___[ Revision ]______________________________________________________________

 ** 10.03.2018 HS Create
 ** 11.03.2018 HS alles ab "switch (par)" neu, war mir nicht deutlich genug
                  in der Ausführung
___________________________________________________________________________*/

int GetCMDLineParameter(int argc, char *argv[], char *fmt,...)
{
    int Parameter;                      // Aktuell bearbeiteter Paramete von fmt
    int max_par;                        // Anzahl der in fmt bedienten Parametern
    char *tmp;                          // zum Überprüfen von fmt
    char chk_par[3];                    // zum Überprüfen der Kommandoparameter via ChkARG()
    int rc;                             // Abschlusskennzeichnung
    va_list argptr;                     // variablelist ...

    // cppcheck-suppress variableScope
    int rslt;                           // Ergebnis von ChkARG
    int *par_bool;                      // wenn BOOL-Parameter
    int *par_int;                       // wenn INT-Parameter
    char *par_char;                     // wenn CHAR Parameter
    char *par_string;                   // wenn CHAR* Parameter

    char par;                           // Zu verarbeitendes Zeichen aus fmt
    int  nodflt;

    rc = 0;                                                                     // Wir gehen von "alles gut" aus
    max_par = strcount(fmt,'-');                                                // Anzahl lesen
    strcpy (chk_par,"- ");                                                      // Vorbeiten...

    va_start (argptr, fmt);                                                     // Auf erstes Argument positionieren
    for (Parameter=1;Parameter<=max_par;Parameter++)                            // Alles in fmt überprüfen
    {
        tmp = strstrarg(fmt,Parameter,'-');                                     // aus fmt den Variablen Type bestimmen
        if (!tmp)                                                               // Fehler ->
        {
            lprintf ("format error in %s", fmt);
            rc++;
            break;
        }
        chk_par[1]=tmp[0];                                                      // chk_par ist jetzt "-i" tmp= "i %s"
        rslt=ChkARG(chk_par,argc,argv);                                         // ist der Parameter angegeben worden -> rslt

        tmp = strchr(tmp,'%');                                                  // Finde '%'
        if (!tmp)                                                               // wenn nicht Fehler ->
        {
            lprintf ("error argc[%i] in %s",Parameter, fmt);
            rc++;
            break;
        }
        nodflt=isupper(tmp[1]);
        par=tolower(tmp[1]);
        switch (par)                                                            // Was soll zugewiesem werden ?
        {
            case 'b': par_bool=va_arg(argptr,int*);                             // Variabletype BOOL der zu füllen ist
                      if (rslt)                                                 // Bool wurde angegeben
                      {
                          if (ARG)                                              // mit Argument
                          {
                              *par_bool=GetIntTrueFalsefromString(ARG);         // dann Argument nutzen Form von TRUE/FALSE 0/1 on/off
                          }else{
                              *par_bool=TRUE;                                   // ohne Argument auf TRUE setzen
                          }
                      }else{
                          if (nodflt==0) *par_bool=FALSE;                       // default nutzen nach Angabe
                      }
                      break;
            case 'i': par_int=va_arg(argptr,int*);                              // Integer !
                      if (rslt)                                                 // Integer wurde angegeben
                      {
                          if (ARG)                                              // ARG ist ein String der eine Zahl enthalten sollte
                          {
                            *par_int=atoi(ARG);                                 // Angabe aus String nutzen
                          }else{
                            *par_int=0;                                         // oder 0, wenn Parameter ohne Argument
                          }
                      }else{
                          if (nodflt==0) *par_int=0;                            // oder dflt 0
                      }
                      break;
            case 's': par_string=va_arg(argptr,char*);                          // String
                      if (rslt)                                                 // Parameter wurde angegeben
                      {
                          if (ARG)                                              // ARG ist ein String der einen Text enthalten sollte
                          {
                              strcpy(par_string,ARG);                           // ARG muss ein String sein
                          }else{
                              par_string[0]=0;                                  // oder Argument wird es ein LeerString (gelöscht)
                          }
                      }else{
                          if (nodflt==0) par_string[0]=0;                       // oder dflt LeerString
                      }
                      break;
            case 'c': par_char=va_arg(argptr,char*);                            // char
                      if (rslt)                                                 // Parameter wurde angegeben
                      {
                          if (ARG)                                              // ARG ist ein mindestens einzelnes Zeichen
                          {
                              *par_char=ARG[0];                                 // von ARG nur das erste Zeichen setzen
                          }else{
                              *par_char=0;                                      // oder Argument wird 0 Byte
                          }
                      }else{
                          if (nodflt==0) *par_char=0;                           // oder dflt 0 Byte
                      }
                      break;
            default:                                                            // Fehler falls was ganz anderes gefunden wird
                    lprintf ("unknown variable used as '%c' in fmt=%s", tmp[1], fmt);
                    rc++;
        }
    }
    va_end(argptr);                                                             // Ende
    return rc;                                                                  // noch mehr Ende
}

/**
 @brief  Configfile lesen; Zeile raus suchen, die zu cfg_name passt
 @return mit malloc geholter Speicher der auf die gültige Zeile verweist
         NULL, wenn Fehler aufgetreten sind
 @param  cfg_name         zu suchender Eintrag
 @param  cfg_max_args     minimale ";" getrennte Argumente sind vorhanden

 Ist nur für wenige Zeilen gedacht. Für größere Sachen siehe @ref ReadVarsConfigFile hier...

___[ Revision ]______________________________________________________________

 ** 05.03.2018 HS Create
___________________________________________________________________________*/

char *ReadConfigLineFromFile(char *cfg_name,int cfg_max_args)
{
    FILE *cfg_S;                    // Stream
    char *cfg_tmp;                  // zum Überprüfen der Zeile
    char *cfg_rslt;                 // Das Ergebnis, kann NULL sein
    char  cfg_Filename[PATH_MAX];   // der Filename, dieser wird aus HSDIR \ etc \ [PRGNAME] .cfg zusammengesetzt
    char  cfg_Line[STRING_MAX];     // aktuell gelesene Zeile
    int n;

//----------------
static char *cfgDIRs[]={
#ifdef OS_LINUX
    "/etc",
    "/hs/etc",
    "%HSDIR%/etc",
    "%HOME%/.config/%PRGNAME%",
    "%HOME%",
#endif
#ifdef OS_WINDOWS
    "%HSDIR%/etc",
    "%APPDATA%" DIR_SEP "hs",
#endif
    NULL
};

#ifdef HS_DEBUG
    cfg_tmp = RCfind (RConfig,"DBG_CFGDIR");
#else
    cfg_tmp = RCfind (RConfig,"CFGDIR");
#endif
    if (cfg_tmp)
    {
        sprintf_ex (cfg_Filename, "%s" DIR_SEP "%s.cfg", univault(cfg_tmp), m_PRGNAME);
        if (!FileOK(cfg_Filename)) cfg_tmp=NULL;
    }
    if (!cfg_tmp)
    {
        for (n=0;;n++)
        {
            if (!cfgDIRs[n])
            {
                lprintf ("ConfigDIR can't set");                                // Wenn Fehler nicht gefunden
                return NULL;
            }
            sprintf_ex (cfg_Filename, "%s" DIR_SEP "%s.cfg", univault(cfgDIRs[n]), m_PRGNAME);
            if (FileOK(cfg_Filename)) break;
        }
    }

    if(( cfg_S = fopen(cfg_Filename,"rt")) == NULL )                            // File öffnen
    {
        lprintf ("ConfigFile can't read '%s'", cfg_Filename);                   // Wenn Fehler ...
        return NULL;
    }

//---------------    Alter Weg
#ifdef OLD_WAY_TO_SOLVE
#ifdef HS_DEBUG
    cfg_tmp = RCfind (RConfig,"DBG_CFGDIR");
#else
    cfg_tmp = RCfind (RConfig,"CFGDIR");
#endif

    //lprintf ("(1)cfg_tmp=%s", (cfg_tmp) ? cfg_tmp : "isNULL");

#ifdef OS_LINUX
    if (!cfg_tmp) cfg_tmp = "/etc";
#endif
    if (!cfg_tmp) if (HSDIR[0]) cfg_tmp = "%HSDIR%" DIR_SEP "etc";
#ifdef OS_WINDOWS
    if (!cfg_tmp) cfg_tmp = "%APPDATA%" DIR_SEP "hs";
#endif
    //lprintf ("(2)cfg_tmp=%s", (cfg_tmp) ? cfg_tmp : "isNULL");
    if (!cfg_tmp)
    {
        lprintf ("ConfigDIR can't set");                                        // Wenn Fehler ...
        return NULL;
    }
    //lprintf ("(3)cfg_tmp=%s", (cfg_tmp) ? cfg_tmp : "isNULL");
    sprintf_ex (cfg_Filename, "%s" DIR_SEP "%s.cfg", univault(cfg_tmp), m_PRGNAME);
    //lprintf ("(4)univault of cfg_tmp=%s", univault(cfg_tmp));
    //lprintf ("(5)cfg_Filename=%s", cfg_Filename);

    if(( cfg_S = fopen(cfg_Filename,"rt")) == NULL )                            // File öffnen
    {
        lprintf ("ConfigFile can't read '%s'", cfg_Filename);                   // Wenn Fehler ...
        return NULL;
    }
#endif // OLD_WAY_TO_SOLVE
//---------------

    cfg_rslt=NULL;                                                              // Default ist "nicht gefunden"
    for (;;)
    {
        if (mgetline(cfg_S,cfg_Line, sizeof(cfg_Line)-3)!=0) break;             // Zeile einlesen
        CL(cfg_Line);                                                           // bereinigen
        if (cfg_Line[0]==0) continue;                                           // Leerzeilen und
        if (cfg_Line[0]=='#') continue;                                         // Zeilen mit einer Raute sind Kommentarzeilen

        cfg_tmp = strstrarg(cfg_Line,cfg_max_args,';');                         // cfg_max_args Parameter sind zu lesen
        if (!cfg_tmp) continue;                                                 // wenn nicht ->
        cfg_tmp = strstrarg(cfg_Line,1,';');                                    // erster Parameter ist unser Identifier
        if (!cfg_tmp) continue;                                                 // wenn nicht ->
        CL(cfg_tmp);                                                            // bereinigen
        if (strcasecmp(cfg_name,cfg_tmp)) continue;                             // Eintrag gefunden ? wenn nicht ->
        cfg_rslt = strdup(cfg_Line);                                            // string makieren
        break;                                                                  // Ende!!
    }
    fclose (cfg_S);
    return cfg_rslt;
}
//** TOD : sprintf_ex (cfg_Filename, "%s" DIR_SEP "etc" DIR_SEP "%s.cfg", HSDIR, m_PRGNAME); // Filename "backen"
//    if (HSDIR[0]==0)                                                            // HSDIR muss gesetzt sein
//    {
//        lprintf ("HSDIR must be set\n");                                        // sonst gibt es diese Fehlermeldung
//        return NULL;
//    }
//#ifdef OS_WINDOWS
//    RCtab = RCread("%APPDATA%/hs/rc");
//    sprintf_ex (cfg_Filename, "/hs/etc/%s.cfg", m_PRGNAME);
//#endif
//#ifdef OS_LINUX
//    RCtab = RCread("/etc/hsrc");
//#endif



/**
 @brief  Zeile vom Configfile lesen (via ReadConfigLineFromFile ) und angegebene Parameter setzen
 @return  Return:
         - 0 OK, alle Parameter gefunden
         - -1 Config Eintrag nicht gefunden
         - >0 Fehler beim verarbeiten der Zeile ( siehe LOG )
 @param  cfg_name       zu suchender Eintrag
 @param  fmt            Format der Parameter Das sind:
                        -  \%i für Integer
                        -  \%s für Strings
 @param ...              Die zu setzenden Variablen, die natürlich mit fmt übereinstimmen müssen.


 @code
 Beispiel:
  int max_files;
  char cfg_FLAGS[9];
  char InDir[PATH_MAX];

  if (ReadVarsConfigFile("C:\\HS\\etc\\dir2file.cfg","%i %s %s", &max_files, &cfg_FLAGS, &InDir)) return EXIT_FAILURE;
 @endcode

___[ Revision ]______________________________________________________________

 ** 08.03.2018 HS Create
___________________________________________________________________________*/

int ReadVarsConfigFile(char *cfg_name, char *fmt, ...)
{
    int Parameter;                  // Schleifenzähler - aktuell verarbeitete Position
    int rc;                         // Rückgabewert
    char par;                       // Zu verarbeitendes Zeichen aus fmt
    char *tmp;                      // fmt-parameter bzw. Zeilenargument
    char *Line;                     // Zu überprüfende Zeile

    //int   itmp     __attribute__ ((unused));
    int* par_int;                   // wenn INT-Parameter
    char* par_char;                 // wenn CHAR* Parameter

    int max_par;                    // max parameter
    va_list argptr;                 // variablelist

    max_par = strcount(fmt,'%');                                                // Zählen, wie oft % vorkommt. Das nehmen wir als Parameteranzahl an
    rc = 0;                                                                     // ReturnCount

    Line = ReadConfigLineFromFile(cfg_name, max_par);                           // Zeile aus der Config lesen
    if (!Line)                                                                  // Fehler ? ->
    {
        lprintf ("%s not found in Configfile", cfg_name);
        return -1;
    }

    va_start (argptr, fmt);                                                     // Auf erstes Argument positionieren
    for (Parameter=1;Parameter<=max_par;Parameter++)                            // Alle Parameter durchlaufen
    {
        tmp = strstrarg(fmt,Parameter,' ');                                     // aus fmt den Variablen Type bestimmen
        if (!tmp)                                                               // Fehler ? ->
        {
            lprintf ("format error for %s", cfg_name);
            rc++;
            break;
        }
        par = tolower(tmp[1]);                                                  // Welcher ist es ?
        tmp = strstrarg(Line,Parameter,';');                                    // Parameter con Configzeile lesen
        if (!tmp)                                                               // Fehler ? ->
        {
            lprintf ("Line can't filled from %s", Line);
            rc++;
            break;
        }
        CL(tmp);                                                                // Cleanup

        switch (par)                                                            // Was soll zugewiesem werden ?
        {
            case 'i': par_int=(int*)va_arg(argptr,int*);                                  // Integer !
                      *par_int=atoi(tmp);
                      break;
            case 's': par_char=va_arg(argptr,char*);                            // String
                      strcpy(par_char,tmp);
                      break;
            default:                                                            // Fehler ? ->
                      lprintf ("unknown command used as '%c' in fmt=%s", par, fmt);
                      rc++;
                      break;
        }
    }
    va_end(argptr);
    free (Line);
    return rc;
}



#ifdef notusedCode
signed int oChkPar( int tmp);
signed int oChkPar( int tmp)
{
 if( (tmp=='\\') ||
     (tmp=='/' ) ||
     (tmp=='-' )
   ) return true;
 return  false;
}

signed int oChkARG(char *Find, int arguc, char *arguv[]);
signed int oChkARG(char *Find, int arguc, char *arguv[])
{
    register unsigned int Slash;
    register unsigned int i;
    unsigned int nc;                                                            // NoCompareFlag

    char    *src;                                                               // erweiterter Zeiger auf Find
    char    *dst;                                                               // Zeiger auf das Arg arguv[]

    ARG     =NULL;                                                              // Global erstmal NULL setzen

    src=Find;                                                                   // Orginal nicht verändern
    Slash=0;                                                                    // Kein Slash

    if(ChkPar(src[0])==true)                                                    // wenn im Find ein Slash ist
    {
        //printf ("IN src ist PAR %s\n",src);
		src++;                                                                  // Auf Zeichen dahinter
        //printf ("src now: %s\n",src);
        Slash++;                                                                // und Slash auf WAHR setzen
    }

    for( i=1; i<arguc; i++ )                                                    // Alle Parameter testen
    {
        nc=0;                                                                   // NoCompare=False
        dst=arguv[i];                                                           // nach dst Kommandoparameter
        //printf ("DST=%s\n",dst);

        if (Slash!=0)                                                           // Wenn in Find Argument war ->
        {
            if (ChkPar(dst[0]))                                                 // und in ARG ein Slash ist
            {
                //printf  ("IN dst ist PAR %s\n",dst);
                dst++;                                                          // dann ARG auch PLUS 1
            }else{
                nc=1;                                                           // NoCompare setzen
            }
        }

        if (nc==0)
        {
            if (!strncmp(dst,src,strlen(src)))                                  // Wenn StartParameter gleich
            {
                dst=dst+strlen(src);                                            // den Parameter nicht(!)
                //printf ("in DST = %02x \n", dst[0]);
                if (Slash)
                {
                    if (dst[0]=='=') dst++;                                     // Gleichheitszeichen nicht
                }
                if (arg_used) arg_used[i]=1;
                if (dst[0]==0)                                                  // Wenn moderne oder Unixstyle benutzt wurde
                {
                    if ( (i+1) < arguc )                                        // noch Extraparameter möglich
                    {
                        if (ChkPar(arguv[i+1][0])!=true)                        // nächster Parameter kein kommando
                        {
                            ARG = arguv[i+1];                                   // Dann Parameter auf nächsten ARG setzen
                            if (arg_used) arg_used[i+1]=1;
                            return i;
                        }
                    }
                }
                //04.11.2016 ??
                ARG=dst;                                                        // Globalen ARG setzen
                return i;                                                       // und FUNDSTELLE zurückgeben
            }
        }
    }
    return 0;                               // Alle Argumente durchlaufen,
}                                           // aber nichts gefunden
#endif // notusedCode
///@}
