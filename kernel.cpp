#define VGA_BLACK         0
#define VGA_BLUE          1
#define VGA_GREEN         2
#define VGA_CYAN          3
#define VGA_RED           4
#define VGA_MAGENTA       5
#define VGA_BROWN         6
#define VGA_LIGHT_GREY    7
#define VGA_DARK_GREY     8
#define VGA_LIGHT_BLUE    9
#define VGA_LIGHT_GREEN   10
#define VGA_LIGHT_CYAN    11
#define VGA_LIGHT_RED     12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN   14
#define VGA_WHITE         15

#define VGA_ATTR(fg, bg) ((bg << 4) | (fg & 0x0F))

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

int cursor_x = 0;
int cursor_y = 0;


static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}


void update_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;

    // send high byte
    outb(0x3D4, 14);
    outb(0x3D5, (pos >> 8) & 0xFF);

    // send low byte
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0xFF);
}


static inline volatile char* vga_addr(int row, int col) {
    return (volatile char*)0xB8000 + (row * 80 + col) * 2;
}


int strlen(const char *s) {
    int n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

void scroll_up() {
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            volatile char* src = vga_addr(col, row);
            volatile char* dst = vga_addr(col, row - 1);
            dst[0] = src[0];         
            dst[1] = src[1];         
        }
    }

    for (int col = 0; col < VGA_WIDTH; col++) {
        volatile char* p = vga_addr(col, VGA_HEIGHT - 1);
        p[0] = ' ';
        p[1] = VGA_ATTR(VGA_LIGHT_GREY, VGA_BLACK);
    }

    cursor_y = VGA_HEIGHT - 1; 
}


void printchar(const char c) {

    // handle newline
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }

    // wrap horizontally
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
      scroll_up();
    }

    volatile char *p = vga_addr(cursor_y, cursor_x);
    p[0] = c;
    p[1] = VGA_ATTR(VGA_LIGHT_GREY, VGA_BLACK);

    cursor_x++;
    update_cursor();
}


void printchar_ex(const char c, const int charoffset, const int fg, const int bg){
  volatile char *p = (volatile char*)0xB8000 + (charoffset * 2);
  p[0] = c;
  p[1] = VGA_ATTR(fg, bg);
}

void print(const char* s){
    int n = 0;
    while (s[n] != '\0') {
        printchar(s[n]);
        n++;
    }
  return;
}


void printarray_ex(const char* chararray, const int fg, int const bg){
  int len = strlen(chararray);
  for(int i = 0; i < len; i++){
    printchar_ex(chararray[i], i, fg, bg);
  }
  return;
}




extern "C" void kernel_main(){
  print("kOS v0.1\n");
  print("> ");
  return;
}
