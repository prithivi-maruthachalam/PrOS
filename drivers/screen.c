#include "ports.h"
#include "screen.h"

//Declaration for private functions
int get_cursor_offset(); //gets the current position of the cursor in video memory
void set_cursor_offset(int offset); //moves cursor to a certain position in video memory

int get_offset(int row, int col); //uses coords to return position in video memory
//uses video memory position to return row and col
int get_offset_row(int offset); 
int get_offset_col(int offset);

int print_char(char c, int row, int col, char attr); //attr describes the color

/*public functions declared in screen.h*/
void kprint_at(char *text, int row, int col){
    int offset;
    if(row >= 0 && col >=0){
        offset = get_offset(row,col);
    } else {
        offset = get_cursor_offset();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
    
    int i = 0;
    while(text[i]){
        offset = print_char(text[i++],row,col,WHITE_ON_BLACK);
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}

void kprint(char *text){
    kprint_at(text,-1,-1); //neg 1 to set to current cursor position
}

void kclear_screen(){
    int vidCellsSize = MAX_COLS * MAX_ROWS;
    int i;
    unsigned char *vid_mem = (unsigned char*)VIDEO_ADDRESS;
    for(i=0;i<vidCellsSize;i++){
        vid_mem[i*2] = ' ';
        vid_mem[i*2 + 1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(get_offset(0,0));
}

/*private functions*/
int print_char(char c, int row, int col, char attr){
    unsigned char *vid_mem = (unsigned char*) VIDEO_ADDRESS;
    if(!attr){
        attr = WHITE_ON_BLACK;
    }

    if(row >= MAX_ROWS || col >= MAX_COLS){
        //let's say this is an error for now
        vid_mem[2*MAX_COLS*MAX_ROWS - 2] = 'E';
        vid_mem[2*MAX_ROWS*MAX_COLS - 1] = RED_ON_WHITE;

        return get_offset(row,col); //maybe exit with 1?
    }

    int offset;
    if(row >= 0 && col >= 0){
        offset = get_offset(row,col);
    } else {
        offset = get_cursor_offset();
    }

    if(c == '\n'){
        row = get_offset_row(offset);
        offset = get_offset(row+1,0);
    } else {
        vid_mem[offset] = c;
        vid_mem[offset+1] = attr;
        offset += 2;
    }

    set_cursor_offset(offset);
    return offset;
}

int get_cursor_offset(){
    port_byte_out(SCREEN_REG_CTRL,14);
    int offset = port_byte_in(SCREEN_REG_DATA) << 8; //high byte
    port_byte_out(SCREEN_REG_CTRL,15);
    offset += port_byte_in(SCREEN_REG_DATA); //add low byte
    return offset * 2; //each char is 2 bytes in memory
}

void set_cursor_offset(int offset){
    offset /= 2;
    port_byte_out(SCREEN_REG_CTRL,14);
    port_byte_out(SCREEN_REG_DATA,(unsigned char)(offset >> 8)); //write high byte
    port_byte_out(SCREEN_REG_CTRL,15);
    port_byte_out(SCREEN_REG_DATA,(unsigned char)(offset & 0xff)); //00000000 11111111 -> 0xff
}

int get_offset(int row, int col){
    return(2 * ((row * MAX_COLS) + col));
}
int get_offset_row(int offset){
    return(offset/(2 * MAX_COLS));
}
int get_offset_col(int offset){
    return(offset - (2 * get_offset_row(offset) * MAX_COLS))/2;
}