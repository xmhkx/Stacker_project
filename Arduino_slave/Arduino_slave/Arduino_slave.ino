
#include <SoftwareSerial.h>
#include <FastLED.h>
#include "protocol.h"

SoftwareSerial picSerial(2, 3);   // RX, TX

#define NUM_LEDS 256
#define DATA_PIN 6
#define BRIGHTNESS 10

#define WIDTH 8
#define HEIGHT 32

unsigned long lastMove = 0;               // last time the block moved
unsigned long moveInterval = 200;         // initial speed
const unsigned int minMoveInterval = 30;  // fastest speed


CRGB leds[NUM_LEDS];    // stores color of each LED, displays output to the matrix

// true = this row/column spot is permanently locked on
bool lockedLeds[HEIGHT][WIDTH] = {false};   //stores row/column postions are locked in place from previous successful moves

int currentRow = 0;     // initial row of moving block
int currentCol = 2;     // leftmost column of moving block
int blockSize = 4;      // initial amount of LEDs wide the moving block is (this can shrink)
int direction = 1;      // 1 = right, -1 = left

bool needsRedraw = true;  // updates (redraws) LED matrix when something changes
bool gameOver = false;    // flags for game ending


// converts logical row/col into actual LED index for serpentine matrix
// For each LED position, the function takes its (row, col) coordinates 
// and returns the corresponding index in the 1D LED array. 
// If the row is even, the indices are mapped left to right; if the row is odd, they are mapped right to left.
int XY(int row, int col) {
  if (row % 2 == 0) {
    return row * WIDTH + col;                  // even row: mapped left to right --> assigns 1D index for each (row, col)
  } else {
    return row * WIDTH + (WIDTH - 1 - col);    // odd row: mapped right to left --> assigns 1D index for each (row, col)
  }
}

//function redraws entire board based on the current game state
void drawGame() {
  FastLED.clear();          // clears LEDs first

  // draw locked rows
  // the LEDs of locked positions are turned blue to label the "saved" pieces
  for (int row = 0; row < currentRow && row < HEIGHT; row++) {    //loops through all completed rows below the moving row
    for (int col = 0; col < WIDTH; col++) {   //across every single column in the corresponding row, each position is checked
      if (lockedLeds[row][col]) {             //if that position is marked as locked
        leds[XY(row, col)] = CRGB::Blue;      //then that corresponding LED is turned blue
      }
    }
  }

  // draw current moving block only if game is still going
  if (!gameOver && currentRow < HEIGHT) {           //only draw the red moving block if the game is still active, having not lost and not reached the end
    for (int i = 0; i < blockSize; i++) {           //loops thru each LED that belongs to the moving block
      int col = currentCol + i;                     //column of that specific piece of moving block

      if (col >= 0 && col < WIDTH) {
        leds[XY(currentRow, col)] = CRGB::Red;      //draw the moving block color red
      }
    }
  }

  FastLED.show();         // sends LED color array to the matrix
}

//shifts moving block one position horizontally
void shiftLed() {
  if (gameOver) {
    return;
  }

  currentCol += direction;      //moves block left/right by 1 column (depending on current direction value)

  if (currentCol >= WIDTH - blockSize) {    //if moving block reaches right edge
    currentCol = WIDTH - blockSize;         //clips moving block's rightmost position at right edge so that it stays inside board
    direction = -1;                         //reverses direction, moving block begins moving left 
  }

  if (currentCol <= 0) {        //if moving block reaches left edge
    currentCol = 0;             //clips moving block's leftmost position at left edge
    direction = 1;              //revereses direction, moving block begins moving right
  }

  needsRedraw = true;         //block position changes, so the display must be redrawn
}

void buttonPressed() {
  if (gameOver || currentRow >= HEIGHT) { //if game is over or last row is reached/passed
    return;                               //ignore the button press, prevents extra inputs changing the finished game
  }

  // first row: nothing below it yet, so keep whole block
  if (currentRow == 0) {                    
    for (int i = 0; i < blockSize; i++) {         //loops thru every position of moving block
      int col = currentCol + i;                   //calculate columns of each block
      if (col >= 0 && col < WIDTH) {              //valid positions
        lockedLeds[currentRow][col] = true;       //whole block is locked (since first row)
      }
    }

    currentRow++;             //moves up to next row
    currentCol = 2;           //starts near middle again
    direction = 1;            //resets direction to moving right
    increaseSpeed();          //speeds game up after a successful placement
    needsRedraw = true;       //redraws display
    return;                   //first row case is done
  }

  //general case for later rows
  for (int col = 0; col < WIDTH; col++) {     // clear current row's locked state first
    lockedLeds[currentRow][col] = false;
  }

  int newSize = 0;      //counts # of blocks that survive after overlap 
  int newStart = -1;    //marks the leftmost surviving column

  // keep only columns that overlap with previous locked row
  for (int i = 0; i < blockSize; i++) {   //checks each position of current moving block
    int col = currentCol + i;             //gets each position's column

    if (col >= 0 && col < WIDTH) {           //for each of those columns the moving block occupies
      if (lockedLeds[currentRow - 1][col]) { //it checks if the previous row has a locked LED in the same column
        lockedLeds[currentRow][col] = true;  //if it does overlap, that position in the current moving block row becomes locked

        if (newStart == -1) {
          newStart = col;     //the first surviving position becomes the new left edge of the block for the next row
        }

        newSize++;  //increments for each block that survives
      }
    }
  }

  // no overlap means lose
  if (newSize == 0) {               //if no blocks overlapped
    Serial.println("Game Over");    //message is sent to serial monitor
    gameOver = true;
    needsRedraw = true;
    return;                         //exit once game ends
  }

  // shrink block to only surviving LEDs
  blockSize = newSize;            //shrink moving block to only surviving pieces for next row
  currentCol = newStart;          //updates next moving block's starting position to start from leftmost surviving position
  currentRow++;                   //moves to next row
  direction = 1;                  //resets direction to right
  increaseSpeed();                //makes games faster after successful placement

  if (currentRow >= HEIGHT) {     //if top of the board is reached
    Serial.println("You Win");    //game is won - sent to serial
    gameOver = true;              //stops game
  }

  needsRedraw = true;             //updates display after changes in state
}



//makes moving block faster after successful placements
void increaseSpeed(){  
  if(moveInterval > minMoveInterval + 10){     //if current interval is comfortably above interval 
    moveInterval -= 5;                         //reduce it a little
  }else{
    moveInterval = minMoveInterval;     //if current interval gets close to minimum, clip interval to minimum allowed speed
  }


}

void resetGame(){
  for(int row = 0; row < HEIGHT; row++){
    for(int col = 0; col < WIDTH; col++){
      lockedLeds[row][col] = false;
    }
  }

currentRow = 0;
currentCol = 2;
blockSize = 4;
direction = 1;

moveInterval = 200;
lastMove = millis();

gameOver = false;
needsRedraw = true;

}



void setup() {
  Serial.begin(9600);       //starts hardware serial connection
  picSerial.begin(9600);    //starts software serial connection

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);   //communicates with FastLED
  FastLED.setBrightness(BRIGHTNESS);      //applies brightness level to matrix

  needsRedraw = true;     //conducts initial draw when loop starts
}

void loop() {
  // move block every 200 ms without delay()
  if (!gameOver && millis() - lastMove >= moveInterval) {   //checks whether enough time has passed for the block to move again
    lastMove = millis();    //record the current time as the new most recent movement time
    shiftLed();             //move block one step left or right
  }

  // read all waiting bytes from PIC
  if (picSerial.available()) {      //checks if PIC has sent a byte
    char c = picSerial.read();      //reads byte into var c

    Serial.print("Received from PIC: ");
    Serial.println(c);      //prints byte to serial

    if (c == CMD_STACK) {                 //if PIC sends 'S', treats that as the signal that the player pressed the button/wants to lock the row

      buttonPressed();              //runs arduino game logic for locking current row//buttonPressed();              //runs arduino game logic for locking current row
     
     if(gameOver){
      if(currentRow >= HEIGHT){
        picSerial.write(STATUS_WON);   // send the comand that player won
      }else{
        picSerial.write(STATUS_LOSS);   // the the comand that the player lost
      }
     }
     else{
      picSerial.write(STATUS_PLACED); // shows that button press came through
     }

      needsRedraw = true;           //redraw display
    }

    else if(c == CMD_RESET){
      resetGame();
    }
  }

  // only redraw when something changed
  if (needsRedraw) {                //only redraw when something changes
    drawGame();                     //rebuild board
    needsRedraw = false;            //clears redraw flag until next event
  }

  
}