/* Written by JelleWho https://github.com/jellewie
  On loop() execute         ExecuteTask()

  HOW TO ADD CUSTOM TASKS
  -"enum xxx"      Add the name to the enum to make the code better to read
  -"TaskString[]"  ^
  -DoTask()        Add what this task does

  Time based tasks will not be auto deleted (execte X at HH:MM:SS)
  Delay based tasks will be auto deleted (execte X in Y ticks)
*/

#define TaskLimit 16        //defined as an byte in for loops, so 255 at max
#define Task_SerialEnabled
enum {NONE, SWITCHMODE, DIMMING, BRIGHTEN, RESETESP, CHANGERGB, SAVEEEPROM}; //Just to make the code easier to read
String TaskString[] = {"NONE", "SWITCHMODE", "DIMMING", "BRIGHTEN", "RESETESP", "CHANGERGB", "SAVEEEPROM"}; //ALL CAPS!

const byte Task_Amount = sizeof(ModesString) / sizeof(ModesString[0]); //Why filling this in if we can automate that? :)

//<ip>/task[?PreFix=Value][&....]     //These are currently HARDCODED into the HTML page, so shouldn't be changed if you want to use the webpage
#define PreFixMode  "o"   //1=Add 2=Remove
#define PreFixID    "i"   //Task ID
#define PreFixVar   "a"   //Task Variable
#define PreFixTimeH "h"   //On which time the task needs to be executed
#define PreFixTimeM "m"   //^
#define PreFixTimeS "s"   //^
#define PreFixTimeT "t"   //In howmany ticks the task needs to be executed

struct TASK {
  byte ID = 0;                     //The ID of the task, these are defined in the enum
  String Var = "";                 //Used for some special variables
  TimeS ExectuteAt = {0, 0, 0, 0}; //The time in {HH:MM:SS:or millis} to execute this task
  bool Executed = false;           //Just here to keep track of it a Time based task has been executed today
};
TASK TaskList[TaskLimit];

//Declare some functions here, so we can cross reference then in the code. Basically just placeholders to the compiler does not whine about that they do not exist yet
extern String ConvertTaskIDToString(byte IN);
extern int    ConvertTaskIDToInt(String IN);
extern void   CutVariable(String _Input, String *_Variable, byte _VariableLength);
extern bool   AddTask(TASK Item);

bool DoTask(TASK Item) {
  bool returnValue = true;
#ifdef Task_SerialEnabled
  Serial.print("T: DoTask " + ConvertTaskIDToString(Item.ID) + " var=" + String(Item.Var));
  if (Item.ExectuteAt.Ticks > 0)
    Serial.println(" due to Ticks, scheduled for " + String(Item.ExectuteAt.Ticks) + " now=" + String(millis()));
  else
    Serial.println(" due to Time, scheduled for " + String(Item.ExectuteAt.HH) + ":" + String(Item.ExectuteAt.MM) + ":" + String(Item.ExectuteAt.SS));
#endif //Task_SerialEnabled
  switch (Item.ID) {
    case SWITCHMODE: {
        //Var = New Mode
        Mode = ConvertModeToInt(Item.Var);
        UpdateLEDs = true;
      } break;
    case DIMMING: {
        //Var = Stepsize,GoTo,TimeInterfall in ms
        String _Vars[3];
        CutVariable(Item.Var, &_Vars[0], 3);
        byte Stepsize = constrain(_Vars[0].toInt(), 0, 255);
        byte GoTo     = constrain(_Vars[1].toInt(), 0, 255);
        int TimeInterfall       = _Vars[2].toInt();
        byte BRI = FastLED.getBrightness();
        if (BRI - Stepsize < GoTo) {
          BRI = GoTo;
        } else {
          BRI = BRI - Stepsize;
          if (TimeInterfall > 0) {     //If we need to go further, and need to create another task
            TASK TempTask = Item;
            TempTask.ExectuteAt.Ticks = millis() + TimeInterfall;
            AddTask(TempTask);
          }
        }
        FastLED.setBrightness(BRI);
        UpdateLEDs = true;
      } break;
    case BRIGHTEN: {
        //Var = Stepsize,GoTo,TimeInterfall in ms
        String _Vars[3];
        CutVariable(Item.Var, &_Vars[0], 3);
        byte Stepsize = constrain(_Vars[0].toInt(), 0, 255);
        byte GoTo     = constrain(_Vars[1].toInt(), 0, 255);
        int TimeInterfall       = _Vars[2].toInt();
        byte Now = FastLED.getBrightness();
        byte To = 255;
        if (GoTo > 0) To = GoTo;
        if (To - Now < Stepsize) {
          Now = To;
        } else {
          Now += Stepsize;
          if (TimeInterfall > 0) {
            TASK TempTask = Item;
            TempTask.ExectuteAt.Ticks = millis() + TimeInterfall;
            AddTask(TempTask);
          }
        }
#ifdef Task_SerialEnabled
        Serial.println("T: BRIGHTEN from " + String(FastLED.getBrightness()) + " to " + String(Now));
#endif //Task_SerialEnabled
        FastLED.setBrightness(Now);
        UpdateLEDs = true;
      } break;
    case RESETESP: {
        ESP.restart();
      } break;
    case CHANGERGB: {
        //Var = new Red value,new Green value,new Blue value
        String _Vars[3];
        CutVariable(Item.Var, &_Vars[0], 3);
        byte GoToR = constrain(_Vars[0].toInt(), 0, 255);
        byte GoToG = constrain(_Vars[1].toInt(), 0, 255);
        byte GoToB = constrain(_Vars[2].toInt(), 0, 255);
        Mode = ON;
        LastMode = Mode;
        fill_solid(&(LEDs[0]), TotalLEDs, CRGB(GoToR, GoToG, GoToB));
        UpdateLEDs = true;
      } break;
    case SAVEEEPROM: {
        WiFiManager_WriteEEPROM();
      } break;
    default:
      returnValue = false;
      break;
  }
  return returnValue;
}

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
void CutVariable(String _Input, String *_Variable, byte _VariableLength) {
  //Takes in a string, and cuts them into parts; "test,yes,clock" => {"test","yes","clock"}
  //Returns the output in the same string, for example
  //  String _Output[3], _Input = "test,yes,good,no,what";
  //  CutVariable(_Input, &_Output[0], 3);
  //  Serial.println(String(_Output[0]) + "_" + String(_Output[1]) + "_" + String(_Output[2]));
  byte _StartAt = 0, _WriteTo = 0;
  for (byte i = 0; i <= _Input.length(); i++) { //For each character in the input string
    if (_Input.charAt(i) == ',') {
      _Variable[_WriteTo] = _Input.substring(_StartAt, i);
      _WriteTo ++;
      _StartAt = i + 1;
      if (_WriteTo >= _VariableLength - 1) break;       //If last one
    }
  }
  _Variable[_WriteTo] = _Input.substring(_StartAt);
}
bool AddTask(TASK Item) {
#ifdef Task_SerialEnabled
  Serial.print("T: AddTask " + String(Item.ID) + " Var=" + String(Item.Var));
  if (Item.ExectuteAt.Ticks > 0) Serial.println(" in " + String(Item.ExectuteAt.Ticks - millis()));
  else Serial.println(" at " + String(Item.ExectuteAt.HH) + ":" + String(Item.ExectuteAt.MM) + ":" + String(Item.ExectuteAt.SS));
#endif //Task_SerialEnabled
  for (int i = 0; i < TaskLimit; i++) { //For each task that is allowed
    if (TaskList[i].ID == 0) {          //If there is no task
      TaskList[i] = Item;
      TaskList[i].Executed = false;     //flag this task as not executed
      return true;
    }
  }
#ifdef Task_SerialEnabled
  Serial.println("TaskList full, couldn't add new task");
#endif //Task_SerialEnabled
  return false;
}
bool RemoveTask(byte i) {
  //Remove a task by its nummer in the TaskList
  if (TaskList[i].ID > 0) {                   //If there is a task
    TaskList[i].ID = 0;                       //Clear this task entry
#ifdef Task_SerialEnabled
    Serial.print("T: RemoveTask " + String(i));
#endif //Task_SerialEnabled
    return true;                              //EXIT, task removed
  }
  return false;
}
void RemoveTasksByID(byte ID) {
  //Remove ALL tasks with a sertain ID
  if (ID == 0) return;
  for (byte i = 0; i < TaskLimit; i++) {      //For each task in the list
    if (TaskList[i].ID == ID) {               //If there is a task
      TaskList[i].ID = 0;                     //Clear this task entry
#ifdef Task_SerialEnabled
      Serial.print("T: RemoveTasksByID " + String(i));
#endif //Task_SerialEnabled
    }
  }
}
void ExecuteTask() {
  for (int i = 0; i < TaskLimit; i++) {                       //For each task in the list
    if (TaskList[i].ID > 0) {                                 //If there is a task
      if (TaskList[i].ExectuteAt.Ticks > 0) {
        if (TaskList[i].ExectuteAt.Ticks <= millis()) {       //If the execute time has pasted
          DoTask(TaskList[i]);                                //Execute this task entry
          RemoveTask(i);                                      //Remove the task, it has been executed
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
                 "<label>Do task </label>" + ConvertTaskIDToString(TaskList[i].ID) + "<input type=\"hidden\" name=\"i\" value=\"" + i + "\">";
      if (TaskList[i].ExectuteAt.Ticks > 0)
        Message += " in " + String(TaskList[i].ExectuteAt.Ticks - millis()) + "ms";
      else
        Message += " at " + String(TaskList[i].ExectuteAt.HH) + ":" + String(TaskList[i].ExectuteAt.MM) + ":" + String(TaskList[i].ExectuteAt.SS) + "";
      Message += " Var=" + String(TaskList[i].Var) + " <button>Remove task</button></form>";
    }
  }
  if (Message == "") Message += "No task in the tasklist<br>";

  Message += "<br><form action=\"/settask?\" method=\"set\">"
             "<input type=\"hidden\" name=\"o\" value=\"1\">"
             "<div><label>Task ID </label><input type=\"text\" name=\"i\" value=\"\"> 1=SWITCHMODE, 2=DIMMING, 3=BRIGHTEN, 4=RESETESP, 5=CHANGERGB</div>"
             "<div><label>Var </label><input type=\"text\" name=\"a\">Custom value to give along, for example for DIMMING:(Stepsize,GoTo,TimeInterfall in ms) OR SWITCHMODE:(NewModeIdOrName)</div>"
             "<div><label>Time h:m:s </label><input type=\"number\" name=\"h\" min=\"0\" max=\"24\"><input type=\"number\" name=\"m\" min=\"0\" max=\"59\"><input type=\"number\" name=\"s\" min=\"0\" max=\"59\"><label> or time in ms from now:</label><input type=\"number\" name=\"t\"> (will clear h:m:s time)</div>"
             "<button>Add task</button></form>";

  Message += "Current time is " + String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS);

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
    TempTask.ExectuteAt.HH = -1;
    TempTask.ExectuteAt.MM = -1;
    TempTask.ExectuteAt.SS = -1;
    TempTask.ExectuteAt.Ticks = 0;
    TempTask.ID = -1;
    byte TaskCommand = 0;
    for (int i = 0; i < server.args(); i++) {
      String ArguName = server.argName(i);
      ArguName.toLowerCase();
      String ArgValue = server.arg(i);
      if (ArgValue != "") {
        if (ArguName == PreFixMode) {
          TaskCommand = ArgValue.toInt();
        } else if (ArguName == PreFixID) {
          TempTask.ID = ConvertTaskIDToInt(ArgValue);
        } else if (ArguName == PreFixVar) {
          TempTask.Var = ArgValue;
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
    }
    if (TempTask.ID == -1) {
      ERRORMSG += "No Task ID given\n";
    } else {
      switch (TaskCommand) {
        case 0: //Unknown ID
          ERRORMSG += "No command given\n";
          break;
        case 1: //Add
          if (TempTask.ExectuteAt.Ticks == 0 and TempTask.ExectuteAt.HH > 24 and TempTask.ExectuteAt.MM > 60 and TempTask.ExectuteAt.SS > 60) {
            ERRORMSG += "No (proper) Task time given\n";
          } else {
            if (TempTask.ExectuteAt.Ticks != 0) {     //If we have a delay, not a alarm based on time
              TempTask.ExectuteAt.Ticks += millis();  //Set the delay to be relative from now
              TempTask.ExectuteAt.HH = 0;             //Reset the time, we dont use these
              TempTask.ExectuteAt.MM = 0;
              TempTask.ExectuteAt.SS = 0;
            } else {
              if (!TimeSet)
                ERRORMSG += "Warning time is not synced\n";
              if (TempTask.ExectuteAt.HH > 24) TempTask.ExectuteAt.HH = 0;
              if (TempTask.ExectuteAt.MM > 60) TempTask.ExectuteAt.MM = 0;
              if (TempTask.ExectuteAt.SS > 60) TempTask.ExectuteAt.SS = 0;
            }
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
