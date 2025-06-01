#pragma once

const uint8_t scaleNotes[] = {69,70,71,72,73,74,75,76,77,78,79,80};
const uint8_t SCALE_LEN = sizeof(scaleNotes);

const uint8_t chords[][3] = {
  {0,3,7},
  {2,5,9},
  {4,7,11},
  {5,8,0},
  {7,10,2},
  {9,0,4},
  {11,2,5}
};

const uint8_t NUM_CHORDS = sizeof(chords) / sizeof(chords[0]);
