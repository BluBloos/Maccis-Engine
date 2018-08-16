#include <game_include.h>

//NOTE(Noah): im going to use this to generate random numbers
#include <stdlib.h>

struct player
{
  unsigned int score;
  vec2 velocity;
  renderable_2D sprite;
};

struct ball
{
  vec2 velocity;
  float radius;
  renderable_2D sprite;
};

struct text_handle
{
  vec2 position; //position of the bottom left of the text on the screen
  unsigned int textLength;
  char *text;
  unsigned int fontId;
};

struct text_engine
{
  unsigned int registeredTextCount;
  text_handle registeredTexts[10];
};

//NOTE(Noah): this game is so small I think its alright that we use globals
INTERNAL player players[2] = {};
INTERNAL ball ball = {};
INTERNAL texture playerTexture = {};
INTERNAL texture ballTexture = {};

INTERNAL camera guiCamera = {};
INTERNAL text_handle scoreBoard = {};
INTERNAL text_engine textEngine = {};
INTERNAL loaded_font font = {};
INTERNAL loaded_asset fontAsset = {};

//NOTE(Noah): These need to be initialized
INTERNAL float playerMaxY = 0;
INTERNAL float playerMinY = 0;

#define playerMaxVelocity 200

INTERNAL bool CircleLineCollisionTest(vec2 circlePos, float circleRadius, vec2 linePos1, vec2 linePos2, vec *posOut)
{
  //calucalte line 1
  float gradient = (linePos2.y - linePos1.y) / (linePos2.x - linePos1.x);
  float beta = linePos2.y - gradient * linePos2.x;

  //calculate line 2
  float pGradient = -1.0f / gradient;
  float beta2 = circlePos.y - pGradient * circlePos.x;

  float x = (beta + beta2) / (gradient - pGradient);
  float y = gradient * x + beta;

  //calculate the length of the line
  float lineLength = GetLineLength(NewVec2(x,y), circlePos);

  //get result
  if (lineLength >= circleRadius)
  {
    if (x > linePos1.x && x < linePos2.x)
    {
      if (y > linePos1.y && y < linePos2.y)
      {
        *posOut = NewVec2(x, y);
        return true;
      }
    }
  }

  return false;
}

INTERNAL text_handle PushText(text_engine *textEngine, memory_arena *arena, unsigned int characterAmount, float x, float y, unsigned int fontId)
{
  text_handle textHandle = {};

  textHandle.text = (char *)arena->push(sizeof(char) * characterAmount);
  textHandle.fontId = fontId;
  textHandle.textLength = characterAmount;
  textHandle.position.x = x;
  textHandle.position.y = y;

  textEngine->registeredTexts[textEngine->registeredTextCount++] = textHandle;

  return textHandle;
}

INTERNAL void RenderText(text_engine *textEngine, batch_renderer_2D *batchRenderer2D)
{
  for (unsigned int i = 0; i < textEngine->registeredTextCount; i++)
  {
    text_handle textHandle = textEngine->registeredTexts[i];
    //NOTE(Noah): we don't actually implemenmt the fontId here,
    //although the game is pong so I don't see the issue lmao
    DebugPushText(textHandle.text, batchRenderer2D, &font, textHandle.position);
  }
}

INTERNAL unsigned int PushTexture(engine_state *engineState, texture tex)
{
  unsigned int index = engineState->textureCount;
  engineState->textures[engineState->textureCount++] = tex;
  return index;
}

#define RESOLUTION 100000
INTERNAL float RandomInRange(float a, float f)
{
  unsigned int result = rand() % (RESOLUTION + 1);
  float randomNumber = (f - a) * (float)result / (float)RESOLUTION + a;
  return randomNumber;
}

extern "C" GAME_INIT(GameInit)
{
  engine_state *engineState = (engine_state *)memory->storage;
  char stringBuffer[256]; //NOTE(Noah): I would like something better than this

  //initialize the random number generator
  float duration = memory->GetClockDeltaTime() * 1000.0f * 1000.0f; //NOTE(Noah): this is in micro seconds
   srand((unsigned int)duration);

  //NOTE(Noah): we still compile fonts to be "font.asset"

  //load in a font
  fontAsset = LoadAsset(memory->ReadFile, memory->FreeFile,
    &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory->maccisDirectory, "res\\font.asset", stringBuffer));

  //extract the texture atlas
  loaded_bitmap *fontAtlasBitmap = (loaded_bitmap *)fontAsset.pWrapper->asset;
  font.atlasID = PushTexture(engineState, engine->CreateTexture(fontAtlasBitmap->pixelPointer,
    fontAtlasBitmap->width, fontAtlasBitmap->height, 2));

  //Create font from the asset!
  LoadFontFromAsset(&engineState->memoryArena, fontAsset, &font);
  //engineState->batchRenderer2D->textureAtlas = engineState->fontAtlas;

  //setup the scoreboard at the top of the screen
  scoreBoard = PushText(&textEngine, &engineState->memoryArena, 100, (float)width / 2.0f, (float)height - (float)font.lineHeight, 0);
  CloneString("Hearty men sit on garbage!?", scoreBoard.text, 100);

  //setup the gui camera
  guiCamera = CreateOrthoCamera((float)width, (float)height);

  //load in the 2D shaders
  read_file_result f1 = memory->ReadFile(MaccisCatStringsUnchecked(memory->maccisDirectory, "res\\2D_shader.vert", stringBuffer));
  read_file_result f2 = memory->ReadFile(MaccisCatStringsUnchecked(memory->maccisDirectory, "res\\2D_shader.frag", stringBuffer));
  shader shader = engine->CreateShader((char *)f1.content, (char *)f2.content);
  memory->FreeFile(f1.content);
  memory->FreeFile(f2.content);

  //initialize the 2D batch renderer
  engineState->batchRenderer2D = (batch_renderer_2D *)engineState->memoryArena.push(sizeof(batch_renderer_2D)); //grab some memory
  renderer->InitializeBatchRenderer2D(engineState->batchRenderer2D, shader);

  //initialize the players and the ball
  loaded_bitmap playerBitmap = LoadBMP(memory->ReadFile, MaccisCatStringsUnchecked(memory->maccisDirectory, "res\\player.bmp", stringBuffer));
  playerTexture = engine->CreateTexture(playerBitmap.pixelPointer, playerBitmap.width, playerBitmap.height, 1);
  FreeBitmap(memory->FreeFile, playerBitmap);

  players[0].sprite = SpriteFromBitmap(playerBitmap);
  players[0].sprite.position.x = playerBitmap.width / 2.0f;
  players[0].sprite.position.y = (float)height / 2.0f;
  players[0].sprite.alignPercentage[1] = 0.5f;
  players[0].sprite.alignPercentage[0] = -0.5f;

  players[1].sprite = SpriteFromBitmap(playerBitmap);
  players[1].sprite.position.x = (float)width - playerBitmap.width / 2.0f;
  players[1].sprite.position.y = (float)height / 2.0f;
  players[1].sprite.alignPercentage[1] = 0.5f;
  players[1].sprite.alignPercentage[0] = -0.5f;

  loaded_bitmap ballBitmap = LoadBMP(memory->ReadFile, MaccisCatStringsUnchecked(memory->maccisDirectory, "res\\ball.bmp", stringBuffer));
  ballTexture = engine->CreateTexture(ballBitmap.pixelPointer, ballBitmap.width, ballBitmap.height, 2);
  FreeBitmap(memory->FreeFile, ballBitmap);

  ball.sprite = SpriteFromBitmap(ballBitmap);
  ball.velocity.x = RandomInRange(50.0f, (float)playerMaxVelocity);
  ball.velocity.y = RandomInRange(50.0f, (float)playerMaxVelocity);
  ball.sprite.position.x = (float)width / 2.0f;
  ball.sprite.position.y = (float)height / 2.0f;
  ball.sprite.alignPercentage[0] = -0.5f;
  ball.sprite.alignPercentage[1] = 0.5f;
  //NOTE(Noah): the ball diameter is equal to the width of the bitmap, because that's how I made the bitmap,
  //there is zero padding.
  ball.radius = (float)ballBitmap.width / 2.0f;

  //intialize the player max Y and min Y
  playerMaxY = (float)height - (float)playerBitmap.height / 2.0f;
  playerMinY = (float)playerBitmap.height / 2.0f;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  engine_state *engineState = (engine_state *)memory->storage;

  //player 1
  //NOTE(Noah): this assumes that the framerate is 60 frames per second
  float framesPerSecond = 60.0f;
  float secondsPerFrame = 1.0f / framesPerSecond;
  float playerAcceleration = 20; //5 units per second

  if (userInput->keyStates[MACCIS_KEY_W].endedDown)
  {
    players[0].velocity.y += playerAcceleration * secondsPerFrame;
    if (players[0].velocity.y > playerMaxVelocity)
    {
      players[0].velocity.y = playerMaxVelocity;
    }
  }
  else if (userInput->keyStates[MACCIS_KEY_S].endedDown)
  {
    players[0].velocity.y -= playerAcceleration * secondsPerFrame;
    if (players[0].velocity.y < -playerMaxVelocity)
    {
      players[0].velocity.y = -playerMaxVelocity;
    }
  }

  //player 2
  if (userInput->keyStates[MACCIS_KEY_UP].endedDown)
  {
    players[1].velocity.y += playerAcceleration * secondsPerFrame;
    if (players[1].velocity.y > playerMaxVelocity)
    {
      players[1].velocity.y = playerMaxVelocity;
    }
  }
  else if (userInput->keyStates[MACCIS_KEY_DOWN].endedDown)
  {
    players[1].velocity.y -= playerAcceleration * secondsPerFrame;
    if (players[1].velocity.y < -playerMaxVelocity)
    {
      players[1].velocity.y = -playerMaxVelocity;
    }
  }

  #define ABSORPTION 0.2
  //update the players position based on their velcoity and of course make sure they never go out of bounds
  //player 1
  players[0].sprite.position.y += players[0].velocity.y * secondsPerFrame;
  if (players[0].sprite.position.y > playerMaxY)
  {
    //NOTE(Noah): the player collided with the wall, bounce them off of it!
    players[0].sprite.position.y = playerMaxY;
    players[0].velocity.y = -players[0].velocity.y * (1 - ABSORPTION);
  } else if (players[0].sprite.position.y < playerMinY)
  {
    players[0].sprite.position.y = playerMinY;
    players[0].velocity.y = -players[0].velocity.y * (1 - ABSORPTION);
  }

  //player 2
  players[1].sprite.position.y += players[1].velocity.y * secondsPerFrame;
  if (players[1].sprite.position.y > playerMaxY)
  {
    //NOTE(Noah): the player collided with the wall, bounce them off of it!
    players[1].sprite.position.y = playerMaxY;
    players[1].velocity.y = -players[1].velocity.y * (1 - ABSORPTION);
  } else if (players[1].sprite.position.y < playerMinY)
  {
    players[1].sprite.position.y = playerMinY;
    players[1].velocity.y = -players[1].velocity.y * (1 - ABSORPTION);
  }

  //do physics with the ball, for example determining whether or not it hit one of the paddles and then calculating the
  //appropriate new velocity
  ball.sprite.position.x += ball.velocity.x * secondsPerFrame;
  ball.sprite.position.y += ball.velocity.y * secondsPerFrame;

  //also if the ball hit a wall give some score to one of the players, and restart the ball back to center

  //render the paddles text and the ball
  renderer->Clear();

  {
    //render any text
    engineState->batchRenderer2D->textureAtlas = engineState->textures[font.atlasID];
    renderer->BeginBatchRenderer2D(engineState->batchRenderer2D);
    //this will do auto rendering of any registerd text
    RenderText(&textEngine, engineState->batchRenderer2D);

    renderer->EndBatchRenderer2D(engineState->batchRenderer2D);
    renderer->Flush(engineState->batchRenderer2D, guiCamera);
  }

  {
    //start the first batch render
    engineState->batchRenderer2D->textureAtlas = playerTexture;
    renderer->BeginBatchRenderer2D(engineState->batchRenderer2D);

    //submit player 1
    Submit(engineState->batchRenderer2D, players[0].sprite);
    //submit player 2
    Submit(engineState->batchRenderer2D, players[1].sprite);

    //finally end the batch renderer and push the things to the screen
    renderer->EndBatchRenderer2D(engineState->batchRenderer2D);
    renderer->Flush(engineState->batchRenderer2D, guiCamera);
  }

  {
    //begin the second batch render
    //we do a seperate batch render for the ball since it will use a different texture
    engineState->batchRenderer2D->textureAtlas = ballTexture;
    renderer->BeginBatchRenderer2D(engineState->batchRenderer2D);
    //submit the ball
    Submit(engineState->batchRenderer2D, ball.sprite);

    //finally end the batch renderer and push the things to the screen
    renderer->EndBatchRenderer2D(engineState->batchRenderer2D);
    renderer->Flush(engineState->batchRenderer2D, guiCamera);
  }
}

extern "C" GAME_CLOSE(GameClose)
{
}
