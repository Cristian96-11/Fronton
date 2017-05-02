/*****************************************************************************/
/*                                                                           */
/*                           Fronton0.c                                      */
/*                                                                           */
/*  Programa inicial d'exemple per a les practiques 2 i 3 d'ISO.	     */
/*                                                                           */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides a "winsuport.c", les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc -c winsuport.c -o winsuport.o				     */
/*	   $ gcc fronton0.c winsuport.o -o fronton0 -lcurses		     */
/*                                                                           */
/*****************************************************************************/
//#FILA Y COLUMNA NO SE ACTUALIZAN pelota siempre valen 0
//#Que variables mas debemos sincronizar para que se vea bien por pantalla?
//#Condiciones de fin de juego
#include <stdio.h>	/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include "winsuport.h"	/* incloure definicions de funcions propies */
#include <pthread.h>


#define MIDA_PALETA 4	/* definicions constants del programa */

			/* variables globals */
char *descripcio[]={
"\n",
"Aquest programa implementa una versio basica del joc del fronto:\n",
"generar un camp de joc rectangular amb una porteria, una paleta que s\'ha\n",
"de moure amb el teclat per a cobrir la porteria, i una Npil[0] que rebota\n",
"contra les parets del camp i contra la paleta. Quan la Npil[0] surt per la\n",
"porteria, el programa acaba la seva execucio. Tambe es pot acabar prement\n",
"la tecla RETURN. El joc consisteix en aguantar la Npil[0] el maxim temps.\n",
"\n",
"  Arguments del programa:\n",
"\n",
"       $ ./fronton0 fitxer_config [retard]\n",
"\n",
"     El primer argument ha de ser el nom d\'un fitxer de text amb la\n",
"     configuracio de la partida, on la primera fila inclou informacio\n",
"     del camp de joc (valors enters), i la segona fila indica posicio\n",
"     i velocitat de la Npil[0] (valors reals):\n", 
"          num_files  num_columnes  mida_porteria\n",
"          pos_fila   pos_columna   vel_fila  vel_columna\n",
"\n",
"     on els valors minims i maxims admesos son els seguents:\n",
"          6 < num_files     < 26\n",
"          9 < num_columnes  < 81\n",
"          0 < mida_porteria < num_files-2\n",
"        1.0 <= pos_fila     <= num_files-2\n",
"        1.0 <= pos_columna  <= num_columnes-1\n",
"       -1.0 <= vel_fila     <= 1.0\n",
"       -1.0 <= vel_columna  <= 1.0\n",
"\n",
"     Alternativament, es pot donar el valor 0 a num_files i num_columnes\n",
"     per especificar que es vol que el tauler ocupi tota la pantalla. Si\n",
"     tambe fixem mida_porteria a 0, el programa ajustara la mida d\'aquesta\n",
"     a 3/4 de l\'altura del camp de joc.\n",
"\n",
"     A mes, es pot afegir un segon argument opcional per indicar el\n",
"     retard de moviment del joc en mil.lisegons; el valor minim es 10,\n",
"     el valor maxim es 1000, i el valor per defecte d'aquest parametre\n",
"     es 100 (1 decima de segon).\n",
"\n",
"  Codis de retorn:\n",
"     El programa retorna algun dels seguents codis:\n",
"	0  ==>  funcionament normal\n",
"	1  ==>  numero d'arguments incorrecte\n",
"	2  ==>  no s\'ha pogut obrir el fitxer de configuracio\n",
"	3  ==>  algun parametre del fitxer de configuracio es erroni\n",
"	4  ==>  no s\'ha pogut crear el camp de joc (no pot iniciar CURSES)\n",
"\n",
"   Per a que pugui funcionar aquest programa cal tenir instal.lada la\n",
"   llibreria de CURSES (qualsevol versio).\n",
"\n",
"*"};		/* final de la descripcio */

int MAXp = 0;

int n_fil, n_col;       /* numero de files i columnes del taulell */
int m_por;		/* mida de la porteria (en caracters) */
int f_pal, c_pal;       /* posicio del primer caracter de la paleta */
int retard;		/* valor del retard de moviment, en mil.lisegons */

char strin[65];		/* variable per a generar missatges de text */

typedef struct{
  int f_pil;
  int c_pil;	/* posicio de la Npil[0], en valor enter */
  float pos_f;
  float pos_c;	/* posicio de la Npil[0], en valor real */
  float vel_f;
  float vel_c;	/* velocitat de la Npil[0], en valor real */
}pilota;

pilota Npil[9];

int fi1=0; //mouPaleta Return
int fi2; //mouPilota Gol
int fi3;
int rebots=10;
int tiempo=0;

pthread_t tid[10];

pthread_mutex_t mutexpant= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexfi= PTHREAD_MUTEX_INITIALIZER;

/* funcio per carregar i interpretar el fitxer de configuracio de la partida */
/* el parametre ha de ser un punter a fitxer de text, posicionat al principi */
/* la funcio tanca el fitxer, i retorna diferent de zero si hi ha problemes  */
int carrega_configuracio(FILE *fit){
  int ret=0;
  
  fscanf(fit,"%d %d %d\n",&n_fil,&n_col,&m_por);	   /* camp de joc */
  while((!feof(fit)) && (MAXp<9)){
  fscanf(fit,"%f %f %f %f\n",&Npil[MAXp].pos_f,&Npil[MAXp].pos_c,&Npil[MAXp].vel_f,&Npil[MAXp].vel_c); /* Npil[0] */
  printf("%f %f %f %f\n",Npil[MAXp].pos_f,Npil[MAXp].pos_c,Npil[MAXp].vel_f,Npil[MAXp].vel_c);
  if ((n_fil!=0) || (n_col!=0))			/* si no dimensions maximes */
  {
    if ((n_fil < 7) || (n_fil > 25) || (n_col < 10) || (n_col > 80))
	ret=1;
    else
    if (m_por > n_fil-3)
	ret=2;
    else
    if ((Npil[MAXp].pos_f < 1) || (Npil[MAXp].pos_f > n_fil-2) || (Npil[MAXp].pos_c < 1) || (Npil[MAXp].pos_c > n_col-1))
	ret=3;
  }
  if ((Npil[MAXp].vel_f < -1.0) || (Npil[MAXp].vel_f > 1.0) || (Npil[MAXp].vel_c < -1.0) || (Npil[MAXp].vel_c > 1.0))
  	ret=4;
  
  if (ret!=0)		/* si ha detectat algun error */
  {
    fprintf(stderr,"Error en fitxer de configuracio:\n");
    switch (ret)
    {
      case 1:	fprintf(stderr,"\tdimensions del camp de joc incorrectes:\n");
		fprintf(stderr,"\tn_fil= %d \tn_col= %d\n",n_fil,n_col);
		break;
      case 2:	fprintf(stderr,"\tmida de la porteria incorrecta:\n");
		fprintf(stderr,"\tm_por= %d\n",m_por);
		break;
      case 3:	fprintf(stderr,"\tposicio de la Npil[0] incorrecta:\n");
		fprintf(stderr,"\tNpil[0].pos_f= %.2f \tpos_c= %.2f\n",Npil[MAXp].pos_f, Npil[MAXp].pos_c);
		break;
      case 4:	fprintf(stderr,"\tvelocitat de la Npil[0] incorrecta:\n");
		fprintf(stderr,"\tNpil[0].vel_f= %.2f \tNpil[0].vel_c= %.2f\n",Npil[MAXp].vel_f,Npil[MAXp].vel_c);
		break;
     }
  }
  MAXp++;
  }
  fclose(fit);
  fi2 = MAXp;
  return(ret);
}


/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
/* retorna diferent de zero si hi ha algun problema */
int inicialitza_joc(void)
{
  int i, retwin;
  int i_port, f_port;		/* inici i final de porteria */

  retwin = win_ini(&n_fil,&n_col,'+',INVERS);	/* intenta crear taulell */

  if (retwin < 0)	/* si no pot crear l'entorn de joc amb les curses */
  { fprintf(stderr,"Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {	case -1: fprintf(stderr,"camp de joc ja creat!\n");
    		 break;
	case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n");
		 break;
	case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n");
		 break;
	case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n");
		 break;
     }
     return(retwin);
  }

  if (m_por > n_fil-2)
	m_por = n_fil-2;	/* limita valor de la porteria */
  if (m_por == 0)
  	m_por = 3*(n_fil-2)/4;		/* valor porteria per defecte */

  i_port = n_fil/2 - m_por/2 -1;	/* crea el forat de la porteria */
  f_port = i_port + m_por -1;
  for (i = i_port; i <= f_port; i++)
	win_escricar(i,0,' ',NO_INV);

  n_fil = n_fil-1;		/* descompta la fila de missatges */

  f_pal = 1;			/* posicio inicial de la paleta per defecte */
  c_pal = 3;
  for (i=0; i< MIDA_PALETA; i++)       /* dibuixar paleta inicialment */
	win_escricar(f_pal+i,c_pal,'0',INVERS);

  for (i=0;i<MAXp;i++){
    if (Npil[i].pos_f > n_fil-1)
  	  Npil[i].pos_f = n_fil-1;	/* limita posicio inicial de la Npil[0] */
    if (Npil[i].pos_c > n_col-1)
  	  Npil[i].pos_c = n_col-1;
    Npil[i].f_pil = Npil[i].pos_f;
    Npil[i].c_pil = Npil[i].pos_c;			 /* dibuixar la Npil[0] inicialment */
    win_escricar(Npil[i].f_pil,Npil[i].c_pil,(i+'1'),INVERS);
  }
  sprintf(strin,"Tecles: \'%c\'-> amunt, \'%c\'-> avall, RETURN-> sortir\n",
  							TEC_AMUNT,TEC_AVALL);
  win_escristr(strin);
  
  pthread_mutex_init(&mutexpant, NULL);
  pthread_mutex_init(&mutexfi, NULL);
  return(0);
}




/* funcio per moure la Npil[0]: retorna un 1 si la Npil[0] surt per la porteria,*/
/* altrament retorna un 0 */
void * mou_pilota(int * index)
{
  int i = (int) index; //Para cuando utilicemos el puntero bien
  //int i = index;
  int f_h, c_h, fip=0;
  char rh,rv,rd;
  
  int inici, final, min, seg;
  
  inici=time(NULL);
  
  pthread_mutex_lock(&mutexfi);//A
  do{
    pthread_mutex_unlock(&mutexfi);//A
    
    f_h = Npil[i].pos_f+Npil[i].vel_f;		/* posicio hipotetica de la Npil[0] (entera) */
    c_h = Npil[i].pos_c+Npil[i].vel_c;
   // result = 0;			/* inicialment suposem que la Npil[0] no surt */
    rh = rv = rd = ' ';
    if ((f_h != Npil[i].f_pil) || (c_h != Npil[i].c_pil))
    {		/* si posicio hipotetica no coincideix amb la posicio actual */
      if (f_h != Npil[i].f_pil) 		/* provar rebot vertical */
      {	

        pthread_mutex_lock(&mutexpant);//A       
        rv = win_quincar(f_h,Npil[i].c_pil);	/* veure si hi ha algun obstacle */
  	    
  	    if (rv != ' ')			/* si hi ha alguna cosa */
  	    {   Npil[i].vel_f = -Npil[i].vel_f;		/* canvia sentit velocitat vertical */
  	        f_h = Npil[i].pos_f+Npil[i].vel_f;		/* actualitza posicio hipotetica */
  	        if (rv == '0'){
  	          pthread_mutex_lock(&mutexfi);
  	          fi3 --;
  	          final=time(NULL);
              seg=difftime(final,inici);
              min=seg/60;
              seg=seg%60;
              sprintf(strin,"REBOTS TOTALS: %d, RESTANTS: %d %d:%d",rebots,fi3,min,seg);
              pthread_mutex_unlock(&mutexfi);
  	          win_escristr(strin);
  	         }
  	    }
  	    pthread_mutex_unlock(&mutexpant);//A
      }
      
      if (c_h != Npil[i].c_pil) 		/* provar rebot horitzontal */
      {	
        pthread_mutex_lock(&mutexpant);//B
        rh = win_quincar(Npil[i].f_pil,c_h);	/* veure si hi ha algun obstacle */

      	if (rh != ' ')			/* si hi ha algun obstacle */
  	    {    Npil[i].vel_c = -Npil[i].vel_c;		/* canvia sentit vel. horitzontal */
  	         c_h = Npil[i].pos_c+Npil[i].vel_c;		/* actualitza posicio hipotetica */
  	         if (rh == '0'){
  	           pthread_mutex_lock(&mutexfi);
  	          fi3 --;
  	          final=time(NULL);
              seg=difftime(final,inici);
              min=seg/60;
              seg=seg%60;
              sprintf(strin,"REBOTS TOTALS: %d, RESTANTS: %d %d:%d",rebots,fi3,min,seg);
              pthread_mutex_unlock(&mutexfi);
  	          win_escristr(strin);
  	         }
  	    }
  	    pthread_mutex_unlock(&mutexpant);//B
      }
      
      if ((f_h != Npil[i].f_pil) && (c_h != Npil[i].c_pil))	/* provar rebot diagonal */
      {	
        pthread_mutex_lock(&mutexpant);//C
        rd = win_quincar(f_h,c_h);

  	    if (rd != ' ')				/* si hi ha obstacle */
  	    {    Npil[i].vel_f = -Npil[i].vel_f; Npil[i].vel_c = -Npil[i].vel_c;	/* canvia sentit velocitats */
  	         f_h = Npil[i].pos_f+Npil[i].vel_f;
  	         c_h = Npil[i].pos_c+Npil[i].vel_c;		/* actualitza posicio entera */
  	         if (rd == '0'){
  	          pthread_mutex_lock(&mutexfi);
  	          fi3 --;
  	          final=time(NULL);
              seg=difftime(final,inici);
              min=seg/60;
              seg=seg%60;
              sprintf(strin,"REBOTS TOTALS: %d, RESTANTS: %d %d:%d",rebots,fi3,min,seg);
              pthread_mutex_unlock(&mutexfi);
  	          win_escristr(strin);
  	         }
  	    }
  	    pthread_mutex_unlock(&mutexpant);//C
      }
      
      pthread_mutex_lock(&mutexpant);//D
      if (win_quincar(f_h,c_h) == ' ')	/* verificar posicio definitiva */
      {					/* si no hi ha obstacle */

  	      win_escricar(Npil[i].f_pil,Npil[i].c_pil,' ',NO_INV);  	/* esborra Npil[0] */

  	      Npil[i].pos_f += Npil[i].vel_f; Npil[i].pos_c += Npil[i].vel_c;
        	Npil[i].f_pil = f_h; Npil[i].c_pil = c_h;		/* actualitza posicio actual */
  	     
  	      if (Npil[i].c_pil > 0){		 		/* si surt del taulell, */
  		      win_escricar(Npil[i].f_pil,Npil[i].c_pil,(i+'1'),INVERS); /* imprimeix Npil[0] */
  	      }
  	     
  	      else{
  		      pthread_mutex_lock(&mutexfi);//B
            fi2 --; 
            fip = 1;
            pthread_mutex_unlock(&mutexfi);//B
  	      }
      }
      pthread_mutex_unlock(&mutexpant);//D
    }
    else { Npil[i].pos_f += Npil[i].vel_f; Npil[i].pos_c += Npil[i].vel_c; }
     
     //printf("F: %f || C: %f",Npil[i].pos_f, Npil[i].pos_c); Prueba de posicion pelota

  win_retard(retard);	
  
  pthread_mutex_lock(&mutexfi);//A
  }while(fi1==0 && fip==0 && fi3!=0);
  pthread_mutex_unlock(&mutexfi);//A
  return(0);
  
}



/* funcio per moure la paleta en segons la tecla premuda */
void * mou_paleta(void * null)
{
  int tecla; //result;
  
  pthread_mutex_lock(&mutexfi);//A
  do{
    pthread_mutex_unlock(&mutexfi);//A
     //pthread_mutex_lock(&mutexpant);//F
  tecla = win_gettec();
    //pthread_mutex_unlock(&mutexpant);//F
  if (tecla != 0)
  {
    if ((tecla == TEC_AVALL) && ((f_pal+MIDA_PALETA)< n_fil-1))
    {
      pthread_mutex_lock(&mutexpant);//A
      win_escricar(f_pal,c_pal,' ',NO_INV);	/* esborra primer bloc */
    	f_pal++;				/* actualitza posicio */
	    win_escricar(f_pal+MIDA_PALETA-1,c_pal,'0',INVERS); /*esc. ultim bloc*/
      pthread_mutex_unlock(&mutexpant);//A
    }
    if ((tecla == TEC_AMUNT) && (f_pal> 1))
    {
      pthread_mutex_lock(&mutexpant);//B
      win_escricar(f_pal+MIDA_PALETA-1,c_pal,' ',NO_INV); /*esborra ultim bloc*/
	    f_pal--;				/* actualitza posicio */
	    win_escricar(f_pal,c_pal,'0',INVERS);	/* escriure primer bloc */
	    pthread_mutex_unlock(&mutexpant);//B
	  }
    if (tecla == TEC_RETURN){
      pthread_mutex_lock(&mutexfi);//B
      fi1=1;		/* final per pulsacio RETURN */
      pthread_mutex_unlock(&mutexfi);//B
    }
  }
  win_retard(retard);
  
  pthread_mutex_lock(&mutexfi);//A
  }while(fi1==0 && fi2!=0 && fi3!=0);
  pthread_mutex_unlock(&mutexfi);//A
  return(0);
}



/* programa principal                               */
int main(int n_args, char *ll_args[])
{
  int i;
  FILE *fit_conf;
  int inici, final, min, seg;
  
  inici=time(NULL);

  if ((n_args != 2) && (n_args !=3))	/* si numero d'arguments incorrecte */
  { i=0;
    do fprintf(stderr,"%s",descripcio[i++]);	/* imprimeix descripcio */
    while (descripcio[i][0] != '*');		/* mentre no arribi al final */
    exit(1);
  }

  fit_conf = fopen(ll_args[1],"rt");		/* intenta obrir el fitxer */
  if (!fit_conf)
  {  fprintf(stderr,"Error: no s'ha pogut obrir el fitxer \'%s\'\n",ll_args[1]);
     exit(2);
  }

  if (carrega_configuracio(fit_conf) !=0)	/* llegir dades del fitxer  */
     exit(3);	/* aborta si hi ha algun problema en el fitxer */

  //rebotes = ll_args[2];
  fi3=rebots;
  
  if (n_args == 3)		/* si s'ha especificat parametre de retard */
  {	retard = atoi(ll_args[2]);	/* convertir-lo a enter */
  	if (retard < 10) retard = 10;	/* verificar limits */
  	if (retard > 1000) retard = 1000;
  }
  else retard = 100;		/* altrament, fixar retard per defecte */

  printf("Joc del Fronto: prem RETURN per continuar:\n"); getchar();

  if (inicialitza_joc() !=0)	/* intenta crear el taulell de joc */
     exit(4);	/* aborta si hi ha algun problema amb taulell */

  //do			/********** bucle principal del joc **********/
  //{	fi1 = mou_paleta();
  //for(z=0;z<MAXp;z++){
	//fi2 = mou_pilota(z);
  //}
	//win_retard(retard);		/* retard del joc */
  //} while (!fi1 && !fi2);
  
 
  for (i=0; i<MAXp; i++){
    pthread_create(&tid[i], NULL, mou_pilota, (void **) (i));  //Cuando utilicemos el puntero bien -> (void **) (i)
  }
  pthread_create(&tid[MAXp], NULL, mou_paleta, NULL); 
  
  do{
    win_retard(1000);
    pthread_mutex_lock(&mutexpant);
    final=time(NULL);
    seg=difftime(final,inici);
    min=seg/60;
    seg=seg%60;
    sprintf(strin,"REBOTS TOTALS: %d, RESTANTS: %d  %d:%d",rebots,fi3,min,seg);
  	          win_escristr(strin);
    pthread_mutex_unlock(&mutexpant);
  }while (fi1==0 && fi2!=0 && fi3!=0);
  
  
  for (i=0; i<=MAXp; i++){
    pthread_join(tid[i], NULL);
  }
  
  
  
  win_fi();				/* tanca les curses */
  if (fi2==0) printf("Final joc perque les pilotes han sortit per la porteria!\n\n");
  else{
    if (fi3==0) printf("Final de joc perque s'han acabat els moviments!\n\n");
    else printf("Final joc perque s'ha premut RETURN!\n\n");
  }  
  
  printf("Temps total de joc %d minuts : %d segons \n", min, seg);
  return(0);			/* retorna sense errors d'execucio */
}




