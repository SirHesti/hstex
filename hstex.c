/** --------------------------------------------------------------------------

#-[ export

english version; german scrolls down

Program to exchange text parts in files.

to change local files that are on a network volume.
e.g. files under /etc
Also great for changing a file on the network and the file is changed on all
computers in the infrastructure.

Similar to sed but still completely different.

-=-=__ German translation __-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 Programm um Textteile in Dateien auszutauschen.

 um lokale Dateien zu ändern, die auf eine Netzwerkplatte liegen.
 zb. Dateien unter /etc
 Auch klasse um eine Datei im Netz zu aendern und auf allen
 Rechnern der Infrastruktur wird die Datei geaendert.
  
 Aehnlich sed aber dennoch voellig anders.

#-[ ende

*/


---[ Revision ]---------------------------------------------------------------

 @date
 06.12.20 HS Proklamation
 24.09.22 HS -b -x neu
 15.10.22 HS new Switch -f -- finde eine sektion
 19.10.22 HS neuer Switch -l -- liste alle sektionen

---[ Aufrufe ]----------------------------------------------------------------
 -r hstex.c -w wichtig.txt
 -l -r hstex.c "-b---[ "
 -r ..\dateinsert\dateinsert.cpp -w create.sh "-b---[ " -e "ExportBash" -x
 -l -r r:/share/sys-crons/systemd-hs_nightjob.conf -e failure.service
 -? -l -r R:/share/sys-crons/systemd-hs_rename_scans.conf
 -x -r hstex.c -w README.md
----------------------------------------------------------------------------*/

// TODO: id_block mit parameter
#include "tools.h"
#include "ctools/multiargs.h"
#include "VERSION.h"

char Line[4096];
char in_FILE[PATH_MAX];
char ex_FILE[PATH_MAX];
char *tmp_FILE;
char in_block[128];
char out_block[128];
char id_block[128];
char rid_block;
int exclusiv;
int findmode;
int listmode;

FILE *f_in=NULL;
FILE *f_out=NULL;
FILE *f_tmp=NULL;

enum errorcodes {
    err_tmpfile_write = 10,
    err_infile_read,
    err_outfile_read,
    err_readto_import,
    err_readto_export,
    err_read_transfer_stuff,
    err_readto_in_mark,

    err_out_blockm,
    err_in_blockm,
    err_in_File,
    err_ex_File,

    err_max_codes
};

char *getSwitches(void)
{
    int i;
    char *rslt;
    rslt = NULL;
    for (i=1;i<m_PRG_argc;i++)
    {
        if (rslt!=NULL) stradd( rslt, " ");
        stradd(rslt, m_PRG_arguv[i]);
    }
    return rslt;
}


void done_RC(int ErrorCode)
{
    switch (ErrorCode)
    {
        case err_readto_in_mark:
            lprintf ("Keine Ende-Markierung \"%s\" in %s", id_block, ex_FILE);
            //lprintf ("Im out_file konnte keine Ende-Markierung gefunden werden");
            break;
        case err_read_transfer_stuff:
            lprintf ("Keine Ende-Markierung \"%s\" in %s", id_block, in_FILE);
            //lprintf ("Im in_file konnte keine Ende-Markierung gefunden werden");
            break;
        case err_readto_export:
            lprintf ("Keine Markierung \"%s\" in %s", out_block, in_FILE);
            break;
        case err_readto_import:
            lprintf ("Keine Markierung \"%s\"in %s", in_block, ex_FILE);
            break;
        case err_infile_read:
            lprintf("File kann nicht zum Lesen geoeffnet werden: %s\n",in_FILE);
            break;
        case err_outfile_read:
            lprintf("File kann nicht zum Lesen geoeffnet werden: %s\n",ex_FILE);
            break;
        case err_tmpfile_write:
            lprintf("tempFile kann nicht zum Schreiben geoeffnet werden: %s",tmp_FILE);
            break;

        case err_in_blockm:
            lprintf("export Markierung NULL");
            break;
        case err_out_blockm:
            lprintf("import Markierung NULL");
            break;
        case err_in_File:
            lprintf("readFile nicht angegeben");
            break;
        case err_ex_File:
            lprintf("writeFile nicht angegeben");
            break;
    }

    if (f_tmp) fclose (f_tmp);
    f_tmp = NULL;
    if (f_in)  fclose (f_in);
    f_in = NULL;
    if (f_out) fclose (f_out);
    f_out = NULL;
    if (ErrorCode)
    {
        if (tmp_FILE)
        {
            remove (tmp_FILE);
            free   (tmp_FILE);
        }
        tmp_FILE = getSwitches();
        if (tmp_FILE)
        {
            lprintf ("Switches are : %s", tmp_FILE);
            free   (tmp_FILE);
        }
        exit   (ErrorCode);
    }
    if ((findmode)||(listmode)) exit (0);
}

signed int main(int argc, char *argv[])
{
    int rc;
	int helpprnt;

	if (InitTools(argc , argv, "%v%t%m", I_MAJOR, I_MINOR, I_BUILD, I_BETA, LOG_LOGFILE | LOG_STDERR)) return -1;
	strcpy (id_block,"#-[ ");
	strcpy (in_block,"import");
	strcpy (out_block,"export");
	//exclusiv = false;
	//findmode = false;
	//listmode = false;
	if ( aChkARGlong("help") ) goto Help;
	rc=GetCMDLineParameter(argc, argv, "-? %b -l %b -f %b -x %b -r %s -w %s -i %S -e %S -b %S", &helpprnt, &listmode, &findmode, &exclusiv , &in_FILE, &ex_FILE, &in_block, &out_block, &id_block);
	if (rc) goto Help;
	if (helpprnt) goto Help;
	if (!listmode)
    {
        if (out_block[0]==0) done_RC(err_out_blockm);
    }else{
        out_block[0]=0;
        strcpy (Line, id_block);
        strCL(Line);
        tmp_FILE = RightStr(Line,1);
        char linemode_open=tmp_FILE[0];
        switch (linemode_open)
        {
        case '{': rid_block = '}'; break;
        case '(': rid_block = ')'; break;
        case '[': rid_block = ']'; break;
        default: rid_block = linemode_open; break;
            break;
        }

    }
	if (in_FILE[0]==0) done_RC(err_in_File);
	if ((findmode==false) && (listmode==false))
    {
        if (in_block[0]==0) done_RC(err_in_blockm);
        if (ex_FILE[0]==0) done_RC(err_ex_File);
    }


	strins(in_block, id_block, 0);
	strins(out_block, id_block, 0);
    if(( f_in  = fopen(in_FILE,"rt"))  == NULL )  done_RC(err_infile_read);

    if ((findmode==false) && (listmode==false))
    {
        tmp_FILE = tempfilename(NULL);
        if(( f_tmp = fopen(tmp_FILE,"wt")) == NULL )  done_RC(err_tmpfile_write);
        if (exclusiv)
        {
            f_out = fopen(ex_FILE,"at");
            if (f_out) fclose(f_out);
        }
        if(( f_out = fopen(ex_FILE,"rt"))  == NULL )  done_RC(err_outfile_read);
    }

    for (;;)
    {
        if (findmode) break;
        if (listmode) break;
        if (exclusiv) break;
        if (mgetline(f_out,Line,sizeof(Line)-3)!=0) done_RC(err_readto_import);         // Übertrage altes "Vorher"
        fprintf (f_tmp,"%s\n",Line);
        //rc=strlen(in_block);
        if (!strncmp(Line,in_block,strlen(in_block))) break;
    }

    for (;;)
    {
        if (mgetline(f_in,Line,sizeof(Line)-3)!=0)
        {
            if (listmode==true) done_RC(0);
            done_RC(err_readto_export);          // Überlese irrelevantes aus Vorher
        }
        if (!strncmp(Line,out_block,strlen(out_block)))
        {
            if (listmode==false) break;
            strdel (Line, 0, strlen(out_block));
            for (rc=0;;rc++)
            {
                if (Line[rc]==' ') Line[rc]=0;
                if (Line[rc]==0) break;
            }
            if (Line[0]!=rid_block) printf("%s\n", Line);
        }
    }
    if (findmode) done_RC(0);

    for (;;)
    {
        if (mgetline(f_in,Line,sizeof(Line)-3)!=0) done_RC(err_read_transfer_stuff);    // Lese Relevantes
        if (!strncmp(Line,id_block,strlen(id_block))) break;
        fprintf (f_tmp,"%s\n",Line);                                                    // und schreibe es
    }

    for (;;)
    {
        if (exclusiv) break;
        if (mgetline(f_out,Line,sizeof(Line)-3)!=0) done_RC(err_readto_in_mark);        // Bis zur Markierung vergessen
        if (!strncmp(Line,id_block,strlen(id_block))) break;
    }
    for (;;)
    {
        if (exclusiv) break;
        fprintf (f_tmp,"%s\n",Line);                                                    // Endemarkierung übertrage
        if (mgetline(f_out,Line,sizeof(Line)-3)!=0) break;
    }
    done_RC(0);
    filecopy (tmp_FILE, ex_FILE);
    remove (tmp_FILE);
    free   (tmp_FILE);
    lprintf("Datei erfolgreich geaendert: %s",ex_FILE);
    return 0;

Help:
    printf ("%s\n\n", m_PRG_INFO);
    printf ("-r fileto Read\n");
    printf ("-w fileto Write\n");
    printf ("-i import block #-[ import ] in Writefile\n");
    printf ("-e export block #-[ export ] in Readfile\n");
    printf ("-b \"#- \" aendern\n");
    printf ("-x export only -i usw. wird nicht gelesen und das File (-w) wird ueberschrieben\n");
    printf ("-f find only; report im ErrorCode 0=gefunden; ansonsten nicht\n");
    printf ("-l ListMode alle moeglichen Bloecke im File auflisten\n");

    //printf ("-a #- identifier ")
    return 0;
}

