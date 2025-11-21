import time
import subprocess

def lock_screen():
    subprocess.run(['loginctl', 'lock-session'], check=True)

def send_msg_init():
    # send a "ping" over uart to ESP32. ESP32 will generate ephemeral key pair, generate shared key, UWB STS parameters
    #encrypt STS parameters and send message to UserBadge's bluetooth module.
    # ESP32 will also send UWB STS parameters to Workstation's UWB module.
    return 

def check_distance():
    #returns distance between Workstation and UserBadge
    return

def check_time():
    #returns last time UserBadge was in range of Workstation
    return

if __name__ == "__main__":
    #print("Locking screen in 5 seconds...")
    time.sleep(2)
    subprocess.run(['notify-send', 'DeAuth', 'Sending Initialization Message'], check=False)

    #lock_screen()
    
    