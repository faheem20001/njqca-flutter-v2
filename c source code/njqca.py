import os
import sys
import subprocess
import time
from datetime import datetime

class Config:
    def __init__(self):
        self._ssid = "NJ Production"
        self._password = "password"
        self._home_dir = os.path.expanduser("~")
        self._njqca_dir = os.path.join(self._home_dir, "Desktop", "njqca")
        self._prod_njqca_dir = os.path.join(self._njqca_dir, "prod")
        self._prod_branch = "main"
        self._uat_njqca_dir = os.path.join(self._njqca_dir, "uat")
        self._uat_branch = "uat"
        self._njbattery_dir = os.path.join(self._home_dir, "Desktop", "NJBatteryTest")
        self._njbattery_branch = "nj-battery"
        self._token = "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"
        self._git_url = "https://github.com/newjaisa/NJQCA-EXE.git"
        self._git_url_with_token = f"https://{self._token}@github.com/newjaisa/NJQCA-EXE.git"

    @property
    def ssid(self):
        return self._ssid

    @ssid.setter
    def ssid(self, value):
        self._ssid = value

    @property
    def password(self):
        return self._password

    @password.setter
    def password(self, value):
        self._password = value

    @property
    def home_dir(self):
        return self._home_dir

    @property
    def njqca_dir(self):
        return self._njqca_dir

    @property
    def prod_njqca_dir(self):
        return self._prod_njqca_dir

    @property
    def prod_branch(self):
        return self._prod_branch

    @property
    def uat_njqca_dir(self):
        return self._uat_njqca_dir

    @property
    def uat_branch(self):
        return self._uat_branch

    @property
    def njbattery_dir(self):
        return self._njbattery_dir

    @property
    def njbattery_branch(self):
        return self._njbattery_branch

    @property
    def token(self):
        return self._token

    @property
    def git_url(self):
        return self._git_url

    @property
    def git_url_with_token(self):
        return self._git_url_with_token


class Logger:
    def __init__(self, log_dir):
        self.log_dir = log_dir
        self.error_log_path = os.path.join(log_dir, '.error.log')
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
            
    def clear_log(self):
        with open(self.error_log_path, 'w') as log_file:
            log_file.write("")

    def log_error(self, message):
        with open(self.error_log_path, 'a') as log_file:
            log_file.write(f"{datetime.now()} - ERROR: {message}\n")


class NJQCAUpdater:
    def __init__(self, config):
        self.config = config
        self.logger = Logger(config.njqca_dir)

    def print_green(self, message):
        print(f"\033[32m{message}\033[0m")

    def print_blue(self, message):
        print(f"\033[34m{message}\033[0m")

    def print_red(self, message):
        print(f"\033[31m{message}\033[0m")

    def run_command(self, cmd):
        try:
            result = subprocess.run(cmd, shell=True, text=True, capture_output=True)
            if result.returncode == 0:
                print(result.stdout)
            else:
                self.print_red(f"Command '{cmd}' failed.")
                self.logger.log_error(f"Command '{cmd}' failed: {result.stderr}")
        except Exception as e:
            self.logger.log_error(f"Exception occurred: {str(e)}")

    def update_requirements(self, requirements_file):
        if os.path.isfile(requirements_file):
            with open(requirements_file, "r") as file:
                for line in file:
                    cmd = line.strip()
                    if cmd:
                        self.print_blue(f"Executing: {cmd}...")
                        self.run_command(cmd)
        else:
            self.print_red(f"{requirements_file} not found.")

    def check_network_connection(self):
        try:
            ethernet_status = subprocess.run(
                "nmcli device status | grep ethernet | grep connected",
                shell=True, text=True, capture_output=True
            )
            current_ssid = subprocess.run(
                "nmcli -t -f active,ssid dev wifi | grep '^yes' | cut -d':' -f2",
                shell=True, text=True, capture_output=True
            ).stdout.strip()

            if ethernet_status.stdout:
                self.print_green("LAN is connected.")
            elif current_ssid:
                self.print_green(f"Wi-Fi is connected to {current_ssid}.")
            else:
                self.print_red("Neither LAN nor Wi-Fi is connected.")
                self.print_red("Attempting to connect to Wi-Fi...")
                self.run_command("nmcli radio wifi on")
                time.sleep(4)
                self.run_command(f"nmcli dev wifi connect '{self.config.ssid}' password '{self.config.password}'")
                time.sleep(2)

                connected_ssid = subprocess.run(
                    "nmcli -t -f active,ssid dev wifi | grep '^yes' | cut -d':' -f2",
                    shell=True, text=True, capture_output=True
                ).stdout.strip()
                if connected_ssid:
                    self.print_green(f"Wi-Fi Connected to {connected_ssid}.")
                else:
                    self.print_red("Wi-Fi Failed to Connect")
                    self.run_command("gnome-control-center wifi")
        except Exception as e:
            self.print_red(f"Error occurred during Network Check: {str(e)}")

    def make_git_pull(self, current_branch):
        try:
            if not os.path.isdir(".git"):
                self.run_command("git init >> /dev/null")
                self.run_command(f"git remote add origin {self.config.git_url} >> /dev/null")
                self.run_command(f"git checkout -b {current_branch} >> /dev/null")

            self.print_blue(f"Updating the latest changes to {current_branch}...")
            result = subprocess.run(
                f"git pull {self.config.git_url_with_token} {current_branch} >> /dev/null",
                shell=True,
                text=True,
                capture_output=True
            )

            if result.returncode == 0:
                self.print_green(f"Successfully updated {current_branch} branch.")
            else:
                self.print_red(f"Failed to update {current_branch} branch.")
                self.logger.log_error(f"Failed to update {current_branch} branch.\n {result.stderr}")
        except Exception as e:
            self.logger.log_error(f"Exception occurred during git pull: {str(e)}")

    def update_all_njqca(self):
        branches = {
            self.config.prod_branch: self.config.prod_njqca_dir,
            self.config.uat_branch: self.config.uat_njqca_dir,
            self.config.njbattery_branch: self.config.njbattery_dir
        }

        for branch, path in branches.items():
            self.print_blue(f"Updating {branch} branch at path: {path}...")

            if os.path.isdir(path):
                os.chdir(path)
                self.make_git_pull(branch)
            else:
                self.print_red(f"Directory not found for {branch}, path: {path}")

            print("")

    def update_njqca(self, source=None):
        self.check_network_connection()

        current_branch = self.config.prod_branch

        if os.path.isdir(self.config.njqca_dir):
            if source in ("-a", "--all"):
                self.update_all_njqca()
                self.update_requirements(".all-requirements.txt")
                return

            elif source == "" or source is None:
                os.chdir(self.config.prod_njqca_dir)
            elif source == "uat":
                current_branch = self.config.uat_branch
                os.chdir(self.config.uat_njqca_dir)
            elif source == "battery":
                current_branch = self.config.njbattery_branch
                os.chdir(self.config.njbattery_dir)
            else:
                self.print_red("Update Command not recognized!")
                self.print_red("Use 'njqca -h' or 'njqca --help' for usage information.")
                return

            self.make_git_pull(current_branch)
            if os.path.isfile(".requirements.txt"):
                self.update_requirements(".requirements.txt")
        else:
            self.print_red(f"NJQCA Directory not found while updating!")

    def print_help(self):
        help_message = """
Usage: njqca [COMMAND]

Commands:
  update [source]                Update NJQCA Dongle to the specified version. If Version is '--all' or '-a', all requirements(latest) will be installed. Source can be empty or 'uat'
  -u                             Execute the NJQCA file with username & password.
  -uat                           Execute the UAT version of the NJQCA file. Optionally, you can use '-u' with this command.
  -h, --help                     Show this help message and exit.

Examples:
 To update dongle --
   njqca update                    # Update NJQCA(Production) to latest version
   njqca update uat                # Update NJQCA(UAT) to latest version
   njqca update battery            # Update NJQCA(NJBattery Test) to latest version
   njqca update -a, --all          # Update complete NJQCA to latest version
  
 To run NJQCA in Production --
   njqca                           # Run the production NJQCA file
   njqca -u                        # Run the production NJQCA file with username & password
  
 To run NJQCA in UAT --
   njqca -uat                      # Run the UAT NJQCA file
   njqca -uat -u                   # Run the UAT NJQCA file with username & password
  
 To run NJBattery Test --
   njqca battery                   # Run the NJBattery Test
"""
        print(help_message)

    def main(self):
        try:
            self.logger.clear_log()
            
            if len(sys.argv) == 1:
                PROD_NJQCA_FILE_PATH = os.path.join(self.config.prod_njqca_dir, "njqca")
                if os.path.isfile(PROD_NJQCA_FILE_PATH) and os.access(PROD_NJQCA_FILE_PATH, os.X_OK):
                    subprocess.run([PROD_NJQCA_FILE_PATH])
                else:
                    self.print_red("NJQCA file not found or not able to run.")
                return

            command = sys.argv[1]

            if command in ("-h", "--help"):
                self.print_help()
                return

            if command == "update":
                source = None
                if len(sys.argv) == 2:
                    pass
                elif len(sys.argv) == 3:
                    source = sys.argv[2]
                else:
                    self.print_red("Command not recognized!")
                    self.print_red("Use 'njqca -h' or 'njqca --help' for usage information.")
                    return

                self.update_njqca(source)

            elif command == "battery":
                NJBATTERY_FILE_PATH = os.path.join(self.config.njbattery_dir, "njbatterytest")
                if os.path.isfile(NJBATTERY_FILE_PATH) and os.access(NJBATTERY_FILE_PATH, os.X_OK):
                    subprocess.run([NJBATTERY_FILE_PATH])
                else:
                    self.print_red("NJBattery Test file not found or not able to run.")
                return

            elif command == "-u":
                PROD_NJQCA_FILE_PATH = os.path.join(self.config.prod_njqca_dir, "njqca")
                if os.path.isfile(PROD_NJQCA_FILE_PATH) and os.access(PROD_NJQCA_FILE_PATH, os.X_OK):
                    subprocess.run([PROD_NJQCA_FILE_PATH, "-u"])
                else:
                    self.print_red("Production NJQCA file not found or not able to run.")
                return

            elif command == "-uat":
                UAT_NJQCA_FILE_PATH = os.path.join(self.config.uat_njqca_dir, "njqca")
                tag = sys.argv[2] if len(sys.argv) > 2 else None
                if tag == "-u" and os.path.isfile(UAT_NJQCA_FILE_PATH) and os.access(UAT_NJQCA_FILE_PATH, os.X_OK):
                    subprocess.run([UAT_NJQCA_FILE_PATH, "-u"])
                elif os.path.isfile(UAT_NJQCA_FILE_PATH) and os.access(UAT_NJQCA_FILE_PATH, os.X_OK):
                    subprocess.run([UAT_NJQCA_FILE_PATH])
                else:
                    self.print_red("UAT NJQCA file not found or not able to run.")
                return

            else:
                self.print_red("Command not recognized. Please check once!")
                self.print_red("Use 'njqca -h' or 'njqca --help' for usage information.")
        except Exception as e:
            self.print_red(f"NJQCA Failed: {str(e)}")
            self.logger.log_error(f"NJQCA Failed: {str(e)}")

if __name__ == "__main__":
    config = Config()
    updater = NJQCAUpdater(config)
    updater.main()
