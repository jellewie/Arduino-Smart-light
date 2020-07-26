/* Written by JelleWho https://github.com/jellewie
  On setup() execute        SetupTask()
  On loop() execute         ExecuteTask()

  HOW TO ADD CUSTOM TASKS
  -"enum xxx"      Add the name to the enum to make the code better to read
  -SetupTask()     [optional] If needed configure set-up stuff
  -DoTask()        Add what this task does
*/

#define TaskLimit 16        //defined as an byte in for loops, so 255 at max
#define Task_SerialEnabled
enum {NONE, SWITCHMODE, DIMMING, BRIGHTEN}; //Just to make the code easier to read, this replaces the

void SetupTask() {
  //For exmaple to set-up some pinmodes
}
struct TASK {
  byte ID = 0;                  //the ID of the task, these are defined in the enum
  byte VariableA = 0;           //If you want to add some data to the task, for example a pin number
  byte VariableB = 0;           //If you want to add even more data to the task, for example a pin state
  int  VariableC = 0;           //If you want to add even more data to the task, for example a time interfal
  unsigned long ExectuteAt = 0; //The time in millis to execute this task
};
TASK TaskList[TaskLimit];
bool AddTask(TASK Item) {
  for (int i = 0; i < TaskLimit; i++) { //For each pin that is allowed
    if (TaskList[i].ID == 0) {          //If there is no task
      TaskList[i] = Item;
#ifdef Task_SerialEnabled
      Serial.println("T:" + String(i) + " add " + String(TaskList[i].ID) + " in " + String(TaskList[i].ExectuteAt - millis()) + "ms");
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
  Serial.println("T: DoTask " + String(Item.ID) + " a=" + String(Item.VariableA) + " b=" + String(Item.VariableB) + " at=" + String(Item.ExectuteAt) + " now=" + String(millis()));
#endif //Task_SerialEnabled
  switch (Item.ID) {
    case SWITCHMODE:
      Serial.print("SWITCHMODE");
      Mode = ConvertModeToInt(String(Item.VariableA));
      break;
    case DIMMING: {
        //VariableA = Stepsize
        //VariableB = GoTo
        //VariableC = TimeInterfal
#ifdef Task_SerialEnabled
        Serial.println("DIMMING from " + String(FastLED.getBrightness()) + " a " + String(Item.VariableA) + " a " + String(Item.VariableB) + " a " + String(Item.VariableC));
#endif //Task_SerialEnabled

        byte BRI = FastLED.getBrightness();

        if (BRI - Item.VariableA < Item.VariableB) {
          BRI = Item.VariableB;
        } else {
          BRI = BRI - Item.VariableA;
          if (Item.VariableC > 0) {     //If we need to go further, and need to create another task
            TASK TempTask = Item;
            TempTask.ExectuteAt = millis() + Item.VariableC;
            AddTask(TempTask);
          }
        }
        FastLED.setBrightness(BRI);
        UpdateLEDs = true;
      }
      break;
    case BRIGHTEN: {
        //VariableA = Stepsize
        //VariableB = GoTo
        //VariableC = TimeInterfal
        byte Now = FastLED.getBrightness();
        byte To = 255;
        if (Item.VariableB > 0) To = Item.VariableB;
        if (To - Now < Item.VariableA) {
          Now = To;
        } else {
          Now += Item.VariableA;
          if (Item.VariableC > 0) {
            TASK TempTask = Item;
            TempTask.ExectuteAt = millis() + Item.VariableC;
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
      if (TaskList[i].ExectuteAt <= millis()) { //If the execute time has pasted
        DoTask(TaskList[i]);                    //Execute this task entry
        TaskList[i].ID = 0;                     //Clear this task entry
      }
    }
  }
}
String GetTaskList() {
  String R;
  for (byte i = 0; i < TaskLimit; i++) {        //For each task in the list
    if (TaskList[i].ID > 0) {                   //If there is a task
      if (R != "") R += "\n";               //If there is already an entry, start a new line
      R += "T:" + String(i) + " ID=" + String(TaskList[i].ID) + " in " + String(TaskList[i].ExectuteAt - millis()) + "ms with values " + String(TaskList[i].VariableA) + " and " + String(TaskList[i].VariableB) + " and " + String(TaskList[i].VariableC);
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
