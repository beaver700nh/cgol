#include <stdlib.h>
#include <sys/time.h>

#include <curses.h>

#define CGOL_LIFE (1 << 0)
#define CGOL_NEXT (1 << 1)

typedef struct {
  int rows, cols;
  int *cells;
  int is_wait;
  int wait;
  char skin;
} cgol_t;

void cgol_init(cgol_t *cgol, int rows, int cols);
void cgol_free(cgol_t *cgol);
void cgol_draw(cgol_t *cgol, WINDOW *win);
void cgol_tick(cgol_t *cgol);
void cgol_next(cgol_t *cgol);
void cgol_edit(cgol_t *cgol);
int *cgol_peek(cgol_t *cgol, int row, int col);
int  cgol_scan(cgol_t *cgol, int row, int col);

cgol_t cgol;

int main(void) {
  void main_loop(void);

  initscr();
  noecho();
  nodelay(stdscr, 1);
  keypad(stdscr, 1);
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  mouseinterval(0);

  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  cgol_init(&cgol, rows, cols);

  main_loop();

  cgol_free(&cgol);

  nocbreak();
  echo();
  curs_set(1);
  endwin();
}

void main_loop(void) {
  long long get_ms(void);

  int ch;
  long long last_tick = 0;

  for (;;) {
    while ((ch = getch()) == ERR && cgol.is_wait);

    if (ch == 'q') {
      break;
    }
    else if (ch == ' ') {
      cgol.is_wait ^= 1;
      continue;
    }
    else if (ch == 'z') {
      if ((cgol.wait & 1) == 0) {
        cgol.wait >>= 1;
      }
    }
    else if (ch == 'x') {
      if (cgol.wait < 1600) {
        cgol.wait <<= 1;
      }
    }
    else if (ch == 'c') {
      if (++cgol.skin > 0x7E) {
        cgol.skin = 0x21;
      }
    }

    if (cgol.is_wait && ch == KEY_MOUSE) {
      cgol_edit(&cgol);
    }

    if (!cgol.is_wait || ch == '\n') {
      long long this_tick = get_ms();

      if (this_tick - last_tick > cgol.wait) {
        last_tick = this_tick;
        cgol_tick(&cgol);
        cgol_next(&cgol);
      }
    }

    cgol_draw(&cgol, stdscr);
    refresh();
  }
}

long long get_ms(void) {
  static struct timeval tv;
  gettimeofday(&tv, NULL);

  return 1000 * (long long) tv.tv_sec + tv.tv_usec / 1000;
}

void cgol_init(cgol_t *cgol, int rows, int cols) {
  cgol->rows = rows;
  cgol->cols = cols;
  cgol->cells = malloc(rows * cols * sizeof (int));
  cgol->is_wait = 0;
  cgol->wait = 50;
  cgol->skin = 'O';

  *cgol_peek(cgol, 0, 2) = 1;
  *cgol_peek(cgol, 1, 0) = 1;
  *cgol_peek(cgol, 1, 1) = 1;
  *cgol_peek(cgol, 2, 1) = 1;
  *cgol_peek(cgol, 2, 2) = 1;
}

void cgol_free(cgol_t *cgol) {
  free(cgol->cells);
}

void cgol_draw(cgol_t *cgol, WINDOW *win) {
  for (int row = 0; row < cgol->rows; ++row) {
    for (int col = 0; col < cgol->cols; ++col) {
      char ch = *cgol_peek(cgol, row, col) & CGOL_LIFE ? cgol->skin : ' ';
      mvwaddch(win, row, col, ch);
    }
  }
}

void cgol_tick(cgol_t *cgol) {
  for (int row = 0; row < cgol->rows; ++row) {
    for (int col = 0; col < cgol->cols; ++col) {
      int neighbors = cgol_scan(cgol, row, col);
      int *cell = cgol_peek(cgol, row, col);

      // Birth
      if ((*cell & CGOL_LIFE) == 0 && neighbors == 3) {
        *cell |= CGOL_NEXT;
        continue;
      }

      // Death
      if ((*cell & CGOL_LIFE) && (2 > neighbors || neighbors > 3)) {
        *cell &= ~CGOL_NEXT;
        continue;
      }

      // Existence
      if (*cell & CGOL_LIFE) {
        *cell |= CGOL_NEXT;
      }
      else {
        *cell &= ~CGOL_NEXT;
      }
    }
  }
}

void cgol_next(cgol_t *cgol) {
  for (int row = 0; row < cgol->rows; ++row) {
    for (int col = 0; col < cgol->cols; ++col) {
      int *cell = cgol_peek(cgol, row, col);

      if (*cell & CGOL_NEXT) {
        *cell |= CGOL_LIFE;
      }
      else {
        *cell &= ~CGOL_LIFE;
      }
    }
  }
}

void cgol_edit(cgol_t *cgol) {
  MEVENT event;

  if (getmouse(&event) != OK) {
    return;
  }

  if (event.bstate & BUTTON1_PRESSED) {
    *cgol_peek(cgol, event.y, event.x) ^= CGOL_LIFE;
  }
}

int *cgol_peek(cgol_t *cgol, int row, int col) {
  if (0 > row) {
    row += cgol->rows;
  }
  if (row >= cgol->rows) {
    row -= cgol->rows;
  }
  if (0 > col) {
    col += cgol->cols;
  }
  if (col >= cgol->cols) {
    col -= cgol->cols;
  }

  return cgol->cells + (row * cgol->cols + col);
}

int cgol_scan(cgol_t *cgol, int row, int col) {
  int count = 0;

  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (dy == 0 && dx == 0) {
        continue;
      }

      if (*cgol_peek(cgol, row + dy, col + dx) & CGOL_LIFE) {
        ++count;
      }
    }
  }

  return count;
}
