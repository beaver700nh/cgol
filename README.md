# CGOL

Minimal implementation of Conway's Game of Life.

## Compilation

```shell
$ gcc main.c -lcurses -o main
$ ./main
```

## Keybinds:

| Key   | Action     | Description                                      |
|-------|------------|--------------------------------------------------|
| Q     | Quit       | Exits the program.                               |
| Z     | Faster     | Doubles the speed.                               |
| X     | Slower     | Halves the speed.                                |
| C     | Skin       | Scrolls through ASCII characters 0x21-0x7E.      |
| space | Play/Pause | Toggle between edit/tick mode and free run mode. |

Use the mouse to toggle cells between alive and dead, while in edit mode.
