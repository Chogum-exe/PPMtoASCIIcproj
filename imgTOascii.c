#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Pixel;

Pixel pix_gscale(Pixel p) {
  Pixel gray;
  unsigned char avg = (p.red + p.green + p.blue) / 3;
  gray.red = gray.blue = gray.green = avg;
  return gray;
}

Pixel avgcol(int startr, int startc, int width, int height, int row, int col, Pixel pixels[row][col]) {
  int count = width*height;
  int red, green, blue;
  red = green = blue = 0;
  //In this rectangle summate the r g b values
  for (int r=startr; r<startr+height; ++r) {
    if (r >= row || r < 0) continue;
    for (int c=startc; c<startc+width; ++c) {
      if (c >= col || c < 0) continue;
      Pixel p = pixels[r][c];
      red += p.red;
      green += p.green;
      blue += p.blue;
    }
  }
  Pixel color;
  //Average r g b values gives avergae color;
  color.red = red/count;
  color.green = green/count;
  color.blue = blue/count;
  return color;
}

/**
 * @brief   Custom method to read in words from file stream.
 *          A word is considered a contionous characters until a space
 * 
 * @param   fill  char* String to read bytes into (terminator char appended)
 * @param   cap   int   Hard limit on number of bytes to read in (including terminator char)
 * @param   file  FILE* file stream from stdio.h
 * @return  char*       Pointer to the filled out string
 */
char *fgetw(char *fill, int cap, FILE *file) {
  char c;
  //Read in 1st to (cap-1)th bytes
  for (int i=0; i<cap-1; ++i) {
    c = fgetc(file);
    //If the current byte is a space
    //set it to terminator & We return the word
    if (isspace(c)) {
      fill[i] = '\0';
      return fill;
    }
    fill[i] = c;
  }
  //Reaches the (cap)th byte and terminate
  fill[cap-1] = '\0';
  //We need to get chars until we hit next space to finish current word
  while (!isspace(c)) c = fgetc(file);
  return fill;
}

/**
 * @brief Assume p is in greyscale
 * 
 * @param val
 * @param colmin 
 * @param colmax
 * @return char 
 */
char col_ascii(int val, int colmin, int colmax) {
  static const char *const light = "$#DR8mHXKAUbG0pV4d9h6PkqwSE2]ayjxY5Zoen[ult13If}C{iF|(7J)vTLs?z/*cr!+<>;=^,_:'-.`";
  int i = (val-colmin)*80/(colmax-colmin);
  return light[i];
}

int main(int argc, char *argv[]) {
  //Get filename from argument
  if (argc != 2) { printf("Usage: ./imgTOascii filname\n"); return 1; }
  char *filename = argv[1];

  //Open the file
  FILE *file;
  file = fopen(filename, "r");
  if (!file) { printf("error opening file: %s\n", filename); return 1; }

  //Use a temp string to copy into
  char strin[64];
  //Throw away first word being P6 file identifier
  (void)fgetw(strin, 64, file);
  //Read words containing file size
  int cols = atoi(fgetw(strin, 64, file));
  int rows = atoi(fgetw(strin, 64, file));
  //Assume the color range 0-255 for rgb
  (void)fgetw(strin, 64, file);
  
  //Read in pixels in the following consecutive bytes
  static Pixel glob_buffer[25000000];
  if (rows*cols*sizeof(Pixel) > sizeof(glob_buffer)) { printf("Image File is too BIG!"); return 1; }
  Pixel (*pixels)[cols] = (Pixel (*)[cols])glob_buffer;
  for (int r=0; r<rows; ++r) {
    for (int c=0; c<cols; ++c) {
      Pixel *p = pixels[r]+c;
      p->red = fgetc(file);
      p->green = fgetc(file);
      p->blue = fgetc(file);
    }
  }
  //We are done reading the file so we can close it
  fclose(file);

  FILE *img = fopen("output.ppm", "w");
  fputs("P6\n85 80\n255\n", img);
  
  //Pixelate the image down
  double blocksize = 52;
  int srows = ceil(rows/blocksize);
  int scols = ceil(cols/blocksize);
  //Goes through blocks and sets greyscale pixels in top-left corner
  for (int r=0; r<srows; ++r) {
    for (int c=0; c<scols; ++c) {
      Pixel t = pixels[r][c] = pix_gscale(avgcol(
        r*blocksize, c*blocksize, blocksize,
        blocksize, rows, cols, pixels));
      fputc(t.red, img);
      fputc(t.green, img);
      fputc(t.blue, img);
    }
  }
  fclose(img);

  //We need to find the min color to make that black to give contrast
  unsigned char colmin = 255;
  unsigned char colmax = 0;
  for (int r=0; r<srows; ++r) {
    for (int c=0; c<scols; ++c) {
      unsigned char col = pixels[r][c].red;
      if (col < colmin) colmin = col;
      if (col > colmax) colmax = col;
    }
  }

  //Print the ascii rep of the pixels  
  for (int r=0; r<srows; ++r) {
    for (int c=0; c<scols; ++c) {
      char ascii = col_ascii(pixels[r][c].red, colmin, colmax);
      printf("%c ", ascii);
    }
    printf("\n");
  }

  return 0;
}