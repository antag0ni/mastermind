# MasterMind

A beautiful CLI implementation of the classic MasterMind (Codebreaker) game written in C with ncurses.

![C](https://img.shields.io/badge/C-99-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)

## Description

MasterMind is a code-breaking game where you try to guess a secret code. After each guess, you receive feedback on how many digits are in the correct position and how many are correct but in the wrong position.

## Features

- **Beautiful ncurses interface** with color-coded feedback
- **Responsive design** that adapts to terminal size
- **Real-time resize support** - window adjusts as you resize
- **4 difficulty levels:**
  - **Zen** - Unlimited guesses (default)
  - **Easy** - 4× digits attempts
  - **Medium** - 3× digits attempts  
  - **Hard** - 2× digits attempts
- **Customizable settings:**
  - 1-10 digit codes
  - With or without repeating numbers
- **ASCII-only display** - works on all terminals
- **Guess history** with scrolling support

## Requirements

- GCC compiler
- ncurses library
- Linux/Unix terminal

### Install Dependencies

**Debian/Ubuntu:**
```bash
sudo apt-get install build-essential libncurses5-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc ncurses-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel ncurses
```

## Installation

### Quick Install (System-wide)

```bash
make
sudo make install
```

After installation, you can run the game from anywhere:
```bash
mastermind
```

### Uninstall

```bash
sudo make uninstall
```

### Local Build (No Installation)

```bash
make
./mastermind
```

## How to Play

1. **Start the game:** Type `mastermind` in your terminal
2. **Choose settings:**
   - Number of digits (default: 4)
   - Allow repetitions (default: no)
   - Difficulty (default: Zen)
3. **Enter your guess** using only digits 0-9
4. **Read the feedback:**
   - **Position:** How many digits are in the correct position
   - **Digit:** How many digits are correct but in the wrong position
5. **Keep guessing** until you crack the code!

### Example Gameplay

```
=== MASTERMIND ===

How many digits (1-10) [default: 4]? 
Allow repeating numbers? (0=no, 1=yes) [default: 0=no]: 

Select difficulty:
  0 = Zen (unlimited guesses) [default]
  1 = Easy (16 guesses)
  2 = Medium (12 guesses)
  3 = Hard (8 guesses)
Choice [default: 0=Zen]: 
```

## Controls

- Type your guess and press **Enter**
- Resize terminal window anytime - the game will adapt
- **Ctrl+C** to quit

## Game Interface

The game features a clean, boxed interface with:
- **Header** - Game title
- **Info Panel** - Shows current settings and remaining attempts
- **Feedback Panel** - Your last guess results
- **History Panel** - All your previous guesses with results
- **Input Area** - Where you enter your guess

## Building from Source

```bash
# Clone the repository
git clone https://github.com/antag0ni/mastermind.git
cd mastermind

# Build
make

# Run locally
./mastermind

# Or install system-wide
sudo make install
mastermind
```

## Makefile Targets

- `make` - Build the game
- `make run` - Build and run
- `make clean` - Remove built files
- `make install` - Install system-wide (requires sudo)
- `make uninstall` - Remove system installation

## Technical Details

- Written in **C99**
- Uses **ncurses** for terminal UI
- Handles **SIGWINCH** for resize events
- Maximum 100 guesses tracked in history
- Supports terminals as small as 30×10

## Troubleshooting

**"Terminal too small!"**
- Resize your terminal to at least 30 columns × 10 rows

**Weird characters appearing**
- The game uses ASCII-only characters for maximum compatibility

**Colors not showing**
- Make sure your terminal supports colors
- Try running with a different terminal emulator

## License

MIT License - feel free to use, modify, and distribute!

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.

## Acknowledgments

- Classic MasterMind game by Mordecai Meirowitz
- Built with ncurses library
