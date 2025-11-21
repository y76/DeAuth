import time
import subprocess

def lock_screen():
    subprocess.run(['loginctl', 'lock-session'], check=True)

if __name__ == "__main__":
    #print("Locking screen in 5 seconds...")
    time.sleep(2)
    subprocess.run(['notify-send', 'DeAuth', 'Sending Initialization Message'], check=False)

    #lock_screen()