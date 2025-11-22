import time
import subprocess
import os
import csv
import random
import urllib.request

START_MSG = 'DEAUTHSTART'
END_MSG = 'DEAUTHEND' 
DELIMITER = ':'
ESP32_IP = '192.168.1.22'  # Set your ESP32's IP here

def lock_screen():
    subprocess.run(['loginctl', 'lock-session'], check=True)



def gen_msg_init(badge_info):

    badge_public_key = badge_info['path_to_public_key']
    with open(badge_public_key, 'r') as f:
        badge_public_key = f.read()
    badge_id = int(badge_info['badgeid'])

    #generate 32 bit random number (challenge)
    chal = random.getrandbits(32)

    #generate 32 bit random number (temporary)
    hash_chain = random.getrandbits(32)

    print(badge_public_key)
    print(chal)
    print(hash_chain)
    print(badge_id)
    
    send_msg_init(badge_public_key, chal, hash_chain, badge_id)

def send_msg_init(badge_public_key, chal, hash_chain, badge_id):
    msg = f'{START_MSG}{DELIMITER}{badge_public_key}{DELIMITER}{chal}{DELIMITER}{hash_chain}{DELIMITER}{badge_id}{DELIMITER}{END_MSG}'
    
    # Send via HTTP
    try:
        url = f'http://{ESP32_IP}/deauth'
        req = urllib.request.Request(url, data=msg.encode('utf-8'), method='POST')
        with urllib.request.urlopen(req, timeout=2) as response:
            print(f'HTTP response: {response.status}')
    except Exception as e:
        print(f'HTTP error: {e}')
    
    return "success"


def lookup_badge_by_username(username):
    csv_path = os.path.join(os.path.dirname(__file__), 'badges.csv')
    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['Username'] == username:
                return {'badgeid': row['badgeid'], 'path_to_public_key': row['path_to_public_key']}
    return None

if __name__ == "__main__":
    # Look up badge and send init message
    username = os.getenv('USER') or os.getlogin()
    badge_info = lookup_badge_by_username(username)
    if badge_info:
        subprocess.run(['notify-send', 'DeAuth', f'Found badge {badge_info["badgeid"]} for {username}'], check=False)
        gen_msg_init(badge_info)
    else:
        subprocess.run(['notify-send', 'DeAuth', f'No badge found for {username}'], check=False)
        #lock_screen()