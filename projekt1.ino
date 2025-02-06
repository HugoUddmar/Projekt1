/*
  Fil: projekt1.ino
  Skapare: Hugo Uddmar
  Datum: 2024-02-06
  Beskrivning: Kod för att kunna spela min egna version av Flappy Bird på en Arduino med en fjärrkontroll
*/

#include "U8glib.h" 
#include <IRremote.hpp> 
//bibliotek

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK); //initiera skärmobjekt

int spelarePos = 30; //int variabel for spelarens y-position

int hastighet = -4; //int variabel för spelarens hastighet

float talUtanDecimaler; //float för matte funktion

float storstaSkillnad;
float xSkillnad;
float ySkillnad;
float xPosPixel;
float yPosPixel;
//variabler för egen funktion som ritar en linje

float x;
float y;
//koordinater för coolt()

int const breddSkarm = 124; 
int hinderMedHastighet[breddSkarm] = {}; 
//variabler för hindrena

int level = 0; 
int highScore = 0;
int levelKopia = 0; 
int gammalLevel = 2; 
//variabler för leveln

bool gameOver = false; //bool variabel för att se om man har förlorat spelet

int gravitation = 1; //int variabel för gravitation 1 eller -1. Den multipliceras med hastighet och acceleration

int lampor[6] = {}; 
int index = 0; 
//variabler för lamporna som visar vilken nivå man är på i det binära talsystemet

int raknare1 = 1; 
int raknare2 = random(4,10); 
//variabler för räknare för gravitation och hindrena. Jag har också en inbyggd räknare i hinder-arrayen

bool irKontroller = false; //kollar om man har tryckt ner på fjärrkontrollen eller inte
 
String gameOverText; //en sträng för när det är game over och man ser sin score

const int IrPin = 3; //pinnen för ir-sensorn

void setup() 
{
  Serial.begin(115200); //ir-sensor biblioteket ville ha en högre hastighet än det vanliga (9600)
  u8g.setFont(u8g_font_unifont);
  IrReceiver.begin(IrPin, true); //startar seriella monitorn, sätter font på OLED-skärmen och startar ir-sensorn
  
  for (int i = 7; i<13; i++){
    pinMode(i,OUTPUT);
  }
  //startar lamporna

  startaSkarm(); //här skriver jag ut "Push button" på skärmen
  startCheck(); //här kollar jag om man trycker på + knappen på min fjärrkontroll så att man kan starta spelet när man vill
}

void loop() 
{
  uppdateraHinderOchLevel();
  irKontroll();
  uppdateraSpelare();
  uppdateraOled();
  uppdateraLampor();
  bytGravitation();
  gameOverFunktion();
}

/*
  Funktionen kollar om man har tryckt ner + knappen på min fjärrkontroll 
  och då blir ett bool-värde positivt för senare funktioner.

  Parametrar: inget
  Returnerar: void (De flesta funktioner är void och har inga parametrar eftersom det är ett spel)
*/
void irKontroll()
{
  if ((IrReceiver.decode())) 
  {
    if (IrReceiver.decodedIRData.decodedRawData == 4161210119) //minus knappen:4094363399 plus knappen:4161210119
    {
      irKontroller = true;
    }
    IrReceiver.resume();
  }
  else
  {
    irKontroller = false;
  }

  /*if(digitalRead(IrPin) == 1)
  {
    irKontroller = true;
  }
  else
  {
    irKontroller = false;
  }*/
}

/*
  Funktion som uppdaterar spelarens y-position. 
  Jag har med en hastighet som ökar hela tiden 
  till en viss hastighet och kan ändra gravitationen.
  Den tar bool-värdet från irkontroll() och om det är sant hoppar man i spelet

  Parametrar: inget
  Returnerar: void
*/
void uppdateraSpelare()
{

  if (irKontroller)
  {
    hastighet = -4 * gravitation; //Här hoppar man i spelet när man trycker på + på fjärrkontrollen. När man "hoppar" sätts bara hastigheten väldigt högt och i motsatt riktning mot gravitationen.
  }

  spelarePos += hastighet; //Här adderas spelarens y-position med hastigheten. Hastigheten är alltså hur många pixlar spelaren rör sig varje gång skärmen uppdaterar.

  if(hastighet < 4 || hastighet > -4)
  {
    hastighet += 1 * gravitation; //Här adderas hastigheten så att det blir en accerelation. Hastigheten begränsas när den blir för stor.
  }
}

/*
  Funktionen skapar hindrena, hastigheterna av hindrena, och räknarna för hastigheterna, allt i en array. Den uppdaterar också leveln. Allt det sker efter skärmen uppdaterats 30 ggr.
  Sen skiftar funktionen arrayen med ett element åt vänster för alla värden varje gång skärmen uppdateras, som vi gjorde nån gång på någon lektion.

  Parametrar: inget
  Returnerar: void
*/
void uppdateraHinderOchLevel()
{
  for (int i = 1; i < breddSkarm; i++) 
  {
    hinderMedHastighet[i-1] = hinderMedHastighet[i]; //Här förs alla värden tillbaka ett steg
  }

  if (raknare1 == 30) 
  {
    raknare1 = 0; 
    hinderMedHastighet[breddSkarm-3] = random(10,35); //Här skapas ett nytt hinder med ett värde mellan 10 och 35 som är y-positionen av den högsta pelaren av hindret.
    hinderMedHastighet[breddSkarm-2] = random(-2,3); //Här är hindrets hastighet. Kan vara 0, negativt eller positivt. 
    hinderMedHastighet[breddSkarm-1] = 0; //Här är räknaren för när hastigheten ska uppdateras. Den börjar på 0 och när den är -10 adderas hindrets hastighet på hindrets position.
    level += 1; //Här adderas leveln med 1
    raknare2--; //Här subtraheras räknaren för när gravitationen ska vända
  }
  else 
  {
    hinderMedHastighet[breddSkarm-1] = 0; //Om det inte ska bli något hinder sätts värdet i arrayen till 0
  }
  raknare1++;
}

/*
  Här skrivs spelaren och alla hindren ut på skärmen. Jag måste även hantera hinder hastigheten här men det är för att det passar perfekt i for-loopen

  Parametrar: inget
  Returnerar: void
*/
void uppdateraOled()
{  
  u8g.firstPage();
  do
  {     
    for(int i = 0; i < breddSkarm; i++) //Det är en for-loop för alla x-värden på skärmen
    { 
      if(hinderMedHastighet[i] > 2) //Det är hinder arrayen som loopas igenom. Det första värdet som kommer att hittas är nästan alltid y-positionen av hindret. Kan inte skriva != 0 för att när man skiftar alla värden vänster kommer programmet ibland få hindrets hastighet istället för hindret om det är >2 är det säkert att man har ett hinder i arrayen.
      { 
        if(hinderMedHastighet[i+2] == -10) //Här kollar den om räknaren för hindrets hastighet är -10 och om hindret ska adderas med hastigheten
        {
          hinderMedHastighet[i+2] = 0;
          if(hinderMedHastighet[i] <= 5 || hinderMedHastighet[i] >= 42)
          {
            hinderMedHastighet[i+1] *= -1; //Om hindret börjar gå utanför skärmen multipliceras hastigheten med *-1 så att den byter riktning
          }
          hinderMedHastighet[i] += hinderMedHastighet[i+1]; //Här adderas hindret med hindrets hastighet.
        }

      
        ritaVertikalLinje(i,0,hinderMedHastighet[i]);  //Här skrivs hindrets båda pelare ut. Det finns ju ett hål i mitten där spelaren hoppar igenom.
        ritaVertikalLinje(i,hinderMedHastighet[i]+20,63); 

        
        hinderMedHastighet[i+2] -= 1; //Hindrets räknare för hastigheten
        i += 29; //När man har hittat ett hinder är det bara att addera i med 29 (eller 30 har inte testat med 30) så kommer man till nästa hinder på direkten eftersom avståndet mellan hinder är 30 pixlar.
      }
    }

    u8g.drawPixel(10,spelarePos); //Här skrivs spelarens position ut

  } while (u8g.nextPage());  
}

/*
  Funktionen kollar om man har förlorat spelet eller inte

  Parametrar: inget
  Returnerar: void
*/
void gameOverFunktion()
{
  if(spelarePos <= 0 || spelarePos >= 63) //Här kollar den om spelarens position är utanför skärmen
  {
    gameOver = true;

    if (level >= highScore)
    {
      highScore = level;
    }
    gameOverText = "Game Over Score:" + String(level) + " Highscore:" + String(highScore); //Leveln och highscoret sätts på en sträng som sedan skrivs ut på OLEDen
  }

  if (hinderMedHastighet[9] > 10) //Om hindrets x-position är samma som spelaren och den är inuti hindret förlorar man spelet. Det är 9 istället för 10 för arrayer börjar på 0.
  {
    if (spelarePos < hinderMedHastighet[9] || spelarePos > hinderMedHastighet[9] + 20)
    {
      gameOver = true;
      
      if (level >= highScore)
      {
        highScore = level;
      }
      gameOverText = "Game Over Score:" + String(level) + " Highscore:" + String(highScore);
    }
  }

  while(gameOver)
  { 
    for(int i=7; i>-130; i-=5)
    {
      u8g.firstPage();
      do
      {
        u8g.drawStr(i,35,gameOverText.c_str());   //Här skrivs gameover strängen ut med score och highscore
            
      } while (u8g.nextPage());
     
      if ((IrReceiver.decode())) 
      {
        if (IrReceiver.decodedIRData.decodedRawData == 4094363399) //minus knappen:4094363399 plus knappen:4161210119. Här kollar den pp om jag trycker på minusknappen och vill starta spelet igen
        {
          i = -999;
          gameOver = false;
          nollstallVariabler();
          coolt();
          startaSkarm();
          startCheck();  
        }
        IrReceiver.resume();
      }
    }
  }        
}

/*
  Funktionen konverterar ett decimaltal till ett omvänt binärt tal och sedan visar det på lamporna

  Parametrar: inget
  Returnerar: void
*/
void uppdateraLampor(){
  if (gammalLevel == level)
  {

  }
  else
  {
    levelKopia = level;
    index = 0;

    for(int i = 0; i < 6; i++)
    { 
      lampor[i] = 0;
    } 

    while (levelKopia >= 1) //Det här är en metod för att konvertera ett decimal tal till ett omvänt binärt tal. Man kollar bland annat på om talet är jämnt eller udda och delar med två.
    {
      if(levelKopia % 2 == 1)
      {
        lampor[index] = 1;
        levelKopia++;
        levelKopia /= 2;
        levelKopia--;
      }
      else
      {
        lampor[index] = 0;
        levelKopia /= 2;
      }
      index++;
    }

    for(int i = 7; i < 13; i++)
    {
      digitalWrite(i,lampor[12-i]); //Sätter på lamporna i omvänd ordning med lampor arrayen som är 0/1 på varje element.
    }

    gammalLevel = level;
  }
}

/*
  Byt gravitationen i spelet

  Parametrar: inget
  Returnerar: void
*/
void bytGravitation()
{
  if (raknare2 == 0)
  {
    u8g.firstPage();
    do
    {           
      u8g.drawStr(10,35,"gravity change");
    } while (u8g.nextPage());  
    delay(1000);
    gravitation *= -1;
    raknare2 = random(4,10);
  }
}

/*
  Skriver ut "push button" på skärmen

  Paramaterar: inget
  Returnerar: void
*/
void startaSkarm()
{
  u8g.firstPage();
  do{
    u8g.drawStr(20,35,"Push button");
  } while (u8g.nextPage());  
}

/*
  Kollar om man trycker på knapp och startar spel då

  Parametrar: inget
  Returnerar: void
*/
void startCheck()
{
  irKontroller = false;
  while (!irKontroller)
  {
    delay(200);
    
    if ((IrReceiver.decode())) 
    {
      if (IrReceiver.decodedIRData.decodedRawData == 4161210119) //minus knappen:4094363399 plus knappen:4161210119*/
      {
        irKontroller = true;
      }
      IrReceiver.resume();
    }

    /*if (digitalRead(IrPin) == 1)
    {
      irKontroller = true;
    }*/
  }
}

/*
  Nollställer alla rörliga variabler för när man startar om spelet

  Parametrar: inget
  Returnerar: void
*/
void nollstallVariabler()
{
  for (int i = 0; i<124; i++)
  {
    hinderMedHastighet[i] = 0;
  }
  spelarePos = 30;
  level = 0;
  gammalLevel = 2;
  lampor[6] = {};
  gameOver = false;
  gravitation = 1;
  raknare1 = 1;
  raknare2 = random(4,10);
  hastighet = -4;
  irKontroller = false;
}

/*
  Ritar en rak vertikal linje 

  Parametrar: x-värde och två y-värden
  Returnerar: void
*/
void ritaVertikalLinje(int x, int y1, int y2)
{
  for(int i = y1; i < y2 + 1; i++){
    u8g.drawPixel(x,i);
  }
}

/*
  Ritar en linje som kan vara rak eller sne åt och funkar åt alla håll

  Parametrar: 
  - x1: en int för första x-koordinaten,
  - y1: en int för första y-koordinaten,
  - x2: en int för andra x-koordinaten,
  - y2: en int för andra y-koordinaten,
  Returnerar: void
*/
void ritaLinje(int x1,int y1,int x2,int y2)
{
  xPosPixel = x1; //koordinaterna för starten av linjen och variablerna som sätts in i drawPixel. Senare adderas ett visst tal på x och yPosPixel så att det blir en linje
  yPosPixel = y1;

  xSkillnad = x2 - x1; //viktigt för senare och för att bestämma största skillnaden.
  ySkillnad = y2 - y1;

  if (absolutVarde(xSkillnad) > absolutVarde(ySkillnad))
  {
    storstaSkillnad = xSkillnad;
  }
  else
  { 
    storstaSkillnad = ySkillnad;
  }

  storstaSkillnad = absolutVarde(storstaSkillnad);

  xSkillnad = xSkillnad/storstaSkillnad;
  ySkillnad = ySkillnad/storstaSkillnad; //xSkillnad och ySkillnad blir det man adderar x och yPosPixel med i for-loopen. Hade varit tydligare med en annan variabel men det är mer effektivt att ha samma. En av de här blir alltid 1 eller -1 och den andra ett decimaltal mellan 0 och 1.

  

  for(int i = -1; i < storstaSkillnad; i++) //for loop som ritar alla pixlarna i linjen.
  {
    u8g.drawPixel(taBortDecimaler(xPosPixel), taBortDecimaler(yPosPixel)); //hade kunnat avrunda argumenten till heltal istället men det är här är mycket effektivare.

    xPosPixel += xSkillnad;
    yPosPixel += ySkillnad;
  }
  
}

/*
  Tar absolutbelopp av värde

  Parametrar: - a: en float
  Returnerar: en positiv float
*/
float absolutVarde(float a)
{
  if (a < 0)
  {
    return -a;
  }
  else
  {
    return a;
  }
}

/*
  tar bort decimaler på ett decimaltal

  Parametrar: - a: en float
  Returnerar ett heltal
*/
int taBortDecimaler(float a)
{
  return int(a); 
}

/*
  Avrundar ett decimaltal till ett heltal

  Parametrar: - a: en float
  Returnerar: ett tal utan decimaler
*/
int avrundaTillHeltal(float a)
{
  talUtanDecimaler = taBortDecimaler(a);

  if (a - talUtanDecimaler >= 0.5)
  {
    return talUtanDecimaler + 1;
  }
  else 
  {
    return talUtanDecimaler;
  }
}

/*
  Egen funktion som gör så att programmet "laddar" innan spelet startar

  Parametrar: inget
  Returnerar: void
*/
void coolt()
{
  x = 81;
  y = 28;

  for(float i=0; i<32 ;i++) //float istället för int för flera olika värden i sin och cos
  {
    u8g.firstPage();
    do
    {
      u8g.drawCircle(60,30,21);
      ritaLinje(60,30,avrundaTillHeltal(x),avrundaTillHeltal(y)); //avrunda talet istället för floor/ceiling funktion så det kan gå åt båda hållen
    } while (u8g.nextPage());      

    y += 4 * cos(i/5); //x och y värdena följer en cirkel.
    x -= 4 * sin(i/5);

  } 
}
