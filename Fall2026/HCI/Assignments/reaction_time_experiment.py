import sys
import os
import math
import random
import time
import csv
import pygame
import numpy as np

# ----------------- CONFIGURATION & CONSTANTS -----------------
SCREEN_WIDTH = 900
SCREEN_HEIGHT = 700
FPS = 60

# Colors (RGB)
BG_COLOR = (240, 242, 245)
COLOR_START = (220, 50, 47)          # Red
COLOR_START_TEXT = (255, 255, 255)
COLOR_TARGET_DEFAULT = (44, 123, 229) # Blue
COLOR_TARGET_ACTIVE = (255, 220, 0)   # Yellow
COLOR_TEXT = (33, 37, 41)
COLOR_WHITE = (255, 255, 255)

START_BTN_W = 120
START_BTN_H = 60
TARGET_BTN_W = 110
TARGET_BTN_H = 75
RADIUS = 240  # Equal distance from center to target centers

BLOCK_TARGET_COUNTS = [2, 4, 8]  # Block 1: 2, Block 2: 4, Block 3: 8
TRIALS_PER_BLOCK = 20

CSV_FILENAME = "reaction_time_data.csv"

# ----------------- SOUND GENERATION -----------------
def generate_beep_sound(frequency=650, duration=0.12, sample_rate=44100):
    """Generates an in-memory sound tone to satisfy the audio feedback requirement without assets."""
    total_samples = int(duration * sample_rate)
    t = np.linspace(0, duration, total_samples, endpoint=False)
    waveform = 0.4 * np.sin(2 * np.pi * frequency * t)
    envelope = np.ones(total_samples)
    attack = int(0.01 * sample_rate)
    decay = int(0.02 * sample_rate)
    if total_samples > attack + decay:
        envelope[:attack] = np.linspace(0, 1, attack)
        envelope[-decay:] = np.linspace(1, 0, decay)
    waveform = waveform * envelope
    waveform_integers = np.int16(waveform * 32767)
    stereo_array = np.column_stack((waveform_integers, waveform_integers))
    return pygame.sndarray.make_sound(stereo_array)

# ----------------- CSV INITIALIZATION -----------------
def init_csv():
    if not os.path.exists(CSV_FILENAME):
        with open(CSV_FILENAME, mode='w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(["user#", "block#", "trial#", "elapsedTime", "numberOferrors"])

def log_trial_data(user_id, block_id, trial_id, elapsed_time_ms, errors):
    row = [user_id, block_id, trial_id, elapsed_time_ms, errors]
    print(f"{user_id}, {block_id}, {trial_id}, {elapsed_time_ms}, {errors}")
    with open(CSV_FILENAME, mode='a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(row)

# ----------------- TARGET POSITION UTILS -----------------
def get_target_rects(num_targets, center_x, center_y):
    """Computes positions arranged in a circle at equal distances from the center."""
    rects = []
    angle_offset = -math.pi / 2  # Start at the top (12 o'clock)
    for i in range(num_targets):
        angle = angle_offset + (2 * math.pi * i / num_targets)
        tx = center_x + RADIUS * math.cos(angle)
        ty = center_y + RADIUS * math.sin(angle)
        rect = pygame.Rect(0, 0, TARGET_BTN_W, TARGET_BTN_H)
        rect.center = (int(tx), int(ty))
        rects.append(rect)
    return rects

# ----------------- MAIN EXPERIMENT APPLICATION -----------------
def run_experiment():
    pygame.init()
    pygame.mixer.init(frequency=44100, size=-16, channels=2)
    beep_sound = generate_beep_sound()
    init_csv()

    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("CS 4065/6065: Choice Reaction Time System")
    clock = pygame.time.Clock()

    font_title = pygame.font.SysFont("Arial", 28, bold=True)
    font_btn = pygame.font.SysFont("Arial", 20, bold=True)
    font_info = pygame.font.SysFont("Arial", 16)

    center_x = SCREEN_WIDTH // 2
    center_y = SCREEN_HEIGHT // 2

    # Layout elements
    start_rect = pygame.Rect(0, 0, START_BTN_W, START_BTN_H)
    start_rect.center = (center_x, center_y)

    continue_rect = pygame.Rect(0, 0, 220, 60)
    continue_rect.center = (center_x, center_y)

    # Completion screen buttons: Resume vs Exit
    next_user_rect = pygame.Rect(center_x - 220, center_y + 60, 200, 55)
    exit_prog_rect = pygame.Rect(center_x + 20, center_y + 60, 200, 55)

    # State variables
    state = "INPUT_USER"
    user_id_text = ""
    current_block_idx = 0
    current_trial = 1
    num_errors = 0
    start_time = 0
    active_target_idx = -1
    target_rects = []

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break

            # ----------------- 1. USER ID PROMPT SCREEN -----------------
            if state == "INPUT_USER":
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_RETURN:
                        if user_id_text.strip():
                            current_block_idx = 0
                            current_trial = 1
                            num_errors = 0
                            target_rects = get_target_rects(BLOCK_TARGET_COUNTS[current_block_idx], center_x, center_y)
                            state = "READY_FOR_START"
                    elif event.key == pygame.K_BACKSPACE:
                        user_id_text = user_id_text[:-1]
                    else:
                        if event.unicode.isprintable():
                            user_id_text += event.unicode

            # ----------------- 2. WAITING TO CLICK CENTER 'START' -----------------
            elif state == "READY_FOR_START":
                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                    if start_rect.collidepoint(event.pos):
                        active_target_idx = random.randint(0, len(target_rects) - 1)
                        num_errors = 0
                        start_time = time.perf_counter()
                        state = "WAITING_TARGET_CLICK"

            # ----------------- 3. TARGET LIT UP -----------------
            elif state == "WAITING_TARGET_CLICK":
                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                    pos = event.pos
                    # Target hit
                    if target_rects[active_target_idx].collidepoint(pos):
                        elapsed_time_ms = int((time.perf_counter() - start_time) * 1000)
                        beep_sound.play()

                        log_trial_data(
                            user_id=user_id_text.strip(),
                            block_id=current_block_idx + 1,
                            trial_id=current_trial,
                            elapsed_time_ms=elapsed_time_ms,
                            errors=num_errors
                        )

                        if current_trial < TRIALS_PER_BLOCK:
                            current_trial += 1
                            state = "READY_FOR_START"
                        else:
                            # 20 trials completed for this block
                            if current_block_idx < len(BLOCK_TARGET_COUNTS) - 1:
                                current_block_idx += 1
                                current_trial = 1
                                target_rects = get_target_rects(BLOCK_TARGET_COUNTS[current_block_idx], center_x, center_y)
                                state = "REST_SCREEN"
                            else:
                                state = "COMPLETE"
                    else:
                        # Misclick
                        num_errors += 1

            # ----------------- 4. REST / CONTINUE SCREEN BETWEEN BLOCKS -----------------
            elif state == "REST_SCREEN":
                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                    if continue_rect.collidepoint(event.pos):
                        state = "READY_FOR_START"

            # ----------------- 5. EXPERIMENT COMPLETE (LOOP OR EXIT) -----------------
            elif state == "COMPLETE":
                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                    if next_user_rect.collidepoint(event.pos):
                        # Reset for next participant
                        user_id_text = ""
                        current_block_idx = 0
                        current_trial = 1
                        num_errors = 0
                        active_target_idx = -1
                        state = "INPUT_USER"
                    elif exit_prog_rect.collidepoint(event.pos):
                        running = False
                        break

        # ----------------- RENDERING -----------------
        screen.fill(BG_COLOR)

        if state == "INPUT_USER":
            prompt_surf = font_title.render("Enter User Number / ID, then press Enter:", True, COLOR_TEXT)
            screen.blit(prompt_surf, (center_x - prompt_surf.get_width() // 2, center_y - 80))

            input_box = pygame.Rect(center_x - 150, center_y - 20, 300, 50)
            pygame.draw.rect(screen, COLOR_WHITE, input_box, border_radius=6)
            pygame.draw.rect(screen, (100, 100, 100), input_box, 2, border_radius=6)

            val_surf = font_title.render(user_id_text, True, COLOR_TEXT)
            screen.blit(val_surf, (input_box.x + 15, input_box.y + 10))

        elif state in ["READY_FOR_START", "WAITING_TARGET_CLICK"]:
            hud_str = f"User: {user_id_text.strip()}   |   Block: {current_block_idx + 1}/3 ({BLOCK_TARGET_COUNTS[current_block_idx]} targets)   |   Trial: {current_trial}/{TRIALS_PER_BLOCK}"
            hud_surf = font_info.render(hud_str, True, COLOR_TEXT)
            screen.blit(hud_surf, (20, 20))

            # Draw outer target buttons
            for idx, rect in enumerate(target_rects):
                if state == "WAITING_TARGET_CLICK" and idx == active_target_idx:
                    color = COLOR_TARGET_ACTIVE
                else:
                    color = COLOR_TARGET_DEFAULT
                pygame.draw.rect(screen, color, rect, border_radius=8)
                pygame.draw.rect(screen, (30, 30, 30), rect, 2, border_radius=8)

            # Draw central start button
            start_color = COLOR_START if state == "READY_FOR_START" else (180, 180, 180)
            pygame.draw.rect(screen, start_color, start_rect, border_radius=8)
            pygame.draw.rect(screen, (30, 30, 30), start_rect, 2, border_radius=8)

            btn_txt = font_btn.render("Start", True, COLOR_START_TEXT)
            screen.blit(btn_txt, (start_rect.centerx - btn_txt.get_width() // 2,
                                  start_rect.centery - btn_txt.get_height() // 2))

        elif state == "REST_SCREEN":
            title_surf = font_title.render(f"Block {current_block_idx} Complete!", True, COLOR_TEXT)
            screen.blit(title_surf, (center_x - title_surf.get_width() // 2, center_y - 100))

            pygame.draw.rect(screen, (40, 167, 69), continue_rect, border_radius=8)
            pygame.draw.rect(screen, (20, 100, 40), continue_rect, 2, border_radius=8)

            cont_txt = font_btn.render("Click to Continue", True, COLOR_WHITE)
            screen.blit(cont_txt, (continue_rect.centerx - cont_txt.get_width() // 2,
                                   continue_rect.centery - cont_txt.get_height() // 2))

        elif state == "COMPLETE":
            done_surf = font_title.render(f"Participant '{user_id_text.strip()}' Complete!", True, (40, 167, 69))
            screen.blit(done_surf, (center_x - done_surf.get_width() // 2, center_y - 70))
            
            sub_surf = font_info.render(f"Data recorded to '{CSV_FILENAME}'.", True, COLOR_TEXT)
            screen.blit(sub_surf, (center_x - sub_surf.get_width() // 2, center_y - 20))

            # Resume button (Next Participant)
            pygame.draw.rect(screen, (44, 123, 229), next_user_rect, border_radius=8)
            pygame.draw.rect(screen, (20, 60, 120), next_user_rect, 2, border_radius=8)
            next_txt = font_btn.render("Next Participant", True, COLOR_WHITE)
            screen.blit(next_txt, (next_user_rect.centerx - next_txt.get_width() // 2,
                                   next_user_rect.centery - next_txt.get_height() // 2))

            # Close button (Exit Program)
            pygame.draw.rect(screen, (220, 53, 69), exit_prog_rect, border_radius=8)
            pygame.draw.rect(screen, (120, 20, 30), exit_prog_rect, 2, border_radius=8)
            exit_txt = font_btn.render("Exit Program", True, COLOR_WHITE)
            screen.blit(exit_txt, (exit_prog_rect.centerx - exit_txt.get_width() // 2,
                                   exit_prog_rect.centery - exit_txt.get_height() // 2))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    run_experiment()