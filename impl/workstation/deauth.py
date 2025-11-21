import subprocess, time, os
subprocess.run(['python3', os.path.join(os.path.dirname(__file__), 'client.py')])
was_locked = True
while True:
    locked = 'yes' in subprocess.run(['loginctl', 'show-session', subprocess.run(['loginctl', 'list-sessions', '--no-legend'], capture_output=True).stdout.split()[0].decode(), '-p', 'LockedHint'], capture_output=True).stdout.decode().lower()
    if was_locked and not locked: subprocess.run(['python3', os.path.join(os.path.dirname(__file__), 'client.py')])
    was_locked = locked
    time.sleep(1)

