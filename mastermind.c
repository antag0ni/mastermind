#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>

#define MAX_DIGITS 10
#define MAX_GUESSES 100

typedef struct {
    int guess[MAX_DIGITS];
    int correct_pos;
    int correct_digit;
    int num_digits;
} GuessEntry;

typedef struct {
    int digits;
    bool allow_repeats;
    int max_attempts;
    int code[MAX_DIGITS];
    GuessEntry history[MAX_GUESSES];
    int guess_count;
    bool won;
} GameState;

GameState game;

void generate_code(int *code, int digits, bool allow_repeats) {
    bool used[10] = {false};
    for (int i = 0; i < digits; i++) {
        int digit;
        do {
            digit = rand() % 10;
        } while (!allow_repeats && used[digit]);
        code[i] = digit;
        used[digit] = true;
    }
}

void draw_box(int start_y, int start_x, int height, int width, const char *title) {
    if (start_y < 0 || start_x < 0 || start_y + height > LINES || start_x + width > COLS) return;
    
    attron(COLOR_PAIR(1));
    for (int x = start_x + 1; x < start_x + width - 1 && x < COLS; x++) {
        mvaddch(start_y, x, '-');
        mvaddch(start_y + height - 1, x, '-');
    }
    for (int y = start_y + 1; y < start_y + height - 1 && y < LINES; y++) {
        mvaddch(y, start_x, '|');
        mvaddch(y, start_x + width - 1, '|');
    }
    mvaddch(start_y, start_x, '+');
    mvaddch(start_y, start_x + width - 1, '+');
    mvaddch(start_y + height - 1, start_x, '+');
    mvaddch(start_y + height - 1, start_x + width - 1, '+');
    
    if (title) {
        int title_len = strlen(title);
        int title_x = start_x + (width - title_len) / 2;
        if (title_x >= start_x && title_x + title_len <= start_x + width) {
            attron(A_BOLD);
            mvprintw(start_y, title_x, "%s", title);
            attroff(A_BOLD);
        }
    }
    attroff(COLOR_PAIR(1));
}

void draw_centered_line(int y, const char *str, int attrs) {
    int len = strlen(str);
    int x = (COLS - len) / 2;
    if (x < 0) x = 0;
    if (y >= 0 && y < LINES) {
        attron(attrs);
        mvprintw(y, x, "%s", str);
        attroff(attrs);
    }
}

void draw_header(void) {
    const char *title = "MASTERMIND";
    int title_len = strlen(title);
    int box_width = title_len + 6;
    if (box_width > COLS) box_width = COLS;
    if (box_width < 10) box_width = 10;
    int start_x = (COLS - box_width) / 2;
    if (start_x < 0) start_x = 0;
    
    draw_box(0, start_x, 3, box_width, NULL);
    draw_centered_line(1, title, COLOR_PAIR(3) | A_BOLD);
}

void draw_info_compact(int y, int x, int width, int digits, bool allow_repeats) {
    if (x + width > COLS) width = COLS - x;
    if (width < 20) return;
    
    int height = 5;
    if (y + height > LINES) return;
    
    draw_box(y, x, height, width, " INFO ");
    
    attron(COLOR_PAIR(2));
    if (y + 2 < LINES && x + 2 < COLS) mvprintw(y + 2, x + 2, "Digits: %d", digits);
    if (y + 3 < LINES && x + 2 < COLS) mvprintw(y + 3, x + 2, "Repeats: %s", allow_repeats ? "Yes" : "No");
    if (game.max_attempts > 0) {
        int remaining = game.max_attempts - game.guess_count;
        if (y + 4 < LINES && x + 2 < COLS) mvprintw(y + 4, x + 2, "Left: %d/%d", remaining, game.max_attempts);
    } else {
        if (y + 4 < LINES && x + 2 < COLS) mvprintw(y + 4, x + 2, "Zen mode");
    }
    attroff(COLOR_PAIR(2));
}

void draw_info_full(int y, int x, int width, int digits, bool allow_repeats) {
    if (x + width > COLS) width = COLS - x;
    if (width < 25) {
        draw_info_compact(y, x, width, digits, allow_repeats);
        return;
    }
    
    int height = 6;
    if (y + height > LINES) return;
    
    draw_box(y, x, height, width, " GAME INFO ");
    
    attron(COLOR_PAIR(2));
    if (y + 2 < LINES && x + 2 < COLS) mvprintw(y + 2, x + 2, "Code length: %d digits", digits);
    if (y + 3 < LINES && x + 2 < COLS) mvprintw(y + 3, x + 2, "Repetitions: %s", allow_repeats ? "Allowed" : "No");
    if (game.max_attempts > 0) {
        int remaining = game.max_attempts - game.guess_count;
        if (y + 4 < LINES && x + 2 < COLS) mvprintw(y + 4, x + 2, "Remaining: %d/%d", remaining, game.max_attempts);
    } else {
        if (y + 4 < LINES && x + 2 < COLS) mvprintw(y + 4, x + 2, "Mode: Zen (unlimited)");
    }
    attroff(COLOR_PAIR(2));
}

void draw_history(int y, int x, int width, int height, GuessEntry *history, int count, int digits) {
    if (y < 0 || x < 0 || height < 5 || width < 20) return;
    if (y + height > LINES) height = LINES - y;
    if (x + width > COLS) width = COLS - x;
    
    draw_box(y, x, height, width, " HISTORY ");
    
    if (count == 0) {
        attron(COLOR_PAIR(4));
        if (y + 2 < LINES && x + 2 < COLS) mvprintw(y + 2, x + 2, "No guesses yet...");
        attroff(COLOR_PAIR(4));
        return;
    }
    
    bool show_pos = (width >= 28);
    bool show_dig = (width >= 35);
    
    attron(COLOR_PAIR(2) | A_BOLD);
    if (y + 2 < LINES && x + 2 < COLS) mvprintw(y + 2, x + 2, "#");
    if (y + 2 < LINES && x + 6 < COLS) mvprintw(y + 2, x + 6, "Guess");
    if (show_pos && y + 2 < LINES && x + 18 < COLS) mvprintw(y + 2, x + 18, "Pos");
    if (show_dig && y + 2 < LINES && x + 25 < COLS) mvprintw(y + 2, x + 25, "Dig");
    attroff(COLOR_PAIR(2) | A_BOLD);
    
    attron(COLOR_PAIR(1));
    for (int i = x + 1; i < x + width - 1 && i < COLS; i++) {
        if (y + 3 < LINES) mvaddch(y + 3, i, '-');
    }
    attroff(COLOR_PAIR(1));
    
    int data_rows = height - 5;
    int start_idx = (count > data_rows) ? count - data_rows : 0;
    
    for (int i = start_idx; i < count; i++) {
        int row = y + 4 + (i - start_idx);
        if (row >= LINES - 1) break;
        
        if ((i - start_idx) % 2 == 0) attron(COLOR_PAIR(5));
        
        if (x + 2 < COLS) mvprintw(row, x + 2, "%2d", i + 1);
        
        char guess_str[11];
        for (int j = 0; j < history[i].num_digits; j++) {
            guess_str[j] = '0' + history[i].guess[j];
        }
        guess_str[history[i].num_digits] = '\0';
        if (x + 6 < COLS) mvprintw(row, x + 6, "%s", guess_str);
        
        if (show_pos) {
            if (history[i].correct_pos == digits) attron(COLOR_PAIR(6) | A_BOLD);
            else attron(COLOR_PAIR(7));
            if (x + 20 < COLS) mvprintw(row, x + 20, "%d", history[i].correct_pos);
            attroff(COLOR_PAIR(6) | A_BOLD);
            attroff(COLOR_PAIR(7));
        }
        
        if (show_dig && x + 27 < COLS) mvprintw(row, x + 27, "%d", history[i].correct_digit);
        
        if ((i - start_idx) % 2 == 0) attroff(COLOR_PAIR(5));
    }
    
    if (count > data_rows && y + height - 2 < LINES && x + 2 < COLS) {
        attron(COLOR_PAIR(4));
        mvprintw(y + height - 2, x + 2, "... %d more", count - data_rows);
        attroff(COLOR_PAIR(4));
    }
}

void draw_feedback(int y, int x, int width, int correct_pos, int correct_digit, int digits) {
    if (x + width > COLS) width = COLS - x;
    if (width < 20) return;
    
    int height = (width < 28) ? 4 : 5;
    if (y + height > LINES) return;
    
    draw_box(y, x, height, width, " FEEDBACK ");
    
    int row = y + 2;
    if (row < LINES && x + 2 < COLS) {
        attron(COLOR_PAIR(6));
        mvprintw(row, x + 2, "Position:");
        attroff(COLOR_PAIR(6));
        attron(A_BOLD);
        if (correct_pos == digits) attron(COLOR_PAIR(6));
        printw(" %d/%d", correct_pos, digits);
        if (correct_pos == digits) attroff(COLOR_PAIR(6));
        attroff(A_BOLD);
    }
    
    if (height > 3) {
        row++;
        if (row < LINES && x + 2 < COLS) {
            attron(COLOR_PAIR(7));
            mvprintw(row, x + 2, "Digit:   ");
            attroff(COLOR_PAIR(7));
            attron(A_BOLD);
            printw(" %d", correct_digit);
            attroff(A_BOLD);
        }
    }
}

void draw_input(int y, int x, int width, int digits) {
    if (x + width > COLS) width = COLS - x;
    if (width < 25 || y + 4 > LINES) return;
    
    draw_box(y, x, 4, width, " YOUR GUESS ");
    
    if (y + 2 < LINES && x + 2 < COLS) {
        attron(COLOR_PAIR(2));
        mvprintw(y + 2, x + 2, "Enter %d digits: ", digits);
        attroff(COLOR_PAIR(2));
    }
}

void calculate_layout(int *info_y, int *info_x, int *info_w,
                     int *hist_y, int *hist_x, int *hist_w, int *hist_h,
                     int *fb_y, int *fb_x, int *fb_w,
                     int *in_y, int *in_x, int *in_w,
                     int guess_count) {
    (void)game.digits; // digits is accessible via game struct
    int padding = 1;
    int header_h = 4;
    int input_h = 4;
    int footer_space = 1;
    
    bool horizontal_layout = (COLS >= 70 && LINES >= 20);
    bool compact = (COLS < 50 || LINES < 18);
    
    int available_h = LINES - header_h - footer_space;
    int available_w = COLS - 2 * padding;
    
    if (horizontal_layout) {
        // Side-by-side layout
        int col_w = (available_w - padding) / 2;
        
        *info_x = padding;
        *info_y = header_h;
        *info_w = col_w;
        
        *hist_x = padding + col_w + padding;
        *hist_y = header_h;
        *hist_w = col_w;
        *hist_h = available_h - input_h;
        
        *fb_x = padding;
        *fb_y = header_h + (compact ? 5 : 7);
        *fb_w = col_w;
        
        *in_x = padding;
        *in_y = LINES - input_h - footer_space;
        *in_w = col_w;
    } else {
        // Vertical stack layout
        *info_x = padding;
        *info_y = header_h;
        *info_w = available_w;
        
        int info_height = compact ? 5 : 7;
        int fb_height = compact ? 0 : 5;
        
        if (guess_count > 0 && !compact) {
            *fb_x = padding;
            *fb_y = header_h + info_height + padding;
            *fb_w = available_w;
        } else {
            *fb_y = -1;
        }
        
        *hist_x = padding;
        *hist_y = header_h + info_height + padding + (fb_height > 0 ? fb_height + padding : 0);
        *hist_w = available_w;
        *hist_h = available_h - info_height - (fb_height > 0 ? fb_height + padding : 0) - input_h - padding * 2;
        
        *in_x = padding;
        *in_y = LINES - input_h - footer_space;
        *in_w = available_w;
    }
}

void draw_screen(void) {
    clear();
    
    if (LINES < 10 || COLS < 30) {
        draw_centered_line(LINES / 2, "Terminal too small!", COLOR_PAIR(8) | A_BOLD);
        return;
    }
    
    draw_header();
    
    int info_y, info_x, info_w;
    int hist_y, hist_x, hist_w, hist_h;
    int fb_y, fb_x, fb_w;
    int in_y, in_x, in_w;
    
    calculate_layout(&info_y, &info_x, &info_w,
                    &hist_y, &hist_x, &hist_w, &hist_h,
                    &fb_y, &fb_x, &fb_w,
                    &in_y, &in_x, &in_w,
                    game.guess_count);
    
    // Draw info panel
    if (info_w >= 30) {
        draw_info_full(info_y, info_x, info_w, game.digits, game.allow_repeats);
    } else {
        draw_info_compact(info_y, info_x, info_w, game.digits, game.allow_repeats);
    }
    
    // Draw history panel - use all available space
    if (hist_h > 5) {
        draw_history(hist_y, hist_x, hist_w, hist_h, game.history, game.guess_count, game.digits);
    }
    
    // Draw feedback
    if (fb_y >= 0 && game.guess_count > 0) {
        draw_feedback(fb_y, fb_x, fb_w,
                     game.history[game.guess_count - 1].correct_pos,
                     game.history[game.guess_count - 1].correct_digit,
                     game.digits);
    }
    
    // Draw input area
    if (in_y >= 0) {
        draw_input(in_y, in_x, in_w, game.digits);
    }
    
    refresh();
}

volatile sig_atomic_t resize_flag = 0;

void handle_resize(int sig) {
    (void)sig;
    resize_flag = 1;
}

void evaluate_guess(int *code, int *guess, int digits, int *correct_pos, int *correct_digit) {
    *correct_pos = 0;
    *correct_digit = 0;
    
    bool code_used[MAX_DIGITS] = {false};
    bool guess_used[MAX_DIGITS] = {false};
    
    for (int i = 0; i < digits; i++) {
        if (guess[i] == code[i]) {
            (*correct_pos)++;
            code_used[i] = true;
            guess_used[i] = true;
        }
    }
    
    for (int i = 0; i < digits; i++) {
        if (guess_used[i]) continue;
        for (int j = 0; j < digits; j++) {
            if (code_used[j]) continue;
            if (guess[i] == code[j]) {
                (*correct_digit)++;
                code_used[j] = true;
                break;
            }
        }
    }
}

bool get_guess(int *guess, int digits) {
    char input[MAX_DIGITS + 1];
    
    echo();
    if (scanw("%s", input) != 1) {
        noecho();
        return false;
    }
    noecho();
    
    if ((int)strlen(input) != digits) {
        return false;
    }
    
    for (int i = 0; i < digits; i++) {
        if (input[i] < '0' || input[i] > '9') {
            return false;
        }
        guess[i] = input[i] - '0';
    }
    
    return true;
}

void draw_win_screen(int attempts) {
    clear();
    
    const char *title = "*** CONGRATULATIONS! ***";
    int box_width = strlen(title) + 10;
    if (box_width > COLS - 4) box_width = COLS - 4;
    if (box_width < 30) box_width = 30;
    
    int start_x = (COLS - box_width) / 2;
    int start_y = LINES / 2 - 3;
    if (start_y < 0) start_y = 0;
    if (start_x < 0) start_x = 0;
    
    draw_box(start_y, start_x, 7, box_width, NULL);
    
    attron(COLOR_PAIR(6) | A_BOLD);
    draw_centered_line(start_y + 2, title, COLOR_PAIR(6) | A_BOLD);
    
    char msg[50];
    snprintf(msg, sizeof(msg), "You cracked the code in %d attempts!", attempts);
    draw_centered_line(start_y + 4, msg, COLOR_PAIR(6) | A_BOLD);
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    draw_centered_line(LINES - 3, "Press any key to exit...", COLOR_PAIR(2));
    
    refresh();
    getch();
}

void draw_game_over_screen(void) {
    clear();
    
    const char *title = "*** GAME OVER ***";
    int box_width = strlen(title) + 10;
    if (box_width > COLS - 4) box_width = COLS - 4;
    if (box_width < 30) box_width = 30;
    
    int start_x = (COLS - box_width) / 2;
    int start_y = LINES / 2 - 3;
    if (start_y < 0) start_y = 0;
    if (start_x < 0) start_x = 0;
    
    draw_box(start_y, start_x, 7, box_width, NULL);
    
    attron(COLOR_PAIR(8) | A_BOLD);
    draw_centered_line(start_y + 2, title, COLOR_PAIR(8) | A_BOLD);
    
    char msg[50];
    if (game.max_attempts > 0) {
        snprintf(msg, sizeof(msg), "Max attempts (%d) reached!", game.max_attempts);
    } else {
        snprintf(msg, sizeof(msg), "Game Over!");
    }
    draw_centered_line(start_y + 4, msg, COLOR_PAIR(2));
    
    char msg2[50];
    snprintf(msg2, sizeof(msg2), "You made %d guesses.", game.guess_count);
    draw_centered_line(start_y + 5, msg2, COLOR_PAIR(2));
    attroff(COLOR_PAIR(8) | A_BOLD);
    
    draw_centered_line(LINES - 3, "Press any key to exit...", COLOR_PAIR(2));
    
    refresh();
    getch();
}

void init_colors(void) {
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_WHITE, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(5, COLOR_BLUE, COLOR_BLACK);
        init_pair(6, COLOR_GREEN, COLOR_BLACK);
        init_pair(7, COLOR_YELLOW, COLOR_BLACK);
        init_pair(8, COLOR_RED, COLOR_BLACK);
    }
}

int main() {
    srand(time(NULL));
    
    char input[20];
    
    system("clear");
    printf("=== MASTERMIND ===\n\n");
    
    printf("How many digits (1-10) [default: 4]? ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) {
            game.digits = 4;
        } else {
            game.digits = atoi(input);
        }
    } else {
        game.digits = 4;
    }
    
    if (game.digits < 1 || game.digits > 10) {
        game.digits = 4;
    }
    
    printf("Allow repeating numbers? (0=no, 1=yes) [default: 0=no]: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) {
            game.allow_repeats = false;
        } else {
            game.allow_repeats = (atoi(input) != 0);
        }
    } else {
        game.allow_repeats = false;
    }
    
    if (!game.allow_repeats && game.digits > 10) {
        printf("Cannot have more than 10 digits without repetition!\n");
        return 1;
    }
    
    printf("\nSelect difficulty:\n");
    printf("  0 = Zen (unlimited guesses) [default]\n");
    printf("  1 = Easy (%d guesses)\n", game.digits * 4);
    printf("  2 = Medium (%d guesses)\n", game.digits * 3);
    printf("  3 = Hard (%d guesses)\n", game.digits * 2);
    printf("Choice [default: 0=Zen]: ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        int difficulty = 0;
        if (strlen(input) > 0) {
            difficulty = atoi(input);
            if (difficulty < 0 || difficulty > 3) {
                difficulty = 0;
            }
        }
        
        switch (difficulty) {
            case 0: game.max_attempts = -1; break;  // Zen: unlimited
            case 1: game.max_attempts = game.digits * 4; break;  // Easy
            case 2: game.max_attempts = game.digits * 3; break;  // Medium
            case 3: game.max_attempts = game.digits * 2; break;  // Hard
            default: game.max_attempts = -1; break;
        }
    } else {
        game.max_attempts = -1;  // Default: Zen
    }
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    
    init_colors();
    signal(SIGWINCH, handle_resize);
    
    generate_code(game.code, game.digits, game.allow_repeats);
    game.guess_count = 0;
    game.won = false;
    
    draw_screen();
    
    while (!game.won) {
        int _, __, ___;
        int in_y, in_x, in_w;
        
        calculate_layout(&_, &__, &___,
                        &_, &_, &_, &_,
                        &_, &_, &_,
                        &in_y, &in_x, &in_w,
                        game.guess_count);
        
        int input_x = in_x + 17;
        
        if (in_y >= 0 && in_y < LINES && input_x < COLS) {
            move(in_y + 2, input_x + 2);
        }
        refresh();
        
        // Check if resize happened via signal
        if (resize_flag) {
            resize_flag = 0;
            // Reinitialize ncurses after resize
            endwin();
            refresh();
            clear();
            draw_screen();
            continue;
        }
        
        timeout(100);
        int ch = getch();
        timeout(-1);
        
        if (ch == ERR) {
            // Timeout - check for resize flag
            if (resize_flag) {
                resize_flag = 0;
                endwin();
                refresh();
                clear();
                draw_screen();
            }
            continue;
        }
        
        if (ch == KEY_RESIZE) {
            endwin();
            refresh();
            clear();
            draw_screen();
            continue;
        }
        
        ungetch(ch);
        
        int guess[MAX_DIGITS];
        if (!get_guess(guess, game.digits)) {
            attron(COLOR_PAIR(8));
            if (in_y - 1 >= 0 && in_y - 1 < LINES) {
                mvprintw(in_y - 1, in_x + 2, "Invalid input!");
            }
            attroff(COLOR_PAIR(8));
            refresh();
            napms(1000);
            draw_screen();
            continue;
        }
        
        int correct_pos, correct_digit;
        evaluate_guess(game.code, guess, game.digits, &correct_pos, &correct_digit);
        
        for (int i = 0; i < game.digits; i++) {
            game.history[game.guess_count].guess[i] = guess[i];
        }
        game.history[game.guess_count].correct_pos = correct_pos;
        game.history[game.guess_count].correct_digit = correct_digit;
        game.history[game.guess_count].num_digits = game.digits;
        game.guess_count++;
        
        // Check if max attempts reached (skip if Zen mode with -1)
        if (game.max_attempts > 0 && game.guess_count >= game.max_attempts && correct_pos != game.digits) {
            draw_screen();
            napms(500);
            draw_game_over_screen();
            endwin();
            return 0;
        }
        
        if (correct_pos == game.digits) {
            game.won = true;
            draw_screen();
            napms(500);
            draw_win_screen(game.guess_count);
        } else {
            draw_screen();
        }
    }
    
    endwin();
    return 0;
}
