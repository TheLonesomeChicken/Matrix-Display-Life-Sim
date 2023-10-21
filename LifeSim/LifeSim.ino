#include <Adafruit_Protomatter.h>

#define worldSizeX 64
#define worldSizeY 32

uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

uint8_t mutationRate = 10; // lowest 2 (50%), 10 is (10%)

Adafruit_Protomatter matrix(64, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

uint64_t** mainWorld;

void setup(void)
{
  Serial.begin(9600);

  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status != PROTOMATTER_OK)
  {
    for(;;);
  }

  randomSeed(analogRead(0));

  mainWorld = fillWorld();
}

void loop(void)
{
  delay(25);
  mainWorld = runWorld(mainWorld);

  for (int x = 0; x < worldSizeX; x++)
  {
    for (int y = 0; y < worldSizeY; y++)
    {
      if (mainWorld[x][y] != 0b0)
      {
        matrix.drawPixel(x, y, mainWorld[x][y] / 2);
      }
    }
  }

  matrix.show();
}

uint64_t randUint64Slow()
{
  uint64_t r = 0;
  for (int i = 0; i < 64; i++)
  {
    r = r * 2 + rand() % 2;
  }
  
  return r;
}

uint64_t** fillWorld()
{
  uint64_t** world = (uint64_t**)malloc(worldSizeX * sizeof(uint64_t*));
  for (int i = 0; i < worldSizeX; i++)
  {
    world[i] = (uint64_t*)malloc(worldSizeY * sizeof(uint64_t));
  }

  for (int x = 0; x < worldSizeX; x++)
  {
    for (int y = 0; y < worldSizeY; y++)
    {
      world[x][y] = randUint64Slow();
    }
  }

  return world;
}

int neighborId(int genomeId, int worldSize)
{
  if (genomeId == 0)
  {
    return genomeId + 1;
  }

  if (genomeId == worldSize - 1)
  {
    return genomeId - 1;
  }

  int neighborId;

  if (random(2) == 0)
  {
    neighborId = (genomeId + 1);
    return neighborId;
  }

  neighborId = (genomeId - 1);
  return neighborId;
}

uint64_t mutate(uint64_t genome)
{
  if (random(mutationRate) == 0)
  {
    genome ^= 0b1 << random(64);
    return genome;
  }
  
  return genome;
}

uint64_t** executeGenomeInstruction(uint64_t** world, int genomeIdX, int genomeIdY, uint8_t instruction)
{
  if (instruction == 0b00) // A
  {
    // do nothing
  }
  else if (instruction == 0b01) // C
  {
    // try to make a copy
    int selectedNeighborIdX;
    int selectedNeighborIdY;
    if (random(2) == 0)
    {
      selectedNeighborIdX = neighborId(genomeIdX, worldSizeX);
      selectedNeighborIdY = genomeIdY;
    }
    else
    {
      selectedNeighborIdX = genomeIdX;
      selectedNeighborIdY = neighborId(genomeIdY, worldSizeY);
    }
    
    if (world[selectedNeighborIdX][selectedNeighborIdY] == 0b0)
    {
      world[selectedNeighborIdX][selectedNeighborIdY] = mutate(world[genomeIdX][genomeIdY]);
    }
  }
  else if (instruction == 0b10) // G
  {
    // kys
    world[neighborId(genomeIdX, worldSizeX)][neighborId(genomeIdY, worldSizeY)] = 0b00;
  }
  else if (instruction == 0b11) // T
  {
    // try to move
    int selectedNeighborIdX;
    int selectedNeighborIdY;
    if (random(2) == 0)
    {
      selectedNeighborIdX = neighborId(genomeIdX, worldSizeX);
      selectedNeighborIdY = genomeIdY;
    }
    else
    {
      selectedNeighborIdX = genomeIdX;
      selectedNeighborIdY = neighborId(genomeIdY, worldSizeY);
    }

    if (world[selectedNeighborIdX][selectedNeighborIdY] == 0b0)
    {
      world[selectedNeighborIdX][selectedNeighborIdY] = world[genomeIdX][genomeIdY];
      world[genomeIdX][genomeIdY] = 0b00;
    }
  }

  return world;
}

uint64_t** runAllGenomeCode(uint64_t** world, int genomeIdX, int genomeIdY, uint64_t genome)
{
  if (world[genomeIdX][genomeIdY] == 0b0)
  {
    return world;
  }
  
  for (uint8_t instructionId = 0; instructionId < 64; instructionId += 2)
  {
    world = executeGenomeInstruction(world, genomeIdX, genomeIdY, (genome >> instructionId) & 0b11);
    
    if (world[genomeIdX][genomeIdY] == 0b0)
    {
      return world;
    }
  }

  return world;
}

uint64_t** runWorld(uint64_t** world)
{
  for (int x = 0; x < worldSizeX; x++)
  {
    for (int y = 0; y < worldSizeY; y++)
    {
      world = runAllGenomeCode(world, x, y, world[x][y]);
    }
  }

  world[random(worldSizeX)][random(worldSizeY)] = 0b0;
  
  return world;
}