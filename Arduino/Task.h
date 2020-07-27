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
enum {NONE, SWITCHMODE, DIMMING, BRIGHTEN}; //Just to make the code easier to read, this replaces the

void SetupTask() {
  //For exmaple to set-up some pinmodes
}
struct TASK {
  byte ID = 0;                  //the ID of the task, these are defined in the enum
  byte A = 0;                   //If you want to add some data to the task, for example a pin number
  byte B = 0;                   //If you want to add even more data to the task, for example a pin state
  int  C = 0;                   //If you want to add even more data to the task, for example a time interfal
  TimeS ExectuteAt = {0, 0, 0, 0}; //The time in millis to execute this task
  bool Executed = false;        //Just here to keep track of it a Time based task has been executed today
};

TASK TaskList[TaskLimit];
bool AddTask(TASK Item) {
  for (int i = 0; i < TaskLimit; i++) { //For each pin that is allowed
    if (TaskList[i].ID == 0) {          //If there is no task
      TaskList[i] = Item;
#ifdef Task_SerialEnabled
      Serial.println("T:" + String(i) + " add " + String(TaskList[i].ID) + " in " + String(TaskList[i].ExectuteAt.Ticks - millis()) + "ms");
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
  Serial.print("T: DoTask " + String(Item.ID) + " a=" + String(Item.A) + " b=" + String(Item.B) + " c=" + String(Item.C));
  if (Item.ExectuteAt.Ticks > 0)
    Serial.println(" due to Ticks, scheduled for " + String(Item.ExectuteAt.Ticks) + " now=" + String(millis()));
  else
    Serial.println(" due to Time, scheduled for " + String(Item.ExectuteAt.HH) + " : " + String(Item.ExectuteAt.MM) + " : " + String(Item.ExectuteAt.SS));
#endif //Task_SerialEnabled
  switch (Item.ID) {
    case SWITCHMODE:
      Serial.print("SWITCHMODE");
      Mode = ConvertModeToInt(String(Item.A));
      break;
    case DIMMING: {
        //A = Stepsize
        //B = GoTo
        //C = TimeInterfal
#ifdef Task_SerialEnabled
        Serial.println("DIMMING from " + String(FastLED.getBrightness()) + " a " + String(Item.A) + " a " + String(Item.B) + " a " + String(Item.C));
#endif //Task_SerialEnabled

        byte BRI = FastLED.getBrightness();

        if (BRI - Item.A < Item.B) {
          BRI = Item.B;
        } else {
          BRI = BRI - Item.A;
          if (Item.C > 0) {     //If we need to go further, and need to create another task
            TASK TempTask = Item;
            TempTask.ExectuteAt.Ticks = millis() + Item.C;
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
        //C = TimeInterfal
        byte Now = FastLED.getBrightness();
        byte To = 255;
        if (Item.B > 0) To = Item.B;
        if (To - Now < Item.A) {
          Now = To;
        } else {
          Now += Item.A;
          if (Item.C > 0) {
            TASK TempTask = Item;
            TempTask.ExectuteAt.Ticks = millis() + Item.C;
            AddTask(TempTask);
          }
        }
#ifdef Task_SerialEnabled
        Serial.println("BRIGHTEN from " + String(FastLED.getBrightness()) + " to " + String(Now));
#endif //Task_SerialEnabled
        FastLED.setBrightness(Now);
        UpdateLEDs = true;
      }
      break;
    default:
      returnValue = false;
      break;
  }
  return returnValue;
}
void ExecuteTask() {
  for (int i = 0; i < TaskLimit; i++) {         //For each task in the list
    if (TaskList[i].ID > 0) {                   //If there is a task
      if (TaskList[i].ExectuteAt.Ticks > 0) {
        if (TaskList[i].ExectuteAt.Ticks <= millis()) { //If the execute time has pasted
          DoTask(TaskList[i]);                    //Execute this task entry
          TaskList[i].ID = 0;                     //Clear this task entry
        }
      } else if (TimeSet and (TaskList[i].ExectuteAt.SS > 0 or TaskList[i].ExectuteAt.MM > 0 or TaskList[i].ExectuteAt.HH > 0)) {

        if (TaskList[i].ExectuteAt.HH < TimeCurrent.HH - 12)  //If we are more than 12 hours away
          TaskList[i].Executed = false;                       //flag this task as not executed

        if (!TaskList[i].Executed and                         //If the task has not been executed yet, and it it time for it to do so
            TaskList[i].ExectuteAt.SS < TimeCurrent.SS and
            TaskList[i].ExectuteAt.MM < TimeCurrent.MM and
            TaskList[i].ExectuteAt.HH == TimeCurrent.HH) {
          DoTask(TaskList[i]);                    //Execute this task entry
          TaskList[i].Executed = true;
        }
      }
    }
  }
}
String GetTaskList() {
  String R;
  for (byte i = 0; i < TaskLimit; i++) {        //For each task in the list
    if (TaskList[i].ID > 0) {                   //If there is a task
      if (R != "") R += "\n";               //If there is already an entry, start a new line
      R += "T : " + String(i) + " ID = " + String(TaskList[i].ID) + " in " + String(TaskList[i].ExectuteAt.Ticks - millis()) + "ms with values " + String(TaskList[i].A) + " and " + String(TaskList[i].B) + " and " + String(TaskList[i].C);
    }
  }
  if (R == "") R = "No task in the tasklist";
  return R;
}
bool RemoveTask(byte ID) {
  if (TaskList[ID].ID > 0) {                   //If there is a task
    TaskList[ID].ID = 0;                       //Clear this task entry
    return true;                              //EXIT, task removed
  }
  return false;
}
