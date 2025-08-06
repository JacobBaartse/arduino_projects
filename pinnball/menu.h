#include "EEPROM.h"

// user status
#define ACTIVE 1
#define INACTIVE 2
#define FREE 0
#define DELETED_USER 3

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
  if (readObject.status==ACTIVE)        return "Name: " + String(readObject.name) + "\nScore: " + String(readObject.score) + "\nStatus:  Active";
  else if (readObject.status==INACTIVE) return "Name: " + String(readObject.name) + "\nScore: " + String(readObject.score) + "\nStatus:Inactive";
  else if (readObject.status==DELETED_USER)  return "Name: " + String(readObject.name) + "\nScore: " + String(readObject.score) + "\nStatus: Deleted";
  else                                  return "Name: " + String(readObject.name) + "\nScore: " + String(readObject.score) + "\nStatus:" + String(readObject.status, HEX);
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
    else if (readObject.status == ACTIVE || all_info)
      if (readObject.status != DELETED_USER) {
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
  UserObject l_readObject;
  EEPROM.get(idx_current_player, l_readObject);  
  if (score_counter > readObject.score){
    readObject.score = score_counter;
    EEPROM.put(idx_current_player, l_readObject);
  }
}

String get_top_scores(){
  UserObject top[7];
  for (int j=0; j<7; j++){
    top[j].score = -1;
  }
  for (int i=0; i<1000; i+=16){
    UserObject l_readObject;
    EEPROM.get(i, l_readObject);
    if (l_readObject.status==FREE) {
      String top_score_string = "";
      for (int j=0; j<6; j++)
        if (top[j].score >=0) 
          top_score_string += String(top[j].name) + " " + top[j].score + "\n";
      top_score_string += String("-- ") + readObject.name + " " + readObject.score + " --";  // at the bottom add current user.
      return top_score_string;
    }
    else{
      if (l_readObject.status == ACTIVE)
      {
        for (int j=0; j<6; j++){
          if ((l_readObject.score > top[j].score)){  // always show the current player
            int tmp_score = top[j].score;
            char tmp_name[10];
            strcpy(tmp_name, top[j].name);
            top[j].score = l_readObject.score;
            strcpy(top[j].name, l_readObject.name);
            l_readObject.score = tmp_score;
            strcpy(l_readObject.name, tmp_name);
          }
        }
      }
    }
  }
}


int find_free_slot(){
  for (int i=0; i<1000; i+=16){
    UserObject l_readObject;
    EEPROM.get(i, l_readObject);
    debug("find free slot Object status: ");
    debugln(readObject.status);
    if ((l_readObject.status==0) || (l_readObject.status ==0xff))
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
  debugln("add user");
  String result;
  String user = onScreenKeyboard_get_string(true, "", "add user name");
  debugln(user);
  while (true){
    int selected_line = show_menu_on_screen(String("Add: ") + user + "\nOk\nCancel");
    switch (selected_line){
      case 1: 
          // store new user
        result = store_new_user(user);
        show_text_on_screen_time(result, 2000);
        return;
        break;
      case 2: 
        show_text_on_screen_time("Canceled", 2000);
        return;
        break;
    }
  }

  return;
}


void edit_user(){
  debugln("edit user");
  idx_current_player = -16;
  get_player(true);
  while (true){
    int selected_line = show_menu_on_screen(get_current_player() + "\n Next\n Save\n Delete");
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
      debugln("edit user next");
      EEPROM.put(idx_current_player, readObject);
      get_player(true);
      break;
    case 4:
      debugln("edit user save");
      EEPROM.put(idx_current_player, readObject);
      get_player(true);
      break;
    case 5:
      debugln("delete user");
      if (readObject.status == DELETED_USER) readObject.status = ACTIVE;
      else readObject.status = DELETED_USER;
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
      case 0: return;  //back to play
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