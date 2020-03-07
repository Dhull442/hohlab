#include "labs/shell.h"
#include "labs/vgatext.h"

// TODO Remove preprocessor directives

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define WHITE 7

static void strcpy(char *source, char *target);
static int max(int a, int b);

//
// initialize shellstate
//
void shell_init(shellstate_t& state){
  state.num_keypresses = 0;
  char heading[] = "SEASHELL";
  strcpy(heading, state.heading);
  for(int i = 0; i < 80; i++) {
    state.contents[0][i] = ' ';
  }
  state.content_ptr = 1;
  state.command_ptr = 0;
  state.f_req = 0;
  state.f_state = 0;
  state.fiber_req = 0;

  // Initialize state for the fiber
  state.h_req = 0;
  state.g_req = 0;
  for (int i = 0; i < 5; i++) {
    state.thread_occupied[i] = false;
  }
  state.num_h = 0;
  state.num_g = 0;
  state.curr_idx = 0;
}

//
// handle keyboard event.
// key is in scancode format.
// For ex:
// scancode for following keys are:
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     | esc |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 |  0 |  - |  = |back|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 01  | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0a | 0b | 0c | 0d | 0e |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     | tab |  q |  w |  e |  r |  t |  y |  u |  i |  o |  p |  [ |  ] |entr|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 0f  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1a | 1b | 1c |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     |ctrl |  a |  s |  d |  f |  g |  h |  j |  k |  l |  ; |  ' |    |shft|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 1d  | 1e | 1f | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 2a |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
// so and so..
//
// - restrict yourself to: 0-9, a-z, esc, enter, arrows
// - ignore other keys like shift, control keys
// - only handle the keys which you're interested in
// - for example, you may want to handle up(0x48),down(0x50) arrow keys for menu.
//

static char getchar(int scankey) {
  const char* top_row = "qwertyuiop";
  const char* middle_row = "asdfghjkl";
  const char* bottom_row = "zxcvbnm";
  const char* numbers = "1234567890";

  if (scankey >= 0x02 && scankey <= 0x0b) {
    return numbers[scankey - 0x02];
  } else if (scankey >= 0x10 && scankey <= 0x19) {
    return top_row[scankey - 0x10];
  } else if (scankey >= 0x1e && scankey <= 0x26) {
    return middle_row[scankey - 0x1e];
  } else if (scankey >= 0x2c && scankey <= 0x32) {
    return bottom_row[scankey - 0x2c];
  } else if (scankey == 0x39) {
    return ' ';
  } else {
    return '?';
  }
}

static bool strcompare(char *s1, char *s2, int length) {
  for(int i = 0; i < length; i++) {
    if (s1[i] != s2[i]) {
      return false;
    }
  }
  return true;
}

void exec_echo(char* command, int command_length, shellstate_t& stateinout) {
  // Append the string to the shell contents
  for(int i = 0; i < command_length; i++) {
    stateinout.contents[stateinout.content_ptr][i] = command[i];
  }
  for(int i = command_length; i < 80; i++) {
    stateinout.contents[stateinout.content_ptr][i] = ' ';
  }
  stateinout.content_ptr++;
}

int read_num(char *num_string, int string_length) {
  int result = 0;
  int ctr = 0;
  while(ctr < string_length && num_string[ctr] != ' ') {
    int tmp = (int)(num_string[ctr] - '0');
    if (tmp >= 0 && tmp <= 9) {
      result = result * 10 + tmp;
    } else {
      return -1;
    }
    ctr++;
  }
  return result;
}

static int int2string(char* input_string, int n) {
  int length = 0;
  while(n > 0) {
    input_string[10 - (++length)] = '0' + (n % 10);
    n /= 10;
  }
  return length;
}

void exec_invalid(shellstate_t& stateinout) {
  char invalid_msg[] = "ERROR: Invalid command";
  int ctr = 0;
  while(invalid_msg[ctr] != '\0') {
    stateinout.contents[stateinout.content_ptr][ctr] = invalid_msg[ctr];
    ctr++;
  }
  for(int i = ctr; i < 80; i++) {
    stateinout.contents[stateinout.content_ptr][i] = ' ';
  }
  stateinout.content_ptr++;
}

void exec_fib(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  hoh_debug(num);
  if (num <= 0) {
    hoh_debug("Invalid fib args");
    char error_msg[] = "Invalid argument to fib. Enter integer between 1 and 46.";
    int error_msg_length = 56;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  } else if (num > 46) {
    // Send overflow message
    char error_msg[] = "ERROR: integer overflow. Argument to fib should be atmost 46.";
    int error_msg_length = 61;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }

  // Compute the fibonacci number
  unsigned int ans = -1, ans_1 = -1;
  if (num <= 2) {
    ans = 1;
  } else {
    ans = 1;
    ans_1 = 1;
    for(int i = 3; i <= num; i++) {
      unsigned int tmp = ans + ans_1;
      ans_1 = ans;
      ans = tmp;
    }
  }

  // Print ans on the terminal

  char ans_string[10];
  int length = int2string(ans_string, ans);

  // Print the answer string to the terminal
  for(int i = 10 - length; i < 10; i++) {
    stateinout.contents[stateinout.content_ptr][i - 10 + length] = ans_string[i];
  }
  for(int i = length; i < 80; i++) {
    stateinout.contents[stateinout.content_ptr][i] = ' ';
  }
  stateinout.content_ptr++;
}

void exec_recfib(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  hoh_debug(num);
  if (num <= 0) {
    hoh_debug("Invalid fib args");
    char error_msg[] = "Invalid argument to fib. Enter integer between 1 and 46.";
    int error_msg_length = 56;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  } else if (num > 46) {
    // Send overflow message
    char error_msg[] = "ERROR: integer overflow. Argument to fib should be atmost 46.";
    int error_msg_length = 61;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  } else if (stateinout.num_g >= 3 || stateinout.num_h + stateinout.num_g >= 5 || stateinout.g_req == 1) {
    char error_msg[] = "ERROR: Too many commands are already in the queue";
    int error_msg_length = 49;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }

  // Request the g function
  stateinout.g_req = 1;
  stateinout.g_arg = num;
}

void exec_consume3(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  if (num <= 0 || num >= 2000) {
    char error_msg[] = "ERROR: Invalid arguments to consume3. Please enter an integer between 1 and 2000.";
    int error_msg_length = 80;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  } else if (stateinout.fiber_state != 0 || stateinout.fiber_req != 0) {
    char error_msg[] = "ERROR: A previous consume3 command is already running.";
    int error_msg_length = 54;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }

  // Activate the state machine
  stateinout.fiber_req = 1;
  stateinout.fiber_arg = num;
}

void exec_consume4(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  if (num <= 0 || num >= 2000) {
    char error_msg[] = "ERROR: Invalid arguments to consume3. Please enter an integer between 1 and 2000.";
    int error_msg_length = 80;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  } else if (stateinout.num_h >= 3 || stateinout.num_h + stateinout.num_g >= 5 || stateinout.h_req == 1) {
    char error_msg[] = "ERROR: Too many commands are already in the queue";
    int error_msg_length = 49;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }

  // Request a fiber for consume4
  stateinout.h_req = 1;
  stateinout.h_arg = num;
}

void exec_consume2(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  if (num <= 0 || num >= 2000) {
    char error_msg[] = "ERROR: Invalid arguments to consume2. Please enter an integer between 1 and 2000.";
    int error_msg_length = 80;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  } else if (stateinout.f_state != 0 || stateinout.f_req != 0) {
    char error_msg[] = "ERROR: A previous consume2 command is already running.";
    int error_msg_length = 54;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }

  // Activate the state machine
  stateinout.f_req = 1;
  stateinout.f_arg = num;
}


void exec_consume(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  hoh_debug(num);
  if (num <= 0 || num >= 2000) {
    char error_msg[] = "ERROR: Invalid arguments to consume. Please enter an integer between 1 and 2000.";
    int error_msg_length = 80;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }
  int ans=1;
  int mod = 10000007;
  for(int i=0;i<num;i++){
    for(int j=0;j<num;j++){
      for(int k=0;k<num;k++){
        ans = 2*ans % mod;
      }
    }
  }

  // Print ans on the terminal

  char ans_string[10];
  int length = int2string(ans_string, ans);

  // Print the answer string to the terminal
  for(int i = 10 - length; i < 10; i++) {
    stateinout.contents[stateinout.content_ptr][i - 10 + length] = ans_string[i];
  }
  for(int i = length; i < 80; i++) {
    stateinout.contents[stateinout.content_ptr][i] = ' ';
  }
  stateinout.content_ptr++;
}

void exec_prime(char* command, int command_length, shellstate_t& stateinout) {
  int num = read_num(command, command_length);
  hoh_debug(num);
  if (num <= 0 || num > 2000) {
    char error_msg[] = "ERROR: Invalid arguments to prime. Please enter an integer between 1 and 2000.";
    int error_msg_length = 78;
    exec_echo(error_msg, error_msg_length, stateinout);
    return;
  }
  // Compute the nth prime number
  int ans = num;
  if(num == 1) {
    ans = 2;
  } else {
    int primes[num + 1];
    primes[1] = 2;
    int nextind = 2;
    for(int i=3;;i++){
      bool isprime = true;
      for(int j=1;j<nextind && isprime;j++){
        if(i%primes[j]==0) {
          hoh_debug(i);
          isprime=false;
        }
      }
      if(isprime){
        primes[nextind] = i;
        nextind++;
        if(nextind == num + 1){
          ans = primes[num];
          break;
        }
      }
    }
  }


  // Print ans on the terminal

  char ans_string[10];
  int length = int2string(ans_string, ans);

  // Print the answer string to the terminal
  for(int i = 10 - length; i < 10; i++) {
    stateinout.contents[stateinout.content_ptr][i - 10 + length] = ans_string[i];
  }
  for(int i = length; i < 80; i++) {
    stateinout.contents[stateinout.content_ptr][i] = ' ';
  }
  stateinout.content_ptr++;
}

void exec_welcome(shellstate_t& stateinout) {
  // Print the welcome text
  char greeting_text[] = "Welcome to";
  int greeting_text_size = 10;
  exec_echo(greeting_text, greeting_text_size, stateinout);
  char welcome_text[45*5 + 1] = "eeeee eeee eeeee eeeee e   e eeee e     e    8     8    8   8 8     8   8 8    8     8    8eeee 8eee 8eee8 8eeee 8eee8 8eee 8e    8e      88 88   88  8    88 88  8 88   88    88   8ee88 88ee 88  8 8ee88 88  8 88ee 88eee 88eee";
  for(int i = 0; i < 5; i++) {
    exec_echo(welcome_text + i * 45, 45, stateinout);
  }
  char help_text[] = "You can try the commands";
  int help_text_size = 24;
  exec_echo(help_text, help_text_size, stateinout);
  char echo_text[] = "echo <string> - prints string to console";
  int echo_text_size = 40;
  exec_echo(echo_text, echo_text_size, stateinout);
  char fib_text[] = "fib <integer N> - prints the Nth fibonnaci number";
  int fib_text_size = 49;
  exec_echo(fib_text, fib_text_size, stateinout);
  char prime_text[] = "prime <integer N> - prints the Nth prime number";
  int prime_text_size = 47;
  exec_echo(prime_text, prime_text_size, stateinout);
  char consume_text[] = "consume <integer N> - consumes time n^3";
  int consume_text_size = 39;
  exec_echo(consume_text, consume_text_size, stateinout);
}

void exec(char* command, int command_length, shellstate_t& stateinout) {
  // Declare the commands
  char echo[] = "echo ";
  char prime[] = "prime ";
  char fib[] = "fib ";
  char consume[] = "consume ";
  char consume2[] = "consume2 ";
  char consume3[] = "consume3 ";
  char consume4[] = "consume4 ";
  char recfib[] = "recfib ";

  if (command_length >= 5 && strcompare(command, echo, 5)) {
    hoh_debug("echo called");
    exec_echo(command + 5, command_length - 5, stateinout);
  } else if (command_length >= 6 && strcompare(command, prime, 6)) {
    exec_prime(command + 6, command_length - 6, stateinout);
  } else if (command_length >= 4 && strcompare(command, fib, 4)) {
    hoh_debug("fib called");
    exec_fib(command + 4, command_length -4, stateinout);
  } else if (command_length >= 8 && strcompare(command,consume,8)){
    hoh_debug("consume time called");
    exec_consume(command + 8, command_length - 8, stateinout);
  } else if (command_length >= 9 && strcompare(command, consume2, 9)) {
    hoh_debug("async consume time called");
    exec_consume2(command + 9, command_length - 9, stateinout);
  } else if (command_length >= 9 && strcompare(command, consume3, 9)) {
    hoh_debug("fiber consume time called");
    exec_consume3(command + 9, command_length - 9, stateinout);
  } else if (command_length >= 9 && strcompare(command, consume4, 9)) {
    hoh_debug("fiber consume time called");
    exec_consume4(command + 9, command_length - 9, stateinout);
  } else if (command_length >= 7 && strcompare(command, recfib, 7)) {
    hoh_debug("recfib called");
    exec_recfib(command + 7, command_length - 7, stateinout);
  } else if (command_length == 0) {
    exec_welcome(stateinout);
  } else {
    exec_invalid(stateinout);
  }
}

// Function called when key pressed
void shell_update(uint8_t scankey, shellstate_t& stateinout){
    stateinout.num_keypresses++;

    // Update the commmand line text
    if (scankey == 0x1c) {
      // Enter pressed
      // Copy the text into the shell contents
      stateinout.contents[stateinout.content_ptr][0] = '>';
      for(int i = 1; i < stateinout.command_ptr + 1; i++) {
        stateinout.contents[stateinout.content_ptr][i] = stateinout.command_text[i-1];
      }
      for(int i = stateinout.command_ptr + 1; i < 80; i++) {
        stateinout.contents[stateinout.content_ptr][i] = ' ';
      }
      stateinout.content_ptr++;

      // Execute the current command
      exec(stateinout.command_text, stateinout.command_ptr, stateinout);
      // Reset buffers
      stateinout.command_ptr=0;
    } else {
      // Add a character to the command text
      if (scankey == 0xe) {
        // Backspace
        if (stateinout.command_ptr > 0) {
          stateinout.command_text[stateinout.command_ptr] = ' ';
          stateinout.command_ptr--;
        }
      } else if (stateinout.command_ptr < 79) {
        char tmp = getchar(scankey);
        if (tmp != '?') {
          stateinout.command_text[stateinout.command_ptr] = getchar(scankey);
          stateinout.command_ptr++;
        }
      }
    }

    hoh_debug("Got: "<<unsigned(scankey));
}


//
// do computation
//
void shell_step(shellstate_t& stateinout) {
  // Do nothing here
}


//
// shellstate --> renderstatee
//
void shell_render(const shellstate_t& shell, renderstate_t& render){
  render.num_keypresses = shell.num_keypresses;
  for(int i = 0; i == 0 || shell.heading[i-1] != '\0'; i++) {
    render.heading[i] = shell.heading[i];
  }

  // Compute the display contents
  int beg = max(0, shell.content_ptr - 24);
  for(int i = 0; i < 24; i++) {
    if (i + beg < shell.content_ptr) {
      // Copy the string
      for(int j = 0; j < 80; j++) {
        render.contents[i][j] = shell.contents[i + beg][j];
      }
    } else {
      for(int j = 0; j < 80; j++) {
        render.contents[i][j] = ' ';
      }
    }
  }

  // Compute the command line contents
  render.command_text[0] = '>';
  for(int i = 1; i < 80 && i - 1 < shell.command_ptr; i++) {
    render.command_text[i] = shell.command_text[i-1];
  }
  for(int i = shell.command_ptr + 1; i < 80; i++) {
    render.command_text[i] = ' ';
  }

  // Compute the cursor position
  render.cursor_position_x = 1 + shell.command_ptr;
  render.cursor_position_y = 24;
}

bool conststrcompare(const char *s1, const char *s2, int length) {
  for(int i = 0; i < length; i++) {
    if (s1[i] != s2[i]) {
      return false;
    }
  }
  return true;
}
//
// compare a and b
//
bool render_eq(const renderstate_t& a, const renderstate_t& b){
  if(a.num_keypresses==b.num_keypresses){
    if(conststrcompare(a.command_text,b.command_text,79)){
      if(a.cursor_position_x==b.cursor_position_x && a.cursor_position_y == b.cursor_position_y){
        for(int i=0;i<24;i++){
          if(!conststrcompare(a.contents[i],b.contents[i],80)){
            return false;
          }
        }
        return true;
      }
    }
  }
  return false;
}

static void fillrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawtext(int x,int y, const char* str, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawnumberinhex(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawnumberindecimal(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void render_counter(uint32_t num_keypresses, int w, int h, addr_t vgatext_base);

static void render_counter(uint32_t num_keypresses, int w, int h, addr_t vgatext_base) {
  const char* counter_description = " Key Count ";
  const char* space = " ";
  drawtext(0, 0, counter_description, 11, BROWN, WHITE + 8, w, h, vgatext_base);
  drawtext(11, 0, space, 1, GREEN, GREEN, w, h, vgatext_base);
  drawnumberindecimal(12, 0, num_keypresses, 5, GREEN, WHITE + 8, w, h, vgatext_base);
}

static void render_commandline(const char *text, int w, int h, addr_t vgatext_base) {
  drawtext(0, 24, text, 80, CYAN, WHITE + 8, w, h, vgatext_base);
}

static void render_statusbar(const char * heading, int w, int h, addr_t vgatext_base) {
  // Render a GREEN BAR
  char statusbar[81];
  for(int i = 0; i < 80; i++) {
    statusbar[i] = ' ';
  }
  statusbar[80] = '\0';
  drawtext(0, 0, statusbar, 80, CYAN, CYAN, w, h, vgatext_base);
  drawtext(40, 0, heading, 11, CYAN, WHITE + 8, w, h, vgatext_base);
}

//
// Given a render state, we need to write it into vgatext buffer
//
void render(const renderstate_t& state, int w, int h, addr_t vgatext_base){
  // Print the shell contents
  for(int i = 0; i < 24; i++) {
    drawtext(0, i, state.contents[i], 80, BLACK, GREEN, w, h, vgatext_base);
  }

  render_statusbar(state.heading, w, h, vgatext_base);
  render_counter(state.num_keypresses, w, h, vgatext_base);
  render_commandline(state.command_text, w, h, vgatext_base);

  // Render the cursor
  const char* cursor = " ";
  drawtext(state.cursor_position_x, state.cursor_position_y, cursor, 1, WHITE, BLACK, w, h, vgatext_base);
}


//
//
// helper functions
//
//

static void writecharxy(int x, int y, uint8_t c, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  vgatext::writechar(y*w+x,c,bg,fg,vgatext_base);
}

static void fillrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  for(int y=y0;y<y1;y++){
    for(int x=x0;x<x1;x++){
      writecharxy(x,y,0,bg,fg,w,h,vgatext_base);
    }
  }
}

static void drawrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){

  writecharxy(x0,  y0,  0xc9, bg,fg, w,h,vgatext_base);
  writecharxy(x1-1,y0,  0xbb, bg,fg, w,h,vgatext_base);
  writecharxy(x0,  y1-1,0xc8, bg,fg, w,h,vgatext_base);
  writecharxy(x1-1,y1-1,0xbc, bg,fg, w,h,vgatext_base);

  for(int x=x0+1; x+1 < x1; x++){
    writecharxy(x,y0, 0xcd, bg,fg, w,h,vgatext_base);
  }

  for(int x=x0+1; x+1 < x1; x++){
    writecharxy(x,y1-1, 0xcd, bg,fg, w,h,vgatext_base);
  }

  for(int y=y0+1; y+1 < y1; y++){
    writecharxy(x0,y, 0xba, bg,fg, w,h,vgatext_base);
  }

  for(int y=y0+1; y+1 < y1; y++){
    writecharxy(x1-1,y, 0xba, bg,fg, w,h,vgatext_base);
  }
}

static void drawtext(int x,int y, const char* str, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  for(int i=0;i<maxw;i++){
    writecharxy(x+i,y,str[i],bg,fg,w,h,vgatext_base);
    if(!str[i]){
      break;
    }
  }
}

static void drawnumberinhex(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  enum {max=sizeof(uint32_t)*2+1};
  char a[max];
  for(int i=0;i<max-1;i++){
    a[max-1-i-1]=hex2char(number%16);
    number=number/16;
  }
  a[max-1]='\0';

  drawtext(x,y,a,maxw,bg,fg,w,h,vgatext_base);
}

static void drawnumberindecimal(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
  enum {max=sizeof(uint32_t)*2+1};
  char a[max], *p = a + max - 1;
  *p = '\0';
  while(number > 0) {
    *(--p) = (number % 10) + '0';
    number /= 10;
  }
  drawtext(x,y,p,maxw,bg,fg,w,h,vgatext_base);
}

static void strcpy(char *source, char *target) {
  int i = -1;
  do {
    i++;
    target[i] = source[i];
  } while(source[i] != '\0');
}

static int max(int a, int b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}
