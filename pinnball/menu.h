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


String screen_text = "_";
byte menu_state = PLAY_MODE;
bool edit_mode = false;

int idx_current_player=-16;
UserObject readObject;

String get_player()
{
  idx_current_player+=16;
  for (; idx_current_player<1000; idx_current_player+=16){
    EEPROM.get(idx_current_player, readObject);
    if (readObject.status==FREE) idx_current_player=-16;  // next loop will increase to 0 again.
    else if (readObject.status == ACTIVE || menu_state!= PLAY_MODE) {
      debug(idx_current_player);
      debugln(readObject.name);
      return String(readObject.name);
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

String menu_process_key(int keyboard_char){
  debugln("State: " + String(menu_state));
  if (keyboard_char == WINDOWS_KEY){
    edit_mode=false;
    menu_state = MAIN_MENU;
    return "1 play\n2 Add user\n3 Edit user"; //\n4 select users";
  }
  if (menu_state == MAIN_MENU){
    switch(keyboard_char){
      case int('1'):
        menu_state = PLAY_MODE;
      break;
      case int('2'):
        menu_state = ADD_USER;
        edit_mode=true;
        screen_text = "New username:\n_";
        return  "New username:\n_";
      break;
      case int('3'):
        menu_state = SELECT_EDIT_USER;
        return get_player() + String("\nDown for more\nEnter For edit");
      break;
      case int('4'):
        menu_state = SELECT_USERS;
      break;
      case int('5'):
        menu_state = DELETE_USER;
      break;
      case int('0'):
        return get_player().c_str();
      break;      
      // case DOWN_ARROW:
      //   return "5 Delete user";
      // break;
      // case UP_ARROW:
      //   return "1 play\n2 Add user\n3 Edit user\n4 select users";
      // break;
      default:
        menu_state = PLAY_MODE;
    }
  }
  if (menu_state == ADD_USER){
    if (keyboard_char == ENTER_KEY){
      // store new user
      int start_of_user = screen_text.indexOf(ENTER_KEY) + 1;
      String user = screen_text.substring(start_of_user, start_of_user+9);
      user.remove(user.length()-1);
      screen_text = "";
      edit_mode=false;
      menu_state = PLAY_MODE;
      return store_new_user(user);
    }
  }
  if (menu_state == SELECT_EDIT_USER){
    switch(keyboard_char){
      case DOWN_ARROW:
        return get_player() + String("\nDown for more\nEnter For edit");
        break;
      case ENTER_KEY:
        menu_state == EDIT_USER;
        return String("1 Change name\n2 Reset score\n3 Disable user\n4 Enable user"); 
        break;
    }
  }

  if (menu_state == SELECT_EDIT_USER){
    switch(keyboard_char){
      case int('1'):
        menu_state = UPDATE_USER_NAME;
        edit_mode=true;
        screen_text = String("New username:\n_") + readObject.name;
        return  String("New username:\n_") + readObject.name;
        break;
      case int('2'):
        menu_state == PLAY_MODE;
        readObject.score=0;
        EEPROM.put(idx_current_player, readObject);
        return String("Score set to 0"); 
        break;
      case int('3'):
        menu_state == PLAY_MODE;
        readObject.status=INACTIVE;
        EEPROM.put(idx_current_player, readObject);
        return String("State set to inactive"); 
        break;
      case int('4'):
        menu_state == PLAY_MODE;
        readObject.status=ACTIVE;
        EEPROM.put(idx_current_player, readObject);
        return String("State set to active"); 
        break;
    }
  }  
  if (menu_state == UPDATE_USER_NAME){
    if (keyboard_char == ENTER_KEY){
      menu_state == PLAY_MODE;
      // store new username
      int start_of_user = screen_text.indexOf(ENTER_KEY) + 1;
      String user = screen_text.substring(start_of_user, start_of_user+9);
      user.remove(user.length()-1);
      strcpy(readObject.name, user.c_str());
      EEPROM.put(idx_current_player, readObject);
      screen_text = "";
      edit_mode=false;
      return "New name stored.";
    }
  }

  if (edit_mode){
    if (screen_text.length() > 0) screen_text.remove(screen_text.length()-1);  // remove cursor
    if (screen_text.length() > 0 && keyboard_char == BACK_SPACE) screen_text.remove(screen_text.length()-1);
    if ((keyboard_char > 31) && (keyboard_char < 128)) screen_text += (char)keyboard_char;
    screen_text += "_";  // add cursor
    return screen_text;
  }

  return "";
}

#undef debug
#undef debugln