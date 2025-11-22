import time
import subprocess
import os
from http.server import HTTPServer, BaseHTTPRequestHandler
import threading

PYTHON_SERVER_PORT = 8080  # Port for Python HTTP server
DISTANCE_TIMEOUT_SECONDS = 13  # Timeout for distance notifications
DISTANCE_THRESHOLD_METERS = 2.2  # Threshold for distance warning

# Audit log file path
AUDIT_LOG_FILE = os.path.join(os.path.dirname(__file__), 'audit_log.txt')

# Store distance and time
distance_history = []  # List of all distance measurements
distance_timestamps = []  # List of timestamps (in ms) for each measurement
last_distance_time = None
last_notification_time = None  # Track when we last sent a timeout notification

def check_distance():
    #returns distance between Workstation and UserBadge
    if distance_history:
        return distance_history[-1]  # Return most recent distance
    return None

def check_time():
    #returns last time UserBadge was in range of Workstation
    return last_distance_time

def get_average_distance():
    #returns average of all recorded distances
    if distance_history:
        return sum(distance_history) / len(distance_history)
    return None

class ESP32Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        """Handle GET requests to view distance data"""
        if self.path == '/status' or self.path == '/':
            import json
            global distance_history, last_distance_time
            
            # Calculate statistics
            stats = {}
            if distance_history:
                sorted_distances = sorted(distance_history)
                stats = {
                    'min': sorted_distances[0],
                    'max': sorted_distances[-1],
                    'median': sorted_distances[len(sorted_distances) // 2] if sorted_distances else None
                }
            
            # Create measurement data with timestamps and intervals
            measurements_with_time = []
            # Ensure both lists are the same length
            min_len = min(len(distance_history), len(distance_timestamps))
            for i in range(min_len):
                dist = distance_history[i]
                ts = distance_timestamps[i]
                interval = None
                if i > 0:
                    interval = (ts - distance_timestamps[i-1]) / 1000  # Convert to seconds
                measurements_with_time.append({
                    'distance': dist,
                    'timestamp': ts,
                    'time_since_previous': interval
                })
            
            status = {
                'total_measurements': len(distance_history),
                'most_recent': check_distance(),
                'average': get_average_distance(),
                'last_time': last_distance_time,
                'recent_measurements': distance_history[-10:] if distance_history else [],
                'all_measurements': distance_history,  # Include all measurements
                'measurements_with_time': measurements_with_time,  # Include timestamps and intervals
                'statistics': stats
            }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(status, indent=2).encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()
    
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        data = self.rfile.read(content_length)
        
        # Check if it's distance data (8 bytes for double)
        if self.path == '/distance' and content_length == 8:
            import struct
            global distance_history, distance_timestamps, last_distance_time
            distance = struct.unpack('d', data)[0]  # 'd' = double (8 bytes)
            current_time_ms = time.time_ns() / 1_000_000  # Store as milliseconds
            distance_history.append(distance)  # Store all measurements
            distance_timestamps.append(current_time_ms)  # Store timestamp for this measurement
            last_distance_time = current_time_ms
            timestamp_sec = current_time_ms / 1000
            
            # Calculate average
            avg_distance = sum(distance_history) / len(distance_history)
            
            print(f"Distance received: {distance:.2f} meters at {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp_sec))}.{int(last_distance_time % 1000):03d}")
            print(f"Average distance: {avg_distance:.2f} meters (from {len(distance_history)} measurements)")
            
            # Check if distance is 3 meters or more
            if distance >= DISTANCE_THRESHOLD_METERS:
                print(f"Distance threshold exceeded: {distance:.2f} meters (threshold: {DISTANCE_THRESHOLD_METERS}m). Locking screen...")
                lock_screen(reason='distance_threshold', distance=distance)
            
            # Process distance here
        else:
            print(f"Received from ESP32 ({content_length} bytes): {data.decode('utf-8', errors='ignore')}")
        
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(b'OK')
    
    def log_message(self, format, *args):
        pass  

def kill_existing_server(port):
    try:
        result = subprocess.run(['lsof', '-ti', f':{port}'], capture_output=True, text=True)
        if result.returncode == 0 and result.stdout.strip():
            pids = result.stdout.strip().split('\n')
            for pid in pids:
                if pid:
                    subprocess.run(['kill', '-9', pid], check=False)
                    print(f"Killed existing server on port {port} (PID: {pid})")
    except FileNotFoundError:
        # lsof not available, try fuser
        try:
            subprocess.run(['fuser', '-k', f'{port}/tcp'], check=False, 
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            pass

def start_http_server():
    kill_existing_server(PYTHON_SERVER_PORT)
    time.sleep(0.2)  # Brief wait for port to be released
    
    server = HTTPServer(('', PYTHON_SERVER_PORT), ESP32Handler)
    print(f"Python HTTP server started on port {PYTHON_SERVER_PORT}")
    server.serve_forever()

def write_audit_log(reason, distance=None, timeout_seconds=None):
    """Write an entry to the audit log"""
    timestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
    timestamp_ms = time.time_ns() / 1_000_000
    timestamp_str = f"{timestamp}.{int(timestamp_ms % 1000):03d}"
    
    with open(AUDIT_LOG_FILE, 'a') as f:
        if reason == 'distance_threshold':
            f.write(f"[{timestamp_str}] SCREEN LOCKED - Distance threshold exceeded: {distance:.2f} meters (threshold: {DISTANCE_THRESHOLD_METERS}m)\n")
        elif reason == 'timeout':
            f.write(f"[{timestamp_str}] SCREEN LOCKED - Timeout: No distance received for {timeout_seconds:.1f} seconds\n")
        else:
            f.write(f"[{timestamp_str}] SCREEN LOCKED - Reason: {reason}\n")
        f.flush()  # Ensure it's written immediately

def reset_distance_state():
    """Reset distance state before locking screen"""
    global distance_history, distance_timestamps, last_distance_time, last_notification_time
    distance_history = []
    distance_timestamps = []
    last_distance_time = None
    last_notification_time = None

def lock_screen(reason='unknown', distance=None, timeout_seconds=None):
    """Lock the screen, reset distance state, and log to audit file"""
    write_audit_log(reason, distance, timeout_seconds)
    reset_distance_state()
    subprocess.run(['loginctl', 'lock-session'], check=False)

def monitor_distance_timeout():
    """Monitor for distance timeout and lock screen"""
    global last_distance_time, last_notification_time
    
    while True:
        time.sleep(1)  # Check every second
        
        if last_distance_time is not None:
            current_time_ms = time.time_ns() / 1_000_000
            time_since_last_ms = current_time_ms - last_distance_time
            time_since_last_sec = time_since_last_ms / 1000
            
            if time_since_last_sec > DISTANCE_TIMEOUT_SECONDS:
                # Lock screen and reset state
                print(f"Distance timeout: No distance received for {int(time_since_last_sec)} seconds. Locking screen...")
                lock_screen(reason='timeout', timeout_seconds=time_since_last_sec)
                # Wait a bit after locking to avoid immediate re-check
                time.sleep(2)

def start_server_thread():
    """Start the HTTP server in a background thread"""
    server_thread = threading.Thread(target=start_http_server, daemon=True)
    server_thread.start()
    time.sleep(0.5)  # Brief wait for server to start
    
    # Start the timeout monitor thread
    monitor_thread = threading.Thread(target=monitor_distance_timeout, daemon=True)
    monitor_thread.start()
    
    return server_thread

