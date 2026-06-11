import time
import win32com.client
import sys
import os
import subprocess
import re
from selenium import webdriver
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.chrome.options import Options
from ctypes import cast, POINTER
import pythoncom
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
import win32gui
from datetime import datetime
import win32con
import threading
import shutil
import argparse
import requests
BASE_DIR = os.path.dirname(os.path.abspath(sys.argv[0]))
import psutil
import socket
import tempfile
import pandas as pd
import pyautogui
import random
import traceback
import json
import atexit
import platform
import pygetwindow as gw
import csv
import statistics
import zipfile

# --------------------------
# JSON logging setup
# --------------------------
RESULT_JSON = {
    "system": {},
    "excel": [],
    "url": [],
    "file_transfer": {},
    "ppt": [],
    "project": [],
    "blender": [],
    "photoshop": [],
    "gaming": {}
}

BASE_DIR = os.path.dirname(os.path.abspath(sys.argv[0]))
json_path = os.path.join(BASE_DIR, "Temp_Data", "performance_result.json")

def save_json():
    os.makedirs(os.path.dirname(json_path), exist_ok=True)
    with open(json_path, "w") as jf:
        json.dump(RESULT_JSON, jf, indent=4)
    print(" Performance JSON generated:", json_path)

atexit.register(save_json)

# --------------------------
# Your existing functions
# --------------------------
def json_log(section, data):
    if section not in RESULT_JSON:
        RESULT_JSON[section] = []

    if isinstance(RESULT_JSON[section], list):
        RESULT_JSON[section].append(data)
    else:
        RESULT_JSON[section].update(data)


# ----------------------------- WINDOW HANDLING -----------------------------
# REPLACE WITH:
def bring_window_to_front(window_title):
    """Bring the window with the given title to the front using win32gui."""
    def enum_windows_proc(hWnd, lParam):
        window_title_from_hwnd = win32gui.GetWindowText(hWnd)
        if window_title.lower() in window_title_from_hwnd.lower():
            win32gui.ShowWindow(hWnd, win32con.SW_RESTORE)
            win32gui.ShowWindow(hWnd, win32con.SW_MAXIMIZE)
            try:
                win32gui.SetForegroundWindow(hWnd)
                print(f"Window '{window_title}' brought to the front: {window_title_from_hwnd}")
            except Exception as e:
                print(f"Warning bring_window_to_front '{window_title}': {e}")
            return False
        return True
    win32gui.EnumWindows(enum_windows_proc, 0)

# ----------------------------- EXCEL FUNCTIONS -----------------------------
def open_excel():
    try:
        excel = win32com.client.Dispatch("Excel.Application")
        excel.Visible = True
        time.sleep(3)
        bring_window_to_front("Excel")
        print("Microsoft Excel has been launched and brought to the front.\n")
        file_obj.write(f"opening Excel susscessful\n")
        return excel
    except Exception as e:
        print(f"Error opening Excel: {str(e)}")
        file_obj.write(f"Error opening Excel: {str(e)}\n")
        return None

def run_excel_macro(excel_path, macro_name=None):
    if not os.path.exists(excel_path):
        print(f"Error: The file {excel_path} does not exist.")
        file_obj.write(f"Error: The file {excel_path} does not exist.\n")
        return

    try:
        excel = open_excel()
        workbook = excel.Workbooks.Open(excel_path)
        print('excel path=======',excel_path)
        time.sleep(2)
        try:
            if macro_name:
                excel.Application.Run(macro_name)
                print(f"Successfully ran Excel macro: {macro_name}")
                file_obj.write(f"Successfully ran Excel macro: {macro_name}\n")
        except Exception as e:
            print(f"Error running the macro '{macro_name}': {str(e)}")
            file_obj.write(f"Error running the macro '{macro_name}': {str(e)}\n")
        workbook.Close(False)
        excel.Quit()
        print("Excel workbook closed without saving.")
        file_obj.write("Excel workbook closed without saving.\n")
    except Exception as e:
        print(f"Error opening Excel or running the macro: {str(e)}")
        file_obj.write(f"Error opening Excel or running the macro: {str(e)}\n")

# ----------------------------- URL HANDLING -----------------------------
def handle_video_and_scrolling(url, play_time, scroll_up_down_moves, scroll_left_right_moves, file_obj, no_of_fishes):
    scroll = False
    try:
        url_start_time = time.time()
        if len(driver.window_handles) > 1:
            driver.switch_to.window(driver.window_handles[-1])
            try:
                driver.execute_script("""
                    var video = document.querySelector('video');
                    if (video && !video.paused) { video.pause(); }
                """)
            except Exception as e:
                print(f"Failed to pause video on the previous tab: {e}")

        driver.execute_script(f"window.open('{url}', '_blank');")
        driver.switch_to.window(driver.window_handles[-1])
        try:
            driver.get(url)
        except Exception as e:
            file_obj.write(f"Page load timeout for {url}: {str(e)}\n")
            file_obj.write(f"url,{url},Timeout\n")
            return None

        while time.time() - url_start_time < 30:
            if driver.execute_script("return document.readyState") == "complete":
                scroll = True
                break
            time.sleep(0.5)
    except Exception as e:
        file_obj.write(f"Error while opening the URL: {url} : {str(e)}\n")
        return None

    if not scroll:
        file_obj.write(f"url,{url},Not Loaded\n")
        return

    if no_of_fishes == 1:
        try:
            driver.execute_script('document.getElementById("setSetting8").click();')
            print("Clicked on element with id='setSetting8'")
        except Exception as e:
            print(f"Failed to click on the element: {e}")

    play_start_time = time.time()
    while time.time() - play_start_time < play_time:
        time.sleep(1)

    if scroll_up_down_moves > 0:
        half_moves = scroll_up_down_moves // 2
        for _ in range(half_moves):
            driver.execute_script("window.scrollBy(0, 500);")
            time.sleep(0.5)
        for _ in range(half_moves):
            driver.execute_script("window.scrollBy(0, -500);")
            time.sleep(0.5)

    if scroll_left_right_moves > 0:
        half_moves = scroll_left_right_moves // 2
        for _ in range(half_moves):
            driver.execute_script("window.scrollBy(500, 0);")
            time.sleep(0.5)
        for _ in range(half_moves):
            driver.execute_script("window.scrollBy(-500, 0);")
            time.sleep(0.5)

    url_end_time = time.time()
    total_url_time = url_end_time - url_start_time
    file_obj.write(f'url,{url},{total_url_time:.2f}\n')
    json_log("url", {
        "url": url,
        "play_time": play_time,
        "scroll_ud": scroll_up_down_moves,
        "scroll_lr": scroll_left_right_moves,
        "total_time_sec": round(total_url_time, 2)
    })

def measure_tab_switching_time():
    start_time = time.time()
    for i in range(len(driver.window_handles) - 1, -1, -1):
        driver.switch_to.window(driver.window_handles[i])
        while driver.execute_script("return document.readyState") != "complete":
            time.sleep(0.5)
        time.sleep(1)

    total_switching_time = time.time() - start_time
    print(f"Total tab switching time: {total_switching_time:.2f} seconds")
    return total_switching_time

# ----------------------------- WORD FUNCTIONS -----------------------------
def open_scroll_and_close_word_docs(logo,num_files=5):
    try:
        pythoncom.CoInitialize()
        # desktop_path = os.path.join(os.path.expanduser("~"), "Desktop")
        # image_path = os.path.join(desktop_path,"WindowsNJQCA" ,"Temp_Data", logo)
        image_path = os.path.join("C:\\WindowsNJQCA", "Temp_Data", logo)
        if not os.path.exists(image_path):
            print("Image not found:", image_path)
            file_obj.write(f"Image not found: {image_path}\n")
            return
        word_app = win32com.client.Dispatch("Word.Application")
        word_app.Visible = True
        time.sleep(2)
        bring_window_to_front("Word")
        docs = []
        for i in range(1, num_files + 1):
            doc = word_app.Documents.Add()
            docs.append(doc)
            lines = [f"Line {j}: Sample scrolling text." for j in range(1, 101)]
            for j, line in enumerate(lines, 1):
                doc.Range(0, 0).InsertAfter(line + "\n")
                if j % 10 == 0:
                    doc.Range(doc.Content.End - 1, doc.Content.End).InsertParagraphAfter()
                    doc.InlineShapes.AddPicture(FileName=image_path, LinkToFile=False, SaveWithDocument=True)
                    doc.Range(doc.Content.End - 1, doc.Content.End).InsertParagraphAfter()
            word_app.ActiveWindow.ActivePane.VerticalPercentScrolled = 50
            time.sleep(5)
            word_app.ActiveWindow.ActivePane.VerticalPercentScrolled = 0
            time.sleep(5)
        for doc in docs:
            doc.Close(False)
        word_app.Quit()
    except Exception as e:
        print(f"Error in ms office: {str(e)}")
        file_obj.write(f"Error in ms office: {str(e)}")

def get_compression_size_from_erp():
    try:
        # base_url = "http://10.150.0.146:8000" # LOCAL CONNECTION
        base_url = "https://erp.newjaisa.com"
        # base_url = "https://amzuat.newjaisa.com" # AMZUAT CONNECTION
        endpoint = "/api/method/nj_lib.utils.common_utils.fetch_compression_size"

        response = requests.get(base_url + endpoint, timeout=5)

        if response.status_code != 200:
            print(f"ERP error: status {response.status_code}")
            return 500

        data = response.json()

        message = data.get("message")
        if not message:
            return 500

        size = message.get("compression_size_mb")

        # validate value
        if not size:
            return 500

        return int(size)

    except Exception as e:
        print("Compression size fetch failed:", e)
        return 500
    
def create_dummy_file(file_path, size_in_bytes):
    chunk_size = 10 * 1024 * 1024  # 10 MB
    written = 0

    with open(file_path, "wb") as f:
        while written < size_in_bytes:
            write_size = min(chunk_size, size_in_bytes - written)
            f.write(b"\0" * write_size)
            written += write_size

# ----------------------------- FILE TRANSFER -----------------------------
def create_and_copy_file(size_in_gb, file_obj):
    file_obj.write(" File transfer started\n")

    source_file = "C:\\Temp\\LargeFile.bin"
    destination_folder = "C:\\Temp\\Backup"
    destination_path = os.path.join(destination_folder, os.path.basename(source_file))

    os.makedirs(destination_folder, exist_ok=True)

    size_in_bytes = size_in_gb * 1024 * 1024 * 1024

    compression_size_mb = get_compression_size_from_erp()
    compression_size_bytes = compression_size_mb * 1024 * 1024
    
    compression_file = f"C:\\Temp\\CompressionTest_{compression_size_mb}MB.bin"
    compression_zip = f"C:\\Temp\\CompressionTest_{compression_size_mb}MB.zip"
    try:
        # Existing local copy test
        create_dummy_file(source_file, size_in_bytes)

        start_time = time.time()
        shutil.copy2(source_file, destination_path)
        time_taken = time.time() - start_time

        speed = (size_in_bytes / (1024 * 1024)) / time_taken if time_taken > 0 else 0

        file_obj.write(f'file_transfer,Total file size,{size_in_gb} GB\n')
        file_obj.write(f'file_transfer,Time taken for file transfer,{time_taken:.2f} seconds\n')
        file_obj.write(f'file_transfer,File transfer speed,{speed:.2f} MB/s\n')

        # New 500 MB compression test
        file_obj.write(f"{compression_size_mb} MB compression test started\n")

        create_dummy_file(compression_file, compression_size_bytes)

        compression_start_time = time.time()

        with zipfile.ZipFile(compression_zip, "w", compression=zipfile.ZIP_DEFLATED) as zipf:
            zipf.write(compression_file, arcname=os.path.basename(compression_file))

        compression_time = time.time() - compression_start_time

        file_obj.write(
            f'compression,Task Execution Time ({compression_size_mb} MB Compression) [s],{compression_time:.2f} seconds\n'
        )

        json_log("file_transfer", {
            "file_size_gb": size_in_gb,
            "time_taken_sec": round(time_taken, 2),
            "speed_mb_s": round(speed, 2),
            f"compression_{compression_size_mb}mb_time_sec": round(compression_time, 2)
        })

    except PermissionError:
        file_obj.write("Permission denied! Try running as Administrator.\n")

    except Exception as e:
        file_obj.write(f"Error while transferring/compressing the File: {str(e)}\n")

    finally:
        for file in [source_file, destination_path, compression_file, compression_zip]:
            if os.path.exists(file):
                os.remove(file)

    file_obj.write("File transfer Completed.\n")

# ----------------------------- PERFORMANCE TEST -----------------------------
def run_url_performance(url_list):
    overall_start_time = time.time()
    for line in url_list:
        parts = line.strip().split(",")
        if len(parts) == 4 or len(parts) == 5:
            url = parts[0].strip()
            try:
                play_time = int(parts[1].strip())
                scroll_up_down_time = int(parts[2].strip())
                scroll_left_right_time = int(parts[3].strip())
                no_of_fishes = int(parts[4].strip()) if len(parts) == 5 else 0
                handle_video_and_scrolling(url, play_time, scroll_up_down_time, scroll_left_right_time,file_obj,no_of_fishes)
            except ValueError:
                print(f"Invalid time values for URL: {line.strip()}")
        else:
            print(f"Invalid format: {line.strip()}")

    try:
        handles = driver.window_handles

        if len(handles) > 1:
            tab_switch_time = measure_tab_switching_time()

            # Old format - keep for backward compatibility
            file_obj.write(
                f"tab_switching,Total time taken for switching tabs,{tab_switch_time:.2f}\n"
            )

            # New direct format - ERP parser will read this
            file_obj.write(
                f"tab_switching_time,{tab_switch_time:.2f} seconds\n"
            )

            file_obj.flush()

            json_log("url", {
                "tab_switching_time_sec": round(tab_switch_time, 2)
            })
        else:
            file_obj.write("tab_switching_time,Not Available\n")
            file_obj.flush()

    except Exception as e:
        file_obj.write(f"tab_switching_time,ERROR: {str(e)}\n")
        file_obj.flush()
    overall_end_time = time.time()
    overall_time_taken = overall_end_time - overall_start_time
    print('run_performance test executed successfully')
    subprocess.run(["taskkill", "/f", "/im", "chrome.exe"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["taskkill", "/f", "/im", "chromedriver.exe"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print("Chrome force killed")
    file_obj.write("Chrome force killed after URL tests\n")
    try:
        driver.quit()
    except:
        pass
def safe_execute(task_name, func, *args):
    try:
        print(f" Starting {task_name}")
        func(*args)
        print(f" Completed {task_name}")
    except Exception as e:
        print(f" {task_name} failed: {e}")
        traceback.print_exc()

def split_excel_and_macro(item):
    if "-" in item:
        file_name, macro_name = item.split("-", 1)
        return file_name.strip(), macro_name.strip()
    return item.strip(), None

# ----------------------------- POWERPOINT FUNCTIONS -----------------------------
def open_ppt(ppt_path):
    pythoncom.CoInitialize() 
    if not os.path.exists(ppt_path):
        print(f"PPT file not found: {ppt_path}")
        file_obj.write(f"PPT file not found: {ppt_path}\n")
        return None, None

    try:
        ppt_app = win32com.client.Dispatch("PowerPoint.Application")
        ppt_app.Visible = True

        presentation = ppt_app.Presentations.Open(
            ppt_path,
            WithWindow=True  # IMPORTANT
        )

        # Activate presentation window
        presentation.Windows(1).Activate()
        time.sleep(1)

        print(f"PPT opened: {ppt_path}")
        file_obj.write(f"PPT opened: {ppt_path}\n")
        return ppt_app, presentation

    except Exception as e:
        print(f"Error opening PPT: {str(e)}")
        file_obj.write(f"Error opening PPT: {str(e)}\n")
        return None, None

def wait_until_ppt_ready(presentation, timeout=30):
    start = time.time()
    while time.time() - start < timeout:
        try:
            if presentation.Slides.Count > 0:
                return True
        except:
            pass
        time.sleep(0.5)
    return False

def get_slide_animation_time(slide,slide_no):
    total_time = 0
    try:
        for effect in slide.TimeLine.MainSequence:
            duration = effect.Timing.Duration
            if duration and duration > 0:
                total_time += duration
    except:
        pass

    # Safety defaults
    if total_time <= 0:
        total_time = 3   # no animation case
    return total_time + 1  # +1 sec buffer

def run_ppt_automatically(ppt_app, presentation):
    try:
        pythoncom.CoInitialize()

        if not wait_until_ppt_ready(presentation):
            file_obj.write(f"PPT not ready after 30s timeout — skipping\n")
            raise Exception("PPT not ready even after waiting")

        try:
            bring_window_to_front("PowerPoint")
        except Exception as e:
            print(f"Warning: Could not bring PowerPoint to front: {e}")

        # ---------------- TOTAL PPT TIMER ----------------
        ppt_start_time = time.time()
        file_obj.write(f"PPT_START_TIME,{datetime.now()}\n")

        # Start slideshow
        slide_show = None
        try:
            slide_show = presentation.SlideShowSettings.Run()
            time.sleep(1)
        except Exception as e:
            print(f"Failed to start slideshow: {e}")

        total_slides = presentation.Slides.Count

        for i in range(1, total_slides + 1):
            slide = presentation.Slides(i)

            # ---------------- SLIDE START ----------------
            slide_start_time = time.time()

            # Animation timing (already measured by PowerPoint)
            anim_time = get_slide_animation_time(slide, i)
            anim_time = min(anim_time, 30)

            file_obj.write(
                f"ppt slide animation time ,Slide_{i},{anim_time:.2f} seconds\n"
            )

            time.sleep(anim_time)

            # ---------------- SLIDE SWITCH ----------------
            if slide_show and i < total_slides:
                switch_start = time.time()
                try:
                    slide_show.View.Next()
                    time.sleep(0.3)  # render buffer
                except:
                    pass
                switch_end = time.time()

                switch_time = switch_end - switch_start
                file_obj.write(
                    f"ppt slide switch time,Slide_{i}_to_{i+1},{switch_time:.3f} seconds\n"
                )

            # ---------------- SLIDE TOTAL ----------------
            slide_end_time = time.time()
            slide_total_time = slide_end_time - slide_start_time

            file_obj.write(
                f"ppt slide total time,Slide_{i},{slide_total_time:.2f} seconds\n"
            )

        # Exit slideshow
        try:
            if slide_show:
                slide_show.View.Exit()
        except:
            pass

        # ---------------- TOTAL PPT TIME ----------------
        ppt_end_time = time.time()
        total_ppt_time = ppt_end_time - ppt_start_time

        file_obj.write(
            f"total time consumed for ppt,{total_ppt_time:.2f} seconds\n"
        )
        json_log("ppt", {
                "file": os.path.basename(presentation.FullName),
                "slides": presentation.Slides.Count,
                "total_time_sec": round(total_ppt_time, 2)
            })
        print("PPT run completed.")
        file_obj.write("PPT run completed.\n")

    except Exception as e:
        print(f"Error during PPT run: {str(e)}")
        file_obj.write(f"Error during PPT run: {str(e)}\n")

# ----------------------------- PPT LOADING TIME -----------------------------
def open_ppt_with_loading_time(ppt_path):
    start_time = time.time()  # start timer

    ppt_app, presentation = open_ppt(ppt_path)  # existing function

    end_time = time.time()  # end timer
    if ppt_app and presentation:
        loading_time = end_time - start_time
        file_obj.write(f"PPT loading time (launch + open file): {loading_time:.2f} seconds\n")
        print(f"PPT loading time: {loading_time:.2f} seconds")
    else:
        loading_time = None
        file_obj.write(f"PPT failed to open: {ppt_path}\n")
    
    return ppt_app, presentation, loading_time

# ----------------------------- POWERPOINT EXECUTION -----------------------------
def run_all_ppt(final):
    ppt_list = final.get('ppt_list', [])
    if ppt_list:
        for ppt_file_name in ppt_list:
            ppt_app = None
            presentation = None
            try:
                ppt_path = os.path.join(BASE_DIR, "Temp_Data", ppt_file_name)
                ppt_app, presentation, loading_time = open_ppt_with_loading_time(ppt_path)
                if ppt_app and presentation:
                    run_ppt_automatically(ppt_app, presentation)
            except Exception as e:
                print(f"Error in PPT execution: {str(e)}")
                file_obj.write(f"Error in PPT execution: {str(e)}\n")
            finally:
                # Ensure PowerPoint closes regardless of errors
                if presentation:
                    try:
                        presentation.Close()
                    except:
                        pass
                if ppt_app:
                    try:
                        ppt_app.Quit()
                        print("PowerPoint closed successfully.")
                        file_obj.write("PowerPoint closed successfully.\n")
                    except:
                        pass
    else:
        file_obj.write("No PPT files to process.\n")

def log_event(file_obj, event_name, start_time, start_dt):
    duration = time.time() - start_time
    end_dt = datetime.now()
    file_obj.write(
        f"{event_name}, start ={start_dt}, end ={end_dt},Time taken ={duration:.2f}s\n"
    )
    return duration

# ------------------ VS CODE ------------------
def get_vscode_title():
    titles = []
    def enum(hwnd, _):
        if win32gui.IsWindowVisible(hwnd):
            title = win32gui.GetWindowText(hwnd)
            if "Visual Studio Code" in title:
                titles.append(title)
    win32gui.EnumWindows(enum, None)
    return titles[0] if titles else ""

def wait_for_vscode(timeout=15):
    start = time.time()
    while time.time() - start < timeout:
        for p in psutil.process_iter(['name']):
            if p.info['name'] and p.info['name'].lower() == 'code.exe':
                return True
        time.sleep(1)
    return False

def find_vscode_exe():
    """Find VS Code Code.exe — skip .cmd wrappers from PATH."""

    # Dynamic user path — works for any username (admin, Administrator, etc.)
    user_local = os.environ.get("LOCALAPPDATA", "")
    user_path = os.path.join(user_local, "Programs", "Microsoft VS Code", "Code.exe")
    if os.path.exists(user_path):
        return user_path

    # System-wide installs
    for path in [
        r"C:\Program Files\Microsoft VS Code\Code.exe",
        r"C:\Program Files (x86)\Microsoft VS Code\Code.exe",
    ]:
        if os.path.exists(path):
            return path

    # Last resort — search PATH but only accept .exe files, reject .cmd
    vscode_in_path = shutil.which("code")
    if vscode_in_path and vscode_in_path.lower().endswith(".exe"):
        return vscode_in_path

    return None

def wait_for_vscode_window(timeout=30):
    """Wait until VS Code window is actually visible on screen."""
    start = time.time()
    while time.time() - start < timeout:
        titles = []
        def enum(hwnd, _):
            if win32gui.IsWindowVisible(hwnd):
                if "Visual Studio Code" in win32gui.GetWindowText(hwnd):
                    titles.append(True)
        win32gui.EnumWindows(enum, None)
        if titles:
            return True
        time.sleep(1)
    return False
def open_vs_code(project_path):
    vscode_exe = find_vscode_exe()

    if not vscode_exe:
        file_obj.write("ERROR: VS Code exe not found on this system\n")
        print("ERROR: VS Code not found")
        return 0

    print(f"VS Code found at: {vscode_exe}")
    file_obj.write(f"VS Code found at: {vscode_exe}\n")

    start_dt = datetime.now()
    start = time.time()

    # Open the project folder in a new VS Code window
    subprocess.Popen([vscode_exe, "-n", project_path])

    wait_for_vscode()          # wait for process to start
    time.sleep(1)              # small gap before checking window
    wait_for_vscode_window(30) # wait for actual window to appear on screen
    time.sleep(2)              # small buffer for VS Code to fully render

    # Log VS Code launch time here (before file open)
    duration = log_event(
        file_obj,
        f"vs code open,{os.path.basename(project_path)}",
        start,
        start_dt
    )

    # Open the main project file in the existing window using -r flag
    for f in ["index.html", "manage.py", "app.py"]:
        file_path = os.path.join(project_path, f)
        if os.path.exists(file_path):
            subprocess.Popen([vscode_exe, "-r", file_path])
            time.sleep(1)
            break

    return duration


def close_vs_code():
    start_dt = datetime.now()
    start = time.time()

    # Kill all code.exe processes including helpers
    subprocess.run(["taskkill", "/F", "/IM", "code.exe"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(2)  # wait for all child processes to fully die

    log_event(file_obj, "vs code close", start, start_dt)

def get_free_port():
    s = socket.socket()
    s.bind(('', 0))
    port = s.getsockname()[1]
    s.close()
    return port

def find_python_for_project(project_path):
    """Find python.exe — venv first (validate it), then system Python."""
    import glob

    # 1. Try venv inside the project folder — but VALIDATE it first
    venv_python = os.path.join(project_path, "venv", "Scripts", "python.exe")
    if os.path.exists(venv_python):
        # Test if venv python actually works
        try:
            result = subprocess.run(
                [venv_python, "--version"],
                timeout=10,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            if result.returncode == 0:
                file_obj.write(f"Venv python valid: {venv_python}\n")
                return venv_python  # venv is good, use it
            else:
                file_obj.write(f"Venv python broken (bad returncode) — rebuilding venv\n")
        except Exception as e:
            file_obj.write(f"Venv python broken ({e}) — rebuilding venv\n")

        # Venv is broken — delete it so we can rebuild
        try:
            shutil.rmtree(os.path.join(project_path, "venv"))
            file_obj.write(f"Deleted broken venv\n")
        except Exception as e:
            file_obj.write(f"Could not delete venv: {e}\n")

    # 2. Find system Python (any user, any version)
    patterns = [
        r"C:\Users\*\AppData\Local\Programs\Python\Python*\python.exe",
        r"C:\Python*\python.exe",
        r"C:\Program Files\Python*\python.exe",
        r"C:\Program Files (x86)\Python*\python.exe",
    ]
    system_python = None
    for pattern in patterns:
        matches = glob.glob(pattern)
        if matches:
            matches.sort(reverse=True)
            system_python = matches[0]
            break

    if not system_python:
        system_python = shutil.which("python")

    if not system_python:
        return None

    file_obj.write(f"System Python found: {system_python}\n")

    # 3. Rebuild venv using system python
    try:
        subprocess.run(
            [system_python, "-m", "venv", os.path.join(project_path, "venv")],
            timeout=60,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        file_obj.write(f"Venv rebuilt successfully\n")

        # Return the newly created venv python
        new_venv_python = os.path.join(project_path, "venv", "Scripts", "python.exe")
        if os.path.exists(new_venv_python):
            return new_venv_python
    except Exception as e:
        file_obj.write(f"Venv rebuild failed ({e}) — using system Python directly\n")

    # 4. Fallback — use system python directly
    return system_python
# ------------------ DJANGO SERVER ------------------
def start_django_server(project_path):
    port = get_free_port()

    python_exe = find_python_for_project(project_path)

    if not python_exe:
        file_obj.write(f"ERROR: No Python found for {project_path}\n")
        print(f"ERROR: No Python found for {project_path}")
        return None, port, 0

    file_obj.write(f"Using Python: {python_exe}\n")
    print(f"Using Python: {python_exe}")

    # Auto-install django
    try:
        subprocess.run(
            [python_exe, "-m", "pip", "install", "django", "django-cors-headers", "--quiet"],
            cwd=project_path,
            timeout=60,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        file_obj.write(f"Django package ensured\n")
    except Exception as e:
        file_obj.write(f"pip install warning: {e}\n")

    # Run migrations
    try:
        subprocess.run(
            [python_exe, "manage.py", "migrate", "--run-syncdb"],
            cwd=project_path,
            timeout=60,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        file_obj.write(f"Migrations applied\n")
    except Exception as e:
        file_obj.write(f"Migration warning: {e}\n")

    start_dt = datetime.now()
    start = time.time()

    process = subprocess.Popen(
        [python_exe, "manage.py", "runserver", f"127.0.0.1:{port}", "--noreload"],
        cwd=project_path,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE
    )

    url = f"http://127.0.0.1:{port}"
    deadline = time.time() + 60
    server_ready = False
    while time.time() < deadline:
        try:
            if requests.get(url, timeout=0.5).status_code == 200:
                server_ready = True
                break
        except:
            time.sleep(0.2)

    if not server_ready:
        err_output = ""
        try:
            err_output = process.stderr.read(2000).decode(errors="replace")
        except:
            pass
        file_obj.write(f"WARNING: Django server did not respond in 60s\n")
        file_obj.write(f"Django stderr: {err_output}\n")
        process.terminate()
        return None, port, 0

    duration = log_event(file_obj, f"Django server start ({port})", start, start_dt)
    return process, port, duration

def stop_django_server(process):
    start_dt = datetime.now()
    start = time.time()

    process.terminate()
    process.wait()

    log_event(file_obj, "Django server stop", start, start_dt)

# ------------------ BROWSER ------------------

# FIX:
def open_browser_and_measure(url):
    options = Options()
    options.add_argument("--start-maximized")
    options.add_argument("--disable-background-networking")
    options.add_argument("--disable-sync")
    options.add_argument("--no-first-run")
    service = Service(ChromeDriverManager().install())
    driver = webdriver.Chrome(service=service, options=options)
    driver.set_page_load_timeout(30)
    driver.set_script_timeout(20)

    start_dt = datetime.now()
    start = time.time()

    try:
        driver.get(url)
    except Exception as e:
        file_obj.write(f"Browser load timeout: {url} : {str(e)}\n")

    deadline = time.time() + 30
    while time.time() < deadline:
        try:
            if driver.execute_script("return document.readyState") == "complete":
                break
        except:
            break
        time.sleep(0.1)

    duration = log_event(file_obj, "Browser initial load", start, start_dt)
    return driver, duration


def static_server_start():
    start_dt = datetime.now()
    start = time.time()

    time.sleep(0.2)  # simulate minimal startup

    duration = log_event(
        file_obj,
        "Static server start",
        start,
        start_dt
    )
    return None, duration


def static_server_stop():
    start_dt = datetime.now()
    start = time.time()

    time.sleep(0.1)

    log_event(file_obj, "Static server stop", start, start_dt)

# FIX:
def open_browser(driver, url):
    start = time.time()
    start_dt = datetime.now()
    try:
        driver.get(url)
    except Exception as e:
        file_obj.write(f"Browser open timeout: {url}: {str(e)}\n")
    deadline = time.time() + 30
    while time.time() < deadline:
        try:
            if driver.execute_script("return document.readyState") == "complete":
                break
        except:
            break
        time.sleep(0.1)
    load_time = time.time() - start
    file_obj.write(f"Browser load,{start_dt},load time : {load_time:.2f}s\n")

# FIX:
def refresh_page_3_times(driver):
    for i in range(1, 4):
        start_dt = datetime.now()
        start = time.time()
        try:
            driver.refresh()
        except Exception as e:
            file_obj.write(f"Refresh {i} timeout: {str(e)}\n")
            log_event(file_obj, f"Browser refresh {i} FAILED", start, start_dt)
            continue
        deadline = time.time() + 30
        while time.time() < deadline:
            try:
                if driver.execute_script("return document.readyState") == "complete":
                    break
            except:
                break
            time.sleep(0.2)
        log_event(file_obj, f"Browser refresh {i}", start, start_dt)

# FIX:
def close_browser(driver):
    start_dt = datetime.now()
    start = time.time()
    subprocess.run(["taskkill", "/f", "/im", "chrome.exe"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["taskkill", "/f", "/im", "chromedriver.exe"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        driver.quit()
    except:
        pass
    log_event(file_obj, "Browser close", start, start_dt)
    
def run_projects(final):
    if final.get("project_list"):
        for item in final.get("project_list", []):

            parts = item.split(",")

            project_type = parts[0].strip().upper()   # STATIC / DJANGO
            project_path = parts[1].strip()
            wait_time = int(parts[2]) if len(parts) > 2 else 10

            if project_type == "DJANGO":
                file_obj.write(
                    f"\n--- Running {project_type} project: {os.path.basename(project_path)} ---\n"
                )

                # VS Code load
                vs_time = open_vs_code(project_path)

                # Django start (dynamic port)
                django_process, port, django_time = start_django_server(project_path)
                if django_process is None:
                    file_obj.write(f"Skipping browser — Django server failed to start\n")
                    close_vs_code()
                    continue
                # Browser open
                driver, browser_time = open_browser_and_measure(
                    f"http://127.0.0.1:{port}"
                )

                # Refresh
                refresh_page_3_times(driver)

                # Interaction wait_time (REAL USER SIMULATION)
                # After browser opens
                interaction_start = time.time()
                time.sleep(wait_time)  # just wait for wait_time seconds

                interaction_time = time.time() - interaction_start
                file_obj.write(f"BROWSER_INTERACTION_TIME,{interaction_time:.2f}s\n")
                # json_log("project", {
                #     "type": project_type,
                #     "path": project_path,
                #     "vs_code_time": round(vs_time, 2),
                #     "server_start_time": round(django_time if project_type=="DJANGO" else server_time, 2),
                #     "browser_interaction_sec": round(interaction_time, 2)
                # })

                # Close browser after wait_time
                close_browser(driver)

                # Stop Django
                stop_django_server(django_process)

                # Close VS Code
                close_vs_code()

            elif project_type == "STATIC":
                file_obj.write(f"\n--- Running {project_type} project: {os.path.basename(project_path)} ---\n")
                # VS Code open
                vs_time = open_vs_code(project_path)

                # STATIC server start (DUMMY)
                server_process, server_time = static_server_start()

                # Browser initial load (reuse common function)
                html_file = os.path.join(project_path, "index.html")
                if not os.path.exists(html_file):
                    file_obj.write(f"Error: {html_file} does not exist.\n")
                    static_server_stop()
                    close_vs_code()
                    continue

                html_url = f"file:///{html_file.replace(os.sep, '/')}"
                driver, browser_time = open_browser_and_measure(html_url)

                # Refresh (same as Django)
                refresh_page_3_times(driver)

                # Interaction (UNCHANGED logic)
                start_time = time.time()
                while time.time() - start_time < wait_time:
                    driver.execute_script("window.scrollBy(0, 500);")
                    time.sleep(1)
                    driver.execute_script("window.scrollBy(0, -500);")
                    time.sleep(1)

                interaction_time = time.time() - start_time
                file_obj.write(
                    f"BROWSER_INTERACTION_TIME,{interaction_time:.2f}s\n"
                )
                json_log("project", {
                    "type": project_type,
                    "path": project_path,
                    "vs_code_time": round(vs_time, 2),
                    "server_start_time": round(server_time, 2),
                    "browser_interaction_sec": round(interaction_time, 2)
                })

                # Browser close
                close_browser(driver)

                # STATIC server stop
                static_server_stop()

                # VS Code close
                close_vs_code()

# ----------------------------- BLENDER SCRIPT -----------------------------
def create_blender_generic_script(run_duration=10):
    """
    Blender script that:
    - Plays animation if it exists.
    - Otherwise waits for run_duration seconds.
    - Quits automatically.
    Logs animation duration or modeling/scene duration.
    """
    return f"""
import bpy
import time

start_time = time.time()
scene = bpy.context.scene

# Check if animation exists
has_animation = scene.frame_start != scene.frame_end
animation_duration = 0.0

if has_animation:
    scene.frame_set(scene.frame_start)

def step():
    global animation_duration
    # Play animation if exists
    if has_animation:
        current = scene.frame_current + 1
        if current > scene.frame_end:
            scene.frame_set(scene.frame_start)
        else:
            scene.frame_set(current)
        animation_duration = time.time() - start_time
    
    # Stop after run_duration
    if time.time() - start_time >= {run_duration}:
        # Log animation or modeling duration
        if has_animation:
            print(f"ANIMATION_DURATION,{{animation_duration:.2f}}s")
        else:
            modeling_duration = time.time() - start_time
            print(f"MODELING_DURATION,{{modeling_duration:.2f}}s")
        bpy.ops.wm.quit_blender()
        return None
    return 0.04  # timer interval (~25 FPS)

bpy.app.timers.register(step)
"""

# ----------------------------- PERFORMANCE FUNCTION -----------------------------
def run_blender_performance(
            blender_file,
            file_obj,
            blender_exe=r"C:\Program Files\Blender Foundation\Blender 5.0\blender.exe",
            run_duration=40
        ):
    """
    Runs Blender with given .blend file.
    Measures:
    - GUI load time
    - CPU/RAM usage
    - Total runtime
    - Animation duration if exists, or modeling/scene duration
    Logs everything to file_obj.
    """
    if not os.path.exists(blender_exe):
        file_obj.write("Blender executable not found\n")
        return
    if not os.path.exists(blender_file):
        file_obj.write(f"Blend file not found: {blender_file}\n")
        return

    start_time = time.time()
    file_obj.write(f"Performance Started\n")
    file_obj.write(f"Blender app opens at ,{datetime.now()}\n")

    # Create temporary blender script
    script_content = create_blender_generic_script(run_duration)
    with tempfile.NamedTemporaryFile(delete=False, suffix=".py") as tmp:
        tmp.write(script_content.encode("utf-8"))
        script_path = tmp.name

    # Launch Blender GUI
    process = subprocess.Popen(
        f'"{blender_exe}" "{blender_file}" --python "{script_path}"',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Small wait for Blender to initialize GUI
    time.sleep(2)
    gui_load_time = time.time() - start_time
    file_obj.write(f"Blender GUI load time ,{gui_load_time:.2f}s\n")

    # Monitor CPU/RAM usage and capture Blender stdout
    animation_duration = None
    modeling_duration = None
    try:
        ps_proc = psutil.Process(process.pid)
        file_obj.write(f"Blender workload start ,{datetime.now()}\n")
        file_obj.write(f"Blender workload duration ,{run_duration}s\n")
        cpu_start = ps_proc.cpu_percent(interval=1)
        ram_start = ps_proc.memory_info().rss / (1024 * 1024)
        file_obj.write(f"Blender resource start,CPU:{cpu_start}%,RAM:{ram_start:.2f}MB\n")

        # Capture stdout while Blender is running
        while True:
            output_line = process.stdout.readline()
            if output_line == '' and process.poll() is not None:
                break
            if output_line:
                line = output_line.strip()
                if line.startswith("ANIMATION_DURATION"):
                    animation_duration = float(line.split(",")[1].replace("s",""))
                if line.startswith("MODELING_DURATION"):
                    modeling_duration = float(line.split(",")[1].replace("s",""))

        # Wait for Blender to finish
        ps_proc.wait()

        cpu_end = ps_proc.cpu_percent(interval=1)
        ram_end = ps_proc.memory_info().rss / (1024 * 1024)
        file_obj.write(f"Blender resource end ,CPU:{cpu_end}%,RAM:{ram_end:.2f}MB\n")
        file_obj.write(f"Blender process exit ,{datetime.now()}\n")

    except psutil.NoSuchProcess:
        pass

    total_time = time.time() - start_time
    file_obj.write(f"Blender total time taken ,{total_time:.2f}s\n")
    file_obj.write(f"Blender app closed at ,{datetime.now()}\n")

    if animation_duration:
        file_obj.write(f"Animation duration ,{animation_duration:.2f}s\n")
    elif modeling_duration:
        file_obj.write(f"Modeling/Scene duration ,{modeling_duration:.2f}s\n")
    else:
        file_obj.write(f"Animation/Modeling duration ,N/A\n")

    # Cleanup temp script
    os.remove(script_path)

    print(" Blender performance test completed correctly")
    json_log("blender", {
        "file": blender_file,
        "gui_load_sec": round(gui_load_time, 2),
        "total_time_sec": round(total_time, 2),
        "animation_sec": animation_duration,
        "modeling_sec": modeling_duration
    })

def run_all_blender(final):
        for item in final.get('blender_list', []):
            parts = item.split(",")
            blender_file = parts[0].strip()
            operation = parts[1].strip() if len(parts) > 1 else "RENDER"
            run_blender_performance(blender_file, file_obj)
# ------------------------------------------------
# Logger (DATE + TIME + MESSAGE)
# ------------------------------------------------
def flog(msg, fobj=None):
    ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    line = f"{ts} | {msg}"
    print(line)
    if fobj:
        fobj.write(line + "\n")
        fobj.flush()

# ------------------------------------------------
# Safe bring-to-front
# ------------------------------------------------
def bring_photoshop_to_front():
    def enum_handler(hwnd, _):
        if win32gui.IsWindowVisible(hwnd):
            title = win32gui.GetWindowText(hwnd)
            if "Photoshop" in title:
                try:
                    # Restore if minimized
                    win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)
                    
                    # Maximize window
                    win32gui.ShowWindow(hwnd, win32con.SW_MAXIMIZE)

                    # Windows focus hack (ALT key trick)
                    shell = win32com.client.Dispatch("WScript.Shell")
                    shell.SendKeys('%')
                    time.sleep(0.2)

                    # Force topmost temporarily
                    win32gui.SetWindowPos(
                        hwnd,
                        win32con.HWND_TOPMOST,
                        0, 0, 0, 0,
                        win32con.SWP_NOMOVE | win32con.SWP_NOSIZE
                    )
                    win32gui.SetWindowPos(
                        hwnd,
                        win32con.HWND_NOTOPMOST,
                        0, 0, 0, 0,
                        win32con.SWP_NOMOVE | win32con.SWP_NOSIZE
                    )

                    # Bring to front
                    win32gui.SetForegroundWindow(hwnd)

                except:
                    pass

    win32gui.EnumWindows(enum_handler, None)

# ------------------------------------------------
# Photoshop helpers
# ------------------------------------------------
def get_photoshop_process():
    for p in psutil.process_iter(['name']):
        if p.info['name'] and 'Photoshop.exe' in p.info['name']:
            return p
    return None

def get_cpu_ram(proc):
    if not proc:
        return 0.0, 0.0
    return round(proc.cpu_percent(0.2), 2), round(proc.memory_info().rss / (1024 * 1024), 2)

def get_photoshop_instance():
    try:
        return win32com.client.GetActiveObject("Photoshop.Application")
    except:
        return win32com.client.Dispatch("Photoshop.Application")

# ------------------------------------------------
# Read local data file
# ------------------------------------------------
def fetch_photoshop_jobs(data_file):
    jobs = []

    with open(data_file, "r") as f:
        for line in f:
            line = line.strip()
            if not line.startswith("PHOTOSHOP"):
                continue

            parts = line.split(",", 1)[1]
            items = parts.split(",")

            img_path = items[0].strip()
            filters = [x.strip() for x in items[1:]]

            jobs.append((img_path, filters))

    return jobs



def apply_filter_dynamic(doc, filter_name, ps_proc, img_path, log_file):
    """
    Dynamically applies any Photoshop filter by name.
    Supports both COM method filters and JavaScript-based filters.
    """
    t0 = time.time()
    layer = doc.ActiveLayer
    f = filter_name.strip().lower()

    # ---- FILTER MAP (name -> lambda) ----
    filter_map = {
        # Blur filters
        "gaussianblur":         lambda: layer.ApplyGaussianBlur(5),
        "motionblur":           lambda: layer.ApplyMotionBlur(45, 10),
        "radialblur":           lambda: layer.ApplyRadialBlur(10, 1, 1),
        "smartblur":            lambda: layer.ApplySmartBlur(5, 25, 0.3, 0),
        "averageblur":          lambda: layer.ApplyAverage(),

        # Sharpen filters
        "sharpen":              lambda: layer.ApplySharpen(),
        "sharpenedges":         lambda: layer.ApplySharpenEdges(),
        "sharpenmorefilter":    lambda: layer.ApplySharpenMore(),
        "unsharpenmask":        lambda: layer.ApplyUnSharpMask(1.0, 1, 0),

        # Color adjustments
        "blackwhite":           lambda: layer.Desaturate(),
        "invert":               lambda: doc.ActiveLayer.Invert(),
        "posterize":            lambda: layer.Posterize(4),

        # Stylize filters
        "emboss":               lambda: layer.ApplyEmboss(135, 3, 1),
        "findedges":            lambda: layer.ApplyFindEdges(),
        "glowingEdges":         lambda: layer.ApplyGlowingEdges(2, 7, 4),
        "solarize":             lambda: layer.ApplySolarize(),
        "diffuse":              lambda: layer.ApplyDiffuse(1),

        # Noise filters
        "addnoise":             lambda: layer.ApplyAddNoise(25, 1, False),
        "medianfilter":         lambda: layer.ApplyMedianNoise(2),
        "despeckle":            lambda: layer.ApplyDespeckle(),
        "reducenoise":          lambda: layer.ApplyReduceNoise(6, 0, 25, 65),

        # Other filters
        "tile":                 lambda: layer.ApplyTile(),
        "tracedcontour":        lambda: layer.ApplyTracedContour(128, 0, 0),
        "highpass":             lambda: layer.ApplyHighPass(10),
        "maximumfilter":        lambda: layer.ApplyMaximum(2),
        "minimumfilter":        lambda: layer.ApplyMinimum(2),
    }

    applied = False

    # ---- Try from filter_map first ----
    if f in filter_map:
        try:
            filter_map[f]()
            applied = True
        except Exception as e:
            flog(f"FILTER_MAP_ERROR,{img_path},{filter_name},{e}", log_file)

    # ---- Fallback: try JavaScript/Action Descriptor ----
    if not applied:
        try:
            js_code = f"""
                var idFlt = stringIDToTypeID("{filter_name}");
                var desc = new ActionDescriptor();
                executeAction(idFlt, desc, DialogModes.NO);
            """
            app = win32com.client.GetActiveObject("Photoshop.Application")
            app.DoJavaScript(js_code)
            applied = True
            flog(f"FILTER_JS_FALLBACK_USED,{img_path},{filter_name}", log_file)
        except Exception as e:
            flog(f"FILTER_JS_FALLBACK_ERROR,{img_path},{filter_name},{e}", log_file)

    # ---- Log result ----
    cpu, ram = get_cpu_ram(ps_proc)
    status = "OK" if applied else "SKIPPED_UNKNOWN"

    flog(
        f"FILTER_TIME,{img_path},{filter_name},{time.time()-t0:.2f}s,CPU={cpu}%,RAM={ram}MB,STATUS={status}",
        log_file
    )

    return applied

# ------------------------------------------------
# MAIN PHOTOSHOP PROCESS (ONE SESSION)
# ------------------------------------------------
def process_photoshop_batch(jobs, export_dir, log_file):
    pythoncom.CoInitialize()

    total_start = time.time()
    flog("PHOTOSHOP_START", log_file)

    # ---- Photoshop Launch ----
    t0 = time.time()
    psApp = get_photoshop_instance()
    psApp.Visible = True
    # Wait until real window exists
    for _ in range(20):
        time.sleep(0.5)
        windows = [w for w in gw.getWindowsWithTitle("Photoshop")]
        if windows:
            break

    bring_photoshop_to_front()
    flog(f"PHOTOSHOP_LAUNCH_TIME,{time.time()-t0:.2f}s", log_file)

    ps_proc = get_photoshop_process()

    if not os.path.exists(export_dir):
        os.makedirs(export_dir)

    # ---------------- IMAGE LOOP ----------------
    for img_path, filters in jobs:

        if not os.path.exists(img_path):
            flog(f"IMAGE_NOT_FOUND,{img_path}", log_file)
            continue

        flog(f"IMAGE_START,{img_path}", log_file)
        img_start = time.time()

        # ---- Image Open ----
        t0 = time.time()
        doc = psApp.Open(img_path)
        time.sleep(2)
        bring_photoshop_to_front()
        flog(f"IMAGE_OPEN_TIME,{time.time()-t0:.2f}s", log_file)

        # ---- Filters ----
        for f in filters:
            try:
                apply_filter_dynamic(doc, f, ps_proc, img_path, log_file)
            except Exception as e:
                flog(f"FILTER_ERROR,{img_path},{f},{e}", log_file)

        # ---- Export ----
        name = os.path.splitext(os.path.basename(img_path))[0]
        export_path = os.path.join(export_dir, f"{name}_exported.jpg")

        t0 = time.time()
        opts = win32com.client.Dispatch("Photoshop.ExportOptionsSaveForWeb")
        opts.Format = 6
        opts.Quality = 80
        doc.Export(ExportIn=export_path, ExportAs=2, Options=opts)

        cpu, ram = get_cpu_ram(ps_proc)
        flog(
            f"EXPORT_TIME,{export_path},{time.time()-t0:.2f}s,CPU={cpu}%,RAM={ram}MB",
            log_file
        )

        # ---- Image Close ----
        t0 = time.time()
        doc.Close(2)
        flog(f"IMAGE_CLOSE_TIME,{time.time()-t0:.2f}s", log_file)

        flog(f"IMAGE_TOTAL_TIME,{time.time()-img_start:.2f}s", log_file)
        json_log("photoshop", {
            "images_processed": len(jobs),
            "total_time_sec": round(time.time() - total_start, 2)
        })

        # ---- Completion message ----
        flog(f"IMAGE_COMPLETED,{img_path}", log_file)
        time.sleep(2)

    # ---- Photoshop Close ----
    t0 = time.time()
    psApp.Quit()
    flog(f"PHOTOSHOP_CLOSE_TIME,{time.time()-t0:.2f}s", log_file)

    flog(f"PHOTOSHOP_TOTAL_TIME,{time.time()-total_start:.2f}s", log_file)

def run_all_photoshop(final):
    photoshop_jobs = []

    for item in final.get("photoshop_list", []):
        parts = item.split(",", 1)

        image_path = parts[0].strip().replace("\\", "\\\\")
        filters = []

        if len(parts) > 1:
            filters = [f.strip() for f in parts[1].split(",") if f.strip()]

        flog(f"DEBUG_ADDED_JOB,{image_path},{filters}", file_obj)

        photoshop_jobs.append((image_path, filters))

    if photoshop_jobs:
        process_photoshop_batch(
            photoshop_jobs,
            r"C:\WindowsNJQCA\Temp_Data\exported",
            file_obj
        )

# =========================================================
# ─── NEW SUPERPOSITION HELPERS ───────────────────────────────────────────────

SUPERPOSITION_BIN_DIR = r"C:\Program Files\Unigine\Superposition Benchmark\bin"
SUPERPOSITION_EXE     = r"C:\Program Files\Unigine\Superposition Benchmark\bin\superposition.exe"
PRESENTMON_PATH       = r"C:\WindowsNJQCA\PresentMon-2.4.0-x64.exe"
OUTPUT_DIR            = r"C:\WindowsNJQCA"

PRESET_MAP = {
    "720p_low": 0, "720p_medium": 1, "1080p_medium": 2,
    "1080p_high": 3, "1080p_extreme": 4, "4k_extreme": 5,
    "vr_hq": 6, "custom": 7,
}

_NVSMI_CANDIDATES = [
    "nvidia-smi",
    r"C:\Windows\System32\nvidia-smi.exe",
    r"C:\Program Files\NVIDIA Corporation\NVSMI\nvidia-smi.exe",
    r"C:\Program Files\NVIDIA\NVSMI\nvidia-smi.exe",
    r"C:\Windows\SysWOW64\nvidia-smi.exe",
]

def _find_nvidia_smi():
    candidates = list(_NVSMI_CANDIDATES)
    from_path = shutil.which("nvidia-smi")
    if from_path and from_path not in candidates:
        candidates.insert(0, from_path)
    for smi in candidates:
        if os.path.isabs(smi) and not os.path.isfile(smi):
            continue
        try:
            r = subprocess.run([smi, "-L"], capture_output=True, text=True, timeout=5)
            if r.returncode == 0 and r.stdout.strip():
                print(f"[GPU] nvidia-smi found: {smi} — {r.stdout.strip()}")
                return smi
        except Exception:
            continue
    print("[WARN] nvidia-smi not found — GPU metrics will show N/A.")
    return None

_NVIDIA_SMI = _find_nvidia_smi()

def get_gpu_metrics():
    empty = {"gpu_%": "N/A", "gpu_mem_used_mb": "N/A",
             "gpu_mem_total_mb": "N/A", "gpu_temp_c": "N/A"}
    if _NVIDIA_SMI is None:
        return empty
    try:
        r = subprocess.run(
            [_NVIDIA_SMI,
             "--query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu",
             "--format=csv,noheader,nounits"],
            capture_output=True, text=True, timeout=5,
        )
        if r.returncode == 0 and r.stdout.strip():
            p = [x.strip() for x in r.stdout.strip().split(",")]
            return {"gpu_%": float(p[0]), "gpu_mem_used_mb": float(p[1]),
                    "gpu_mem_total_mb": float(p[2]), "gpu_temp_c": float(p[3])}
    except Exception:
        pass
    return empty

def kill_superposition():
    for name in ["superposition.exe", "superposition_cli.exe", "launcher.exe"]:
        subprocess.run(["taskkill", "/F", "/T", "/IM", name],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def is_process_alive(name):
    name_lower = name.lower()
    for p in psutil.process_iter(["name"]):
        try:
            if p.info["name"] and p.info["name"].lower() == name_lower:
                return True
        except Exception:
            pass
    return False

def wait_for_process(name, timeout=60):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if is_process_alive(name):
            return True
        time.sleep(1)
    return False

def nuke_presentmon_etw_session(session_name="PresentMon"):
    try:
        p = subprocess.Popen([PRESENTMON_PATH, "--stop_existing_session"],
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        p.wait(timeout=8)
    except subprocess.TimeoutExpired:
        p.kill()
    except Exception:
        pass
    for action in ["stop", "delete"]:
        try:
            subprocess.run(["logman", action, session_name, "-ets"],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=8)
        except Exception:
            pass
    for exe_name in ["PresentMon-2.4.0-x64.exe", "PresentMon.exe"]:
        subprocess.run(["taskkill", "/F", "/IM", exe_name],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(2)

def launch_superposition(preset="1080p_medium"):
    preset_num = PRESET_MAP.get(preset, 2)
    cmd = [
        SUPERPOSITION_EXE,
        "-preset",          str(preset_num),
        "-video_app",       "direct3d11",
        "-video_vsync",     "0",
        "-video_mode",      "-1",
        "-console_command", "world_load superposition/superposition && render_manager_create_textures 1",
        "-project_name",    "Superposition",
        "-video_fullscreen","1",
        "-video_width",     "1920",
        "-video_height",    "1080",
        "-virtual_width",   "1920",
        "-virtual_height",  "1080",
        "-extern_plugin",   "GPUMonitor",
        "-mode",            "0",
        "-sound",           "0",
        "-tooltips",        "1",
    ]
    env = os.environ.copy()
    env["SHIM_MCCOMPAT"] = "0x800000001"
    env["__NV_PRIME_RENDER_OFFLOAD"] = "1"
    env["__GLX_VENDOR_LIBRARY_NAME"] = "nvidia"
    return subprocess.Popen(cmd, cwd=SUPERPOSITION_BIN_DIR,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                            shell=False, env=env)

# ─── NEW MAIN FUNCTION ────────────────────────────────────────────────────────
def run_superposition_performance(final, file_obj=None):
    if not final.get("gaming_list"):
        print("[WARN] No gaming_list provided.")
        return

    for item in final["gaming_list"]:
        clean_item        = item["name"].strip().upper()
        GAMEPLAY_DURATION = item.get("duration", 60)
        preset            = item.get("preset", "1080p_medium")

        if clean_item not in ["SUPERPOSITION", "UNIGINE"]:
            continue

        if not os.path.isfile(SUPERPOSITION_EXE):
            print(f"[ERROR] Not found: {SUPERPOSITION_EXE}")
            continue

        os.makedirs(OUTPUT_DIR, exist_ok=True)
        ts         = datetime.now().strftime("%Y%m%d_%H%M%S")
        FPS_LOG    = os.path.join(OUTPUT_DIR, f"fps_log_{ts}.csv")
        SYSTEM_LOG = os.path.join(OUTPUT_DIR, f"system_metrics_{ts}.csv")
        PM_SESSION = f"PMonSP_{ts}"

        print("\n===== CLEANING OLD PROCESSES =====")
        kill_superposition()
        nuke_presentmon_etw_session("PresentMon")
        time.sleep(2)

        print("===== LAUNCHING SUPERPOSITION =====")
        try:
            proc = launch_superposition(preset=preset)
        except Exception as e:
            print(f"[ERROR] Launch failed: {e}")
            continue

        print("Waiting for superposition.exe...")
        if not wait_for_process("superposition.exe", timeout=60):
            print("[ERROR] superposition.exe did not start — aborting.")
            kill_superposition()
            continue

        print("Process running — waiting 35s for world to load...")
        time.sleep(35)

        if not is_process_alive("superposition.exe"):
            print("[ERROR] superposition.exe exited during world load — aborting.")
            kill_superposition()
            continue

        USE_PRESENTMON     = os.path.isfile(PRESENTMON_PATH)
        presentmon_process = None

        if USE_PRESENTMON:
            print("===== STARTING FPS CAPTURE (PresentMon) =====")
            try:
                presentmon_process = subprocess.Popen(
                    [PRESENTMON_PATH,
                     "--process_name",           "superposition.exe",
                     "--output_file",            FPS_LOG,
                     "--timed",                  str(GAMEPLAY_DURATION + 10),
                     "--session_name",           PM_SESSION,
                     "--v1_metrics",
                     "--terminate_after_timed",
                     "--terminate_on_proc_exit"],
                    stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                )
                time.sleep(3)
                if presentmon_process.poll() is not None:
                    err = presentmon_process.stderr.read().decode(errors="replace")
                    print(f"[WARN] PresentMon exited immediately: {err.strip()}")
                    USE_PRESENTMON     = False
                    presentmon_process = None
                else:
                    print(f"  PresentMon capturing -> {FPS_LOG}")
            except Exception as e:
                print(f"[WARN] PresentMon failed: {e}")
                USE_PRESENTMON     = False
                presentmon_process = None

        print(f"===== MONITORING FOR {GAMEPLAY_DURATION}s =====")
        data       = []
        start_time = time.time()

        while time.time() - start_time < GAMEPLAY_DURATION:
            if not is_process_alive("superposition.exe"):
                print("[INFO] Benchmark process finished early.")
                break
            cpu  = psutil.cpu_percent(interval=1)
            ram  = psutil.virtual_memory().percent
            disk = psutil.disk_usage("C:").percent
            gpu  = get_gpu_metrics()
            row  = {
                "timestamp":        datetime.now().isoformat(timespec="seconds"),
                "cpu_%":            cpu, "ram_%": ram, "disk_%": disk,
                "gpu_%":            gpu["gpu_%"],
                "gpu_mem_used_mb":  gpu["gpu_mem_used_mb"],
                "gpu_mem_total_mb": gpu["gpu_mem_total_mb"],
                "gpu_temp_c":       gpu["gpu_temp_c"],
            }
            data.append(row)
            elapsed = int(time.time() - start_time)
            print(f"  [{elapsed:>4}s/{GAMEPLAY_DURATION}s]  "
                  f"CPU={cpu:.1f}%  RAM={ram:.1f}%  "
                  f"GPU={gpu['gpu_%']}%  Temp={gpu['gpu_temp_c']}C")

        print("===== CLOSING SUPERPOSITION =====")
        try:
            proc.terminate()
            proc.wait(timeout=10)
        except Exception:
            pass
        kill_superposition()

        if presentmon_process:
            try:
                presentmon_process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                presentmon_process.kill()
                presentmon_process.wait(timeout=5)
            nuke_presentmon_etw_session(PM_SESSION)

        time.sleep(3)

        print("===== SAVING RESULTS =====")
        fieldnames = ["timestamp", "cpu_%", "ram_%", "disk_%",
                      "gpu_%", "gpu_mem_used_mb", "gpu_mem_total_mb", "gpu_temp_c"]
        if data:
            with open(SYSTEM_LOG, "w", newline="", encoding="utf-8") as f:
                writer = csv.DictWriter(f, fieldnames=fieldnames)
                writer.writeheader()
                writer.writerows(data)
            print(f"  System log : {SYSTEM_LOG} ({len(data)} rows)")
        else:
            with open(SYSTEM_LOG, "w", newline="", encoding="utf-8") as f:
                f.write(",".join(fieldnames) + "\n")
                f.write(f"{datetime.now().isoformat(timespec='seconds')},"
                        + ",".join(["N/A"] * (len(fieldnames) - 1)) + "\n")

        if file_obj:
            if USE_PRESENTMON:
                file_obj.write(f"gaming_fps_log,{FPS_LOG}\n")
            file_obj.write(f"gaming_system_log,{SYSTEM_LOG}\n")
            file_obj.flush()

        print(f"  FPS log    : {FPS_LOG if USE_PRESENTMON else 'N/A'}")
        print("===== SUPERPOSITION COMPLETE =====\n")

# ----------------------------- ERP FETCH -----------------------------
def fetch_performance_links_from_erp(model_name, processor, ram, storage):
    print('---------erp data function is executing------------\n\n')

    # base_url = "http://10.150.0.146:8000" # LOCAL CONNECTION
    base_url = "https://erp.newjaisa.com" # ERP PRODUCTION CONNECTION
    # base_url = "https://amzuat.newjaisa.com" # AMZUAT CONNECTION

    
    endpoint = "/api/method/nj_lib.utils.common_utils.fetch_performance_data_from_erp"
    params = {
        "model": model_name,
        "processor": processor,
        "ram": ram,
        "storage": storage
    }
    print("params:",params)
    response = requests.get(base_url + endpoint, params=params, timeout=30)
    data = response.json()

    excel_list, url_list, word_list, macro_list, = [], [], [], []
    ppt_list , project_list = [] , []
    blender_list , photoshop_list = [] , []
    gaming_list = []


    if 'message' in data and 'parameter_links' in data['message']:
        for item in data['message']['parameter_links']:
            param_type = item.get("type")
            param_value = item.get("value")
            if param_type == "Excel":
                excel_list.append(param_value)
            elif param_type == "URL":
                url_list.append(param_value)
            elif param_type == "Word Img":
                word_list.append(param_value)
            elif param_type == "Macro":
                macro_list.append(param_value)
            elif param_type == "PPT":
                ppt_list.append(param_value)
            elif param_type == "Project":
                project_list.append(param_value)
            elif param_type == "Blender":
                blender_list.append(param_value)
            elif param_type == "Photoshop":
                photoshop_list.append(param_value)  
            elif param_type == "Gaming":
                parts = param_value.strip().split(",")
                game_name = parts[0].strip()
                duration = int(parts[1].strip()) if len(parts) > 1 else 60
                preset = parts[2].strip() if len(parts) > 2 else "1080p_medium"
                gaming_list.append({
                    "name": game_name,
                    "duration": duration,
                    "preset": preset
                })

    combined_excel_list = []
    for i, excel_file in enumerate(excel_list):
        if i < len(macro_list) and macro_list[i]:
            combined_excel_list.append(f"{excel_file}-{macro_list[i]}")
        else:
            combined_excel_list.append(excel_file)

    final_list = combined_excel_list + url_list + word_list + ppt_list + project_list + blender_list + photoshop_list + gaming_list
    return {'excel_list': combined_excel_list,   # <-- use combined, not original excel_list
            'url_list': url_list, 
            'word_list': word_list, 
            "ppt_list":ppt_list,
            "project_list":project_list,
            "blender_list":blender_list,
            "photoshop_list":photoshop_list,
            "gaming_list":gaming_list,
            'macro_list': macro_list, 
            "final_list": final_list
            }

# # ----------------------------------------- LOCAL DATA  -----------------------------------
# # this method is for fetching local  data and perform performance test

def fetch_performance_links_from_offline(data_file_path):
    print('---------local data function is executing------------\n\n')
    excel_list, word_list, url_list,ppt_list,project_list,blender_list,photoshop_list,gaming_list = [], [], [],[],[],[],[],[]
    restart_count=1

    if not os.path.exists(data_file_path):
        print("Local data file not found")
        return {
            "restart_count": restart_count,
            "excel_list": excel_list,
            "word_list": word_list,
            "url_list": url_list,
            "ppt_list":ppt_list,
            "project_list":project_list,
            "blender_list":blender_list,
            "photoshop_list":photoshop_list,
            "gaming_list":gaming_list,
            "macro_list": [],
            "final_list": []
        }

    with open(data_file_path, "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith("PPT"):
                ppt_list.append(line.split(",", 1)[1])

            elif line.startswith("EXCEL"):
                excel_list.append(line.split(",", 1)[1])

            elif line.startswith("WORD"):
                word_list.append(line.split(",", 1)[1])

            elif line.startswith("URL"):
                url_list.append(",".join(line.split(",")[1:]))
            elif line.startswith("PROJECT"):
                # project_list.append(",".join(line.split(",")[1:]))
                project_list.append(",".join([x.strip() for x in line.split(",")[1:]]))
            elif line.startswith("BLENDER"):
                # Use everything after "BLENDER," as the full path
                blender_list.append(line.split(",", 1)[1].strip())
            elif line.startswith("PHOTOSHOP"):
                # Take everything after "PHOTOSHOP," and fix backslashes
                photoshop_list.append(line.split(",", 1)[1].strip())
            elif line.startswith("GAMING"):
                parts = line.strip().split(",")
                game_name = parts[1].strip()
                duration = int(parts[2].strip()) if len(parts) > 2 else 60
                preset = parts[3].strip() if len(parts) > 3 else "1080p_medium"
                gaming_list.append({
                    "name": game_name,
                    "duration": duration,
                    "preset": preset
                })

            

    final_list = excel_list + url_list + word_list+ppt_list+project_list + blender_list + photoshop_list + gaming_list

    # print("Local Execution Order:", final_list)

    return {
        "restart_count": restart_count,
        "excel_list": excel_list,
        "word_list": word_list,
        "url_list": url_list,
        "ppt_list":ppt_list,
        "project_list":project_list,
        "blender_list":blender_list,
        "photoshop_list":photoshop_list,
        "gaming_list":gaming_list,
        "macro_list": [],
        "final_list": final_list
    }

# ----------------------------- MAIN -----------------------------
file_path = os.path.join(BASE_DIR, "Temp_Data", "restart_times.txt")
if not os.path.exists(file_path):
    open(file_path, "w").close()
file_obj = open(file_path, "a")
file_obj.write("Performance Started\n")


parser = argparse.ArgumentParser(description="Performance Test Script")
# parser.add_argument("-m", "--model", help="Model name")
parser.add_argument("-m", "--model",nargs='+', help="Model name")
parser.add_argument("-ft", action="store_true", help="Run file transfer only")
parser.add_argument("-excel", action="store_true", help="Run Excel and Word only")
parser.add_argument("-urls", action="store_true", help="Run urls only")
parser.add_argument("-ppt", action="store_true", help="Run PowerPoint presentations only")
parser.add_argument("-project",action="store_true",help="Run Projects only")
parser.add_argument("-blender", action="store_true", help="Run Blender performance only")
parser.add_argument("-photoshop", action="store_true", help="Run Photoshop performance only")
parser.add_argument("-gaming",action="store_true",help="Run Gaming performance test")
parser.add_argument("-p", "--processor", help="Processor name")
parser.add_argument("-r", "--ram", help="RAM size")
parser.add_argument("-s", "--storage", help="Storage type")
args = parser.parse_args()
print(args)

# -----------------------ERP CALLL---------
data_file_path = os.path.join(BASE_DIR, "Temp_Data", "offline_data.txt")

try:
    final = fetch_performance_links_from_erp(
    args.model,
    args.processor,
    args.ram,
    args.storage
)

    # IMPORTANT CHECK
    if not final.get("final_list"):
        print("ERP returned empty data  by using offline data")
        final = fetch_performance_links_from_offline(data_file_path)
    else:
        print("Using ERP data")

except Exception as e:
    print("ERP failed. Using offline data.")
    final = fetch_performance_links_from_offline(data_file_path)

print("final fetched:", final)
# ------------------------------------------------
# ENTRY POINT
# ------------------------------------------------
if args.gaming:
    run_superposition_performance(final, file_obj)
    file_obj.close()
    sys.exit(0)


if args.photoshop:
    run_all_photoshop(final)
    file_obj.close()
    sys.exit(0)

if args.blender:
    run_all_blender(final)
    file_obj.close()
    sys.exit(0)

if args.project:
    run_projects(final)
    file_obj.close()
    sys.exit(0)

if args.ppt and not final.get('ppt_list'):
    print("No PPT entries found in offline_data.txt")
    file_obj.write("No PPT entries found in offline_data.txt\n")
    sys.exit(0)

if args.ppt:
    try:
        run_all_ppt(final)
    except Exception as e:
        print(f"Error in PPT execution: {str(e)}")
        file_obj.write(f"Error in PPT execution: {str(e)}\n")
    file_obj.close()
    sys.exit(0)

if args.excel:
    try:
        for excel_item in final.get('excel_list', []):
            excel_file, macro_file = split_excel_and_macro(excel_item)
            word_logo = final.get('word_list', [])
            word_logo = word_logo[0] if word_logo else None
            word_docs_thread = None
            if word_logo:
                word_docs_thread = threading.Thread(
                    target=open_scroll_and_close_word_docs,
                    args=(word_logo,), daemon=True
                )
            excel_path = os.path.join(BASE_DIR, "Temp_Data", excel_file)
            excel_app = open_excel()
            if word_docs_thread:
                word_docs_thread.start()
            excel_macro_name = macro_file

            excel_start_time = time.time()
            if excel_app is not None:
                run_excel_macro(excel_path, excel_macro_name)
            excel_end_time = time.time()
            if word_docs_thread:
                word_docs_thread.join()
            excel_time = excel_end_time - excel_start_time
            file_obj.write(f'excel,Total time for excel,{excel_time:.2f}\n')
            json_log("excel", {
                "file": excel_file,
                "macro": excel_macro_name,
                "total_time_sec": round(excel_time, 2)
            })


    except Exception as e:
        print(f"Error in EXcel and office: {str(e)}")
        file_obj.write(f"Error in EXcel and office: {str(e)}")
    file_obj.close()
    sys.exit(0)

if args.ft:
    create_and_copy_file(5,file_obj)
    file_obj.close()
    sys.exit(0)

if args.urls:
    try:
        chrome_options = Options()
        chrome_options.add_argument("--start-maximized")
        chrome_options.add_argument("--disable-infobars")
        chrome_options.add_argument("--autoplay-policy=no-user-gesture-required")
        chrome_options.add_argument("--disable-background-networking")
        chrome_options.add_argument("--disable-sync")
        chrome_options.add_argument("--no-first-run")

        service = Service(ChromeDriverManager().install())
        driver = webdriver.Chrome(service=service, options=chrome_options)
        driver.set_page_load_timeout(30)
        driver.set_script_timeout(20)
        run_url_performance(final.get('url_list', []))
    except Exception as e:
        print(f"Error while launching the chrome: {str(e)}")
        file_obj.write(f"Error while launching the chrome: {str(e)}\n")
    
    file_obj.write("chrome is opened sucessfully.\n")
    file_obj.close()
    sys.exit(0)

if not any([args.ft, args.excel, args.urls, args.ppt, args.project, args.blender, args.photoshop, args.gaming]):
    print("\n=== RUNNING TESTS BASED ON ERP DATA ===\n")
    if final.get('url_list'):
        try:
            chrome_options = Options()
            chrome_options.add_argument("--start-maximized")
            chrome_options.add_argument("--disable-infobars")
            chrome_options.add_argument("--autoplay-policy=no-user-gesture-required")
            chrome_options.add_argument("--disable-background-networking")
            chrome_options.add_argument("--disable-sync")
            chrome_options.add_argument("--no-first-run")

            service = Service(ChromeDriverManager().install())
            driver = webdriver.Chrome(service=service, options=chrome_options)
            driver.set_page_load_timeout(30)
            driver.set_script_timeout(20)
            # Run URL performance test
            run_url_performance(final.get('url_list', []))

        except Exception as e:
            print(f"Error while launching the chrome: {str(e)}")
            file_obj.write(f"Error while launching the chrome: {str(e)}\n")
        finally:
            # Always force kill Chrome no matter what happens
            subprocess.run(["taskkill", "/f", "/im", "chrome.exe"],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            subprocess.run(["taskkill", "/f", "/im", "chromedriver.exe"],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            print("Chrome force killed in main block")
            file_obj.write("Chrome URL tests completed.\n")

        file_obj.write("chrome is opened sucessfully.\n")

    # Run PPT if available
    if final.get('ppt_list'):
        print("Running PPT tasks...")
        safe_execute("PowerPoint", run_all_ppt, final)

    # Run Projects if available
    if final.get('project_list'):
        print("Running Project tasks...")
        safe_execute("Projects", run_projects, final)

    # Run Blender if available
    if final.get('blender_list'):
        print("Running Blender tasks...")
        safe_execute("Blender", run_all_blender, final)

    # Run Photoshop if available
    if final.get('photoshop_list'):
        print("Running Photoshop tasks...")
        safe_execute("Photoshop", run_all_photoshop, final)

    # Run Gaming if available
    if final.get('gaming_list'):
        print("Running Gaming tasks...")
        safe_execute("Gaming (SUPERPOSITION)", run_superposition_performance, final, file_obj)


# safe_execute("Photoshop", run_all_photoshop, final)
# safe_execute("Blender", run_all_blender, final)
# safe_execute("PPT", run_all_ppt, final)
# safe_execute("Project", run_projects, final)
# safe_execute("Gaming (CS2)", run_cs2_performance, final, file_obj)

    # Run Excel if available
    if final.get('excel_list'):
        try:
            for excel_item in final.get('excel_list', []):
                excel_file, macro_file = split_excel_and_macro(excel_item)
                word_logo = final.get('word_list', [])
                word_logo = word_logo[0] if word_logo else None
                word_docs_thread = None
                if word_logo:
                    word_docs_thread = threading.Thread(
                        target=open_scroll_and_close_word_docs,
                        args=(word_logo,), daemon=True
                    )
                excel_path = os.path.join(BASE_DIR, "Temp_Data", excel_file)  # ✅
                excel_app = open_excel()
                if word_docs_thread:
                    word_docs_thread.start()
                excel_macro_name = macro_file
                excel_start_time = time.time()
                if excel_app is not None:
                    run_excel_macro(excel_path, excel_macro_name)
                excel_end_time = time.time()
                if word_docs_thread:
                    word_docs_thread.join()
                excel_time = excel_end_time - excel_start_time
                file_obj.write(f'excel,Total time for excel,{excel_time:.2f}\n')
                json_log("excel", {
                    "file": excel_file,
                    "macro": excel_macro_name,
                    "total_time_sec": round(excel_time, 2)
                })


        except Exception as e:
                print(f"Error in EXcel and office: {str(e)}")
                file_obj.write(f"Error in EXcel and office: {str(e)}")

    create_and_copy_file(5,file_obj) 
        
file_obj.write(f"PERFORMANCE_COMPLETED")
file_obj.close()
sys.exit(0)

