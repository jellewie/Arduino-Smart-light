/* Written by JelleWho https://github.com/jellewie
  On setup() execute        SetupTask()
  On loop() execute         ExecuteTask()

  HOW TO ADD CUSTOM TASKS
  -"enum xxx"      Add the name to the enum to make the code better to read
  -SetupTask()     [optional] If needed configure set-up stuff
  -DoTask()        Add what this task does

  Time based tasks will not be auto deleted (execte X at HH:MM:SS)
  Delay based tasks will be auto deleted (execte X in Y ticks)
*/

#define TaskLimit 16        //defined as an byte in for loops, so 255 at max
#define Task_SerialEnabled
enum {NONE, SWITCHMODE, DIMMING, BRIGHTEN, RESETESP, CHANGERGB}; //Just to make the code easier to read
String TaskString[] = {"NONE", "SWITCHMODE", "DIMMING", "BRIGHTEN", "RESETESP", "CHANGERGB"}; //ALL CAPS!

const byte Task_Amount = sizeof(ModesString) / sizeof(ModesString[0]); //Why filling this in if we can automate that? :)

//<ip>/task[?PreFix=Value][&....]     //These are currently HARDCODED into the HTML page, so shouldn't be changed if you want to use the webpage
#define PreFixMode  "o"   //1=Add 2=Remove
#define PreFixID    "i"   //Task ID
#define PreFixA     "a"   //Task A
#define PreFixB     "b"   //Task B
#define PreFixC     "c"   //Task B
#define PreFixTimeH "h"   //On which time the task needs to be executed
#define PreFixTimeM "m"   //^
#define PreFixTimeS "s"   //^
#define PreFixTimeT "t"   //In howmany ticks the task needs to be executed

void SetupTask() {
  //For exmaple to set-up some pinmodes
}
struct TASK {
  byte ID = 0;                     //the ID of the task, these are defined in the enum
  byte A = 0;                      //If you want to add some data to the task, for example a pin number
  byte B = 0;                      //If you want to add even more data to the task, for example a pin state
  String C = "";                   //If you want to add even more data to the task, for example a time interfal
  TimeS ExectuteAt = {0, 0, 0, 0}; //The time in millis to execute this task
  bool Executed = false;           //Just here to keep track of it a Time based task has been executed today
};
TASK TaskList[TaskLimit];

String ConvertTaskIDToString(byte IN) {
#ifdef Convert_SerialEnabled
  Serial.println("ConvertTaskToString '" + String(IN) + "'");
#endif //Convert_SerialEnabled
  if (IN < Task_Amount)
    return TaskString[IN];
  return "UNK";
}
int ConvertTaskIDToInt(String IN) {
#ifdef Convert_SerialEnabled
  Serial.println("ConvertTaskToInt '" + IN + "'");
#endif //Convert_SerialEnabled
  if (StringisDigit(IN)) {
    if (IN.toInt() < Task_Amount)
      return IN.toInt();
    else
      return 0;
  }
  IN.toUpperCase();
  for (byte i = 0; i < Task_Amount; i++) {
    if (IN == TaskString[i])
      return i;
  }
  return 0;
}
bool AddTask(TASK Item) {
  for (int i = 0; i < TaskLimit; i++) { //For each task that is allowed
    if (TaskList[i].ID == 0) {          //If there is no task
      TaskList[i] = Item;
      TaskList[i].Executed = false;     //flag this task as not executed

#ifdef Task_SerialEnabled
      Serial.print("T: AddTask " + String(TaskList[i].ID) + " a=" + String(TaskList[i].A) + " b=" + String(TaskList[i].B) + " c=" + TaskList[i].C);
      if (Item.ExectuteAt.Ticks > 0)
        Serial.println(" in " + String(Item.ExectuteAt.Ticks - millis()));
      else
        Serial.println(" at " + String(Item.ExectuteAt.HH) + ":" + String(Item.ExectuteAt.MM) + ":" + String(Item.ExectuteAt.SS));
#endif //Task_SerialEnabled
      return true;
    }
  }
#ifdef Task_SerialEnabled
  Serial.println("TaskList full, couldn't add new task");
#endif //Task_SerialEnabled
  return false;
}
bool DoTask(TASK Item) {
  bool returnValue = true;
#ifdef Task_SerialEnabled
  Serial.print("T: DoTask " + ConvertTaskIDToString(Item.ID) + " a=" + String(Item.A) + " b=" + String(Item.B) + " c=" + Item.C);
  if (Item.ExectuteAt.Ticks > 0)
    Serial.println(" due to Ticks, scheduled for " + String(Item.ExectuteAt.Ticks) + " now=" + String(millis()));
  else
    Serial.println(" due to Time, scheduled for " + String(Item.ExectuteAt.HH) + ":" + String(Item.ExectuteAt.MM) + ":" + String(Item.ExectuteAt.SS));
#endif //Task_SerialEnabled
  switch (Item.ID) {
    case SWITCHMODE:
      //C = New Mode
#ifdef Task_SerialEnabled
      Serial.println("T: SWITCHMODE to " + Item.C + "(" + String(ConvertModeToInt(Item.C)) + ")");
#endif //Task_SerialEnabled
      Mode = ConvertModeToInt(Item.C);
      UpdateLEDs = true;
      break;
    case DIMMING: {
        //A = Stepsize
        //B = GoTo
        //C = TimeInterfall in ms
#ifdef Task_SerialEnabled
        Serial.println("T: DIMMING from " + String(FastLED.getBrightness()) + " a " + String(Item.A) + " a " + String(Item.B) + " a " + Item.C);
#endif //Task_SerialEnabled

        byte BRI = FastLED.getBrightness();

        if (BRI - Item.A < Item.B) {
          BRI = Item.B;
        } else {
          BRI = BRI - Item.A;
          if (Item.C.toInt() > 0) {     //If we need to go further, and need to create another task
            TASK TempTask = Item;
            TempTask.ExectuteAt.Ticks = millis() + Item.C.toInt();
            AddTask(TempTask);
          }
        }
        FastLED.setBrightness(BRI);
        UpdateLEDs = true;
      }
      break;
    case BRIGHTEN: {
        //A = Stepsize
        //B = GoTo
        //C = TimeInterfal in ms
        byte Now = FastLED.getBrightness();
        byte To = 255;
        if (Item.B > 0) To = Item.B;
        if (To - Now < Item.A) {
          Now = To;
        } else {
          Now += Item.A;
          if (Item.C.toInt() > 0) {
            TASK TempTask = Item;
            TempTask.ExectuteAt.Ticks = millis() + Item.C.toInt();
            AddTask(TempTask);
          }
        }
#ifdef Task_SerialEnabled
        Serial.println("T: BRIGHTEN from " + String(FastLED.getBrightness()) + " to " + String(Now));
#endif //Task_SerialEnabled
        FastLED.setBrightness(Now);
        UpdateLEDs = true;
      }
      break;
    case RESETESP: {
        ESP.restart();
      }
    case CHANGERGB: {
        //A = new Red value
        //B = new Green value
        //C = new Blue value
        Mode = ON;
        LastMode = Mode;
        fill_solid(&(LEDs[0]), TotalLEDs, CRGB(Item.A, Item.B, Item.C.toInt()));
        UpdateLEDs = true;
      }
      break;
    default:
      returnValue = false;
      break;
  }
  return returnValue;
}
bool RemoveTask(byte i) {
  if (TaskList[i].ID > 0) {                   //If there is a task
    TaskList[i].ID = 0;                       //Clear this task entry
    return true;                              //EXIT, task removed
  }
  return false;
}
void ExecuteTask() {
  for (int i = 0; i < TaskLimit; i++) {                     //For each task in the list
    if (TaskList[i].ID > 0) {                               //If there is a task
      if (TaskList[i].ExectuteAt.Ticks > 0) {
        if (TaskList[i].ExectuteAt.Ticks <= millis()) {     //If the execute time has pasted
          DoTask(TaskList[i]);                              //Execute this task entry
          RemoveTask(i);                                    //Remove the task, it has been executed
        }
      } else if (TimeSet and (TaskList[i].ExectuteAt.HH > 0 or TaskList[i].ExectuteAt.MM > 0 or TaskList[i].ExectuteAt.SS > 0)) {

        if (TaskList[i].ExectuteAt.HH < TimeCurrent.HH - 12)  //If we are more than 12 hours away
          TaskList[i].Executed = false;                       //flag this task as not executed

        if (!TaskList[i].Executed and                         //If the task has not been executed yet, and it it time for it to do so
            TaskList[i].ExectuteAt.SS <= TimeCurrent.SS and
            TaskList[i].ExectuteAt.MM <= TimeCurrent.MM and
            TaskList[i].ExectuteAt.HH == TimeCurrent.HH) {
          DoTask(TaskList[i]);                                //Execute this task entry (Do not remove, since TimeTasks (h:m:s) are defined as static)
          TaskList[i].Executed = true;
        }
      }
    }
  }
}
void Tasks_handle_Connect() {
  String ERRORMSG = "";
  String Message;
  for (byte i = 0; i < TaskLimit; i++) {        //For each task in the list
    if (TaskList[i].ID > 0) {                   //If there is a task

      Message += "<form action=\"/settask?\" method=\"set\">#" + String(i) + " "
                 "<input type=\"hidden\" name=\"o\" value=\"2\">"
                 "<label>Do task </label>" + ConvertTaskIDToString(TaskList[i].ID) + "<input type=\"hidden\" name=\"i\" value=\"i\">";
      if (TaskList[i].ExectuteAt.Ticks > 0)
        Message += " in " + String(TaskList[i].ExectuteAt.Ticks - millis()) + "ms";
      else
        Message += " at " + String(TaskList[i].ExectuteAt.HH) + ":" + String(TaskList[i].ExectuteAt.MM) + ":" + String(TaskList[i].ExectuteAt.SS) + "";
      Message += " ABC=" + String(TaskList[i].A) + "," + String(TaskList[i].B) + "," + TaskList[i].C + " "
                 "<button>Remove task</button></form>";
    }
  }
  if (Message == "") Message += "No task in the tasklist<br>";

  Message += "<br><form action=\"/settask?\" method=\"set\">"
             "<input type=\"hidden\" name=\"o\" value=\"1\">"
             "<div><label>Task ID </label><input type=\"text\" name=\"i\" value=\"\"> 1=SWITCHMODE, 2=DIMMING, 3=BRIGHTEN, 4=RESETESP, 5=CHANGERGB</div>"
             "ABC = Custom value to give along, for example for DIMMING: Stepsize,GoTo,TimeInterfall in ms"
             "<div><label>ABC </label><input type=\"text\" name=\"a\"><input type=\"text\" name=\"b\"><input type=\"text\" name=\"c\"></div>"
             "<div><label>Time h:m:s</label><input type=\"number\" name=\"h\" min=\"0\" max=\"24\"><input type=\"number\" name=\"m\" min=\"0\" max=\"59\"><input type=\"number\" name=\"s\" min=\"0\" max=\"59\"></div>"
             "<div><label>Time in ms from now</label><input type=\"number\" name=\"t\"> (will clear h:m:s time)</div>"
             "<button>Add task</button></form>";

  Message += "<br>Current time is " + String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS);

  if (ERRORMSG != "")
    server.send(400, "text/html", "<html>" + ERRORMSG + "<br><br>" + Message + "<html>");
  else
    server.send(200, "text/html", "<html>" + Message + "<html>");

#ifdef Server_SerialEnabled
  Serial.println("SV: 200/400 Tasks_Connect " + ERRORMSG + Message);
#endif //Server_SerialEnabled
}
void Tasks_handle_Settings() {
  String ERRORMSG = "";
  if (server.args() > 0) {                      //If manual time given
    TASK TempTask;
    byte TaskCommand = 0;
    for (int i = 0; i < server.args(); i++) {
      String ArguName = server.argName(i);
      ArguName.toLowerCase();
      String ArgValue = server.arg(i);
      if (ArguName == PreFixMode) {
        TaskCommand = ArgValue.toInt();
      } else if (ArguName == PreFixID) {
        TempTask.ID = ConvertTaskIDToInt(ArgValue);
      } else if (ArguName == PreFixA) {
        TempTask.A = ArgValue.toInt();
      } else if (ArguName == PreFixB) {
        TempTask.B = ArgValue.toInt();
      } else if (ArguName == PreFixC) {
        TempTask.C = ArgValue;
      } else if (ArguName == PreFixTimeS) {
        TempTask.ExectuteAt.SS = ArgValue.toInt();
      } else if (ArguName == PreFixTimeM) {
        TempTask.ExectuteAt.MM = ArgValue.toInt();
      } else if (ArguName == PreFixTimeH) {
        TempTask.ExectuteAt.HH = ArgValue.toInt();
      } else if (ArguName == PreFixTimeT) {
        TempTask.ExectuteAt.Ticks = ArgValue.toInt();
      } else
        ERRORMSG += "Unknown arg '" + ArguName + "' with value '" + ArgValue + "'\n";
    }
    if (TempTask.ID == 0) {
      ERRORMSG += "No Task ID given\n";
    } else {
      switch (TaskCommand) {
        case 0: //Unknown ID
          ERRORMSG += "No command given\n";
          break;
        case 1: //Add
          if (TempTask.ExectuteAt.Ticks == 0 and TempTask.ExectuteAt.SS == 0 and TempTask.ExectuteAt.MM == 0 and TempTask.ExectuteAt.HH == 0) {
            ERRORMSG += "No Task time given\n";
          } else {
            if (TempTask.ExectuteAt.Ticks != 0) {     //If we have a delay, not a alarm based on time
              TempTask.ExectuteAt.Ticks += millis();  //Set the delay to be relative from now
              TempTask.ExectuteAt.SS = 0;             //Reset the time, we dont use these
              TempTask.ExectuteAt.MM = 0;
              TempTask.ExectuteAt.HH = 0;
            } else if (!TimeSet)
              ERRORMSG += "Warning time is not synced\n";
            if (!AddTask(TempTask))
              ERRORMSG += "Could not add tasks\n";
          }
          break;
        case 2: //Remove
          if (!RemoveTask(TempTask.ID))
            ERRORMSG += "Could not find task " + String(TempTask.ID) + " in the tasklist\n";
          break;
      }
    }
  }
  if (ERRORMSG != "")
    server.send(400, "text/plain", ERRORMSG);
  else
    server.send(200, "text/plain", "OK");

#ifdef Server_SerialEnabled
  Serial.println("SV: 200/400 Tasks_Settings " + ERRORMSG);
#endif //Server_SerialEnabled
}
