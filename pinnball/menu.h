#include "EEPROM.h"

#define MAIN_MENU  0
#define ADD_USER 1
#define EDIT_USER 2
#define SELECT_USERS 3
#define DELETE_USER 4
#define PLAY_MODE 5
#define SELECT_EDIT_USER 6
#define UPDATE_USER_NAME 7

#define DOWN_ARROW  0xE072
#define UP_ARROW  0xE075
#define ENTER_KEY 0XA
#define BACK_SPACE 0x08
#define WINDOWS_KEY 0xE01F

// user status
#define ACTIVE 1
#define INACTIVE 2
#define FREE 0
#define DELETED 0xff

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif



struct UserObject {
  byte status;
  char name[10];
  long score;
};


int idx_current_player=-16;
UserObject readObject;


String get_current_player(){
  if (readObject.status==ACTIVE) return "Name: " + String(readObject.name) + "\nScore: " + String(readObject.score) + "\nStatus:  Active";
  else                           return "Name: " + String(readObject.name) + "\nScore: " + String(readObject.score) + "\nStatus:Inactive";
}

String get_player(bool all_info)
{
  debugln("get player");
  idx_current_player+=16;
  int count = 0;
  for (; idx_current_player<1000; idx_current_player+=16){
    count+=1;
    debug("idx curreent player");
    debugln(idx_current_player);
    EEPROM.get(idx_current_player, readObject);
    debug(readObject.status);
    if (readObject.status==FREE) idx_current_player=-16;  // next loop will increase to 0 again.
    else if (readObject.status == ACTIVE || all_info) {
      debug(idx_current_player);
      debugln(readObject.name);
      if (all_info) return get_current_player();
      return readObject.name;
    }
    if (count > 63){
      if (all_info) return "No user found\nScore: --\nStatus: --";
      else return "xxx";
    }
  }
}

void store_score(uint32_t score_counter){
  UserObject readObject;
  EEPROM.get(idx_current_player, readObject);  
  if (score_counter > readObject.score){
    readObject.score = score_counter;
    EEPROM.put(idx_current_player, readObject);
  }
}

String get_top_scores(){
  UserObject top[4];
  for (int j=0; j<4; j++){
    top[j].score = -1;
  }
  for (int i=0; i<1000; i+=16){
    UserObject readObject;
    EEPROM.get(i, readObject);
    if ((readObject.status==0) || (readObject.status ==0xff)){
      String top_score_string = "";
      for (int j=0; j<4; j++)
        if (top[j].score >=0) 
          top_score_string += String(top[j].name) + String(" ") + String(top[j].score) + String("\n");
      return top_score_string;
    }
    else{
      if (readObject.status == ACTIVE)
      {
        for (int j=0; j<4; j++){
          if (readObject.score > top[j].score){
            int tmp_score = top[j].score;
            char tmp_name[10];
            strcpy(tmp_name, top[j].name);
            top[j].score = readObject.score;
            strcpy(top[j].name, readObject.name);
            readObject.score = tmp_score;
            strcpy(readObject.name, tmp_name);
          }
        }
      }
    }
  }
}


int find_free_slot(){
  for (int i=0; i<1000; i+=16){
    UserObject readObject;
    EEPROM.get(i, readObject);
    debug("find free slot Object status: ");
    debugln(readObject.status);
    if ((readObject.status==0) || (readObject.status ==0xff))
      return i;
  }
  return -1;
}


String store_new_user(String username){
  int address = find_free_slot();
  if (address >= 0){
    debug("store_new_user : ");
    debugln(username);
    UserObject newUser;
    newUser.status = ACTIVE;
    strcpy(newUser.name, username.c_str());
    newUser.score = 0;
    EEPROM.put(address, newUser);
    return "Stored.";
  }
  else{
    return "failed.";
  }
}


void add_user(){
  Serial.println("add user");
  String user = onScreenKeyboard_get_string(true, "", "add user name");
  Serial.println(user);
  // store new user
  String result = store_new_user(user);
  show_text_on_screen_time(result, 2000);
  return;
}


void edit_user(){
  Serial.println("edit user");
  idx_current_player = -16;
  get_player(true);
  while (true){
    int selected_line = show_menu_on_screen(get_current_player() + "\n next\n save");
    switch (selected_line){
    case 0: 
      strcpy(readObject.name, onScreenKeyboard_get_string(true, "", "new user name").c_str());
      break;
    case 1: 
      readObject.score = 0;
      break;
    case 2: 
      if (readObject.status == ACTIVE) readObject.status = INACTIVE;
      else readObject.status = ACTIVE;
      break;
    case 3:
      Serial.println("edit user next");
      EEPROM.put(idx_current_player, readObject);
      get_player(true);
      break;
    default: 
      EEPROM.put(idx_current_player, readObject);
      return;
    }
  }
}

void do_menu(){
  while(true){
    int selected_line = show_menu_on_screen("play\nAdd user\nEdit user");

    switch (selected_line){
      case 0: return;
      case 1: 
        add_user();
        break;
      case 2: 
        edit_user();
        break;
      default: return;
    }
  }

}

#undef debug
#undef debugln