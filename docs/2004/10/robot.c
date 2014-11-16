/****************************************************************
 * robot Automaton for MORDOR
 ****************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<ctype.h>
#include<varargs.h>
#include<time.h>

#define MUDHOST "tesla.tech.cerritos.edu"
#define MUDPORT 4040

#define ctrl(C) ((C) & 037)
#define streq(A,B) (!strcmp ((A), (B)))
#define READ 0
#define WRITE 1
#define frand() ((double) rand()/ (RAND_MAX+1.0))

char *MYNAME, *MYPASS;
int fmud, tmud;
FILE *fp;
int recordlog = 0;
int debug = 0;
int solo = 0;
int maxhp = 0;
int maxmp = 0;
int pray = 0;
int praytime = 0;
int haste = 0;
int hastetime = 0;
int janitor = 0;
int janitor_still_need = 0;    /* flag for needing 2nd pass of janitor */
int time_to_eject = 0;
int talk_mode = 0;
char *enemy;

/*************************************************************************/
/*********************CHANGE THE FOLLOWING LINES**************************/

char *MASTER = "Pyroman";

char *monsterlist[] =  {" crow", "10",
			"scarecrow", "10",
			"field", "15",
			"wife", "10", 
			"gypsy-bard", "23",
			"merchant", "23",
			"aristocrat", "23",
			"waitress", "23",
			"barmaid", "23",
			"peasant", "23",
			"nobleman", "23",
			"traveler", "23",
			"herald", "23",
                   	"drunk", "23",
			"bartender", "25", 
			NULL};

/* '0' is to stop "get all"'ing, 1 is to "drop all" */
char janitor_script[] = "0swws4n1s5neen";

/*************************************************************************/
/*************************************************************************/

void help();
void login();
int waitfor(char *);
int hangaround(int sec);
void readmsg(char *);
void readskillmsg(char *);
void quit_robot(int min);
int connectmud();
char *getmud();
int charsavail(int fd);
int strindex(char *, char *);
int command(char *msg);
void eject();
void forward(char *msg);
char *hackoff(char *);
void passwd(char *);

int health(int minhp);
int check_mp(int minmp);
void attack();
void readattackmsg(char *);
void flee();
int prepare(char *);
void vigor();
char *figureoutwho(char *);
void report(char *);
int monster_database(char *, int *);
int confirm(char *);
char *gettime();
void follow_script();
void janitor_hangaround(int sec);
void jonesing();
void talk_sequence();
void broad_report();
void buy_bags();

/****************************************************************
 * Main routine
 ****************************************************************/

main (int argc, char *argv[]) 
{
  int i=0;

  /* Get the options from the command line */
  if (argc == 1) 
    help();
  while (--argc > 0 && (*++argv)[0] == '-') {
      switch (*++(*argv)) {
        case 'd': debug++; break;
        case 's': solo++; break;
        case 'p': pray++; break;
        case 'h': haste++; break;
        case 'r': recordlog++; break;
        case 'j': janitor++; break;
        case 'e': time_to_eject = (360 * atoi(*++argv)); --argc; break;
        case 't': talk_mode++; break;
        case 'l': MYNAME = (*++argv); --argc; 
                  MYPASS = (*++argv); --argc; break; 
        default:  help(); 
      }
  }
if (debug) fprintf (stderr, "MYNAME is %s\n", MYNAME); 

  if ((argc < 0) || (MYNAME == NULL)) {
    help();
  }

/*  while (monsterlist[i] != NULL && (debug) && (solo)) {
    fprintf(stderr, "Item: %s, %sminhp.\n", monsterlist[i++], 
                     monsterlist[i++]);
  }
*/

if (debug && time_to_eject) fprintf(stderr, "Will eject after %d hours\n", 
time_to_eject/360);

  tmud = fmud = connectmud (); 

  if (tmud < 0) {
    fprintf (stderr,"Connect failed\n");
    exit (-1);
  }

  enemy = NULL;
  login ();

  if (debug) fprintf(stderr, "Main is exiting.....\n");  
  close (tmud);
/*  fclose(fp); */
  exit (0);
}

/****************************************************************
* void help: displays help information
*****************************************************************/
void help() {

  fprintf(stderr, 
"*************************************************************************\n");
  fprintf(stderr, "Robot for Mordor\n\n");
  fprintf(stderr, 
"usage: ga [-d] [-s] [-p] [-r] -l <name> <password> [&]\n\n");
  fprintf(stderr, 
"       -d: displays all output to screen; for monitoring and debugging.\n");
  fprintf(stderr, 
"       -s: puts robot in solo mode; will fight by itself.\n");
  fprintf(stderr, 
"       -p: for clerics or paladins; will pray automatically.\n");
  fprintf(stderr,
"       -r: open file to keep a record of kills, problems, etc\n\n");
  fprintf(stderr, 
" ***   -l: needed for login; name and password required.\n\n");
  fprintf(stderr, 
"        &: type \"&\" to run the robot in the background.\n");
  fprintf(stderr, 
"           note: you cannot have \"-d\" and \"&\" at the same time!\n\n");

  exit(0);
}


/****************************************************************
 * void login: Automaton.  logs into MORDOR
 ****************************************************************/

void login () { 
  waitfor ("Kawado"); 
  sendmud ("%s", MYNAME);
  sendmud ("%s", MYPASS);

  if (recordlog) {
    fp = fopen("robot.log", "w");
    record("Logging in.");
  }

  if (pray)
    praytime = time(0) + 600;     /* after 10 minutes, try and pray */
  if (haste)
    hastetime = time(0) + 600;

  if (janitor) {
    while (1)
      follow_script();
  }
  sendmud("clear ansi");
  sendmud("clear prompt");
  sendmud("set ignore");

  if (time_to_eject)
    time_to_eject += time (0);   /* xhrs + current time later will eject */

  if (time_to_eject) {
    while (time (0) < (time_to_eject)) {
       hangaround(15);
      sendmud("l");
      sendmud("save");
      jonesing();
      if (talk_mode)
        talk_sequence();
    }
  }
  else {
    while (1) {
       hangaround(15);
      sendmud("l");
      sendmud("save");
      jonesing();
      if (talk_mode)
        talk_sequence();
    }
  }
}

/****************************************************************
 * int waitfor:
 ****************************************************************/

int waitfor (char *pat) {
  char *msg;

  while (1) {
    if (msg = getmud ()) {
      readmsg (msg);
      if (strindex (msg, pat)) return (1);
    }
    else {
      sleep (1);
      if (debug) fprintf(stderr, "Waiting for .....%s\n", pat); }
  }
}

/****************************************************************
 * int hangaround: MAIN DRIVER...gets messages from MORDOR
 * and distributes them.
 ****************************************************************/

int hangaround (int sec) {
  char *msg;
  int alarm;

  alarm = time (0) + sec;
 
  while (time (0) < alarm) {
    if (msg = getmud ()) {
      readmsg (msg);                 /* send msg thru normal checker */
      readattackmsg(msg);          /* send msg thru attack checker */
      if ((praytime != 0) && (time(0) > praytime)) {
        sendmud("pray");
        praytime=0;
      }
      if ((hastetime != 0) && (time(0) > hastetime)) {
        sendmud("haste");
        hastetime=0;
      }
      readskillmsg(msg);             /* send msg thru skill checker */
    }
    else { 
      sleep (1); }
  }
  
  return (1);
}

/****************************************************************
 * void readmsg: Handle messages
 ****************************************************************/

void readmsg (char *msg) {

  char temp[50];
  char temp2[80];
  int i=0;

  if (MASTER != NULL) {
    sprintf(temp, "### %s just flashed", MASTER);
  }

  if (debug) fprintf (stderr, "Msg: %s\n", msg);

  if (strindex(msg, "how much xp?") &&
      strindex(msg, "just flashed"))
    report(msg); 

  if (strindex(msg, "how much xp?") &&
      strindex(msg, "broadcasted"))
    broad_report();    


  else if (MASTER != NULL)
    if (strindex(msg, temp))
      command (hackoff(msg));

/*  else if (strindex(msg, "just flashed"))
    forward (msg);
*/

  else if (strindex(msg, "stinks up") && (strindex(msg, MASTER))) {
    strcpy(temp2, "give sc ");
    strcat(temp2, MASTER);
    for (i=0; i < 20; i++)
      sendmud(temp2);
  }

  else if (strindex(msg, "sucks you") && strindex(msg, MASTER))
    sendmud("clear ignore");
  else if (strindex(msg, "hugs you") && strindex(msg, MASTER))
    sendmud("set ignore");

  else if (strindex(msg, "just arrived") && 
           (solo) &&
           (enemy == NULL)) {
    if (prepare(msg)) {  /* if attackable monster, returns TRUE when ready */
      attack(msg);
    }
  } 

  else if (strindex(msg, "gave a hornblade to you") && (solo)) {
    sendmud("put horn br");
    sendmud("say thanks!");
    sendmud("smile");
  }

  else if (strindex(msg, "bag is full") && (solo))
    sendmud("drop horn");

/*  else if (strindex(msg, "does not exist"))
    forward (msg);
*/

  else if (strindex(msg, "You failed to escape")) {
    hangaround(3);
    flee();
  }


  else if (janitor) {
    if (strindex(msg, "### Shutting down in"))
      janitor_still_need = 0;
  }

  else if (strindex(msg, "### Shutting down now") && (solo))
    quit_robot(5);   

}

/****************************************************************
* readskillmsg: loops for praying, tracking?, etc
*****************************************************************/
void readskillmsg(char *msg) {

  if (pray) {
    if (strindex(msg, "Your prayers were not answered")) 
      praytime = time(0) + 12;

    else if (strindex(msg, "You feel extremely pious"))
      praytime = 0;
      
    else if (strindex(msg, "You've already prayed"))
      praytime = 0; 

    else if (strindex(msg, "You feel less pious"))
      praytime = time (0) + 360;   /* 6 minutes */
  }
  if (haste) {
    if (strindex(msg, "Your attempt to hasten failed")) 
      hastetime = time(0) + 12;

    else if (strindex(msg, "You feel yourself moving faster"))
      hastetime = 0;

    else if (strindex(msg, "You've already hastened"))
      hastetime = 0;

    else if (strindex(msg, "You feel slower"))
      hastetime = time (0) + 660;  /* 11 minutes */
  }
}


/****************************************************************
 * void quit_robot: We are exiting.  Write out any long term memory first.
 ****************************************************************/

void quit_robot (int min) {

  close (tmud);
  close (fmud);
  /* Write out any memory files needed */

start:
  if (debug) fprintf(stderr, "Sleeping 5 minutes......\n");
  record("Shutdown....sleeping.");
  sleep(min*60);
  tmud = fmud = connectmud (); 
  record("Attempting to connect.");

  if (tmud < 0) {
    if (debug) fprintf (stderr,"Connect failed\n");
    record("Connect failed.");
    goto start;
  }
  enemy = NULL;    /* reset enemy pointer */
  janitor_still_need = 0;
  login();
}

/****************************************************************
 * int connectmud: Open the MUD socket
 ****************************************************************/

int connectmud() {

  struct sockaddr_in sin;
  struct hostent *hp;
  int fd;

  if (debug) fprintf (stderr, "Connecting...\n");

  bzero((char *) &sin, sizeof(sin));

  sin.sin_port = htons(MUDPORT);

  if ((hp = gethostbyname(MUDHOST)) == 0) return (-1);

  bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
  sin.sin_family = hp->h_addrtype;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) return -1;

  if (connect(fd,(struct sockaddr *) &sin, sizeof(sin)) < 0) return -1;

  return fd;
}

/****************************************************************
 * sendmud: Send a command to the MUD process
 ****************************************************************/

sendmud (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) 
char *fmt;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
{
  int len;
  char buf[10240];
  
  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  strcat (buf, "\n");
  len = strlen (buf);

  if (debug) fprintf (stderr, "Send: %s\n", buf);

  if (write (tmud, buf, len) != len) {
    if (debug) fprintf (stderr, "Write failed: %s", buf);
    quit_robot (5);
  }
}

/****************************************************************
 * char *getmud: Read one line from mud
 ****************************************************************/

char *getmud () {
  int len, result = 0;
  static char buf[BUFSIZ], rbuf[4];
  register char *s=buf;

  /* No input waiting */
  if (!charsavail (fmud)) return (NULL);

  /* Read one line, save printing chars only */
  while ((len = read (fmud, rbuf, 1)) > 0) {
    if (*rbuf == '\n')
      break;
    if (isprint (*rbuf))
      *s++ = *rbuf;
  }
  *s = '\0';

  /* Check for error */  
  if (len < 0) {
    fprintf (stderr, "Error %d reading from mud\n", len);
    quit_robot (5);
  }

  return (s = buf);
}

/*****************************************************************
 * int charsavail: check for input available from 'fd'
 *****************************************************************/

int charsavail (int fd) {
  long n;
  int retc;
  
  if (retc = ioctl (fd, FIONREAD, &n)) {
    fprintf (stderr, "Ioctl returns %d, n=%ld.\n", retc, n);
    quit_robot (5);
  }

  return ((int) n);
}


/************************************************************
* int strindex: checks for occurance of pat in str
************************************************************/

int strindex (char msg[], char pat[]) {

  int i, j, k;
  char temp[100];

  sprintf(temp, "%s\0", msg);  /* just to MAKE SURE there is a null */

  for (i=0; temp[i] != '\0'; i++) {
    for (j=i, k=0; pat[k]!='\0' && temp[j]==pat[k]; j++, k++)
      ;
    if (k > 0 && pat[k] == '\0') {
      if (debug) fprintf (stderr, "strindex finds a match!.....%s\n", pat);
      return 1;
    }
  }
  return 0;
}

/***********************************************************
* int command: Top function for MASTER-commands
***********************************************************/

int command (char *msg) {
  int toggle=0, i;
  char temp[100];

if (debug) fprintf(stderr, "I am being commanded....\n");

  if (msg == NULL) return 0;

  if ((strindex(msg, "@solo")) && (solo == 0)) {
    solo = 1;
    return 0;
  }
  if ((strindex(msg, "@solo")) && (solo == 1)) {
    solo = 0;
    return 0;
  } 
  if (strindex(msg, "@kill ")) {
    msg++; msg++; msg++; msg++; msg++; msg++;
    enemy = (char *) calloc(1, 15);
    strcpy(enemy, msg);

    while (health((2*maxhp)/3) == 0) {
      vigor();
      if (enemy == NULL) return 0;
      hangaround(5);
    }
    attack();
    return 0;
  }

  if (debug) fprintf (stderr, "I have produced %s.\n", msg);

  strcpy(temp, msg);
  sendmud(temp);

  return 0;
}

/*******************************************************
* void eject: properly exits the mud
********************************************************/

void eject() {

  sendmud ("quit\n");

  exit(0);

}

/************************************************************
* void forward: passes msg onto MASTER
*************************************************************/

void forward (char *msg) {

  record(msg);         /* records all flashes */

  if (MASTER != NULL) {
    char temp[100], temp2[100];

    if (!strindex(msg, "Message sent to") &&
        !strindex(msg, "Send to whom?") &&
        !strindex(msg, "That person is not")) {
      strcpy(temp, "send ");
      strcat(temp, MASTER);
      strcat(temp, " fwd: ");
      strcat(temp, msg);
      sendmud (temp);
    }
  }
}

/*************************************************************
* char *hackoff: removes prefix 'Xxxxx just flashed, "' for command()
**************************************************************/
char *hackoff(char *msg) {
  int i;
  char foo[50];

  while ((*msg != '\"') && (*msg != '\0'))
    msg++;

  if (*msg != '\0')
    msg++;
  else
    return NULL;

  for(i=0; (*msg != '\"') && (*msg != '\0'); i++)
    foo[i] = *msg++;
  foo[i] = '\0';

  if (debug) fprintf(stderr, "Hackoff is returning....%s\n", foo);

  return foo;
}

/*******************************************************************
* int health: returns true if hp is greater than minhp
********************************************************************/
int health(int minhp) {
  int tot, counter=0;
  char *msg;

  sendmud("sc");
  while ((counter < 30) &&
         (!strindex((msg = getmud()), "Hit P") &&
          !strindex(msg, "/"))) {
    readmsg(msg);
    readattackmsg(msg);
    counter++;        /* counter will keep it from going out of control */
  }
  if (counter < 30) {
    if (sscanf(msg, "%d/%d Hit Point", &tot, &maxhp) == 2) {
      if (debug) fprintf(stderr, "health: %d/%d hp\n", tot, maxhp);
    }
    else {
      if (debug) fprintf(stderr, "health: didn't match cast: %s\n", msg);
      return 0;
    }

    if (tot >= minhp)
      return 1;
    else 
      return 0;
  }
  else {
    if (debug) fprintf(stderr, "Health messed up..%d...\n", counter);
    return -1;
  }
}


/*******************************************************************
* int check_mp: returns true if mp is greater or equal to minmp
********************************************************************/
int check_mp(int minmp) {
  int i, tot=0, hp, hp2, counter=0;
  char *msg;

  sendmud("sc");
  while ((counter < 30) &&
         (!strindex((msg = getmud()), "Hit P") &&
          !strindex(msg, "/"))) {
    readmsg(msg);
    readattackmsg(msg);
    counter++;        /* counter will keep it from going out of control */
  }
  if (counter < 30) {
    if (sscanf(msg, "%d/%d Hit Points %d/%d Magic Points", &hp, &hp2, &tot, &maxmp) == 4) {
      if (debug) fprintf(stderr, "check_mp: %d/%d mp\n", tot, maxmp);
    }
    else {
      if (debug) fprintf(stderr, "check_mp: didn't match cast: %s\n", msg);
      return 0;
    }

    if (tot >= minmp)
      return 1;
    else 
      return 0;
  }
  else {
    if (debug) fprintf(stderr, "check_mp messed up..%d...\n", counter);
    return -1;
  }
}


/**********************************************************
* void attack: attacks an enemy until killed
***********************************************************/
void attack()
{
  while (enemy != NULL) {
      if (debug) fprintf(stderr, "Attacking the %s.....\n", enemy);
      sendmud("kill %s", enemy);
/*      sendmud("use sc %s", enemy); */
      sleep(1);
      sendmud("look");    /* just to flush the output */
      hangaround(1);    /* waits 2 seconds total before attacking again */
  }
}

/*************************************************************
* void readattackmsg: directs messages important to attacking
**************************************************************/
void readattackmsg(char *msg) {
  char *temp;

  if (strindex(msg, "You killed")) {
    if (debug) fprintf(stderr, "Yahoo! Killed the %s...\n", enemy);
      record("Killed the %s.", enemy);
      sleep(1);
      free(enemy);        /* clear memory used by enemy */
      enemy = NULL;
      sendmud("get gold");
      sendmud("get pend");
      sendmud("get dust");
      sendmud("get sc");
      sendmud("get sc");
  }

  else if (strindex(msg, "You FUMBLED"))
    sendmud("wield horn");

  else if (strindex(msg, "a place you go when you die")) {
    if (debug) fprintf(stderr, "Killed by %s...\n", enemy);
    record("Killed by %s...", enemy);
    enemy = NULL;
    hangaround(4);
/*    sendmud("broad Awwwwwwwwwwww.......shit!!"); */
    eject();
  }

  else if (strindex(msg, "shatters") &&
           strindex(msg, "Your")) {
    sendmud("get horn br");
    sendmud("wield horn");
  }

  else if (strindex(msg, "is broken") &&
           strindex(msg, "Your")) {
    sendmud("drop horn");
    sendmud("get horn br");
    sendmud("wield horn");
  }

  else if ((strindex(msg, "don't see that here")) ||
           (strindex(msg, "Attack what?"))) {    /* safety check */
    free(enemy);
    enemy = NULL;
  }

  else if ((strindex(msg, "hit you")) || (strindex(msg, "attacks you"))) {
      if (!confirm(msg)) {
        enemy = (char *) calloc(1, 30);
        temp = figureoutwho(msg);
        if (temp != NULL)
          strncpy(enemy, temp, 30);
        attack();
      }
      if (!health((maxhp*2)/3)) {
        if (!health((maxhp / 4)))
          flee();
        vigor();
      }
  }
}

/*********************************************************
* void flee: flees from enemy, then re-equips
**********************************************************/
void flee() {

  sendmud("rem all");
  sendmud("flee");

  free(enemy);
  enemy = NULL;

  sendmud("wear all");
  sendmud("wield horn");
  record("Had to flee from %s!!", enemy);

  while (!health((2*maxhp)/3)) {
    if (debug) fprintf(stderr, "In flee().....vigging....\n");
    vigor();
    hangaround(4);
  }
  sendmud("e");     /* for Rusty's */
}

/******************************************************************
* int prepare: considers enemy; if attackable, vigs until has minhp
*******************************************************************/
int prepare(char *msg) {
  int minhp, a, counter=0;

  if(!monster_database(msg, &minhp))  /* will assign minhp if TRUE */
    return 0;

  while (((a=health(minhp)) <= 0) &&
         (counter < 30)) {      /* will just keep trying to vig until */
    vigor();                    /* min hp is reached */
    if (debug) fprintf(stderr, "Prepare is trying to vig.....\n");
    if (enemy == NULL) return 0;
    hangaround(5);
    counter++;
  }
  if (counter >= 30) {  /* if true, something messed up..... */
    if (debug) fprintf(stderr, "prepare: counter exceeded 30...???");
    return 0;
  }

  if (a == -1) {
    if (debug) fprintf(stderr, "Prepare doesn't like Health's answer.\n");
    enemy = NULL;
    return 0;
  }
  return 1;
}

/******************************************************************
* void vigor: cast vigor
*******************************************************************/
void vigor() {

  sendmud("c v");
}

/*****************************************************************
* char figureoutwho: returns name from ### REPORT or unknown attacker
******************************************************************/
char *figureoutwho(char *msg) {
  int i=0, j=0, dam=0;
  char *name, temp[10];

  name = (char *) calloc(30, sizeof(char));

  strncpy(temp, msg, 10);

  if (debug) fprintf(stderr, "Figureoutwho received %s.\n", msg);

if (!strindex(msg, "###")) {
  if (sscanf(msg, ": The %s hit you for %d damage.", name, &dam) == 2) {
    if (debug) fprintf(stderr, "figureoutwho1: attacker=%s, hit for %d damage\n", name, dam);
  }
  else if (sscanf(msg, " The %s hit you for %d damage.", name, &dam) ==2) {
    if (debug) fprintf(stderr, "figureoutwho2: attacker=%s, hit for %d damage\n", name, dam);
  }
  else if (sscanf(msg, ": %s hit you for %d damage.", name, &dam) == 2) {
    if (debug) fprintf(stderr, "figureoutwho3: attacker=%s, hit for %d damage\n", name, dam);
  }
  else if (sscanf(msg, " %s hit you for %d damage.", name, &dam) == 2) {
    if (debug) fprintf(stderr, "figureoutwho4: attacker=%s, hit for %d damage\n", name, dam);
  }
  else if (sscanf(msg, ": The %s attacks you", name) == 1) {
    if (debug) fprintf(stderr, "figureoutwho5: attacker=%s\n", name);
  }
  else if (sscanf(msg, " The %s attacks you", name) == 1) {
    if (debug) fprintf(stderr, "figureoutwho6: attacker=%s\n", name);
  }
  else if (sscanf(msg, ": %s attacked you", name) == 1) {
    if (debug) fprintf(stderr, "figureoutwho7: attacker=%s\n", name);
  }
  else if (sscanf(msg, " %s attacked you", name) == 1) {
    if (debug) fprintf(stderr, "figureoutwho8: attacker=%s\n", name);
  }
  else {
    if (debug) fprintf(stderr, "figureoutwho*: could not figure out name\n");
    return NULL;
  }
}
else {
  if (sscanf(msg, ": ### %s just flashed", name) == 1) {
    if (debug) fprintf(stderr, "figureoutwho9: flasher=%s\n", name);
  }
  else if (sscanf(msg, " ### %s just flashed", name) == 1) {
    if (debug) fprintf(stderr, "figureoutwho10: flasher=%s\n", name);
  }
  else {
    if (debug) fprintf(stderr, "figureoutwho: could not figure out name\n");
    return NULL;
  }
}

  return name;
}

/**************************************************************
* void report: forwards score information to whoever
***************************************************************/
void report(char *line) {
  int counter=0, i=0, xp=0;
  char *msg, *name, temp[50];

  name = figureoutwho(line);
  if (name == NULL)    /* could not figure out who */
    return;  

  sendmud("sc");
  while ((counter < 30) &&
         (!strindex((msg = getmud()), "Exper") &&
          !strindex(msg, "Gold"))) {
    readmsg(msg);
    readattackmsg(msg);
    readskillmsg(msg);
    counter++;
  }
  if (counter < 30) {
    if (sscanf(msg, "%d Experience", &xp) == 1) {
      if (debug) fprintf(stderr, "report: xp = %d\n", xp);
    }
    else {
      if (debug) fprintf(stderr, "report: msg does not fit cast...%s\n", 
msg);
      return;
    }

    sprintf(temp, "send %s I have %d xp.", name, xp);
    if (debug) fprintf(stderr, "Report has produced %s.\n", temp);
    sendmud(temp);
  }
  else if (debug) fprintf(stderr, "Report messed up...%d...\n", counter);
}

void broad_report() {
  int counter=0, i=0, xp=0;
  char *msg, *name, temp[50];

  sendmud("sc");
  while ((counter < 30) &&
         (!strindex((msg = getmud()), "Exper") &&
          !strindex(msg, "Gold"))) {
    readmsg(msg);
    readattackmsg(msg);
    readskillmsg(msg);
    counter++;
  }
  if (counter < 30) {
    if (sscanf(msg, "%d Experience", &xp) == 1) {
      if (debug) fprintf(stderr, "report: xp = %d\n", xp);
    }
    else {
      if (debug) fprintf(stderr, "report: msg does not fit cast...%s\n", 
msg);
      return;
    }

    sprintf(temp, "broad I have %d xp.", xp);
    if (debug) fprintf(stderr, "Report has produced %s.\n", temp);
    sendmud(temp);
  }
  else if (debug) fprintf(stderr, "Report messed up...%d...\n", counter);
}

/*************************************************************
* int monster_database: returns TRUE if thing arrived is on the 
* list, and thus attackable; also assigns *minhp
**************************************************************/
int monster_database(char *msg, int *minhp) {
  int i=0;

  while (monsterlist[i] != NULL) {
    if (strindex(msg, monsterlist[i])) {
      enemy = (char *) calloc(1, 15);
      strcpy (enemy, monsterlist[i]);
      *minhp = atoi(monsterlist[(i+1)]);
    }
    i+=2;   /* increment 2 entries...we don't want to check numbers! */
  }
  if ((enemy != NULL) && (*minhp > 0)) {
    if (debug) fprintf(stderr, "Monster_database returns %s: %dhp.\n", 
                       enemy, *minhp);
    return 1;
  }
  else return 0;     /* no matches found....don't attack */
}

/************************************************************
* int confirm: makes sure thing attacking you is your "enemy"
*************************************************************/
int confirm(char *msg) {

  if (enemy != NULL)
    if (strindex(msg, enemy)) 
      return 1;
  if (debug) fprintf(stderr, "confirm: negative confirmation!!\n");
  return 0;
}

/*******************************************************************
* record: writes msg to FILE
********************************************************************/
record (msg, a1, a2, a3, a4)
char *msg;
int a1, a2, a3, a4;
{
  char buf[10240];

  if (recordlog) {
    sprintf (buf, msg, a1, a2, a3, a4);
    strcat (buf, "\n");

    if (debug) fprintf(stderr, "Recording \"%s\".\n", msg);

    fputs((gettime()), fp);
    fputs(buf, fp);

    if (ferror(fp)) {
      if (debug) fprintf(stderr, "Error trying to record to file.\n");
    }
  }
}

/******************************************************************
* char *gettime: returns correct time/date
*******************************************************************/
char *gettime() {
  char *temp;
  time_t dong;
  time(&dong);       /* necessary to fill dong with correct time */

  return ctime(&dong); 

}

/*******************************************************************
* void follow_script: follows global janitor_script, getting all
********************************************************************/
void follow_script()
{
  char move[2];
  int i, j, flag=0;

  for (i=0; janitor_script[i] != '\0'; i++) {

    if (janitor_script[i] == '0') 
      buy_bags();
    else if (janitor_script[i] == '1') 
      sendmud("drop all");
    else if (janitor_script[i] == '4') {
        sendmud("se");
        sleep(5);
    }
    else if (janitor_script[i] == '5')
        sendmud("nw");
    else {
      sprintf(move, "%c", janitor_script[i]);
      if (debug) fprintf(stderr, "follow_script: moving: %s\n", move);
      sleep(4);
      sendmud(move);    
    }

  }
}

/*************************************
* void buy_bags: purchases 10k small bags
**************************************/
void buy_bags(void)
{
  int i, j;

  for (i=0; i < 5; i++) {
    for (j=0; j < 25; j++)    /* buys 50 small bags at a time */
      sendmud("buy hoe");
    sendmud("cackle");
    sleep(4);                  /* and pauses in between each batch */
  } 
}

/**********************************************************************
* janitor_hangaround: specific for janitors only - doesn't readmsg();
**********************************************************************/
void janitor_hangaround(int sec)
{
  char *msg;
  int alarm;

  alarm = time(0) + sec;

  while (time(0) < alarm) {
    msg = getmud();
    if (debug) fprintf(stderr, "Msg: %s\n", msg);
    if (strindex(msg, "You weren't able")) 
      janitor_still_need = 1;
  }
}

/**********************************************************************
* jonesing: restores hp while waiting around
**********************************************************************/
void jonesing()
{
  if (!health((3*maxhp)/4)) {
    if (debug) fprintf(stderr, "jonesing: need to vig\n");
    if (check_mp((3*maxmp)/4)) {
      vigor();
    }
  }
}

/********************************************************************
* void talk_sequence: randomly performs an action
********************************************************************/
void talk_sequence()
{
  int a=0, num=17;   /* num is # of strings you have */
  char *talkq[] = {"giggle",
		 "taunt barm",
		 "yell come out!!!!!",
	         "yell come out!!",
                 "moon bart",
                 "yawn",
                 "twid",
		 "mutter",
		 "grin",
		 "eye bart",
                 "eye barm",
                 "eye trav",
                 "eye waitr",
		 "eye gyp",
		 "cackle",
		 "chuckle",
                 "smirk"};

  a = (num * frand());
  if (debug) fprintf(stderr, "Doing: %s\n", talkq[a]);
  if (a < (num-1))
    sendmud("%s", talkq[a]);
}

