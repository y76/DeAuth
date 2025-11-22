import subprocess, time, os
from distance_server import start_server_thread

# Start the HTTP server once (runs in background)
start_server_thread()

# Helper function to check if screen is locked
def is_locked():
    try:
        session_id = subprocess.run(['loginctl', 'list-sessions', '--no-legend'], capture_output=True).stdout.split()[0].decode()
        result = subprocess.run(['loginctl', 'show-session', session_id, '-p', 'LockedHint'], capture_output=True)
        return 'yes' in result.stdout.decode().lower()
    except:
        return False

# Run client.py once at startup
subprocess.run(['python3', os.path.join(os.path.dirname(__file__), 'client.py')])

# Monitor for unlock events and run client.py on each unlock
was_locked = is_locked()  # Initialize with current state
while True:
    locked = is_locked()
    if was_locked and not locked: 
        print(f"Unlock detected! Running client.py...")
        subprocess.run(['python3', os.path.join(os.path.dirname(__file__), 'client.py')])
    was_locked = locked
    time.sleep(1)

