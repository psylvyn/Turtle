/*
  Lost Turtle
  By Charlotte and Daddy
*/

enum tileTypes {TILE_NONE, TILE_TURTLE, TILE_OBSTACLE, TILE_MOTHER, TILE_VISITED};
byte tileType = TILE_NONE;

enum gameModes {GAMEMODE_INERT, GAMEMODE_INITIALIZE, GAMEMODE_GAME, GAMEMODE_END_WIN, GAMEMODE_END_LOSE};
byte gameMode = GAMEMODE_INERT;

enum obstacleTypes {OBSTACLE_NONE, OBSTACLE_SMALL, OBSTACLE_LARGE, OBSTACLE_HELPFUL};
byte obstacleType = OBSTACLE_NONE;

enum messageType {MSG_NONE, MSG_GAME_MODE, MSG_MOTHER, MSG_MOVE, MSG_REVEAL, MSG_PROMOTE};
enum resetStates {RESET_INITALIZE, RESET_WIN, RESET_LOSE};

int turtleTile = -1;
Timer turtleTimer;
#define TURTLE_TIMER_DURATION 10000
#define TURTLE_TIMER_SLICE (TURTLE_TIMER_DURATION / 6)

#define OBSTACLE_PERCENT_NOTHING 30
#define OBSTACLE_PERCENT_OBSTACLE_SMALL 55
#define OBSTACLE_PERCENT_OBSTACLE_LARGE 25
#define OBSTACLE_PERCENT_OBSTACLE_HELPFUL 20

bool isStateInitialized = false;
bool isClickable = false;

void setup() {
  randomize();
  initializeTile();
}

void loop() {
  switch(gameMode)
  {
    case GAMEMODE_INERT:
      inertLoop();
      //setColorOnFace(MAGENTA, 1);
      break;
    case GAMEMODE_INITIALIZE:
      initializeLoop();
      //setColorOnFace(MAGENTA, 2);
      break;
    case GAMEMODE_GAME:
      gameLoop();
      //setColorOnFace(MAGENTA, 3);
      break;
    case GAMEMODE_END_WIN:
    case GAMEMODE_END_LOSE:
      endLoop();
      //setColorOnFace(MAGENTA, 4);
      break;
  }

  // hard reset
  if(buttonLongPressed()) {
    //console.log("HARD RESET");
    setGameMode(GAMEMODE_INERT, true, -1);
  }

  // dump extra actions
  if(buttonSingleClicked() || buttonDoubleClicked()){

  }
}

//
// Mode loops
//
void inertLoop() {
  if(!isStateInitialized) {
    isStateInitialized = true;
    initializeTile();
  }

  setColor(BLUE);

  if(buttonDoubleClicked()) {
    setupGame();
    setTileType(TILE_TURTLE);
  }

  listenForSetup();
  inertRender();
}

void initializeLoop() {
  if(!isStateInitialized) {
    isStateInitialized = true;
    switch(tileType){
      case TILE_TURTLE:
        setMotherTile();
        break;
      case TILE_MOTHER:
        break;
      case TILE_NONE:
        // Decide on type of obstacle if any
        if(isObstacle()) {
          setTileType(TILE_OBSTACLE);
        }
        break;
      case TILE_OBSTACLE:
        obstacleType = getRandomObstacle();
        break;
    }
  }

  if(buttonSingleClicked() && isStateInitialized && tileType == TILE_TURTLE) {
    startGame();
  }

  listenForInitialize();
  initializeRender();
}

void gameLoop() {
  if(!isStateInitialized) {
    isStateInitialized = true;

    switch(tileType) {
      case TILE_TURTLE:
        turtleTimer.set(TURTLE_TIMER_DURATION);
        break;
      case TILE_OBSTACLE:
        obstacleType = getRandomObstacle();
        break;
    }
  }

  if(tileType == TILE_TURTLE) {
    broadcastReveal();
  }

  if(buttonSingleClicked() && isClickable) {
    setValueSentOnFace(setMessage(MSG_MOVE, 0), turtleTile);
  }

  if(tileType == TILE_TURTLE) {
    checkForLoss();
  }

  listenForMove();
  gameRender();
}

void endLoop() {
  if(gameMode == GAMEMODE_END_WIN) {
    setColor(CYAN);
  } else {
    setColor(RED);
  }

  listenForReset();
}

//
//  Game Loops
//
void turtleGameLoop() {
}

void noneGameLoop() {
}

void obstacleGameLoop() {
}

void motherGameLoop() {
}

//
//  Methods
//
void setupGame() {
  //console.clear();
  setGameMode(GAMEMODE_INITIALIZE, true, -1);
  resetTiles();
}

void initializeTile() {
  turtleTile = -1;
  tileType = TILE_NONE;
  gameMode = GAMEMODE_INERT;
  obstacleType = OBSTACLE_NONE;
  isClickable = false;
}

void setMotherTile() {
  int steps = 5 + random(2);
  int face = getRandomFace();

  if(face != -1) {
    setValueSentOnFace(setMessage(MSG_MOTHER, steps), face);
  }
}

void setTileType(byte newTileType) {
  tileType = newTileType;
}

void checkForLoss() {
  if(tileType == TILE_TURTLE && turtleTimer.isExpired()) {
    setGameMode(GAMEMODE_END_LOSE, true, -1);
  }
}

void setGameMode(byte newGameMode, bool startPropogate, int sourceFace) {
  isStateInitialized = false;
  gameMode = newGameMode;

  if(startPropogate) {
    setValueSentOnAllFaces(0);

    for(int i = 0; i < 6; ++i) {
      if(sourceFace == -1 || i != sourceFace){
        setValueSentOnFace(setMessage(MSG_GAME_MODE, newGameMode), i);
      }
    }
  }
}

void resetTiles() {
  setTileType(TILE_NONE);
}

void startGame() {
  setGameMode(GAMEMODE_GAME, true, -1);
}

bool isObstacle() {
  return (random(100) > OBSTACLE_PERCENT_NOTHING);
}

byte getRandomObstacle() {
  int randomObstacle = random(100);

  if(randomObstacle <= OBSTACLE_PERCENT_OBSTACLE_HELPFUL) {
    return OBSTACLE_HELPFUL;
  }
  randomObstacle -= OBSTACLE_HELPFUL;

  if(randomObstacle <= OBSTACLE_PERCENT_OBSTACLE_SMALL) {
    return OBSTACLE_SMALL;
  }
  randomObstacle -= OBSTACLE_PERCENT_OBSTACLE_SMALL;

  if(randomObstacle <= OBSTACLE_PERCENT_OBSTACLE_LARGE) {
    return OBSTACLE_LARGE;
  }

  return OBSTACLE_NONE;
}

int getRandomFace() {
  int faceCount = 0;

  FOREACH_FACE(f) {
    if(!isValueReceivedOnFaceExpired(f)) {
      faceCount++;
    }
  }

  if(faceCount > 0) {
    int randomFace = random(faceCount - 1) + 1;

    FOREACH_FACE(f) {
      if(!isValueReceivedOnFaceExpired(f)) {
        --randomFace;
        if(randomFace == 0) {
          return f;
        }
      }
    }
    return -1;
  }
}

void broadcastReveal() {
  if(tileType == TILE_TURTLE) {
    setValueSentOnAllFaces(setMessage(MSG_REVEAL, true));
  }
}

void processMove(int timeRemaining) {
  
  //console.log("time remaining: " + timeRemaining);

  switch(tileType){
    case TILE_NONE:
    case TILE_VISITED:
      setTileType(TILE_TURTLE);
      break;
    case TILE_MOTHER:
      setGameMode(GAMEMODE_END_WIN, true, -1);
      break;
    case TILE_OBSTACLE:
      switch(obstacleType) {
        case OBSTACLE_HELPFUL:
          timeRemaining += 1;
          break;
        case OBSTACLE_SMALL:
          timeRemaining -= 1;
          break;
        case OBSTACLE_LARGE:
          timeRemaining -= 2;
          break;
      }
      setTileType(TILE_TURTLE);
      break;
  }

  //console.log("time remaining: " + (TURTLE_TIMER_SLICE * timeRemaining));
  turtleTimer.set(TURTLE_TIMER_SLICE * timeRemaining);
}

//
// Listen Methods
//
void listenForInitialize() {

  bool isMotherMessage = false;
  int motherMessageSource = -1;
  int motherScore = -1;

  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f) && didValueOnFaceChange(f)){
      switch(getMessageType(getLastValueReceivedOnFace(f))) {
        case MSG_MOTHER:
          //console.log("mother message");
          isMotherMessage = true;
          motherMessageSource = f;
          motherScore = getMessageValue(getLastValueReceivedOnFace(f));
          break;
        case MSG_GAME_MODE:
          byte newGameMode = getMessageValue(getLastValueReceivedOnFace(f));
          if(gameMode != newGameMode){
            setGameMode(newGameMode, true, f);
          }
      }
    }
  }

  if(isMotherMessage){
    
    if(motherScore == 0 && tileType != TILE_TURTLE) {
      //console.log("setting a mother");
      setTileType(TILE_MOTHER);
    } else {
      int face = getRandomFace();

      // heavily favor NOT going backwards
      if(face == motherMessageSource && random(100) > 80){
        face = getRandomFace();
      }

      motherScore -= 1;
      if(motherScore < 0) {
        motherScore = 0;
      }
      if(face != -1) {
        setValueSentOnAllFaces(0);
        setValueSentOnFace(setMessage(MSG_MOTHER, motherScore), face);
      }
    }
  }
}

void listenForSetup() {
  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f) && didValueOnFaceChange(f)){
      switch(getMessageType(getLastValueReceivedOnFace(f))) {
        case MSG_GAME_MODE:
          byte newGameMode = getMessageValue(getLastValueReceivedOnFace(f));
          if(gameMode != newGameMode){
            setGameMode(newGameMode, true, f);
          }
      }
    }
  }
}

void listenForMove() {
  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f)){
      switch(getMessageType(getLastValueReceivedOnFace(f))) {
        case MSG_MOVE:
          if(didValueOnFaceChange(f)) {
            if(tileType == TILE_TURTLE) {
              setTileType(TILE_VISITED);

              int before = f - 1;
              if(before == -1) {
                before = 5;
              }

              int after = f + 1;
              if(after == 6) {
                after = 0;
              }

              for(int i = 0; i < 6; ++i) {
                if(i == f) {
                  int timeRemaining = (turtleTimer.getRemaining() / TURTLE_TIMER_SLICE) + 1;
                  if(timeRemaining > 6){
                    timeRemaining = 6;
                  }
                  setValueSentOnFace(setMessage(MSG_PROMOTE, timeRemaining), i);
                } else {
                  if( i != before && i != after) {
                    setValueSentOnFace(setMessage(MSG_REVEAL, false), i);
                  }
                }
              }
            }
          }
          break;
        case MSG_PROMOTE:
          if(didValueOnFaceChange(f)) {
            processMove(getMessageValue(getLastValueReceivedOnFace(f)));
          }
          break;
        case MSG_REVEAL:
          if(didValueOnFaceChange(f)) {
            bool shouldReveal = getMessageValue(getLastValueReceivedOnFace(f));
            if(shouldReveal) {
              turtleTile = f;
            } else {
              turtleTile = -1;
            }
            isClickable = shouldReveal;
            break;
          }
        case MSG_GAME_MODE:
          if(didValueOnFaceChange(f)) {
            byte newGameMode = getMessageValue(getLastValueReceivedOnFace(f));
            //console.log("mode change " + newGameMode);
            if(gameMode != newGameMode){
              setGameMode(newGameMode, true, f);
            }
          }
          break;
      }
    }
  }
}

void listenForReset() {
  FOREACH_FACE(f){
    if(!isValueReceivedOnFaceExpired(f) && didValueOnFaceChange(f)){
      switch(getMessageType(getLastValueReceivedOnFace(f))) {
        case MSG_GAME_MODE:
          byte newGameMode = getMessageValue(getLastValueReceivedOnFace(f));
          if(gameMode != newGameMode){
            setGameMode(newGameMode, true, f);
          }
      }
    }
  }
}

//
// Rendering
//
void inertRender() {
  int step = (millis() / 1000) % 3;
  setColorOnFace(WHITE, step);
  setColorOnFace(WHITE, step + 3);
}

void initializeRender() {
  bool shouldRender = false;
  if(tileType == TILE_TURTLE) {
    shouldRender = true;
  }

  if(shouldRender){
    switch(tileType){
      case TILE_MOTHER:
        setColor(CYAN);
        break;
      case TILE_TURTLE:
        setColor(GREEN);
        break;
      case TILE_OBSTACLE:
        setColor(MAGENTA);
        switch(obstacleType){
          case OBSTACLE_SMALL:
            setColor(ORANGE);
            break;
          case OBSTACLE_LARGE:
            setColor(RED);
            break;
          case OBSTACLE_HELPFUL:
            setColor(YELLOW);
            break;
        }
        break;
      case TILE_NONE:
        setColor(OFF);
        break;
    }
  } else {
    setColor(BLUE);
  }
}

void gameRender() {
  bool shouldRender = false;
  if(tileType == TILE_TURTLE || isClickable || tileType == TILE_VISITED) {
    shouldRender = true;
  }

  if(shouldRender){
    switch(tileType){
      case TILE_MOTHER:
        setColor(CYAN);
        break;
      case TILE_TURTLE:
        setColor(GREEN);
  
        if(!turtleTimer.isExpired()) {
          int timeLeft = 6 - (turtleTimer.getRemaining() / TURTLE_TIMER_SLICE + 1);
          
          for(int x = 0; x < timeLeft; ++x) {
            setColorOnFace(OFF, x);
          } 
        }
        break;
      case TILE_OBSTACLE:
        switch(obstacleType){
          case OBSTACLE_SMALL:
            setColor(ORANGE);
            break;
          case OBSTACLE_LARGE:
            setColor(RED);
            break;
          case OBSTACLE_HELPFUL:
            setColor(YELLOW);
            break;
          case OBSTACLE_NONE:
            setColor(dim(BLUE, 120));
        }
        break;
      case TILE_NONE:
        setColor(OFF);
        break;
      case TILE_VISITED:
      default:
        setColor(dim(GREEN, 120));
        break;
    }
  } else {
    setColor(BLUE);
  }
}

//
//  Messaging
//
byte getMessageValue(byte data) {
  return (data & 7);
}

byte getMessageType(byte data) {
  return ((data >> 3) & 7);
}

byte setMessage(byte msg, byte value){
  return (msg << 3) | value;
}
