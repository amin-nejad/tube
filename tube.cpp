#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <cstring>
#include <string.h>
#include <cctype>
#include <cstdlib>

using namespace std;

#include "tube.h"

/* You are pre-supplied with the functions below. Add your own 
   function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char **allocate_2D_array(int rows, int columns) {
  char **m = new char *[rows];
  assert(m);
  for (int r=0; r<rows; r++) {
    m[r] = new char[columns];
    assert(m[r]);
  }
  return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char **m, int rows) {
  for (int r=0; r<rows; r++)
    delete [] m[r];
  delete [] m;
}

/* internal helper function which gets the dimensions of a map */
bool get_map_dimensions(const char *filename, int &height, int &width) {
  char line[512];
  
  ifstream input(filename);

  height = width = 0;

  input.getline(line,512);  
  while (input) {
    if ( (int) strlen(line) > width)
      width = strlen(line);
    height++;
    input.getline(line,512);  
  }

  if (height > 0)
    return true;
  return false;
}

/* pre-supplied function to load a tube map from a file*/
char **load_map(const char *filename, int &height, int &width) {

  bool success = get_map_dimensions(filename, height, width);
  
  if (!success)
    return NULL;

  char **m = allocate_2D_array(height, width);
  
  ifstream input(filename);

  char line[512];
  char space[] = " ";

  for (int r = 0; r<height; r++) {
    input.getline(line, 512);
    strcpy(m[r], line);
    while ( (int) strlen(m[r]) < width )
      strcat(m[r], space);
  }
  
  return m;
}

/* pre-supplied function to print the tube map */
void print_map(char **m, int height, int width) {
  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    if (c && (c % 10) == 0) 
      cout << c/10;
    else
      cout << " ";
  cout << endl;

  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    cout << (c % 10);
  cout << endl;

  for (int r=0; r<height; r++) {
    cout << setw(2) << r << " ";    
    for (int c=0; c<width; c++) 
      cout << m[r][c];
    cout << endl;
  }
}

/* pre-supplied helper function to report the errors encountered in Question 3 */
const char *error_description(int code) {
  switch(code) {
  case ERROR_START_STATION_INVALID: 
    return "Start station invalid";
  case ERROR_ROUTE_ENDPOINT_IS_NOT_STATION:
    return "Route endpoint is not a station";
  case ERROR_LINE_HOPPING_BETWEEN_STATIONS:
    return "Line hopping between stations not possible";
  case ERROR_BACKTRACKING_BETWEEN_STATIONS:
    return "Backtracking along line between stations not possible";
  case ERROR_INVALID_DIRECTION:
    return "Invalid direction";
  case ERROR_OFF_TRACK:
    return "Route goes off track";
  case ERROR_OUT_OF_BOUNDS:
    return "Route goes off map";
  }
  return "Unknown error";
}

/* presupplied helper function for converting string to direction enum */
Direction string_to_direction(const char *token) {
  const char *strings[] = {"N", "S", "W", "E", "NE", "NW", "SE", "SW"};
  for (int n=0; n<8; n++) {
    if (!strcmp(token, strings[n])) 
      return (Direction) n;
  }
  return INVALID_DIRECTION;
}

// Get symbol position
bool get_symbol_position (char** map,
			  int &height,
			  int &width,
			  char target,
			  int &r,
			  int &c){

  for (int i = 0; i < height; i++){

    for (int j = 0; j < width; j++){

      if (map[i][j] == target){

	c = j;
	r = i;
	
	return true;
      }
    }
  }
  
  // failed to find
  r = -1;
  c = -1;
  
  return false;
}

char get_symbol_for_station_or_line(string name){

  fstream station_stream;
  fstream line_stream;

  station_stream.open("stations.txt");
  line_stream.open("lines.txt");

  if (line_stream.fail() ||
      station_stream.fail()){

    cerr << "Error opening files" << endl;
    return ' ';
  }

  string word;
  string stations;
  string lines;
  string stations_and_lines;
  
  while (!station_stream.eof()){
    getline(station_stream, word);
    stations += word + '\n';
  }

  while (!line_stream.eof()){
    getline(line_stream, word);
    lines += word + '\n';
  }

  stations_and_lines = stations + lines;

  int name_occurrence = 0;  
  name_occurrence = stations_and_lines.find(name);

  if (name_occurrence == -1){
    return ' ';
  }
  //cout << stations_and_lines << endl;
  return stations_and_lines[name_occurrence -2];
}

// check that the directions given from a starting station are valid
// return number of changes required if so
int validate_route(char** map,
		   int &height,
		   int &width,
		   string start_station,
		   string route,
		   char* destination){

  // check start station
  
  char start_station_symbol;
  int station_change_count = 0;
  start_station_symbol = get_symbol_for_station_or_line(start_station);

  if (start_station_symbol == ' '){
    return ERROR_START_STATION_INVALID;
  }

  // check route/ directions

  int i = 0;
  string direction;
  
  int row;
  int col;
  char current_symbol = start_station_symbol;
  char previous_symbol;
  char prior_to_prev_symbol;
  string previous_rc;
  string prior_to_prev_rc;
  string current_rc;
  
  get_symbol_position(map, height, width, start_station_symbol, row, col);
  
  i = 0;
  direction.clear();
  while (route.length() > 0){
    
    int checkpoint = 0;
    int comma = route.find_first_of(",");
    if (comma == -1){
      checkpoint = route.length();
    } else {
      checkpoint = comma;
    }
    
    direction = route.substr(i, checkpoint - i);

    //cout << "direction: " << direction << endl;
    //cout << "route: " << route << endl;
    
    if (comma != -1){
      route = route.substr(checkpoint + 1, route.length() - checkpoint);
    } else {
      route.clear();
    }
    
    prior_to_prev_rc = previous_rc;
    prior_to_prev_symbol = previous_symbol;
    previous_rc = to_string(row) + to_string(col);
    previous_symbol = map[row][col];
    
    Direction Dir;
    Dir = string_to_direction (direction.c_str());

    switch (Dir) {
    case N:
      row--;
      break;
    case NE:
      col++;
      row--; 
      break;
    case E:
      col++;
      break;
    case SE:
      col++;
      row++;
      break;
    case S:
      row++;
      break;
    case SW:
      col--;
      row++;
      break;
    case W:
      col--;
      break;
    case NW:
      row--;
      col--;
      break;
    case INVALID_DIRECTION:
      return ERROR_INVALID_DIRECTION;
    }
    
    if (row > height || row < 0 || col > width || col < 0){
      return ERROR_OUT_OF_BOUNDS;
    }

    current_symbol = map[row][col];
    current_rc = to_string(row) + to_string(col);
    
    if (current_symbol == ' '){
      return ERROR_OFF_TRACK;
    }

    if (!isalnum(previous_symbol) &&
	!isalnum(current_symbol)){

      if (previous_symbol != current_symbol){
	return ERROR_LINE_HOPPING_BETWEEN_STATIONS;
      } else {
	if (current_rc == prior_to_prev_rc){
	  return ERROR_BACKTRACKING_BETWEEN_STATIONS;
	}
      }
    }

    if (isalnum(previous_symbol) &&
	current_symbol != prior_to_prev_symbol){

      station_change_count++;
    }
  }

  // ROUTE FINISHED!
  
  if (!isalnum(current_symbol)){
    return ERROR_ROUTE_ENDPOINT_IS_NOT_STATION;
  }

  fstream station_stream;
  station_stream.open("stations.txt");

  if (station_stream.fail()){
    cerr << "Error getting station name" << endl;
  }

  char station_char;
  string temp;
  
  while(!station_stream.eof()){
    station_stream.get(station_char);
    temp += station_char;
  }

  string find = "\n" + string(1,current_symbol) + " ";
  int location = temp.find(find);

  temp = temp.substr(location+1, temp.length() - location);
  location = temp.find("\n");

  string symb_str = temp.substr(2, location -2);
  strcpy(destination, symb_str.c_str());
  
  //default
  return station_change_count;
}
